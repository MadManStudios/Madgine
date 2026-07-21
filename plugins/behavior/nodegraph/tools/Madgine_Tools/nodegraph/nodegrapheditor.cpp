#include "../nodegraphtoolslib.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"
#include "Generic/projections.h"

#include "Platform/filesystem/fsapi.h"
#include "Platform/log/logsenders.h"

#include "Meta/type/storageops.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/codegen/codegen_cpp.h"
#include "Madgine/nodegraph/nodebase.h"
#include "Madgine/nodegraph/nodecollector.h"
#include "Madgine/nodegraph/nodeinterpreter.h"
#include "Madgine/nodegraph/nodes/accessornode.h"
#include "Madgine/nodegraph/nodes/behaviornode.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/behaviortool.h"
#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "NodeEditor/imgui_node_editor.h"
#include "debugvisualizer.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Madgine_Tools/util/trace_imgui.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"
#include "imguihelpers.h"
#include "nodegrapheditor.h"
#include "tests/nodegrapheditortests.h"

UNIQUECOMPONENT(Engine::Tools::NodeGraphEditor);

METATABLE_BEGIN_BASE(Engine::Tools::NodeGraphEditor, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::NodeGraphEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::NodeGraphEditor, Engine::Tools::ToolBase)
    FIELD(mHierarchyVisible)
    FIELD(mNodeDetailsVisible)
// ENCAPSULATED_FIELD(Current, getCurrentName, load)
SERIALIZETABLE_END(Engine::Tools::NodeGraphEditor)

namespace Engine {
namespace Tools {

    NodeGraphEditor::NodeGraphEditor(ImRoot &root)
        : Tool<NodeGraphEditor, ResourceEditor>(root)
        , ResourceFile(*this, "")
    {
        mVisible = false;
    }

    Threading::Task<bool> NodeGraphEditor::init()
    {
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeDebugLocation>();

        createEditor();

#if MODULES_HAS_THREADS
        registerNodeGraphEditorTests(mRoot.testEngine());
#endif

        co_return co_await ResourceEditor::init(Behavior::NodeGraph::NodeGraphLoader::getSingleton(), "Node Graph");
    }

    Threading::Task<void> NodeGraphEditor::finalize()
    {
        mGraphHandle.reset();
        mEditor.reset();

        co_await ToolBase::finalize();
    }

