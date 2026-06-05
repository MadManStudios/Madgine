
namespace Engine {
namespace Base64 {

    META_EXPORT std::string encode(const Memory::ByteBuffer &data);
    META_EXPORT bool decode(Memory::ByteBuffer &b, std::string_view string);

}
}