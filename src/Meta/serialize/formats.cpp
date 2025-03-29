#include "../metalib.h"

#include "formats.h"

#include "formatter/iniformatter.h"
#include "formatter/jsonformatter.h"
#include "formatter/safebinaryformatter.h"
#include "formatter/unsafebinaryformatter.h"
#include "formatter/xmlformatter.h"

namespace Engine {
namespace Serialize {

    template <typename T>
    constexpr auto format = []() -> std::unique_ptr<Formatter> {
        return std::make_unique<T>();
    };

    Format Formats::ini = format<IniFormatter>;
    Format Formats::json = format<JSONFormatter>;
    Format Formats::safebinary = format<SafeBinaryFormatter>;
    Format Formats::unsafebinary = format<UnsafeBinaryFormatter>;
    Format Formats::xml = format<XMLFormatter>;

}
}