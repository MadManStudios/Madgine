#pragma once

namespace Engine {

struct CoWString {
    CoWString() = default;

    CoWString(const std::string &s)
        : mString(s.c_str())
        , mSize(s.size())
    {
    }

    CoWString(const std::string &&s)
        : mOwningString(new char[s.size() + 1])
        , mSize(s.size())
        , mOwning(true)
    {
#if _MSC_VER >= 1400
        strncpy_s(mOwningString, s.size() + 1, s.c_str(), mSize);
#else
        strncpy(mOwningString, s.c_str(), mSize);
#endif
    }

    CoWString(std::string &&s)
        : CoWString(static_cast<const std::string &&>(s))
    {
    }

    CoWString(std::string_view s)
        : mString(s.data())
        , mSize(s.size())
    {
    }

    CoWString(const CoWString &other)
    {
        if (other.mOwning) {
            *this = std::string { other };
        } else {
            *this = std::string_view { other };
        }
    }

    CoWString(CoWString &&other)
        : mString(std::exchange(other.mString, nullptr))
        , mSize(std::exchange(other.mSize, 0))
        , mOwning(std::exchange(other.mOwning, false))
    {
    }

    ~CoWString()
    {
        reset();
    }

    CoWString &operator=(const CoWString &other)
    {
        if (other.mOwning) {
            *this = std::string { other };
        } else {
            *this = std::string_view { other };
        }
        return *this;
    }

    CoWString &operator=(CoWString &&other)
    {
        std::swap(mString, other.mString);
        std::swap(mSize, other.mSize);
        std::swap(mOwning, other.mOwning);
        return *this;
    }

    CoWString &operator=(const std::string &s)
    {
        reset();
        mString = s.c_str();
        mSize = s.size();
        return *this;
    }

    CoWString &operator=(const std::string &&s)
    {
        reset();
        mSize = s.size();
        mOwningString = new char[mSize + 1];
#if _MSC_VER >= 1400
        strncpy_s(mOwningString, mSize + 1, s.c_str(), mSize);
#else
        strncpy(mOwningString, s.c_str(), mSize);
#endif
        mOwningString[mSize] = '\0';
        mOwning = true;
        return *this;
    }

    CoWString &operator=(std::string &&s)
    {
        (*this) = static_cast<const std::string &&>(s);
        return *this;
    }

    CoWString &operator=(std::string_view s)
    {
        reset();
        mString = s.data();
        mSize = s.size();
        return *this;
    }

    bool operator==(const CoWString &other) const
    {
        if (mSize != other.mSize)
            return false;
        if (mString == other.mString)
            return true;
        return strncmp(mString, other.mString, mSize) == 0;
    }

    std::strong_ordering operator<=>(const CoWString &other) const
    {
        if (!mString && !other.mString)
            return std::strong_ordering::equal;
        if (!other.mString)
            return std::strong_ordering::less;
        if (!mString)
            return std::strong_ordering::greater;
        return strcmp(mString, other.mString);
    }

    operator std::string_view() const &
    {
        return { mString, mSize };
    }

    operator std::string() const
    {
        return { mString, mSize };
    }

    void reset()
    {
        if (mOwning) {
            delete[] mOwningString;
            mOwning = false;
        }
        mString = nullptr;
        mSize = 0;
    }

    void resize(size_t size)
    {
        if (!mOwning || mSize < size) {
            char *newString = new char[size + 1];
#if _MSC_VER >= 1400
            strncpy_s(newString, mSize + 1, mString, std::min<size_t>(mSize, size));
#else
            strncpy(newString, mString, std::min<size_t>(mSize, size));
#endif
            if (mOwning)
                delete[] mString;
            mOwningString = newString;
            mOwningString[size] = '\0';
            mOwning = true;
        }
        mSize = size;
    }

    constexpr const char *data() const
    {
        return mString;
    }

    char *data()
    {
        makeOwning();
        assert(mOwning);
        return mOwningString;
    }

    constexpr size_t size() const
    {
        return mSize;
    }

    friend std::ostream &operator<<(std::ostream &stream, const CoWString &s)
    {
        return stream << static_cast<std::string_view>(s);
    }

    friend std::istream &operator>>(std::istream &stream, CoWString &s)
    {
        std::string buffer;
        stream >> buffer;
        s = std::move(buffer);
        return stream;
    }

private:
    void makeOwning()
    {
        if (!mOwning) {
            char *newString = new char[mSize + 1];
#if _MSC_VER >= 1400
            strncpy_s(newString, mSize + 1, mString, mSize);
#else
            strncpy(newString, mString, mSize);
#endif
            mOwningString = newString;
            mOwningString[mSize] = '\0';
            mOwning = true;
        }
    }

private:
    union {
        const char *mString = nullptr;
        char *mOwningString;
    };
    unsigned int mSize = 0;
    bool mOwning = false;
};

}