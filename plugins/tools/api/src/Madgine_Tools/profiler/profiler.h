#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

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