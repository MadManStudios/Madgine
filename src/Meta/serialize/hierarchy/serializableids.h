#pragma once

namespace Engine {
namespace Serialize {

    enum SerializableIds {
        NULL_UNIT_ID = 0,
        NO_ID = 1,
        SERIALIZE_MANAGER = 2,
        BEGIN_USER_ID_SPACE,
        RESERVED_ID_COUNT = 256
    };

}
}
