#pragma once

namespace Engine {
namespace Serialize {

    struct EnumTag;
    struct FlagsTag;
    struct DataTag;

    using SerializePrimitives = type_pack<
        bool,
        uint8_t,
        int8_t,
        uint16_t,
        int16_t,
        uint32_t,
        int32_t,
        uint64_t,
        int64_t,
        float,
        SyncableUnitBase *,
        SerializableDataPtr,
        std::string,
        ByteBuffer,
        Void,
        Vector2,
        Vector3,
        Vector4,
        Vector2i,
        Matrix3,
        EnumTag,
        FlagsTag,
        Color3,
        Color4,
        Quaternion,
        std::chrono::nanoseconds>;

    template <typename T, typename = void>
    struct PrimitiveReducer {
        typedef T type;
    };

    template <std::convertible_to<const SyncableUnitBase *> T>
    struct PrimitiveReducer<T> {
        typedef SyncableUnitBase *type;
    };

    template <typename T>
    concept SerializableUnitPtrHelper = !std::convertible_to<T, const SyncableUnitBase *> && std::is_pointer_v<T>;

    template <SerializableUnitPtrHelper T>
    struct PrimitiveReducer<T> {
        typedef SerializableDataPtr type;
    };

    template <>
    struct PrimitiveReducer<SerializableDataConstPtr> {
        typedef SerializableDataPtr type;
    };

    template <Enum T>
    struct PrimitiveReducer<T> {
        typedef std::underlying_type_t<T> type;
    };

    template <typename T>
    struct PrimitiveReducer<EnumType<T>> {
        typedef EnumTag type;
    };

    template <>
    struct PrimitiveReducer<EnumHolder> {
        typedef EnumTag type;
    };

    template <typename T>
    struct PrimitiveReducer<Flags<T>> {
        typedef FlagsTag type;
    };

    template <>
    struct PrimitiveReducer<FlagsHolder> {
        typedef FlagsTag type;
    };

    template <typename T, T invalid>
    struct PrimitiveReducer<IndexType<T, invalid>> {
        typedef T type;
    };

    template <String S>
    struct PrimitiveReducer<S> {
        typedef std::string type;
    };

    template <typename _Rep, typename _Period>
    struct PrimitiveReducer<std::chrono::duration<_Rep, _Period>> {
        typedef std::chrono::nanoseconds type;
    };

    template <typename T>
    const constexpr size_t PrimitiveTypeIndex_v = SerializePrimitives::index<uint8_t, typename PrimitiveReducer<T>::type>;

    template <typename T>
    concept PrimitiveType = SerializePrimitives::contains<typename PrimitiveReducer<T>::type>;

    template <typename T>
    struct PrimitiveHolder {
    };

    template <>
    struct PrimitiveHolder<DataTag> {
        const SerializeTable *mTable;
    };

    template <>
    struct PrimitiveHolder<SyncableUnitBase> {
        const SerializeTable *mTable;
    };

    template <>
    struct PrimitiveHolder<EnumTag> {
        const EnumMetaTable *mTable;
    };

    template <>
    struct PrimitiveHolder<FlagsTag> {
        const EnumMetaTable *mTable;
    };

}
}
