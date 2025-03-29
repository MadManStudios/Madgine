#pragma once

#if WINDOWS

namespace Engine {
namespace StringUtil {

    INTERFACES_EXPORT std::wstring toWString(std::string_view input);
    INTERFACES_EXPORT std::string fromWString(std::wstring_view input);

}
}

#endif