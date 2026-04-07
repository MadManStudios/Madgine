#include "../toolslib.h"

#include "inspector.h"

#include "Meta/keyvalue/keyvaluepair.h"
#include "Meta/keyvalue/scopeiterator.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../imguiicons.h"
#include "../renderer/imroot.h"
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

    struct Trace {

        struct TraceHelper {
            ~TraceHelper()
            {
                mTrace.mAccessPath.pop_back();
            }

            operator Trace &()
            {
                return mTrace;
            }

            Trace &mTrace;
        };

        TraceHelper trace(ValueType v)
        {
            mAccessPath.push_back(std::move(v));
            return { *this };
        }

        std::string name()
        {
            std::string name;
            if (mAccessPath.empty()) {
                return mValue.toShortString();
            } else {
                return mAccessPath.back().toShortString();
            }
        }

        ValueType mValue;
        std::vector<ValueType> mAccessPath;
    };

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
                    ValueType value;
                    KeyValueResult result = followTrace(value, trace);
                    if (result) {
                        std::stringstream ss;
                        ss << result;
                        ImGui::Text(ss.str());
                    } else {
                        drawValue(trace.name().c_str(), value, false, value.type());
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

    bool Inspector::drawRemainingMembers(ScopePtr scope, Trace &trace, std::set<std::string> &drawn)
    {
        bool changed = false;

        for (ScopeIterator it = scope.begin(); it != scope.end(); ++it) {
            if (drawn.count(it->key()) == 0) {
                ImGui::TableNextRow();
                changed |= drawMember(it, trace.trace(ValueType { std::string_view { it->key() } }));
                // drawn.insert(it->key());
            }
        }

        return changed;
    }

    bool Inspector::drawMember(const ScopeIterator &it, Trace &trace)
    {
        ValueType value;
        if (streq(it->key(), "__proxy")) {
            it->value(value);
            return drawMembers(value.as<ScopePtr>(), trace, {});
        }

        std::string_view id = it->key();
        bool editable = it->isEditable();

        it->value(value);
        std::pair<bool, bool> modified = drawValue(id, value, editable, trace, it->type());

        if (modified.first || (modified.second && !value.isReference()))
            *it = value;
        return modified.first || modified.second;
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ValueType &value, bool editable, ExtendedValueTypeDesc possibleTypes)
    {
        Trace trace { value };
        return drawValue(id, value, editable, trace, possibleTypes);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ValueType &value, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes)
    {
        ValueTypeDesc actualType = value.type();

        std::pair<bool, bool> modified = value.visit(overloaded {
            [&](ScopePtr &scope) {
                return drawValue(id, scope, editable, trace, possibleTypes, &actualType);
            },
            [&](OwnedScopePtr scope) {
                return drawValue(id, scope, editable, trace, possibleTypes, &actualType);
            },
            [&](KeyValueVirtualSequenceRange &range) {
                return std::make_pair(false, drawValue(id, range, editable, trace));
            },
            [&](KeyValueVirtualAssociativeRange &range) {
                return std::make_pair(false, drawValue(id, range, editable, trace));
            },
            [&](BoundApiFunction &function) {
                drawValue(id, function, editable, trace);
                return std::make_pair(false, false);
            },
            [&](ObjectPtr &object) {
                return drawValue(id, object, editable, trace, possibleTypes, &actualType);
            },
            [&](KeyValueBinding &binding) {
                std::pair<bool, bool> result;
                if (!Execution::access_binding(binding.mPtr, [&](const ValueType &v) {
                        ValueType v_copy = v;
                        result = drawValue(id, v_copy, false, v_copy.type());
                    })) {
                    ValueType v;
                    result = drawValue(id, v, false, v.type());
                }
                if (!possibleTypes.mType.isRegular()) {
                    ImGui::SameLine(0, 0);
                    result.first |= drawTypeDecorations(actualType, possibleTypes);
                }
                return result;
            },
            [&](auto &other) {
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
                std::pair<bool, bool> result = std::make_pair(ImGui::ValueTypeDrawer::draw(other), false);

                ImGui::PopItemWidth();

                if (!possibleTypes.mType.isRegular()) {
                    ImGui::SameLine(0, 0);
                    result.first |= drawTypeDecorations(actualType, possibleTypes);
                }
                ImGui::EndDisabled();

                ImGui::PopID();

                return result;
            } });

        if (modified.first && actualType.mType != value.type().mType) {
            value.setType(actualType);
        }

        return modified;
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ScopePtr &scope, bool editable, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        Trace trace { ValueType { scope } };
        return drawValue(id, scope, editable, trace, possibleTypes, type);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ScopePtr &scope, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        bool modified = false;
        bool changed = false;

        auto it = mPtrSuggestionsByType.find(scope.mType);
        bool hasSuggestions = editable && it != mPtrSuggestionsByType.end();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool hovered = ImGui::TableGetHoveredRow() == ImGui::TableGetRowIndex();

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

        float button_x = ImGui::GetColumnWidth() - ImGui::GetTextLineHeight();

        if (hovered) {
            ImGui::GetCurrentWindow()->WorkRect.Max.x -= ImGui::GetTextLineHeight();
            ImGui::PushClipRect(ImGui::GetCurrentWindow()->WorkRect.Min, ImGui::GetCurrentWindow()->WorkRect.Max, false);
        }

        if (hasSuggestions) {
            ImGui::PushID(id.data());
            ImGui::PushItemWidth(-1.0f - (ImGui::GetFrameHeight() * !possibleTypes.mType.isRegular()));
            if (ImGui::BeginCombo("##suggestions", scope.name().c_str())) {
                if (ImGui::Selectable("<None>")) {
                    scope.mScope = nullptr;
                    modified = true;
                }
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
        } else if (scope.mType) {
            ImGui::TextDisabled("%s", scope.mType->mTypeName);
        }

        if (!possibleTypes.mType.isRegular() && type) {
            ImGui::SameLine(0, 0);
            modified |= drawTypeDecorations(*type, possibleTypes);
        }

        ImGui::DraggableValueTypeSource(id, scope, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            if (ImGui::AcceptDraggableValueType(scope, [&](const ScopePtr &ptr) {
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

        if (hovered) {
            ImGui::PopClipRect();
            ImGui::SameLine(button_x + ImGui::GetCurrentWindow()->DC.Indent.x - ImGui::GetCurrentWindow()->DC.GroupOffset.x, 0.0f);
            ImGui::GetCurrentWindow()->WorkRect.Max.x += ImGui::GetTextLineHeight();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, 0);
            if (ImGui::Button(IMGUI_ICON_EYE, { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() })) {
                mViews.emplace_back(trace);
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);
            if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                ImGui::BeginTooltip();
                ImGui::Text(trace.mValue.toShortString());
                for (auto step : trace.mAccessPath) {
                    ImGui::Text(step.toShortString());
                }
                ImGui::EndTooltip();
            }
        }

        if (open) {
            changed |= drawMembers(scope, trace, {});
            ImGui::TreePop();
        }
        return std::make_pair(modified, changed);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        Trace trace { ValueType { scope } };
        return drawValue(id, scope, editable, trace, possibleTypes, type);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, OwnedScopePtr &scope, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        ScopePtr ptr = scope.get();
        return drawValue(id, ptr, editable, trace, possibleTypes, type);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ObjectPtr &object, bool editable, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        Trace trace { ValueType { object } };
        return drawValue(id, object, editable, trace, possibleTypes, type);
    }

    std::pair<bool, bool> Inspector::drawValue(std::string_view id, ObjectPtr &object, bool editable, Trace &trace, ExtendedValueTypeDesc possibleTypes, ValueTypeDesc *type)
    {
        bool modified = false;
        bool changed = false;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // ImGui::BeginGroup();

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

        if (!possibleTypes.mType.isRegular() && type) {
            ImGui::SameLine(0, 0);
            modified |= drawTypeDecorations(*type, possibleTypes);
        }

        // ImGui::EndGroup();

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
                std::pair<bool, bool> p = drawValue(key, v, value.isReference(), trace.trace(ValueType { key }), v.type());
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
        Trace trace { ValueType { range } };
        return drawValue(id, range, editable, trace);
    }

    bool Inspector::drawValue(std::string_view id, KeyValueVirtualSequenceRange &range, bool editable, Trace &trace)
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
                std::pair<bool, bool> modified = drawValue("[" + std::to_string(i) + "]", value, editable, trace.trace(ValueType { i }), value.type());
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
        Trace trace { ValueType { range } };
        return drawValue(id, range, editable, trace);
    }

    bool Inspector::drawValue(std::string_view id, KeyValueVirtualAssociativeRange &range, bool editable, Trace &trace)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource(id, range);

        ImGui::TableNextColumn();

        if (b) {
            // size_t i = 0;
            for (auto [vKey, vValue] : range) {
                ImGui::TableNextRow();
                ValueType value = vValue;
                std::string key = vKey.toShortString() /* + "##" + std::to_string(i)*/;
                std::pair<bool, bool> result = drawValue(key, value, editable, trace.trace(vKey), value.type());
                if (result.first)
                    vValue = value;
                changed |= result.second;
                //++i;
            }
            ImGui::TreePop();
        }
        return changed;
    }

    void Inspector::drawValue(std::string_view id, BoundApiFunction &function, bool editable)
    {
        Trace trace { ValueType { function } };
        drawValue(id, function, editable, trace);
    }

    void Inspector::drawValue(std::string_view id, BoundApiFunction &function, bool editable, Trace &trace)
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
        Trace trace { ValueType { scope } };
        return drawMembers(scope, trace, drawn);
    }

    bool Inspector::drawMembers(ScopePtr scope, Trace &trace, std::set<std::string> drawn)
    {
        assert(scope);

        bool changed = drawRemainingMembers(scope, trace, drawn);

        auto it2 = mPreviews.find(scope.mType);
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

    KeyValueResult Inspector::followTrace(ValueType &retVal, const Trace &trace)
    {
        retVal = trace.mValue;
        for (const auto &step : trace.mAccessPath) {

            KeyValueResult result = retVal.visit(overloaded {
                [&](ScopePtr &scope) {
                    return ValueType_unwrap(retVal, [&](ValueType &retVal, std::string_view key) -> KeyValueResult {
                        ScopeIterator it = scope.find(key);
                        if (it == scope.end()) {
                            return KEYVALUE_UNKNOWN_ERROR();
                        } else {
                            it->value(retVal);
                            return {};
                        } }, step);
                },
                [&](OwnedScopePtr &scope) {
                    return ValueType_unwrap(retVal, [&](ValueType &retVal, std::string_view key) -> KeyValueResult {
                        ScopeIterator it = scope.get().find(key);
                        if (it == scope.get().end()) {
                            return KEYVALUE_UNKNOWN_ERROR();
                        } else {
                            it->value(retVal);
                            return {};
                        } }, step);
                },
                [&](KeyValueVirtualSequenceRange &range) {
                    return ValueType_unwrap(retVal, [&](ValueType &retVal, size_t index) -> KeyValueResult {
                        auto it = range.begin();
                        for (size_t i = 0; i < index; ++i) {
                            if (it.ended()) {
                                return KEYVALUE_UNKNOWN_ERROR();
                            }
                            ++it;
                        }
                        retVal = *it;
                        return {}; }, step);
                },
                [&](KeyValueVirtualAssociativeRange &range) -> KeyValueResult {
                    auto it = std::ranges::find(range, step, &KeyValuePair::mKey);
                    if (it == range.end()) {
                        return KEYVALUE_UNKNOWN_ERROR();
                    } else {
                        retVal = it->mValue;
                        return {};
                    };
                },
                [&](auto &) -> KeyValueResult {
                    return KEYVALUE_UNKNOWN_ERROR();
                } });

            if (result)
                return result;
        }
        return {};
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
