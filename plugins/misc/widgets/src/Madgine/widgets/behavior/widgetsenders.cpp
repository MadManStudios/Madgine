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
        WidgetBase *widget = co_await widgetBinding;

        Matrix3 start = widget->getPos();
        Matrix3 end = start + dist;

        std::chrono::microseconds acc = 0ms;

        while (acc < duration) {
            widget->setPos(lerp(start, end, std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(acc) / duration));
            acc += co_await yield_frame();
        }

        widget->setPos(end);
    }

    Behavior animate_opacity(float dist, std::chrono::nanoseconds duration, WidgetBinding widgetBinding)
    {
        WidgetBase *widget = co_await widgetBinding;

        float start = widget->opacity();
        float end = start + dist;

        std::chrono::microseconds acc = 0ms;

        while (acc < duration) {
            widget->setOpacity(lerp(start, end, std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(acc) / duration));
            acc += co_await yield_frame();
        }

        widget->setOpacity(end);
    }

}
}


