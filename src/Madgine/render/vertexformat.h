#pragma once

#include "vertex.h"
#include "Generic/offsetptr.h"

namespace Engine {
namespace Render {

    struct VertexFormat {

        VertexFormat() = default;

        template <typename VertexType>
        VertexFormat(type_holder_t<VertexType>)
            : mFormat(toVertexFormat<VertexType>(std::make_index_sequence<VertexElements::size> {}))
        {
        }

        auto operator<=>(const VertexFormat &) const = default;

        bool has(uint8_t index) const
        {
            return mFormat & (1 << index);
        }

        operator uint16_t() const
        {
            return mFormat;
        }

        size_t stride() const
        {
            size_t stride = 0;
            for (size_t i = 0; i < VertexElements::size; ++i) {
                if (has(i))
                    stride += sVertexElementSizes[i];
            }
            return stride;
        }

    private:
        template <typename VertexType, size_t... Is>
        static uint16_t toVertexFormat(std::index_sequence<Is...>)
        {
            uint16_t format = 0;

            OffsetPtr ptr { 0 };

            static constexpr void (*fs[])(uint16_t &, OffsetPtr &) = { [](uint16_t &format, OffsetPtr &ptr) {
                //Can't be select<Is> because MSVC
                using T = typename VertexElements::helpers::template recurse<Is>::type;
                if constexpr (VertexType::template holds<T>) {
                    format += (1 << Is);
                    OffsetPtr nextPtr = OffsetPtr { type_holder<VertexType>, type_holder<T> };
                    assert(ptr == nextPtr);
                    ptr = nextPtr + sizeof(T);
                }
            }... };
            for (size_t i = 0; i < sizeof...(Is); ++i) {
                fs[i](format, ptr);
            }

            return format;
        }

        uint16_t mFormat = std::numeric_limits<uint16_t>::max();
    };

}
}