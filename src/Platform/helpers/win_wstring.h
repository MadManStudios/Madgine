#pragma once

#if WINDOWS

namespace Engine {
namespace StringUtil {

    PLATFORM_EXPORT std::wstring toWString(std::string_view input);
    PLATFORM_EXPORT std::string fromWString(std::wstring_view input);

}
}

#endif