    void NodeGraphEditor::render()
    {
        bool open = true;
        if (Begin(&open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar)) {

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Dev")) {

                    if (ImGui::MenuItem("Debug", "", false)) {
                        Debug::ContextInfo &context = Debug::Debugger::getSingleton().createContext();
                        Execution::detach(
                            Behavior::Behavior { mGraph.interpret() }
                            | Execution::then([](Reflect::ArgumentList) { LOG("SUCCESS"); })
                            | Execution::with_debug_location(context.mChild)
                            | Debug::with_debug_context(context)
                            | Platform::Log::log_result());
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Panels")) {

                    ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);
                    ImGui::MenuItem("Node Details", nullptr, &mNodeDetailsVisible);

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if (beginContent(ImGuiWindowFlags_NoScrollbar)) {

                ImVec2 topLeftScreen = ImGui::GetCursorScreenPos();

                ed::SetCurrentEditor(mEditor.get());

                ed::Begin("Node editor");

                std::optional<Reflect::ExtendedType> hoveredPin;

                ed::PushStyleVar(ed::StyleVar_NodePadding, { 0, 0, 0, 0 });
                ImVec2 specialPosition = ed::ScreenToCanvas(topLeftScreen);

                ed::SetNodePosition(std::numeric_limits<int>::max() - 1, { floorf(specialPosition.x), floorf(specialPosition.y) });
                ed::BeginNode(std::numeric_limits<int>::max() - 1);
                ImGui::BeginVertical("inputPins", ImVec2(0, 0), 0.0f);
                ImGui::Dummy({ sPinIconSize, 10 });
                uint32_t pinId = 0;
                for (Behavior::NodeGraph::FlowOutPinPrototype &flowPin : mGraph.mFlowOutPins) {
                    FlowOutPin(nullptr, 0, pinId, 0, mGraph.node(flowPin.mTarget.mNode)->flowInMask(flowPin.mTarget.mIndex), true);
                    ++pinId;
                }
                if (mDragPin && mDragPin->mDir == Behavior::NodeGraph::PinDir::In && mDragPin->mType == Behavior::NodeGraph::PinType::Flow) {
                    FlowOutPin(nullptr, 0, pinId, 0, mDragMask, false);
                }
                pinId = 0;
                for (Behavior::NodeGraph::DataOutPinPrototype &dataPin : mGraph.mDataOutPins) {
                    Behavior::NodeGraph::NodeBase *node = mGraph.node(dataPin.mTargets.front().mNode);
                    Reflect::ExtendedType type = mGraph.dataOutType({ 0, pinId });
                    if (DataOutPin(nullptr, 0, pinId, 0, type, node->dataInMask(dataPin.mTargets.front().mIndex), true))
                        hoveredPin = type;
                    ++pinId;
                }
                if (mDragPin && mDragPin->mDir == Behavior::NodeGraph::PinDir::In && mDragPin->mType == Behavior::NodeGraph::PinType::Data) {
                    DataOutPin(nullptr, 0, pinId, 0, *mDragType, mDragMask, false);
                }

                pinId = 0;
                for (const Behavior::NodeGraph::NodeGraph::NamedInput &input : mGraph.mNamedInputs) {
                    if (DataOutPin(input.mDescriptor.mName.data(), 0, pinId, 1, (*input.mDescriptor.mType)->mType, Behavior::NodeGraph::NodeExecutionMask::ALL, !input.mTargets.empty()))
                        hoveredPin = (*input.mDescriptor.mType)->mType;
                    ++pinId;
                }

                ImGui::EndVertical();
                ed::EndNode();

                specialPosition = ed::ScreenToCanvas({ topLeftScreen.x + ed::GetScreenSize().x - sPinIconSize / ed::GetCurrentZoom(), topLeftScreen.y });
                ed::SetNodePosition(std::numeric_limits<int>::max() - 2, { floorf(specialPosition.x), floorf(specialPosition.y) });
                ed::BeginNode(std::numeric_limits<int>::max() - 2);
                ImGui::BeginVertical("outputPins", ImVec2(0, 0), 0.0f);
                ImGui::Dummy({ sPinIconSize, 10 });
                pinId = 0;
                for (Behavior::NodeGraph::FlowInPinPrototype &flowPin : mGraph.mFlowInPins) {
                    Behavior::NodeGraph::NodeBase *node = mGraph.node(flowPin.mSources.front().mNode);
                    FlowInPin(nullptr, 0, pinId, 0, node->flowInMask(flowPin.mSources.front().mIndex), true);
                    ++pinId;
                }
                if (mDragPin && mDragPin->mDir == Behavior::NodeGraph::PinDir::Out && mDragPin->mType == Behavior::NodeGraph::PinType::Flow) {
                    FlowInPin(nullptr, 0, pinId, 0, mDragMask, false);
                }
                pinId = 0;
                for (Behavior::NodeGraph::DataInPinPrototype &dataPin : mGraph.mDataInPins) {
                    assert(dataPin.mSource && dataPin.mSource.mNode);
                    Behavior::NodeGraph::NodeBase *node = mGraph.node(dataPin.mSource.mNode);
                    Reflect::ExtendedType type = node->dataOutType(dataPin.mSource.mIndex);

                    if (DataInPin(nullptr, 0, pinId, 0, type, node->dataOutMask(dataPin.mSource.mIndex), static_cast<bool>(dataPin.mSource)))
                        hoveredPin = type;
                    ++pinId;
                }
                if (mDragPin && mDragPin->mDir == Behavior::NodeGraph::PinDir::Out && mDragPin->mType == Behavior::NodeGraph::PinType::Data) {
                    DataInPin(nullptr, 0, pinId, 0, *mDragType, mDragMask, {});
                }
                ImGui::EndVertical();
                ed::EndNode();
                ed::PopStyleVar();

                uint32_t nodeId = 1;
                Behavior::NodeGraph::NodeBase *hoveredNode = nullptr;
                for (Behavior::NodeGraph::NodeBase *node : mGraph.nodes() | std::views::transform(projectionUniquePtrToPtr)) {

                    if (std::optional<Reflect::ExtendedType> hovered = BeginNode(node, nodeId, mDragPin, mDragType))
                        hoveredPin = hovered;

                    EndNode();

                    if (ImGui::IsItemHovered()) {
                        hoveredNode = node;
                    }

                    ++nodeId;
                }

                pinId = 0;
                for (Behavior::NodeGraph::FlowOutPinPrototype &pin : mGraph.mFlowOutPins) {
                    assert(pin.mTarget);
                    uint32_t id = Behavior::NodeGraph::NodeBase::flowOutId(pinId);
                    ed::Link(id, id, 60000 * pin.mTarget.mNode + Behavior::NodeGraph::NodeBase::flowInId(pin.mTarget.mIndex), FlowColor(mGraph.flowOutMask({ 0, pinId })));
                    ++pinId;
                }
                pinId = 0;
                for (Behavior::NodeGraph::DataInPinPrototype &pin : mGraph.mDataInPins) {
                    assert(pin.mSource);
                    uint32_t id = Behavior::NodeGraph::NodeBase::dataInId(pinId);
                    ed::Link(id, 60000 * pin.mSource.mNode + Behavior::NodeGraph::NodeBase::dataOutId(pin.mSource.mIndex), id, DataColor(mGraph.dataInMask({ 0, pinId })));
                    ++pinId;
                }
                nodeId = 1;
                for (Behavior::NodeGraph::NodeBase *node : mGraph.nodes() | std::views::transform(projectionUniquePtrToPtr)) {
                    NodeLinks(node, nodeId);
                    ++nodeId;
                }

                if (ed::BeginCreate()) {

                    queryLink();

                    ed::PinId pinId = 0;
                    if (ed::QueryNewNode(&pinId)) {
                        uintptr_t pinIdN = pinId.Get();

                        Behavior::NodeGraph::PinDesc pin = Behavior::NodeGraph::NodeBase::pinFromId(pinIdN);

                        if (!pin.mPin.mNode) {
                            ed::RejectNewItem();
                        } else {
                            setDragPin(pin);
                        }
                    }
                } else {
                    mDragPin.reset();
                    mDragType.reset();
                }
                ed::EndCreate();

                if (ed::BeginDelete()) {
                    std::vector<uint32_t> nodesToDelete;

                    ed::NodeId nodeId = 0;
                    while (ed::QueryDeletedNode(&nodeId)) {
                        if (ed::AcceptDeletedItem()) {
                            nodesToDelete.push_back(nodeId.Get() / 60000);
                        }
                    }

                    for (auto it = nodesToDelete.begin(); it != nodesToDelete.end(); ++it) {
                        ImVec2 pos = ed::GetNodePosition(60000 * mGraph.nodes().size());
                        ed::SetNodePosition(60000 * *it, pos);
                        mGraph.removeNode(*it);
                        for (auto it2 = std::next(it); it2 != nodesToDelete.end(); ++it2) {
                            if (*it2 > *it)
                                --*it2;
                        }
                    }

                    if (nodesToDelete.empty()) {
                        ed::LinkId linkId = 0;
                        while (ed::QueryDeletedLink(&linkId)) {
                            if (ed::AcceptDeletedItem()) {
                                uintptr_t pinIdN = linkId.Get();
                                Behavior::NodeGraph::PinDesc pin = Behavior::NodeGraph::NodeBase::pinFromId(pinIdN);

                                if (pin.mType == Behavior::NodeGraph::PinType::Flow) {
                                    mGraph.disconnectFlow(pin.mPin);
                                } else {
                                    mGraph.disconnectData(pin.mPin);
                                }
                            }
                        }
                    }
                }
                ed::EndDelete();

                ImVec2 popupPosition = ImGui::GetMousePos();
                ed::Suspend();
                if (ed::ShowNodeContextMenu(&mContextNode)) {
                    mContextPin = {};
                    mContextLink = {};
                    mPopupPosition = popupPosition;
                    ImGui::OpenPopup("NodeGraphPopup");
                } else if (ed::ShowPinContextMenu(&mContextPin)) {
                    mContextNode = {};
                    mContextLink = {};
                    mPopupPosition = popupPosition;
                    ImGui::OpenPopup("NodeGraphPopup");
                } else if (ed::ShowLinkContextMenu(&mContextLink)) {
                    mContextNode = {};
                    mContextPin = {};
                    mPopupPosition = popupPosition;
                    ImGui::OpenPopup("NodeGraphPopup");
                } else if (ed::ShowBackgroundContextMenu()) {
                    mContextLink = {};
                    mContextNode = {};
                    mContextPin = {};
                    mPopupPosition = popupPosition;
                    ImGui::OpenPopup("NodeGraphPopup");
                }

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
                if (ImGui::BeginPopup("NodeGraphPopup")) {

                    if (mContextLink) {
                        if (ImGui::MenuItem(IMGUI_ICON_X " Delete")) {
                            ed::DeleteLink(mContextLink);
                        }
                        ImGui::Separator();
                    } else if (mContextNode) {
                        if (ImGui::MenuItem("Refresh")) {
                            mGraph.node(mContextNode.Get() / 60000)->refresh();
                        }
                        if (ImGui::MenuItem(IMGUI_ICON_X " Delete")) {
                            ed::DeleteNode(mContextNode);
                        }
                        ImGui::Separator();
                    } else if (mContextPin) {
                        ImGui::Separator();
                    } else {
                        if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Node")) {
                            ImGuiTextFilter *filter;
                            if (Behavior::BehaviorHandle behavior = BehaviorSelector(&filter)) {
                                mPendingLibraryBehavior = behavior;
                            }
                            bool hasMenu = false;
                            for (const std::pair<const std::string_view, IndexType<uint32_t>> &nodeDesc : Behavior::NodeGraph::NodeRegistry::sComponentsByName()) {
                                if (filter->PassFilter(nodeDesc.first.data())) {

                                    if (!hasMenu) {
                                        hasMenu = ImGui::BeginMenu("Nodes");
                                        if (!hasMenu)
                                            break;
                                    }

                                    if (ImGui::MenuItem(nodeDesc.first.data())) {
                                        Behavior::NodeGraph::NodeBase *node = mGraph.addNode(construct(Behavior::NodeGraph::NodeRegistry::get(nodeDesc.second), mGraph));
                                        ed::SetNodePosition(60000 * mGraph.nodeIndex(node), mPopupPosition);
                                    }
                                }
                            }
                            if (hasMenu)
                                ImGui::EndMenu();

                            if (ImGui::BeginMenu("Accessors")) {
                                ImGui::TypeIterate([&](const Type::TypeName &type) {
                                    const Reflect::MetaTable *table = type.mMetaTable;
                                    if (table) {
                                        for (const Reflect::Accessor *accessor = table->mMembers; accessor->mName; ++accessor) {
                                            if (filter->PassFilter(table->mTypeName) || filter->PassFilter(accessor->mName)) {
                                                if (ImGui::InstantiateLazyMenus()) {
                                                    if (ImGui::MenuItem(accessor->mName)) {
                                                        Behavior::NodeGraph::NodeBase *node = mGraph.addNode(std::make_unique<Behavior::NodeGraph::AccessorNode>(mGraph, "Accessor/"s + table->mTypeName + "/" + accessor->mName));
                                                        ed::SetNodePosition(60000 * mGraph.nodeIndex(node), mPopupPosition);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    return false;
                                });
                                ImGui::EndMenu();
                            }
                            ImGui::EndMenu();
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();

                if (mPendingLibraryBehavior && mPendingLibraryBehavior.state().is_ready()) {
                    if (mPendingLibraryBehavior.state()) {
                        Behavior::NodeGraph::NodeBase *node = mGraph.addNode(std::make_unique<Behavior::NodeGraph::BehaviorNode>(mGraph, std::move(mPendingLibraryBehavior)));
                        ed::SetNodePosition(60000 * mGraph.nodeIndex(node), mPopupPosition);
                    }
                    mPendingLibraryBehavior.reset();
                }

                ed::NodeId selectedNode[2];
                if (ed::GetSelectedNodes(selectedNode, 2) == 1) {
                    uintptr_t nodeId = selectedNode[0].Get();

                    if (nodeId < std::numeric_limits<int>::max() - 2) {
                        mSelectedNodeIndex = nodeId / 60000 - 1;
                    } else {
                        mSelectedNodeIndex.reset();
                    }

                    mSelectedInputs = nodeId == std::numeric_limits<int>::max() - 1;
                } else {
                    mSelectedNodeIndex.reset();
                }

                ed::Resume();

                ed::End();

                ed::SetCurrentEditor(nullptr);

                if (hoveredPin)
                    HoverPin(*hoveredPin);
            }
            ImGui::End();

            renderHierarchy();
            renderSelection();
        }
        ImGui::End();

        if (!open) {
            if (mHistory.isDirty()) {
                root().dialogs().showGrouped("Close", closeDialog(), [this]() { mVisible = false; });
            } else {
                mVisible = false;
            }
        }
    }

    void NodeGraphEditor::renderHierarchy()
    {
        if (mHierarchyVisible) {
            if (beginSubPanel("Hierarchy", &mHierarchyVisible, ImGuiDir_Left)) {
            }
            ImGui::End();
        }
    }

    void NodeGraphEditor::renderSelection()
    {
        if (mNodeDetailsVisible) {
            if (beginSubPanel("Node Details", &mNodeDetailsVisible, ImGuiDir_Right)) {
                if (mSelectedInputs) {
                    if (ImGui::BeginTable("inputs", 2, ImGuiTableFlags_Resizable)) {
                        TracedRoot<Reflect::SequenceRange> range { mHistory, Reflect::SequenceRange { mGraph.mNamedInputs } };
                        getTool<Inspector>().drawValue("Inputs", range, true);
                        ImGui::EndTable();
                    }
                }
                if (mSelectedNodeIndex) {
                    if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_Resizable)) {
                        TracedRoot<Reflect::ScopePtr> traced { mHistory, Reflect::ScopePtr { mGraph.nodes()[mSelectedNodeIndex].get() } };
                        getTool<Inspector>().drawMembers(traced);
                        ImGui::EndTable();
                    }
                }
            }
            ImGui::End();
        }
    }

    void NodeGraphEditor::setDragPin(Behavior::NodeGraph::PinDesc pin)
    {
        mDragPin = pin;
        switch (pin.mType) {
        case Behavior::NodeGraph::PinType::Data:
            if (pin.mDir == Behavior::NodeGraph::PinDir::In) {
                mDragType = mGraph.dataInType(pin.mPin);
                mDragMask = mGraph.dataInMask(pin.mPin);
            } else {
                mDragType = mGraph.dataOutType(pin.mPin);
                mDragMask = mGraph.dataOutMask(pin.mPin);
            }
            break;
        case Behavior::NodeGraph::PinType::Flow:
            if (pin.mDir == Behavior::NodeGraph::PinDir::In) {
                mDragMask = mGraph.flowInMask(pin.mPin);
            } else {
                mDragMask = mGraph.flowOutMask(pin.mPin);
            }
            break;
        default:
            throw 0;
        }
    }

    std::string_view NodeGraphEditor::key() const
    {
        return "NodeGraphEditor";
    }

    void NodeGraphEditor::saveAs(const Platform::Filesystem::Path &path)
    {
        mPath = path;

        mGraph.saveToFile(path);

        if (!mGraphHandle)
            mGraphHandle.create(path.stem(), path);

        mHistory.onSave();
    }

    void NodeGraphEditor::open(Resources::ResourceBase *res)
    {
        auto callback = [this](bool b) {
            mEditor.reset();
            if (b) {
                mGraph = *mGraphHandle;
                mPath = mGraphHandle.info()->resource()->path();
            } else {
                mGraph = {};
                mPath.clear();
            }
            createEditor();
        };

        if (res) {
            mRoot.taskQueue()->queueTask(mGraphHandle.load(static_cast<Behavior::NodeGraph::NodeGraphLoader::Resource *>(res)).then(std::move(callback)));
        } else {
            mGraphHandle.reset();
            callback(false);
        }

        mVisible = true;

        Focus();
    }

    std::string_view NodeGraphEditor::getCurrentName() const
    {
        return mPath.stem();
    }

    Dialog<> NodeGraphEditor::closeDialog()
    {
        return ResourceFile::closeDialog();
    }

    bool NodeGraphEditor::saveImpl(std::string_view view, ed::SaveReasonFlags reason)
    {
        // verify();

        if (mInitialLoad) {
            mInitialLoad = false;
        } else if ((reason & (ed::SaveReasonFlags::User | ed::SaveReasonFlags::AddNode | ed::SaveReasonFlags::RemoveNode)) != ed::SaveReasonFlags::None) {
            // mIsDirty = true; //TODO
        }

        mGraph.mLayoutData = view;

        return true;
    }

    size_t NodeGraphEditor::loadImpl(char *data)
    {
        if (data) {
            strcpy_s(data, mGraph.mLayoutData.size(), mGraph.mLayoutData.c_str());
        }
        return mGraph.mLayoutData.size();
    }

    void NodeGraphEditor::createEditor()
    {
        assert(!mEditor);

        ed::Config config;

        config.UserPointer = this;

        config.SaveSettings = [](const char *data, size_t size, ed::SaveReasonFlags reason, void *userPointer) {
            return static_cast<NodeGraphEditor *>(userPointer)->saveImpl({ data, size }, reason);
        };

        config.LoadSettings = [](char *data, void *userPointer) {
            return static_cast<NodeGraphEditor *>(userPointer)->loadImpl(data);
        };

        mEditor = { ed::CreateEditor(&config), &ed::DestroyEditor };

        mInitialLoad = true;
    }

    void NodeGraphEditor::queryLink()
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId)) {

            uintptr_t inputPinIdN = inputPinId.Get();

            Behavior::NodeGraph::PinDesc inputPin = Behavior::NodeGraph::NodeBase::pinFromId(inputPinIdN);

            setDragPin(inputPin);

            uintptr_t outputPinIdN = outputPinId.Get();

            Behavior::NodeGraph::PinDesc outputPin = Behavior::NodeGraph::NodeBase::pinFromId(outputPinIdN);

            if (outputPin.mDir == inputPin.mDir) {
                ShowLabel("x Incompatible Pin Directions", ImColor(45, 32, 32, 180));
                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                return;
            }

            if (outputPin.mDir == Behavior::NodeGraph::PinDir::In) {
                std::swap(inputPin, outputPin);
            }

            // make this a < check
            if (inputPin.mPin.mNode == outputPin.mPin.mNode) {
                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                return;
            }

            if (!outputPin.isCompatible(inputPin)) {
                ShowLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                return;
            }

            uint32_t inputMask, outputMask;
            if (outputPin.mType == Behavior::NodeGraph::PinType::Data) {
                Reflect::ExtendedType inputType = mGraph.dataInType(inputPin.mPin);
                Reflect::ExtendedType outputType = mGraph.dataOutType(outputPin.mPin);

                if (!inputType.isCompatible(outputType)) {
                    ShowLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    return;
                }

                if (mGraph.dataInSource(inputPin.mPin)) {
                    ShowLabel("x Cannot connect multiple links to pin", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    return;
                }

                inputMask = mGraph.dataInMask(inputPin.mPin);
                outputMask = mGraph.dataOutMask(outputPin.mPin);
            } else {
                if (mGraph.flowOutTarget(outputPin.mPin)) {
                    ShowLabel("x Cannot connect multiple links to pin", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    return;
                }

                inputMask = mGraph.flowInMask(inputPin.mPin);
                outputMask = mGraph.flowOutMask(outputPin.mPin);
            }

            if (!(inputMask & outputMask)) {
                ShowLabel("x Incompatible Execution Masks", ImColor(45, 32, 32, 180));
                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                return;
            }

            ShowLabel("+ Create Link", ImColor(32, 45, 32, 180));
            if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {
                if (outputPin.mType == Behavior::NodeGraph::PinType::Flow) {
                    mGraph.connectFlow(outputPin.mPin, inputPin.mPin);
                } else {
                    mGraph.connectData(inputPin.mPin, outputPin.mPin);
                }
            }
        }
    }

    void NodeGraphEditor::verify()
    {
        /* for (const std::unique_ptr<NodeGraph::NodeBase> &node : mGraph.nodes()) {
            NodeMessages messages;

            for (uint32_t i = 0; i < node->dataInCount(); ++i) {
                if (!node->dataInSource(i) && node->dataInDefault(i).is<std::monostate>()) {
                    std::ostringstream ss;
                    ss << "No Input provided for pin '" << node->dataInName(i) << "'!";
                    messages.mErrorMessages.push_back(ss.str());
                }
            }

            if (messages.mErrorMessages.empty() && messages.mWarningMessages.empty()) {
                mNodeMessages.erase(node.get());
            } else {
                auto pib = mNodeMessages.try_emplace(node.get(), std::move(messages));
                if (!pib.second)
                    pib.first->second = std::move(messages);
            }
        }*/
    }
}
}
