#pragma once

#include "Meta/math/matrix3.h"

#include "Madgine/behaviorhandle.h"
#include "Madgine/parametertuple.h"

namespace Engine {
namespace Tools {

    struct WidgetSettings {
        WidgetSettings(Widgets::WidgetBase *widget, Inspector &inspector);
        ~WidgetSettings();

        Widgets::WidgetBase *widget();

        void render();

        void saveGeometry();
        void applyGeometry();
        void resetGeometry();

        void setSize(const Matrix3 &size);
        void setPos(const Matrix3 &pos);

        std::pair<Matrix3, Matrix3> savedGeometry();

        std::optional<float> aspectRatio();
        void setAspectRatio(std::optional<float> ratio);

        void enforceAspectRatio();

    private:
        Widgets::WidgetBase *mWidget;
        Inspector &mInspector;

        Matrix3 mSavedPos, mSavedSize;

        bool mEnforceAspectRatio = false;
        float mAspectRatio = 1.0f;

        uint16_t mCurrentConditional = 0;
        std::vector<bool> mBoolBuffer;

        struct {
            Threading::TaskFuture<ParameterTuple> mFuture;
            ParameterTuple mParameters;
            BehaviorHandle mHandle;
        } mPendingBehavior;
    };

}
}