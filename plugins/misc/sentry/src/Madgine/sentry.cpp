#include "sentrylib.h"

#include "sentry.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include <sentry.h>

UNIQUECOMPONENT(Engine::Sentry)

METATABLE_BEGIN(Engine::Sentry)
METATABLE_END(Engine::Sentry)

namespace Engine {

std::string_view Sentry::key() const
{
    return "Sentry";
}

}

Engine::Sentry::Sentry(Root::Root &root)
    : Root::RootComponent<Sentry>(root)
{
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://f374f10cbc10297d79d8b7be16390381@o4508897651785728.ingest.de.sentry.io/4508897675968592");
    // This is also the default-path. For further information and recommendations:
    // https://docs.sentry.io/platforms/native/configuration/options/#database-path
    sentry_options_set_database_path(options, ".sentry-native");
    sentry_options_set_release(options, "Madgine@1.0.0");
    sentry_options_set_debug(options, 1);
    sentry_init(options);
}

Engine::Sentry::~Sentry()
{
    sentry_close();
}
