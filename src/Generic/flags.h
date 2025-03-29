
#include "bits/array.h"

namespace Engine {

template <typename Enum>
struct Flags {

    using Representation = typename Enum::Representation;

    Flags() = default;

    Flags(BitArray<64> bits)
        : mBits(bits)
    {    
    }

    BitArray<64> values() const {
        return mBits;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Flags<Enum> &value)
    {
        return Representation::sTable.printFlags(stream, value.mBits);
    }

    friend std::istream &operator>>(std::istream &stream, Flags<Enum> &value)
    {
        return Representation::sTable.readFlags(stream, value.mBits);
    }

private:
    BitArray<64> mBits;
};

}