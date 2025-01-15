#include "../nodegraphtoolslib.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/nodegraph/nodecollector.h"
#include "nodegrapheditor.h"

#include "NodeEditor/imgui_node_editor.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Madgine/nodegraph/nodebase.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "Madgine/nodegraph/nodeinterpreter.h"

#include "Meta/keyvalue/valuetype.h"

#include "Madgine_Tools/renderer/imroot.h"

#include "Madgine_Tools/imguiicons.h"

#include "Generic/projections.h"

#include "Generic/execution/algorithm.h"

#include "Madgine/codegen/codegen_cpp.h"
#include "Madgine/codegen/fromsender.h"

#include "Madgine/behavior.h"

#include "Madgine_Tools/debugger/debuggerview.h"

#include "Madgine/nodegraph/nodes/util/librarynode.h"

#include "imguihelpers.h"

#include "debugvisualizer.h"

#include "Interfaces/log/logsenders.h"

#include "Madgine_Tools/behaviortool.h"

#include "Madgine/nodegraph/nodes/accessornode.h"
#include "Madgine/nodegraph/nodes/functionnode.h"

UNIQUECOMPONENT(Engine::Tools::NodeGraphEditor);

METATABLE_BEGIN_BASE(Engine::Tools::NodeGraphEditor, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::NodeGraphEditor)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::NodeGraphEditor, Engine::Tools::ToolBase)
FIELD(mHierarchyVisible)
FIELD(mNodeDetailsVisible)
ENCAPSULATED_FIELD(Current, getCurrentName, load)
SERIALIZETABLE_END(Engine::Tools::NodeGraphEditor)

namespace Engine {
namespace Tools {

    NodeGraphEditor::NodeGraphEditor(ImRoot &root)
        : Tool<NodeGraphEditor>(root)
    {
    }

