
namespace Engine {
namespace Base64 {

    META_EXPORT std::string encode(const ByteBuffer &data);
    META_EXPORT bool decode(ByteBuffer &b, std::string_view string);

}
}