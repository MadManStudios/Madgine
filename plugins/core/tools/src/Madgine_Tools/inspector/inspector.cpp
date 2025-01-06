#include "../toolslib.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "inspector.h"

#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/scopeiterator.h"

#include "Meta/keyvalue/keyvaluepair.h"

#include "functiontool.h"

#include "Madgine/root/keyvalueregistry.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "../renderer/imroot.h"

#include "../imguiicons.h"

UNIQUECOMPONENT(Engine::Tools::Inspector);

METATABLE_BEGIN_BASE(Engine::Tools::Inspector, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Inspector)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Inspector, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Inspector)

namespace Engine {
namespace Tools {

    Inspector::Inspector(ImRoot &root)
        : Tool<Inspector>(root)
    {
        addPtrSuggestion<FunctionTable>([]() {
            std::vector<std::pair<std::string_view, ScopePtr>> result;
            const FunctionTable *table = sFunctionList();
            while (table) {
                result.emplace_back(table->mName, const_cast<FunctionTable *>(table));
                table = table->mNext;
            }
            return result;
        });
    }

    Inspector::~Inspector()
    {
    }

    void Inspector::render()
    {
        if (ImGui::Begin("Inspector", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);

            auto drawList = [this](const std::map<std::string_view, ScopePtr> &items) {
                for (const std::pair<const std::string_view, ScopePtr> &p : items) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::TreeNode(p.first.data())) {
                        drawMembers(p.second, {});
                        ImGui::TreePop();
                    }
                }
            };