    Threading::Task<bool> NodeGraphEditor::init()
    {
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeDebugLocation>();

        createEditor();

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> NodeGraphEditor::finalize()
    {
        mGraphHandle.reset();
        mEditor.reset();

        co_await ToolBase::finalize();
    }

    void NodeGraphEditor::render()
    {
        renderEditor();
        renderHierarchy();
        renderSelection();
    }

    void NodeGraphEditor::renderEditor()
    {

        ImGui::PushID(this);

        std::string fileName;
        if (!mFilePath.empty()) {
            fileName = mFilePath.filename().str();
        } else {
            fileName = "Node graph";
        }
        fileName += "###editor";

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
        if (mIsDirty)
            flags |= ImGuiWindowFlags_UnsavedDocument;

        ImGui::SetNextWindowDockID(mRoot.dockSpaceId(), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(fileName.c_str(), &mVisible, flags)) {

            ImVec2 topLeftScreen = ImGui::GetCursorScreenPos();

            ImRect oldViewport = BeginNodeEditor(mEditor.get());

            std::optional<ExtendedValueTypeDesc> hoveredPin;

            ed::PushStyleVar(ed::StyleVar_NodePadding, { 0, 0, 0, 0 });
            ImVec2 specialPosition = ed::ScreenToCanvas(topLeftScreen);

            ed::SetNodePosition(std::numeric_limits<int>::max() - 1, { floorf(specialPosition.x), floorf(specialPosition.y) });
            ed::BeginNode(std::numeric_limits<int>::max() - 1);
            ImGui::BeginVertical("inputPins", ImVec2(0, 0), 0.0f);
            ImGui::Dummy({ sPinIconSize, 10 });
            uint32_t pinId = 0;
            for (NodeGraph::FlowOutPinPrototype &flowPin : mGraph.mFlowOutPins) {
                FlowOutPin(nullptr, 0, pinId, 0, mGraph.node(flowPin.mTarget.mNode)->flowInMask(flowPin.mTarget.mIndex), true);
                ++pinId;
            }
            if (mDragPin && mDragPin->mDir == NodeGraph::PinDir::In && mDragPin->mType == NodeGraph::PinType::Flow) {
                FlowOutPin(nullptr, 0, pinId, 0, mDragMask, false);
            }
            pinId = 0;
            for (NodeGraph::DataOutPinPrototype &dataPin : mGraph.mDataOutPins) {
                NodeGraph::NodeBase *node = mGraph.node(dataPin.mTarget.mNode);
                ExtendedValueTypeDesc type = mGraph.dataOutType({ 0, pinId });
                if (DataOutPin(nullptr, 0, pinId, 0, type, node->dataReceiverMask(dataPin.mTarget.mIndex), static_cast<bool>(dataPin.mTarget)))
                    hoveredPin = type;
                ++pinId;
            }
            if (mDragPin && mDragPin->mDir == NodeGraph::PinDir::In && mDragPin->mType == NodeGraph::PinType::DataInstance) {
                DataOutPin(nullptr, 0, pinId, 0, *mDragType, mDragMask, {});
            }
            pinId = 0;
            for (NodeGraph::DataProviderPinPrototype &dataPin : mGraph.mDataProviderPins) {
                NodeGraph::NodeBase *node = mGraph.node(dataPin.mTargets.front().mNode);
                ExtendedValueTypeDesc type = mGraph.dataProviderType({ 0, pinId });
                if (DataProviderPin(nullptr, 0, pinId, 0, type, node->dataInMask(dataPin.mTargets.front().mIndex), true))
                    hoveredPin = type;
                ++pinId;
            }
            if (mDragPin && mDragPin->mDir == NodeGraph::PinDir::In && mDragPin->mType == NodeGraph::PinType::Data) {
                DataProviderPin(nullptr, 0, pinId, 0, *mDragType, mDragMask, false);
            }

            pinId = 0;
            for (const NodeGraph::NodeGraph::InputBinding& binding : mGraph.mInputBindings) {
                if (DataProviderPin(binding.mDescriptor.mName.c_str(), 0, pinId, 1, binding.mDescriptor.mType, NodeGraph::NodeExecutionMask::ALL, !binding.mTargets.empty()))
                    hoveredPin = binding.mDescriptor.mType;
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
            for (NodeGraph::FlowInPinPrototype &flowPin : mGraph.mFlowInPins) {
                NodeGraph::NodeBase *node = mGraph.node(flowPin.mSources.front().mNode);
                FlowInPin(nullptr, 0, pinId, 0, node->flowInMask(flowPin.mSources.front().mIndex), true);
                ++pinId;
            }
            if (mDragPin && mDragPin->mDir == NodeGraph::PinDir::Out && mDragPin->mType == NodeGraph::PinType::Flow) {
                FlowInPin(nullptr, 0, pinId, 0, mDragMask, false);
            }
            pinId = 0;
            for (NodeGraph::DataInPinPrototype &dataPin : mGraph.mDataInPins) {
                assert(dataPin.mSource && dataPin.mSource.mNode);
                NodeGraph::NodeBase *node = mGraph.node(dataPin.mSource.mNode);
                ExtendedValueTypeDesc type = node->dataProviderType(dataPin.mSource.mIndex);

                if (DataInPin(nullptr, 0, pinId, 0, type, node->dataProviderMask(dataPin.mSource.mIndex), static_cast<bool>(dataPin.mSource)))
                    hoveredPin = type;
                ++pinId;
            }
            if (mDragPin && mDragPin->mDir == NodeGraph::PinDir::Out && mDragPin->mType == NodeGraph::PinType::DataInstance) {
                DataInPin(nullptr, 0, pinId, 0, *mDragType, mDragMask, {});
            }
            pinId = 0;
            for (NodeGraph::DataReceiverPinPrototype &dataPin : mGraph.mDataReceiverPins) {
                NodeGraph::NodeBase *node = mGraph.node(dataPin.mSources.front().mNode);
                ExtendedValueTypeDesc type = mGraph.dataOutType({ 0, pinId });
                if (DataReceiverPin(nullptr, 0, pinId, 0, type, node->dataOutMask(dataPin.mSources.front().mNode), true))
                    hoveredPin = type;
                ++pinId;
            }
            if (mDragPin && mDragPin->mDir == NodeGraph::PinDir::Out && mDragPin->mType == NodeGraph::PinType::Data) {
                DataReceiverPin(nullptr, 0, pinId, 0, *mDragType, mDragMask, false);
            }
            ImGui::EndVertical();
            ed::EndNode();
            ed::PopStyleVar();

            uint32_t nodeId = 1;
            NodeGraph::NodeBase *hoveredNode = nullptr;
            for (NodeGraph::NodeBase *node : mGraph.nodes() | std::views::transform(projectionUniquePtrToPtr)) {

                if (std::optional<ExtendedValueTypeDesc> hovered = BeginNode(node, nodeId, mDragPin, mDragType))
                    hoveredPin = hovered;

                EndNode();

                if (ImGui::IsItemHovered()) {
                    hoveredNode = node;
                }

                ++nodeId;
            }

            pinId = 0;
            for (NodeGraph::FlowOutPinPrototype &pin : mGraph.mFlowOutPins) {
                assert(pin.mTarget);
                uint32_t id = NodeGraph::NodeBase::flowOutId(pinId);
                ed::Link(id, id, 60000 * pin.mTarget.mNode + NodeGraph::NodeBase::flowInId(pin.mTarget.mIndex), FlowColor(mGraph.flowOutMask({ 0, pinId })));
                ++pinId;
            }
            pinId = 0;
            for (NodeGraph::DataInPinPrototype &pin : mGraph.mDataInPins) {
                assert(pin.mSource);
                uint32_t id = NodeGraph::NodeBase::dataInId(pinId);
                ed::Link(id, 60000 * pin.mSource.mNode + NodeGraph::NodeBase::dataProviderId(pin.mSource.mIndex), id, DataColor(mGraph.dataInMask({ 0, pinId })));
                ++pinId;
            }
            pinId = 0;
            for (NodeGraph::DataOutPinPrototype &pin : mGraph.mDataOutPins) {
                assert(pin.mTarget);
                uint32_t id = NodeGraph::NodeBase::dataOutId(pinId);
                ed::Link(id, id, 60000 * pin.mTarget.mNode + NodeGraph::NodeBase::dataReceiverId(pin.mTarget.mIndex), DataColor(mGraph.dataOutMask({ 0, pinId })));
                ++pinId;
            }
            nodeId = 1;
            for (NodeGraph::NodeBase *node : mGraph.nodes() | std::views::transform(projectionUniquePtrToPtr)) {
                NodeLinks(node, nodeId);
                ++nodeId;
            }

            if (ed::BeginCreate()) {

                queryLink();

                ed::PinId pinId = 0;
                if (ed::QueryNewNode(&pinId)) {
                    uintptr_t pinIdN = pinId.Get();

                    NodeGraph::PinDesc pin = NodeGraph::NodeBase::pinFromId(pinIdN);

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
                            NodeGraph::PinDesc pin = NodeGraph::NodeBase::pinFromId(pinIdN);

                            if (pin.mType == NodeGraph::PinType::Flow) {
                                mGraph.disconnectFlow(pin.mPin);
                            } else {
                                if (pin.mDir == NodeGraph::PinDir::In) {
                                    mGraph.disconnectDataIn(pin.mPin);
                                } else {
                                    mGraph.disconnectDataOut(pin.mPin);
                                }
                            }
                        }
                    }
                }
            }
            ed::EndDelete();

            ImVec2 popupPosition = ImGui::GetMousePos();
            ed::Suspend();
            /*if (ed::ShowNodeContextMenu(&contextNodeId))                
            else if (ed::ShowPinContextMenu(&contextPinId))
            else if (ed::ShowLinkContextMenu(&contextLinkId))                
            else */
            if (ed::ShowBackgroundContextMenu()) {
                mPopupPosition = popupPosition;
                ImGui::OpenPopup("NodeGraphPopup");
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
            if (ImGui::BeginPopup("NodeGraphPopup")) {
                if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Node")) {
                    if (ImGui::BeginMenu("Nodes")) {
                        for (const std::pair<const std::string_view, IndexType<uint32_t>> &nodeDesc : NodeGraph::NodeRegistry::sComponentsByName()) {
                            if (ImGui::MenuItem(nodeDesc.first.data())) {
                                NodeGraph::NodeBase *node = mGraph.addNode(NodeGraph::NodeRegistry::get(nodeDesc.second).construct(mGraph));
                                ed::SetNodePosition(60000 * mGraph.nodeIndex(node), mPopupPosition);
                            }
                        }
                        ImGui::EndMenu();
                    }
                    if (BehaviorHandle behavior = BehaviorSelector()) {
                        NodeGraph::NodeBase *node = mGraph.addNode(std::make_unique<NodeGraph::LibraryNode>(mGraph, behavior));
                        ed::SetNodePosition(60000 * mGraph.nodeIndex(node), mPopupPosition);
                    }
                    if (ImGui::BeginMenu("Accessors")) {
                        const MetaTable *type = sTypeList();
                        while (type) {
                            if (ImGui::BeginMenu(type->mTypeName)) {
                                for (const std::pair<const char *, Accessor> *accessor = type->mMembers; accessor->first; ++accessor) {
                                    if (ImGui::MenuItem(accessor->first)) {
                                        NodeGraph::NodeBase *node;
                                        if (accessor->second.mType.mType == ValueTypeEnum::ApiFunctionValue || accessor->second.mType.mType == ValueTypeEnum::BoundApiFunctionValue) {
                                            node = mGraph.addNode(std::make_unique<NodeGraph::FunctionNode>(mGraph, *accessor->second.mType.mSecondary.mFunctionTable));
                                        } else {
                                            node = mGraph.addNode(std::make_unique<NodeGraph::AccessorNode>(mGraph, type->mSelf, accessor));
                                        }
                                        ed::SetNodePosition(60000 * mGraph.nodeIndex(node), mPopupPosition);
                                    }
                                }
                                ImGui::EndMenu();
                            }
                            type = type->mNext;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();

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

            EndNodeEditor(oldViewport);

            if (hoveredPin)
                HoverPin(*hoveredPin);
        }
        ImGui::End();

        ImGui::PopID();
    }

    void NodeGraphEditor::renderHierarchy()
    {
        if (mHierarchyVisible) {
            if (ImGui::Begin("Node graph - Hierarchy", &mHierarchyVisible)) {
                ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);
            }
            ImGui::End();
        }
    }

    void NodeGraphEditor::renderSelection()
    {
        if (mNodeDetailsVisible) {
            if (ImGui::Begin("Node graph - Node Details", &mNodeDetailsVisible)) {
                ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Right, 0.2f, false, ImGuiCond_FirstUseEver);
                if (mSelectedInputs) {
                    if (ImGui::BeginTable("inputs", 2, ImGuiTableFlags_Resizable)) {
                        KeyValueVirtualSequenceRange range { mGraph.mInputBindings };
                        mIsDirty |= getTool<Inspector>().drawValue("Inputs", range, true);
                        ImGui::EndTable();
                    }
                }
                if (mSelectedNodeIndex) {
                    if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_Resizable)) {
                        mIsDirty |= getTool<Inspector>().drawMembers(mGraph.nodes()[mSelectedNodeIndex].get());
                        ImGui::EndTable();
                    }
                }
            }
            ImGui::End();
        }
    }

