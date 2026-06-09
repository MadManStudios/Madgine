#pragma once

/**
 * @brief Main namespace for the Madgine
 */
namespace Engine {

template <typename C, typename Base>
struct container_api_impl;

template <typename...>
struct type_pack;

namespace __Generic_impl__ {
    template <typename T>
    struct to_type_pack_helper {
        using type = type_pack<T>;
    };

    template <typename... Ty>
    struct to_type_pack_helper<std::tuple<Ty...>> {
        using type = type_pack<Ty...>;
    };

    template <typename... Ty>
    struct to_type_pack_helper<type_pack<Ty...>> {
        using type = type_pack<Ty...>;
    };

    template <>
    struct to_type_pack_helper<void> {
        using type = type_pack<>;
    };
}

template <typename T>
using to_type_pack = typename __Generic_impl__::to_type_pack_helper<T>::type;

struct CompoundAtomicOperation;

struct MemberOffsetPtrTag;
template <typename T, size_t>
struct TaggedPlaceholder;

struct Any;

namespace Memory {
    template <typename>
    struct TypedByteBuffer;
    using ByteBuffer = TypedByteBuffer<const void>;
    using WritableByteBuffer = TypedByteBuffer<void>;
}

struct CoWString;
template <typename T>
struct CoW;

template <typename, typename...>
struct EnumImpl;
struct EnumMetaTable;
template <typename>
struct Flags;

struct DefaultAssign;

struct OffsetPtr;

struct Stream;

enum class AccessMode {
    READ,
    WRITE
};

namespace Execution {

    template <typename>
    struct CoroutineHandle;

    template <typename R, typename... V>
    struct Sender;

    template <auto... cpos>
    struct Lifetime;

    template <typename R, typename VPack, auto... cpo>
    struct VirtualReceiverBaseEx;

    template <typename R, typename... V>
    using VirtualReceiverBase = VirtualReceiverBaseEx<to_type_pack<R>, type_pack<V...>>;

    template <typename R, typename... _Ty>
    struct SignalStub;
    template <typename T, typename... _Ty>
    struct ConnectionInstance;

    struct StopSource;
    struct StopCallback;
    using StopToken = StopSource *;

    template <typename T>
    struct BindingPtr;
    template <typename T>
    struct ConstantBinding;

    struct BindingError;

    namespace State {
        struct Text;
        struct Progress;
        struct BeginBlock;
        struct EndBlock;
        struct PushDisabled;
        struct PopDisabled;
        struct DebugLocation;
        struct Breakpoint;
        struct Marker;
        struct FunctionPtr;
    }

    using StateDescriptor = std::variant<State::Text, State::Progress, State::BeginBlock, State::EndBlock, State::PushDisabled, State::PopDisabled, State::DebugLocation, State::Breakpoint, State::Marker, State::FunctionPtr>;


}

namespace Containers {

    template <typename>
    struct Generator;

    template <typename RefT>
    struct VirtualIterator;
    template <typename RefT, typename AssignDefault = DefaultAssign>
    struct VirtualRange;

}

}