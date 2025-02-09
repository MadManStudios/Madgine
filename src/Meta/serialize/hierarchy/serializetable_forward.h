#pragma once

namespace Engine {
namespace FirstParty {
    struct Lobby;
}
}

DLL_IMPORT_VARIABLE2(const Engine::Serialize::SerializeTable, serializeTable, Engine::NoneOf<Engine::Serialize::SerializableDataConstPtr, void, std::optional<Engine::FirstParty::Lobby>> T);

namespace Engine {
namespace Serialize {

	DERIVE_FUNCTION(onActivate)

}
}
