#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TEMPLATES_EXPORT Templates : Tool<Templates> {

        Templates(ImRoot &root);

        Threading::Task<bool> init() override;

        void renderMenu() override;

        std::string_view key() const override;

#ifndef STATIC_BUILD
        void showTemplateDialog(std::string_view name, Closure<void(const Filesystem::Path &)> cb = {});
        void showUniqueComponentTemplateDialog(Closure<void(const Filesystem::Path &)> cb = {});
#endif

    private:
        Inspector *mInspector;
    };

}
}