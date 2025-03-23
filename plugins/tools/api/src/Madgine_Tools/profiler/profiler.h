#pragma once

#include "../toolscollector.h"
#include "../toolbase.h"

namespace Engine {
namespace Tools {

    struct Profiler : Tool<Profiler> {
        Profiler(ImRoot &root);

        virtual void render() override;

        std::string_view key() const override;

    private:
        Debug::Profiler::Profiler &mProfiler;
    };

}
}

REGISTER_TYPE(Engine::Tools::Profiler)