            if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Resizable)) {
                drawList(KeyValueRegistry::globals());
                drawList(KeyValueRegistry::workgroupLocals());
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    bool Inspector::drawRemainingMembers(ScopePtr scope, std::set<std::string> &drawn)
    {
        bool changed = false;

        for (ScopeIterator it = scope.begin(); it != scope.end(); ++it) {
            if (drawn.count(it->key()) == 0) {
                ImGui::TableNextRow();
                changed |= drawMember(scope, it);
                //drawn.insert(it->key());
            }
        }

        return changed;
    }

    bool Inspector::drawMember(ScopePtr parent, const ScopeIterator &it)
    {
        ValueType value;
        if (streq(it->key(), "__proxy")) {
            it->value(value);
            return drawMembers(value.as<ScopePtr>(), {});
        }

        std::string_view id = it->key();
        bool editable = it->isEditable();
        bool generic = it->type().mType == ExtendedValueTypeEnum::GenericType;

        it->value(value);
        std::pair<bool, bool> modified = drawValue(id, value, editable, generic);

        if (modified.first || (modified.second && !value.isReference()))
            *it = value;
        return modified.first || modified.second;
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ValueType &value, bool editable, bool generic)
    {
        std::pair<bool, bool> modified = value.visit(overloaded {
            [&](ScopePtr &scope) {
                return drawValue(id, scope, editable, generic ? &value : nullptr);
            },
            [&](OwnedScopePtr scope) {
                return drawValue(id, scope, editable, generic ? &value : nullptr);
            },
            [&](KeyValueVirtualSequenceRange &range) {
                return std::make_pair(false, drawValue(id, range, editable));
            },
            [&](KeyValueVirtualAssociativeRange &range) {
                return std::make_pair(false, drawValue(id, range, editable));
            },
            [&](BoundApiFunction &function) {
                drawValue(id, function, editable);
                return std::make_pair(false, false);
            },
            [&](ObjectPtr &object) {
                return drawValue(id, object, editable, generic ? &value : nullptr);
            },
            [&](auto &other) {
                int columns = ImGui::TableGetColumnCount();
                assert(columns == 2);

                ImGui::TableNextColumn();

                ImGui::Indent();
                ImGui::AlignTextToFramePadding();
                ImGui::Text(id);
                ImGui::Unindent();

                ImGui::TableNextColumn();

                ImGui::PushID(id.data());

                if (generic) {
                    ImGui::BeginGroup();
                    ImGui::PushItemWidth(-2.0f * ImGui::GetFrameHeight());
                } else {
                    ImGui::PushItemWidth(-1.0f);
                }

                ImGui::BeginDisabled(!editable);
                std::pair<bool, bool> result = std::make_pair(ImGui::ValueTypeDrawer::draw(other), false);

                ImGui::PopItemWidth();

                if (generic) {
                    ImGui::SameLine(0);
                    ValueTypeDesc desc;
                    if (ImGui::ValueTypeTypePicker(desc)) {
                        result.first = true;
                        value.setType(desc);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndDisabled();

                ImGui::PopID();

                return result;
            } });

        return modified;
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ScopePtr &scope, bool editable, ValueType *generic)
    {
        bool modified = false;
        bool changed = false;

        auto it = mPtrSuggestionsByType.find(scope.mType);
        bool hasSuggestions = editable && it != mPtrSuggestionsByType.end();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        //ImGui::BeginGroup();

        bool open = false;
        if (scope)
            open = ImGui::TreeNode(id.data());
        else {
            ImGui::Indent();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(id);
            ImGui::Unindent();
        }

        ImGui::TableNextColumn();

        if (hasSuggestions) {
            ImGui::PushID(id.data());
            ImGui::PushItemWidth(-1.0f + (-2.0f * ImGui::GetFrameHeight() * bool(generic)));
            if (ImGui::BeginCombo("##suggestions", scope.name().c_str())) {
                for (std::pair<std::string_view, ScopePtr> p : it->second()) {
                    if (ImGui::Selectable(p.first.data())) {
                        scope = p.second;
                        modified = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopID();

            if (generic)
                ImGui::SameLine();
        } else if (scope.mType) {
            ImGui::TextDisabled("%s", scope.mType->mTypeName);
        }

        if (generic) {
            ValueTypeDesc desc;
            if (ImGui::ValueTypeTypePicker(desc)) {
                modified = true;
                generic->setType(desc);
            }
        }

        //ImGui::EndGroup();

        ImGui::DraggableValueTypeSource(id, scope, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            if (ImGui::AcceptDraggableValueType(scope, nullptr, [&](const ScopePtr &ptr) {
                    return ptr.mType->isDerivedFrom(scope.mType);
                })) {
                modified = true;
            }
            /*OwnedScopePtr dummy;
            if (ImGui::AcceptDraggableValueType(dummy, nullptr, [&](const OwnedScopePtr &ptr) {
                    return ptr.type()->isDerivedFrom(scope.mType);
                })) {
                scope = dummy;
                modified = true;
            }*/
            ImGui::EndDragDropTarget();
        }

        if (open) {
            changed |= drawMembers(scope, {});
            ImGui::TreePop();
        }
        return std::make_pair(modified, changed);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, ValueType *generic)
    {
        ScopePtr ptr = scope.get();
        return drawValue(id, ptr, editable, generic);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ObjectPtr &object, bool editable, ValueType *generic)
    {
        bool modified = false;
        bool changed = false;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        //ImGui::BeginGroup();

        bool open = false;
        if (object)
            open = ImGui::TreeNode(id.data());
        else {
            ImGui::Indent();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(id);
            ImGui::Unindent();
        }

        ImGui::TableNextColumn();

        ImGui::Text(object.descriptor());

        if (generic) {
            ImGui::SameLine();
            ValueTypeDesc desc;
            if (ImGui::ValueTypeTypePicker(desc)) {
                modified = true;
                generic->setType(desc);
            }
        }

        //ImGui::EndGroup();

        ImGui::DraggableValueTypeSource(id, object, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            if (ImGui::AcceptDraggableValueType(object)) {
                modified = true;
            }
            /*OwnedScopePtr dummy;
            if (ImGui::AcceptDraggableValueType(dummy, nullptr, [&](const OwnedScopePtr &ptr) {
                    return ptr.type()->isDerivedFrom(scope.mType);
                })) {
                scope = dummy;
                modified = true;
            }*/
            ImGui::EndDragDropTarget();
        }

        if (open) {

            for (auto &[key, value] : object.values()) {
                ValueType v = value;
                std::pair<bool, bool> p = drawValue(key, v, value.isReference());
                changed |= p.first || p.second;
                if (p.first) {
                    value = v;
                }
            }
            ImGui::TreePop();
        }
        return std::make_pair(modified, changed);
    }

    bool Inspector::drawValue(std::string_view id, KeyValueVirtualSequenceRange &range, bool editable)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource(id, range);

        ImGui::TableNextColumn();

        if (range.canInsert()) {
            if (ImGui::Button(IMGUI_ICON_PLUS))
                range.insert(range.end());
        }

        if (b) {
            size_t i = 0;
            for (auto vValue : range) {
                ImGui::TableNextRow();
                ValueType value = vValue;
                std::pair<bool, bool> modified = drawValue("[" + std::to_string(i) + "]", value, editable, false);
                if (modified.first)
                    vValue = value;
                changed |= modified.first || (modified.second && !range.isReference());
                ++i;
            }
            ImGui::TreePop();
        }

        return changed;
    }

    bool Inspector::drawValue(std::string_view id, KeyValueVirtualAssociativeRange &range, bool editable)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource(id, range);

        ImGui::TableNextColumn();

        if (b) {
            size_t i = 0;
            for (auto [vKey, vValue] : range) {
                ImGui::TableNextRow();
                ValueType value = vValue;
                std::string key = vKey.toShortString() /* + "##" + std::to_string(i)*/;
                std::pair<bool, bool> result = drawValue(key, value, editable, false);
                if (result.first)
                    vValue = value;
                changed |= result.second;
                ++i;
            }
            ImGui::TreePop();
        }
        return changed;
    }

    void Inspector::drawValue(std::string_view id, BoundApiFunction &function, bool editable)
    {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        std::string extended = "-> " + std::string { id };
        if (ImGui::Button(extended.c_str())) {
            getTool<FunctionTool>().setCurrentFunction(id, function);
        }
        ImGui::DraggableValueTypeSource(id, function);
    }

    bool Inspector::drawMembers(ScopePtr scope, std::set<std::string> drawn)
    {
        assert(scope);

        bool changed = drawRemainingMembers(scope, drawn);

        auto it2 = mPreviews.find(scope.mType);
        if (it2 != mPreviews.end()) {
            changed |= it2->second(scope);
        }
        return changed;
    }

    std::string_view Inspector::key() const
    {
        return "Inspector";
    }

    void Inspector::addPtrSuggestion(const MetaTable *type, std::function<std::vector<std::pair<std::string_view, ScopePtr>>()> getter)
    {
        mPtrSuggestionsByType[type] = getter;
    }

    bool Inspector::hasPtrSuggestion(const MetaTable *type) const
    {
        return mPtrSuggestionsByType.contains(type);
    }

    void Inspector::addPreviewDefinition(const MetaTable *type, std::function<bool(ScopePtr)> preview)
    {
        mPreviews[type] = preview;
    }
}
}
