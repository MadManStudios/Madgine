#include "../toolslib.h"

#include "inspector.h"

#include "Meta/reflect/scopeiterator.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
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
        addTypeHandler<Reflect::FunctionTable>([](const Traced<Reflect::ScopePtr &> &scope, bool editable) {
            bool modified = false;
            if (ImGui::BeginCombo("##suggestions", scope->name().c_str())) {
                if (ImGui::Selectable("<None>")) {
                    scope->mScope = nullptr;
                    modified = true;
                }
                const Reflect::FunctionTable *table = Reflect::sFunctionList();
                while (table) {
                    if (ImGui::Selectable(table->mName.data())) {
                        scope.get() = const_cast<Reflect::FunctionTable *>(table);
                        modified = true;
                    }
                    table = table->mNext;
                }
                ImGui::EndCombo();
            }
            return modified;
        });
    }

    Inspector::~Inspector()
    {
    }

    void Inspector::update()
    {
        ToolBase::update();

        assert(mAccessorFlagsStack.empty());

        std::erase_if(mViews, [](Trace &trace) {
            bool open = true;
            void *ptr = &trace;
            ImGuiID id = ImHashData(&ptr, sizeof(ptr));
            if (ImGui::Begin((trace.name() + "###" + std::format("{:x}", id)).c_str(), &open)) {
                if (ImGui::BeginTable("Values", 2, ImGuiTableFlags_Resizable)) {
                    Reflect::Result result = trace.follow();
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

    bool Inspector::drawRemainingMembers(const Traced<const Reflect::ScopePtr &> &scope, std::set<std::string> &drawn)
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

    bool Inspector::drawMember(const Traced<Reflect::ScopeIterator> &it)
    {
        Reflect::AccessorFlags memberFlags = it.get()->flags();
        if ((memberFlags & flags()) != memberFlags) {
            return false;
        }

        auto f = [](const Traced<const Reflect::ScopeIterator &> &tracedIt) {Reflect::Value v; tracedIt.get()->value(v, tracedIt); return v; };
        const Traced<Reflect::Value> &value = it.traceEx(
            std::move(f),
            static_cast<bool (*)(const TracedAccess<Reflect::ScopeIterator, decltype(f)> &, bool)>([](const TracedAccess<Reflect::ScopeIterator, decltype(f)> &value, bool modified) {
                if (modified)
                    *value.mParent.get() = value.get();
                return modified;
            }));

        if (streq(it.get()->key(), "__proxy")) {
            return drawMembers(Value_as<Reflect::ScopePtr>(value), {});
        }

        std::string_view id = it.get()->key();
        bool editable = it.get()->isEditable();

        bool modified = drawValue(id, value, editable, it.get()->type());

        if (modified)
            it.get()->set(value.get(), it);
        return modified;
    }

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::Value &> &value, bool editable, Reflect::ExtendedType possibleTypes)
    {
        Reflect::Type actualType = value->type();

        bool modified = value.visit(overloaded {
            [&](const Traced<Reflect::ScopePtr &> &scope) {
                return drawValue(id, scope, false, editable, possibleTypes, &actualType);
            },
            [&](const Traced<Reflect::OwnedValue &> &scope) {
                return drawValue(id, scope, editable, possibleTypes, &actualType);
            },
            [&](const Traced<Reflect::SequenceRange &> &range) {
                return drawValue(id, range, editable);
            },
            [&](const Traced<Reflect::AssociativeRange &> &range) {
                return drawValue(id, range, editable);
            },
            [&](const Traced<Reflect::BoundApiFunction &> &function) {
                drawValue(id, function, editable);
                return false;
            },
            [&](const Traced<Reflect::ObjectPtr &> &object) {
                return drawValue(id, object, editable, possibleTypes, &actualType);
            },
            [&](const Traced<Reflect::ScopeBinding &> &binding) {
                return drawValue(id, binding, editable, possibleTypes, &actualType);
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

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::ScopePtr &> &scope, bool isOwned, bool editable, Reflect::ExtendedType possibleTypes, Reflect::Type *type)
    {
        bool modified = false;
        bool changed = false;

        auto it = mTypeHandlers.find(scope->mType);
        bool hasHandler = it != mTypeHandlers.end();

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

        if (hasHandler) {
            ImGui::PushID(id.data());
            ImGui::PushItemWidth(-1.0f - (ImGui::GetFrameHeight() * !possibleTypes.mType.isRegular()));
            modified |= it->second(scope, editable);
            ImGui::PopItemWidth();
            ImGui::PopID();
        } else if (scope->mType) {
            ImGui::TextDisabled("%s", scope->mType->mTypeName);
        }

        if (!possibleTypes.mType.isRegular() && type) {
            ImGui::SameLine(0, 0);
            modified |= drawTypeDecorations(*type, possibleTypes);
        }

        ImGui::DraggableValueTypeSource<Reflect::ScopePtr>(id, scope, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            Reflect::ScopePtr newScope;
            if (ImGui::AcceptDraggableValueType(newScope, [&](const Traced<const Reflect::ScopePtr &> &ptr) {
                    return ptr->mType->isDerivedFrom(scope->mType) ? Reflect::Result {} : REFLECT_UNKNOWN_ERROR();
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

            if (ImGui::InlineContextButton(IMGUI_ICON_EYE)) {
                if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                    throw 0;
                }
                mViews.push_back({ [trace { scope.build() }, this, isOwned]() { return trace([this, isOwned](const Traced<Reflect::ScopePtr &> &scope) { return std::make_pair(Reflect::Result {}, drawValue("TODO", scope, false, isOwned)); }); } });
            }
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

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::OwnedValue &> &value, bool editable, Reflect::ExtendedType possibleTypes, Reflect::Type *type)
    {
        const Traced<Reflect::Value> &ptr = value.trace([](const Reflect::OwnedValue &value) {
            Reflect::Value result;
            value.get(result);
            return result;
        });
        if (drawValue(id, ptr, editable, possibleTypes)) {
            value.get().set(ptr.get());
            return true;
        }
        return false;
    }

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::ObjectPtr &> &object, bool editable, Reflect::ExtendedType possibleTypes, Reflect::Type *type)
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

        ImGui::DraggableValueTypeSource<Reflect::ObjectPtr>(id, object, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            Reflect::ObjectPtr newObject;
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

            for (const auto &[key, value] : object.trace(&Reflect::ObjectPtr::values)) {
                TracedCast<Reflect::Value &, Reflect::Value> v = value;
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

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::SequenceRange &> &range, bool editable)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource<Reflect::SequenceRange>(id, range);

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

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::AssociativeRange &> &range, bool editable)
    {
        ImGui::TableNextColumn();

        bool changed = false;
        bool b = ImGui::TreeNodeEx(id.data());
        ImGui::DraggableValueTypeSource<Reflect::AssociativeRange>(id, range);

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

    void Inspector::drawValue(std::string_view id, const Traced<Reflect::BoundApiFunction &> &function, bool editable)
    {
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        std::string extended = "-> " + std::string { id };
        if (ImGui::Button(extended.c_str())) {
            getTool<FunctionTool>().setCurrentFunction(id, function.get());
        }
        ImGui::DraggableValueTypeSource<Reflect::BoundApiFunction>(id, function);
    }

    bool Inspector::drawValue(std::string_view id, const Traced<Reflect::ScopeBinding &> &binding, bool editable, Reflect::ExtendedType possibleTypes, Reflect::Type *type)
    {
        bool modified;
        if (!Execution::access_binding(binding, [&](const Traced<const Reflect::Value &> &v) {
                TracedCast<const Reflect::Value &, Reflect::Value> v_copy = v;
                modified = drawValue(id, v_copy, false, v_copy->type());
            })) {
            TracedRoot<Reflect::Value> v { binding.undoStack() };
            modified = drawValue(id, v, false, v->type());
        }

        if (!possibleTypes.mType.isRegular() && type) {
            ImGui::SameLine(0, 0);
            modified |= drawTypeDecorations(*type, possibleTypes);
        }

        ImGui::DraggableValueTypeSource<Reflect::ScopeBinding>(id, binding, ImGuiDragDropFlags_SourceAllowNullID);
        if (editable && ImGui::BeginDragDropTarget()) {
            Reflect::ScopeBinding newBinding;
            if (ImGui::AcceptDraggableValueType(newBinding)) {
                binding.get() = newBinding;
                modified = true;
            }
            ImGui::EndDragDropTarget();
        }

        return modified;
    }

    bool Inspector::drawMembers(const Traced<const Reflect::ScopePtr &> &scope, std::set<std::string> drawn)
    {
        assert(scope.get());

        bool changed = drawRemainingMembers(scope, drawn);

        auto it2 = mPreviews.find(scope->mType);
        if (it2 != mPreviews.end()) {
            changed |= it2->second(scope);
        }
        return changed;
    }

    bool Inspector::drawTypeDecorations(Reflect::Type &type, Reflect::ExtendedType possibleTypes)
    {
        bool isSet = type != static_cast<Reflect::Type>(Reflect::toType<std::monostate>());
        switch (possibleTypes.mType) {
        case Reflect::ExtendedTypeEnum::GenericType:
            if (ImGui::ValueTypeTypePicker(type)) {
                return true;
            }
            break;
        case Reflect::ExtendedTypeEnum::VariantType: // Very hacky
        {
            auto [first, second] = possibleTypes.unwrapVariant();
            if (second.mType == Reflect::TypeEnum::NullValue || second.mType == Reflect::TypeEnum::BindingValue) {
                std::swap(first, second);
            }

            if (first.mType == Reflect::TypeEnum::NullValue) {
                if (second.mType != Reflect::TypeEnum::ScopeValue) {
                    if (ImGui::Checkbox("##Optional", &isSet)) {
                        if (isSet) {
                            type = second;
                        } else {
                            type = Reflect::toType<std::monostate>();
                        }
                        return true;
                    }
                }
            } else if (first.mType == Reflect::TypeEnum::BindingValue) {
                assert(second.mSecondary.mDummy == first.mSecondary.mDummy);
                if (ImGui::LED("##Bindable", type.mType == Reflect::TypeEnum::BindingValue, { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() })) {
                }
            } else {
                throw 0;
            }
        } break;

        default:
            throw 0;
        }
        return false;
    }

    std::string_view Inspector::key() const
    {
        return "Inspector";
    }

    void Inspector::addTypeHandler(const Reflect::MetaTable *type, std::function<bool(const Traced<Reflect::ScopePtr &> &scope, bool editable)> getter)
    {
        mTypeHandlers[type] = getter;
    }

    bool Inspector::hasTypeHandler(const Reflect::MetaTable *type) const
    {
        return mTypeHandlers.contains(type);
    }

    void Inspector::addPreviewDefinition(const Reflect::MetaTable *type, std::function<bool(const Traced<const Reflect::ScopePtr &> &)> preview)
    {
        mPreviews[type] = preview;
    }

    void Inspector::pushFlags(Reflect::AccessorFlags flags)
    {
        mAccessorFlagsStack.push_back(flags);
    }

    void Inspector::popFlags()
    {
        assert(!mAccessorFlagsStack.empty());
        mAccessorFlagsStack.pop_back();
    }

    Reflect::AccessorFlags Inspector::flags() const
    {
        Reflect::AccessorFlags flags = Reflect::AccessorFlags_Default;
        if (!mAccessorFlagsStack.empty())
            flags = mAccessorFlagsStack.back();
        return flags;
    }

}
}
