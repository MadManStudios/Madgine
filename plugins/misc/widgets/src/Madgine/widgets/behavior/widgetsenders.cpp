#include "../../widgetslib.h"

#include "widgetsenders.h"

#include "../widgetmanager.h"

#include "Madgine/awaitables/awaitablesender.h"

NATIVE_BEHAVIOR(Yield_Frame, Engine::Widgets::yield_frame)
NATIVE_BEHAVIOR(Wait_Frame, Engine::Widgets::wait_frame, Engine::InputParameter<"Duration", std::chrono::steady_clock::duration>)
NATIVE_BEHAVIOR(Animate_Move, Engine::Widgets::animate_move, Engine::InputParameter<"Distance", Engine::Matrix3>, Engine::InputParameter<"Duration", std::chrono::nanoseconds>)
NATIVE_BEHAVIOR(Animate_Opacity, Engine::Widgets::animate_opacity, Engine::InputParameter<"Delta", float>, Engine::InputParameter<"Duration", std::chrono::nanoseconds>)

namespace Engine {
namespace Widgets {

    Behavior animate_move(Matrix3 dist, std::chrono::nanoseconds duration, WidgetBinding widgetBinding)
    {
        auto getPos = (widgetBinding->*&WidgetBase::getPos)();
        auto setPos = widgetBinding->*&WidgetBase::setPos;

        Matrix3 start = co_await getPos;
        Matrix3 end = start + dist;

        std::chrono::microseconds acc = 0ms;

        while (acc < duration) {
            co_await setPos(lerp(start, end, std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(acc) / duration));
            acc += co_await yield_frame({}, duration, acc);
        }

        co_await setPos(end);
    }

    Behavior animate_opacity(float dist, std::chrono::nanoseconds duration, WidgetBinding widgetBinding)
    {
        auto getOpacity = (widgetBinding->*&WidgetBase::opacity)();
        auto setOpacity = widgetBinding->*&WidgetBase::setOpacity;

        float start = co_await getOpacity;
        float end = start + dist;

        std::chrono::microseconds acc = 0ms;

        while (acc < duration) {
            co_await setOpacity(lerp(start, end, std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(acc) / duration));
            acc += co_await yield_frame({}, duration, acc);
        }

        co_await setOpacity(end);
    }

}
}