    void NodeGraphEditor::setDragPin(NodeGraph::PinDesc pin)
    {
        mDragPin = pin;
        switch (pin.mType) {
        case NodeGraph::PinType::Data:
            if (pin.mDir == NodeGraph::PinDir::In) {
                mDragType = mGraph.dataInType(pin.mPin);
                mDragMask = mGraph.dataInMask(pin.mPin);
            } else {
                mDragType = mGraph.dataOutType(pin.mPin);
                mDragMask = mGraph.dataOutMask(pin.mPin);
            }
            break;
        case NodeGraph::PinType::DataInstance:
            if (pin.mDir == NodeGraph::PinDir::In) {
                mDragType = mGraph.dataReceiverType(pin.mPin);
                mDragMask = mGraph.dataReceiverMask(pin.mPin);
            } else {
                mDragType = mGraph.dataProviderType(pin.mPin);
                mDragMask = mGraph.dataProviderMask(pin.mPin);
            }
            break;
        case NodeGraph::PinType::Flow:
            if (pin.mDir == NodeGraph::PinDir::In) {
                mDragMask = mGraph.flowInMask(pin.mPin);
            } else {
                mDragMask = mGraph.flowOutMask(pin.mPin);
            }
            break;
        default:
            throw 0;
        }
    }

    void NodeGraphEditor::renderMenu()
    {
        ToolBase::renderMenu();
        if (mVisible) {

            if (ImGui::BeginMenu("Node Graph Editor")) {

                if (ImGui::MenuItem("New Graph...")) {
                    create();
                }
                if (ImGui::MenuItem("Open Graph")) {
                    Execution::detach(mRoot.dialog(
                                          [](std::string &selection) {
                                              bool alreadyClicked = false;

                                              ImGui::BeginChild("GraphList", { 0.0f, -ImGui::GetFrameHeightWithSpacing() });

                                              for (const std::pair<std::string_view, ScopePtr> &res : NodeGraph::NodeGraphLoader::getSingleton().typedResources()) {

                                                  bool selected = selection == res.first;

                                                  if (ImGui::Selectable(res.first.data(), selected)) {
                                                      selection = res.first;
                                                  }

                                                  if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                                      selection = res.first;
                                                      alreadyClicked = true;
                                                  }
                                              }

                                              ImGui::EndChild();

                                              return DialogFlags { .acceptPossible = !selection.empty(), .implicitlyAccepted = alreadyClicked };
                                          },
                                          std::make_tuple(""s),
                                          { .acceptText = "Open" })
                        | Execution::then(
                            [this](DialogResult result, const std::string &selection) {
                                if (result == DialogResult::Accepted)
                                    load(selection);
                            }));
                }
                if (ImGui::MenuItem("Save Graph", "", false)) {
                    if (mFilePath.empty()) {
                        Execution::detach(mRoot.filePicker(true)
                            | Execution::then(
                                [this](DialogResult result, const Filesystem::Path &path) {
                                    if (result == DialogResult::Accepted) {
                                        mFilePath = path;
                                        save();
                                    }
                                }));
                    } else {
                        save();
                    }
                }

                if (ImGui::MenuItem("Debug", "", false)) {
                    Debug::ContextInfo *context = &Debug::Debugger::getSingleton().createContext();
                    Execution::detach(Behavior { mGraph.interpret() } | Execution::then([](ArgumentList) { LOG("SUCCESS"); }) | Execution::with_debug_location() | Execution::with_sub_debug_location(context) | Log::log_error());
                }

                ImGui::Separator();

                ImGui::MenuItem("Hierarchy", nullptr, &mHierarchyVisible);
                ImGui::MenuItem("Node Details", nullptr, &mNodeDetailsVisible);

                ImGui::EndMenu();
            }

        }
    }

