#pragma once

#include "Meta/math/matrix3.h"

#include "Madgine/behavior/behaviorhandle.h"

namespace Engine {
namespace Tools {

    struct WidgetSettings {
        WidgetSettings(Widgets::WidgetBase *widget, Inspector &inspector);
        ~WidgetSettings();

        Widgets::WidgetBase *widget();

        bool render(UndoStack &history);

        void saveGeometry();
        void applyGeometry();
        void resetGeometry();

        void setSize(const Math::Matrix3 &size);
        void setPos(const Math::Matrix3 &pos);

        std::pair<Math::Matrix3, Math::Matrix3> savedGeometry();

        std::optional<float> aspectRatio();
        void setAspectRatio(std::optional<float> ratio);

        void enforceAspectRatio();

    private:
        Widgets::WidgetBase *mWidget;
        Inspector &mInspector;

        Math::Matrix3 mSavedPos, mSavedSize;

        bool mEnforceAspectRatio = false;
        float mAspectRatio = 1.0f;

        uint16_t mCurrentConditional = 0;
        std::vector<bool> mBoolBuffer;
    };

}
}