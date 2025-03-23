#pragma once

namespace Engine {
namespace Window {


    struct MainWindowListener {
        virtual void onActivate(bool active) = 0;
    };


}
}