    std::string_view NodeGraphEditor::key() const
    {
        return "NodeGraphEditor";
    }

    void NodeGraphEditor::save()
    {
        mSaveQueued = true;
    }

    void NodeGraphEditor::load(std::string_view name)
    {
        mRoot.taskQueue()->queueTask(mGraphHandle.load(name).then([this](bool b) {
            mEditor.reset();
            if (b) {
                mGraph = *mGraphHandle;
                mFilePath = mGraphHandle.info()->resource()->path();
            } else
                mGraph = {};
            createEditor();
        }));
    }

    void NodeGraphEditor::create()
    {
        mEditor.reset();
        mGraph = {};
        mGraphHandle.reset();
        mFilePath.clear();
        createEditor();
    }

    std::string_view NodeGraphEditor::getCurrentName() const
    {
        return mFilePath.stem();
    }

    bool NodeGraphEditor::saveImpl(std::string_view view)
    {
        //verify();

        if (mInitialLoad) {
            mInitialLoad = false;
            return true;
        }

        mGraph.mLayoutData = view;

        if (!mSaveQueued) {
            mIsDirty = true;
            return false;
        }

        mSaveQueued = false;
        mIsDirty = false;

        mGraph.saveToFile(mFilePath);

        if (!mGraphHandle)
            mGraphHandle.create(mFilePath.stem(), mFilePath);

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
            return static_cast<NodeGraphEditor *>(userPointer)->saveImpl({ data, size });
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

            NodeGraph::PinDesc inputPin = NodeGraph::NodeBase::pinFromId(inputPinIdN);

            setDragPin(inputPin);

            uintptr_t outputPinIdN = outputPinId.Get();

            NodeGraph::PinDesc outputPin = NodeGraph::NodeBase::pinFromId(outputPinIdN);

            if ((outputPin.mType == NodeGraph::PinType::Flow && outputPin.mDir == NodeGraph::PinDir::In) || outputPin.mType == NodeGraph::PinType::DataInstance) {
                std::swap(inputPin, outputPin);
            }

            //make this a < check
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
            if (outputPin.mType == NodeGraph::PinType::Data) {
                ExtendedValueTypeDesc inputType = outputPin.mDir == NodeGraph::PinDir::Out ? mGraph.dataReceiverType(inputPin.mPin) : mGraph.dataProviderType(inputPin.mPin);
                ExtendedValueTypeDesc outputType = outputPin.mDir == NodeGraph::PinDir::Out ? mGraph.dataOutType(outputPin.mPin) : mGraph.dataInType(outputPin.mPin);

                if (!inputType.isCompatible(outputType)) {
                    ShowLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    return;
                }

                if ((outputPin.mDir == NodeGraph::PinDir::In && (mGraph.dataInSource(outputPin.mPin))) || (outputPin.mDir == NodeGraph::PinDir::Out && mGraph.dataOutTarget(outputPin.mPin))) {
                    ShowLabel("x Cannot connect multiple links to pin", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                    return;
                }

                inputMask = outputPin.mDir == NodeGraph::PinDir::Out ? mGraph.dataReceiverMask(inputPin.mPin) : mGraph.dataProviderMask(inputPin.mPin);
                outputMask = outputPin.mDir == NodeGraph::PinDir::Out ? mGraph.dataOutMask(outputPin.mPin) : mGraph.dataInMask(outputPin.mPin);
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
                if (outputPin.mType == NodeGraph::PinType::Flow) {
                    mGraph.connectFlow(outputPin.mPin, inputPin.mPin);
                } else {
                    if (outputPin.mDir == NodeGraph::PinDir::In)
                        mGraph.connectDataIn(outputPin.mPin, inputPin.mPin);
                    else
                        mGraph.connectDataOut(outputPin.mPin, inputPin.mPin);
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
