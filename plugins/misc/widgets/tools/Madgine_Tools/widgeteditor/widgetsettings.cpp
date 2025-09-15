#include "../widgetstoolslib.h"

#include "widgetsettings.h"

#include "imgui/imgui.h"

#include "Madgine/widgets/widget.h"

#include "imgui/imguiaddons.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "imgui/imgui_internal.h"

#include "Madgine/widgets/geometry.h"

#include "Madgine_Tools/imguiicons.h"

#include "Madgine_Tools/behaviortool.h"

#include "Madgine/behavior.h"

#include "Madgine_Tools/debugger/debuggerview.h"

namespace Engine {
namespace Tools {

    WidgetSettings::WidgetSettings(Widgets::WidgetBase *widget, Inspector &inspector)
        : mWidget(widget)
        , mInspector(inspector)
    {
    }

    WidgetSettings::~WidgetSettings()
    {
    }

    Widgets::WidgetBase *WidgetSettings::widget()
    {
        return mWidget;
    }

    void WidgetSettings::render()
    {

        if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_SpanFullWidth)) {

            std::vector<Widgets::Condition *> conditions;

            uint16_t activeConditions = mWidget->fetchActiveConditions(&conditions);

            if (ImGui::BeginPopup("AddConditional")) {
                mBoolBuffer.resize(conditions.size());
                size_t i = 0;
                uint16_t selection = 0;
                for (Widgets::Condition *cond : conditions) {
                    auto ref = mBoolBuffer[i];
                    bool v = ref;
                    if (ImGui::Checkbox(cond->mName.empty() ? "<>" : cond->mName.c_str(), &v)) {
                        ref = v;
                    }
                    selection |= v << i;
                    ++i;
                }
                if (selection == 0)
                    ImGui::BeginDisabled();
                if (ImGui::Button("Create")) {
                    mWidget->addConditional(selection);
                    mCurrentConditional = selection;
                    ImGui::CloseCurrentPopup();
                }
                if (selection == 0)
                    ImGui::EndDisabled();
                ImGui::EndPopup();
            }

            if (ImGui::Button("+ Conditionals")) {
                mBoolBuffer.clear();
                ImGui::OpenPopup("AddConditional");
            }

            if (ImGui::Selectable("Effective", mCurrentConditional == std::numeric_limits<uint16_t>::max())) {
                mCurrentConditional = std::numeric_limits<uint16_t>::max();
            }

            if (ImGui::Selectable("Base", mCurrentConditional == 0)) {
                mCurrentConditional = 0;
            }

            for (Widgets::PropertyDescriptor property : mWidget->conditionals()) {
                uint16_t conditional = property.mAnnotator1;

                std::stringstream out;
                if ((activeConditions & conditional) == conditional)
                    out << "x ";
                else
                    out << "  ";

                StringUtil::StreamJoiner join { out, "|" };
                for (size_t i = 0; i < conditions.size(); ++i) {
                    if (conditional & (1 << i)) {
                        join.next() << (conditions[i]->mName.empty() ? "<>" : conditions[i]->mName);
                    }
                }
                
                if (ImGui::Selectable(out.str().c_str(), mCurrentConditional == conditional)) {
                    mCurrentConditional = conditional;
                }
            }

            ImGui::Separator();

            uint16_t effectiveConditional = mCurrentConditional;
            if (mCurrentConditional == std::numeric_limits<uint16_t>::max()) {
                ImGui::BeginDisabled();
                effectiveConditional = activeConditions;
            }

            Widgets::GeometrySourceInfo source;
            Widgets::Geometry geometry = mWidget->calculateGeometry(effectiveConditional, &source);

            if (ImGui::BeginTable("Geometry-columns", 2, ImGuiTableFlags_Resizable)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Indent();
                ImGui::Text("Pos");
                ImGui::Unindent();
                ImGui::TableNextColumn();

                ImGui::BeginGroup();
                ImGui::PushID("##pos");

                for (int i = 0; i < 3; ++i) {
                    ImGui::PushMultiItemsWidths(3, std::min(300.0f, ImGui::CalcItemWidth()));
                    for (int j = 0; j < 3; ++j) {
                        int compoundId = 3 * i + j;
                        ImGui::PushID(compoundId);
                        if (j > 0)
                            ImGui::SameLine(0, 5.0f);
                        if (source.mPos[compoundId] == mCurrentConditional && mCurrentConditional != 0) {
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(150, 150, 0, 255));
                            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255, 255, 0, 255));
                            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(220, 220, 0, 255));
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
                        }
                        if (ImGui::DragFloat("", &geometry.mPos[i][j], j == 2 ? 1.0f : 0.01f)) {
                            mWidget->setPosValue(compoundId, geometry.mPos[i][j], mCurrentConditional);
                        }
                        if (source.mPos[compoundId] == mCurrentConditional && mCurrentConditional != 0) {
                            ImGui::PopStyleColor(4);
                            if (ImGui::BeginPopupCompoundContextItem()) {
                                if (ImGui::MenuItem("Reset")) {
                                    mWidget->unsetPosValue(compoundId, mCurrentConditional);
                                }
                                ImGui::EndPopup();
                            }
                        }
                        ImGui::PopID();
                        ImGui::PopItemWidth();
                    }
                }

                ImGui::PopID();
                ImGui::EndGroup();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Indent();
                ImGui::Text("Size");
                ImGui::Unindent();
                ImGui::TableNextColumn();

                ImGui::BeginGroup();
                ImGui::PushID("##size");

                for (int i = 0; i < 3; ++i) {
                    ImGui::PushMultiItemsWidths(3, std::min(300.0f, ImGui::CalcItemWidth()));
                    for (int j = 0; j < 3; ++j) {
                        int compoundId = 3 * i + j;
                        ImGui::PushID(compoundId);
                        if (j > 0)
                            ImGui::SameLine(0, 5.0f);
                        if (source.mSize[compoundId] == mCurrentConditional && mCurrentConditional != 0) {
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(150, 150, 0, 255));
                            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(255, 255, 0, 255));
                            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(220, 220, 0, 255));
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
                        }
                        if (ImGui::DragFloat("", &geometry.mSize[i][j], j == 2 ? 1.0f : 0.01f)) {
                            mWidget->setSizeValue(compoundId, geometry.mSize[i][j], mCurrentConditional);
                        }
                        if (source.mSize[compoundId] == mCurrentConditional && mCurrentConditional != 0) {
                            ImGui::PopStyleColor(4);
                            if (ImGui::BeginPopupCompoundContextItem()) {
                                if (ImGui::MenuItem("Reset")) {
                                    mWidget->unsetSizeValue(compoundId, mCurrentConditional);
                                }
                                ImGui::EndPopup();
                            }
                        }
                        ImGui::PopID();
                        ImGui::PopItemWidth();
                    }
                }

                ImGui::PopID();
                ImGui::EndGroup();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Indent();
                ImGui::Text("AspectRatio");
                ImGui::Unindent();
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("##aspectratioEnabled", &mEnforceAspectRatio))
                    enforceAspectRatio();
                ImGui::SameLine();

                if (!mEnforceAspectRatio)
                    ImGui::BeginDisabled();
                if (ImGui::DragFloat("##aspectratio", &mAspectRatio, 0.05f))
                    enforceAspectRatio();
                if (!mEnforceAspectRatio)
                    ImGui::EndDisabled();

                ImGui::EndTable();
            }

            if (mCurrentConditional == std::numeric_limits<uint16_t>::max()) {
                ImGui::EndDisabled();
            }
        }

        if (ImGui::CollapsingHeader("WidgetData")) {
            if (ImGui::BeginTable("WidgetData-columns", 2, ImGuiTableFlags_Resizable)) {
                mInspector.drawMembers(mWidget, { "Pos", "Size", "Children", "mConditions" });
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Conditions")) {
            if (ImGui::Button("+")) {
                mWidget->mConditions.emplace_back();
            }
            if (ImGui::BeginTable("Conditions-columns", 2, ImGuiTableFlags_Resizable)) {
                for (Widgets::Condition &condition : mWidget->mConditions) {
                    ImGui::TableNextRow();
                    ImGui::PushID(&condition);
                    ImGui::TableNextColumn();
                    ImGui::InputText("##condName", &condition.mName);
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 4.0f);
                    ImGui::EnumCombo("##formula", &condition.mFormula);
                    ImGui::SameLine();
                    ImGui::EnumCombo("##operator", &condition.mOperator);
                    ImGui::SameLine();
                    ImGui::DragFloat("##reference", &condition.mReferenceValue);
                    ImGui::PopItemWidth();
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Behaviors")) {
            mInspector.getTool<DebuggerView>().renderLifetime(mWidget->lifetime());
        }

        bool showParameters = false;
        if (ImGui::BeginPopupCompoundContextWindow()) {

            if (ImGui::BeginMenu(IMGUI_ICON_PLUS " Add Behavior")) {
                if (BehaviorHandle behavior = BehaviorSelector()) {
                    mPendingBehavior.mHandle = std::move(behavior);
                    mPendingBehavior.mFuture = mPendingBehavior.mHandle.createParameters();
                    mPendingBehavior.mParameters.reset();
                    showParameters = true;
                }
                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        if (showParameters)
            ImGui::OpenPopup("BehaviorParameters");

        if (ImGui::BeginPopup("BehaviorParameters")) {
            if (!mPendingBehavior.mFuture.is_ready()) {
                ImGui::Text("Loading...");
            } else {
                if (!mPendingBehavior.mParameters)
                    mPendingBehavior.mParameters = mPendingBehavior.mFuture;
                if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_SizingStretchProp)) {
                    mInspector.drawMembers(&mPendingBehavior.mParameters);
                    ImGui::EndTable();
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Create Behavior")) {
                    mWidget->addBehavior(mPendingBehavior.mHandle.create(mPendingBehavior.mParameters));
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }

    }

    void WidgetSettings::saveGeometry()
    {
        mSavedPos = mWidget->getPos();
        mSavedSize = mWidget->getSize();
    }

    void WidgetSettings::applyGeometry()
    {
    }

    void WidgetSettings::resetGeometry()
    {
        mWidget->setPos(mSavedPos);
        mWidget->setSize(mSavedSize);
    }

    void WidgetSettings::setSize(const Matrix3 &size)
    {
        mWidget->setSize(size);
        enforceAspectRatio();
    }

    void WidgetSettings::setPos(const Matrix3 &pos)
    {
        mWidget->setPos(pos);
    }

    std::pair<Matrix3, Matrix3> WidgetSettings::savedGeometry()
    {
        return { mSavedPos, mSavedSize };
    }

    std::optional<float> WidgetSettings::aspectRatio()
    {
        return mEnforceAspectRatio ? mAspectRatio : std::optional<float> {};
    }

    void WidgetSettings::setAspectRatio(std::optional<float> ratio)
    {
        mEnforceAspectRatio = ratio.has_value();
        if (mEnforceAspectRatio)
            mAspectRatio = *ratio;
    }

    void WidgetSettings::enforceAspectRatio()
    {
        if (mEnforceAspectRatio) {
            Matrix3 size = mWidget->getSize();
            for (int i = 0; i < 3; ++i)
                size[1][i] = size[0][i] / mAspectRatio;
            mWidget->setSize(size);
        }
    }

}
}