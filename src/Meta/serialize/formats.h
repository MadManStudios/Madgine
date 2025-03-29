#pragma once

namespace Engine {
namespace Serialize {

    struct META_EXPORT Formats {
        static Format ini;
        static Format json;
        static Format safebinary;
        static Format unsafebinary;
        static Format xml;        
    };

}
}