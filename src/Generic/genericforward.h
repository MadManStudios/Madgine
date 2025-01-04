#pragma once

/**
 * @brief Main namespace for the Madgine
*/
namespace Engine {

template <typename C, typename Base>
struct container_api_impl;

template <typename C>
struct GenerationContainer;

template <typename T>
using GenerationVector = GenerationContainer<std::vector<T>>;

template <typename...>
struct type_pack;

template <typename T>
struct make_type_pack {
    using type = type_pack<T>;
};
template <typename... T>
struct make_type_pack<type_pack<T...>> {
    using type = type_pack<T...>;
};
template <typename T>
using make_type_pack_t = typename make_type_pack<T>::type;


struct CompoundAtomicOperation;

struct MemberOffsetPtrTag;
template <typename T, size_t>
struct TaggedPlaceholder;

struct Any;

template <typename>
struct ByteBufferImpl;
using ByteBuffer = ByteBufferImpl<const void>;
using WritableByteBuffer = ByteBufferImpl<void>;

struct CoWString;
template <typename T>
struct CoW;

template <typename>
struct EnumType;
template <typename, typename>
struct BaseEnum;
struct EnumMetaTable;
template <typename>
struct Flags;

template <typename>
struct Generator;
template <typename>
struct CoroutineHandle;

struct OffsetPtr;

struct Stream;

enum class AccessMode {
    READ,
    WRITE
};

namespace Execution {

    struct Lifetime;

    template <typename R, typename VPack, auto... cpo>
    struct VirtualReceiverBaseEx;

    template <typename R, typename... V>
    using VirtualReceiverBase = VirtualReceiverBaseEx<make_type_pack_t<R>, type_pack<V...>>;

    template <typename... _Ty>
    struct SignalStub;
    template <typename T, typename... _Ty>
    struct ConnectionInstance;
}

}