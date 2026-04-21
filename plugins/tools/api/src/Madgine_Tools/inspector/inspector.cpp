#include "../toolslib.h"

#include "inspector.h"

#include "Meta/keyvalue/keyvaluepair.h"
#include "Meta/keyvalue/scopeiterator.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../imguiicons.h"
#include "../renderer/imroot.h"
#include "../util/trace_imgui.h"
#include "functiontool.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

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

    void Inspector::update()
    {
        ToolBase::update();

        std::erase_if(mViews, [this](Trace &trace) {
            bool open = true;
            void *ptr = &trace;
            ImGuiID id = ImHashData(&ptr, sizeof(ptr));
            if (ImGui::Begin((trace.name() + "###" + std::format("{:x}", id)).c_str(), &open)) {
                if (ImGui::BeginTable("Values", 2, ImGuiTableFlags_Resizable)) {
                    KeyValueResult result = trace.follow();
                    if (result) {
                        std::stringstream ss;
                        ss << result;
                        ImGui::Text(ss.str());
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::End();
            return !open;
        });
    }

    void Inspector::render()
    {
    }

    void Inspector::renderMenu()
    {
    }

    bool Inspector::drawRemainingMembers(const Traced<const ScopePtr &> &scope, std::set<std::string> &drawn)
    {
        bool changed = false;

        for (auto it = scope.begin(); it != scope.end(); ++it) {
            if (drawn.count((*it)->key()) == 0) {
                ImGui::TableNextRow();
                changed |= drawMember(it);
                drawn.insert((*it)->key());
            }
        }

        return changed;
    }

    bool Inspector::drawMember(const Traced<ScopeIterator> &it)
    {
        auto f = [&](const ScopeIterator &it) {ValueType v; it->value(v); return v; };
        const Traced<ValueType> &value = it.traceEx(
            std::move(f),
            static_cast<bool(*)(const TracedAccess<ScopeIterator, decltype(f)> &, bool)>([](const TracedAccess<ScopeIterator, decltype(f)> &value, bool modified) {
                if (modified)
                    *value.mParent.get() = value.get();
                return modified;
            }));

        if (streq(it.get()->key(), "__proxy")) {
            return drawMembers(ValueType_as<ScopePtr>(value), {});
        }

        std::string_view id = it.get()->key();
        bool editable = it.get()->isEditable();

        bool modified = drawValue(id, value, editable, it.get()->type());

        if (modified)
            *it.get() = value.get();
        return modified;
    }

    bool Inspector::drawValue(std::string_view id, const Traced<ValueType &> &value, bool editable, ExtendedValueTypeDesc possibleTypes)
    {
        ValueTypeDesc actualType = value->type();

        bool modified = value.visit(overloaded {
            [&](const Traced<ScopePtr &> &scope) {
                return drawValue(id, scope, false, editable, possibleTypes, &actualType);
            },
            [&](const Traced<OwnedScopePtr &> &scope) {
                return drawValue(id, scope, editable, possibleTypes, &actualType);
            },
            [&](const Traced<KeyValueVirtualSequenceRange &> &range) {
                return drawValue(id, range, editable);
            },
            [&](const Traced<KeyValueVirtualAssociativeRange &> &range) {
                return drawValue(id, range, editable);
            },
            [&](const Traced<BoundApiFunction &> &function) {
                drawValue(id, function, editable);
                return false;
            },
            [&](const Traced<ObjectPtr &> &object) {
                return drawValue(id, object, editable, possibleTypes, &actualType);
            },
            [&](const Traced<KeyValueBinding &> &binding) {
                bool result;
                if (!Execution::access_binding(binding.trace(&KeyValueBinding::mPtr), [&](const Traced<const ValueType &> &v) {
                        TracedCast<const ValueType &, ValueType> v_copy = v;
                        result = drawValue(id, v_copy, false, v_copy->type());
                    })) {
                    TracedRoot<ValueType> v { binding.undoStack() };
                    result = drawValue(id, v, false, v->type());
                }
                if (!possibleTypes.mType.isRegular()) {
                    ImGui::SameLine(0, 0);
                    result |= drawTypeDecorations(actualType, possibleTypes);
                }
                return result;
            },
            [&]<typename T>(const Traced<T &> &other) {
                assert(ImGui::TableGetColumnCount() == 2);

                ImGui::TableNextColumn();

                ImGui::Indent();
                ImGui::AlignTextToFramePadding();
                ImGui::Text(id);
                ImGui::Unindent();

                ImGui::TableNextColumn();

                ImGui::PushID(id.data());

                ImGui::PushItemWidth(-1.0f - (ImGui::GetFrameHeight() * !possibleTypes.mType.isRegular()));

                ImGui::BeginDisabled(!editable);
                bool result = ImGui::ValueTypeDrawer::draw(other);

                ImGui::PopItemWidth();

                if (!possibleTypes.mType.isRegular()) {
                    ImGui::SameLine(0, 0);
                    result |= drawTypeDecorations(actualType, possibleTypes);
                }
                ImGui::EndDisabled();

                ImGui::PopID();

                return result;
            } });

        if (modified && actualType.mType != value->type().mType) {
            value->setType(actualType);
        }

        return modified;
    }

    bool Inspector::drawValue(std::string_view id, const Traced<ScopePtr &> &scope, bool isOwned, bool editable, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        bool modified = false;
        bool changed = false;

        auto it = mPtrSuggestionsByType.find(scope->mType);
        bool hasSuggestions = editable && it != mPtrSuggestionsByType.end();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool hovered = ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex();

        bool open = false;
        if (scope.get())
            open = ImGui::TreeNode(id.data());
        else {
            ImGui::Indent();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(id);
            ImGui::Unindent();
        }

        ImGui::TableNextColumn();

        float button_x = ImGui::GetColumnWidth() - ImGui::GetTextLineHeight();

        if (hovered) {
            ImGui::GetCurrentWindow()->WorkRect.Max.x -= ImGui::GetTextLineHeight();
            ImGui::PushClipRect(ImGui::GetCurrentWindow()->WorkRect.Min, ImGui::GetCurrentWindow()->WorkRect.Max, false);
        }

        if (hasSuggestions) {
            ImGui::PushID(id.data());
            ImGui::PushItemWidth(-1.0f - (ImGui::GetFrameHeight() * !possibleTypes.mType.isRegular()));
            if (ImGui::BeginCombo("##suggestions", scope->name().c_str())) {
                if (ImGui::Selectable("<None>")) {
                    scope->mScope = nullptr;
                    modified = true;
                }
                for (std::pair<std::string_view, ScopePtr> p : it->second()) {
                    if (ImGui::Selectable(p.first.data())) {
                        scope.get() = p.second;
                        modified = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopID();
        } else if (scope->mType) {
            ImGui::TextDisabled("%s", scope->mType->mTypeName);
        }

        if (!possibleTypes.mType.isRegular() && type) {
            ImGui::SameLine(0, 0);
            modified |= drawTypeDecorations(*type, possibleTypes);
        }

        ImGui::DraggableValueTypeSource<ScopePtr>(id, scope, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            ScopePtr newScope;
            if (ImGui::AcceptDraggableValueType(newScope, [&](const Traced<const ScopePtr &> &ptr) {
                    return ptr->mType->isDerivedFrom(scope->mType) ? KeyValueResult {} : KEYVALUE_UNKNOWN_ERROR();
                })) {
                scope.get() = newScope;
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

        if (hovered) {
            ImGui::PopClipRect();
            ImGui::SameLine(button_x + ImGui::GetCurrentWindow()->DC.Indent.x - ImGui::GetCurrentWindow()->DC.GroupOffset.x, 0.0f);
            ImGui::GetCurrentWindow()->WorkRect.Max.x += ImGui::GetTextLineHeight();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, 0);
            if (ImGui::Button(IMGUI_ICON_EYE, { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() })) {
                if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                    throw 0;
                }
                mViews.push_back({ [trace { scope.build() }, this, isOwned]() { return trace([this, isOwned](const Traced<ScopePtr &> &scope) { return std::make_pair(KeyValueResult {}, drawValue("TODO", scope, false, isOwned)); }); } });
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);
            if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                ImGui::BeginTooltip();
                std::stringstream ss;
                ss << scope;
                ImGui::Text(ss.str());
                ImGui::EndTooltip();
            }
        }

        if (open) {
            changed |= drawMembers(scope, {});
            ImGui::TreePop();
        }
        return modified || (changed && isOwned);
    }

    bool Inspector::drawValue(std::string_view id, const Traced<OwnedScopePtr &> &scope, bool editable, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        const Traced<ScopePtr> &ptr = scope.trace(&OwnedScopePtr::get);
        return drawValue(id, ptr, true, editable, possibleTypes, type);
    }

    bool Inspector::drawValue(std::string_view id, const Traced<ObjectPtr &> &object, bool editable, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        bool modified = false;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // ImGui::BeginGroup();

        bool open = false;
        if (object.get())
            open = ImGui::TreeNode(id.data());
        else {
            ImGui::Indent();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(id);
            ImGui::Unindent();
        }

        ImGui::TableNextColumn();

        ImGui::Text(object->descriptor());

        if (!possibleTypes.mType.isRegular() && type) {
            ImGui::SameLine(0, 0);
            modified |= drawTypeDecorations(*type, possibleTypes);
        }

        // ImGui::EndGroup();

        ImGui::DraggableValueTypeSource<ObjectPtr>(id, object, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            ObjectPtr newObject;
            if (ImGui::AcceptDraggableValueType(newObject)) {
                object.get() = newObject;
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

            for (const auto &[key, value] : object.trace(&ObjectPtr::values)) {
                TracedCast<ValueType &, ValueType> v = value;
                bool changed = drawValue(key.get(), v, editable, value->type());
                if (changed) {
                    // value = v;
                    throw 0;
                }
            }
            ImGui::TreePop();
        }
        return modified;
    }

    bool Inspector::drawValue(std::string_view id, const Traced<KeyValueVirtualSequenceRange &> &range, bool editable)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource<KeyValueVirtualSequenceRange>(id, range);

        ImGui::TableNextColumn();

        if (range->canInsert()) {
            if (ImGui::Button(IMGUI_ICON_PLUS))
                range->insert(range->end());
        }

        if (b) {
            size_t i = 0;
            for (auto vValue : range) {
                ImGui::TableNextRow();
                bool modified = drawValue("[" + std::to_string(i) + "]", vValue, editable, vValue->type());
                if (modified) {
                    throw 0;
                    // vValue = value;
                }
                changed |= modified;
                ++i;
            }
            ImGui::TreePop();
        }

        return changed && !range->isReference();
    }

    bool Inspector::drawValue(std::string_view id, const Traced<KeyValueVirtualAssociativeRange &> &range, bool editable)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource<KeyValueVirtualAssociativeRange>(id, range);

        ImGui::TableNextColumn();

        if (b) {
            // size_t i = 0;
            for (auto [vKey, vValue] : range) {
                ImGui::TableNextRow();
                std::string key = vKey->toShortString() /* + "##" + std::to_string(i)*/;
                bool modified = drawValue(key, vValue, editable, vValue->type());
                if (modified) {
                    throw 0;
                    // vValue = value;
                }
                changed |= modified;
                //++i;
            }
            ImGui::TreePop();
        }
        return changed && !range->isReference();
    }

    void Inspector::drawValue(std::string_view id, const Traced<BoundApiFunction &> &function, bool editable)
    {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        std::string extended = "-> " + std::string { id };
        if (ImGui::Button(extended.c_str())) {
            getTool<FunctionTool>().setCurrentFunction(id, function.get());
        }
        ImGui::DraggableValueTypeSource<BoundApiFunction>(id, function);
    }

    bool Inspector::drawMembers(const Traced<const ScopePtr &> &scope, std::set<std::string> drawn)
    {
        assert(scope.get());

        bool changed = drawRemainingMembers(scope, drawn);

        auto it2 = mPreviews.find(scope->mType);
        if (it2 != mPreviews.end()) {
            changed |= it2->second(scope);
        }
        return changed;
    }

    bool Inspector::drawTypeDecorations(ValueTypeDesc &type, ExtendedValueTypeDesc possibleTypes)
    {
        bool isSet = type != static_cast<ValueTypeDesc>(toValueTypeDesc<std::monostate>());
        switch (possibleTypes.mType) {
        case ExtendedValueTypeEnum::GenericType:
            if (ImGui::ValueTypeTypePicker(type)) {
                return true;
            }
            break;
        case ExtendedValueTypeEnum::VariantType: // Very hacky
            if (ImGui::Checkbox("##Optional", &isSet)) {
                if (isSet) {
                    type = possibleTypes.unwrap();
                } else {
                    type = toValueTypeDesc<std::monostate>();
                }
                return true;
            }
            break;
        /* case ExtendedValueTypeEnum::BindableType:
            if (ImGui::LED("##Bindable", type.mType == ValueTypeEnum::BindingValue, { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() })) {

            }
            break;*/
        default:
            throw 0;
        }
        return false;
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

    void Inspector::addPreviewDefinition(const MetaTable *type, std::function<bool(const Traced<const ScopePtr &> &)> preview)
    {
        mPreviews[type] = preview;
    }

}
}
