#pragma once

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

namespace Engine {

    struct MADGINE_SENTRY_EXPORT Sentry : Root::RootComponent<Sentry> {

        Sentry(Root::Root &root);
        ~Sentry();

        virtual std::string_view key() const override;

        
    private:
    };

}

REGISTER_TYPE(Engine::Sentry)