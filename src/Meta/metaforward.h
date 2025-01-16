#pragma once

#include "Generic/callerhierarchy.h"
#include "Generic/enum.h"

namespace Engine {

struct ValueType;
struct KeyValuePair;

template <typename T, typename Base>
struct VirtualScope;
template <typename Base>
struct VirtualScopeBase;
struct ProxyScopeBase;
struct MetaTable;
struct ScopeIterator;
struct Accessor;
struct ScopeField;
struct ScopePtr;
struct OwnedScopePtr;
struct TypeInfo;
struct ApiFunction;
struct BoundApiFunction;
template <auto f>
struct TypedBoundApiFunction;
struct FunctionTable;
struct FunctionArgument;
struct KeyValueFunction;
struct EnumHolder;
struct FlagsHolder;
struct KeyValueSender;
struct ArgumentList;

struct ExtendedValueTypeDesc;
struct ValueTypeDesc;

struct ObjectInstance;
struct ObjectPtr;

template <typename RefT>
struct VirtualIterator;
template <typename RefT, typename AssignDefault = DefaultAssign>
struct VirtualRange;

enum KeyValueValueFlags : uint8_t;

struct Functor_to_KeyValuePair;
struct Functor_to_ValueType;

using KeyValueVirtualAssociativeIterator = VirtualIterator<KeyValuePair>;
using KeyValueVirtualAssociativeRange = VirtualRange<KeyValuePair, Functor_to_KeyValuePair>;
using KeyValueVirtualSequenceIterator = VirtualIterator<ValueType>;
using KeyValueVirtualSequenceRange = VirtualRange<ValueType, Functor_to_ValueType>;

namespace Serialize {
    struct SerializeStream;
    struct SerializableUnitBase;
    struct SyncableUnitBase;
    struct TopLevelUnitBase;
    struct SyncableBase;
    struct SerializableBase;
    struct FileBuffer;
    struct MessageHeader;
    struct SerializeManager;
    struct SyncManager;
    struct SerializeStreamData;
    struct SyncStreamData;
    struct FormattedSerializeStream;
    struct FormattedMessageStream;

    struct ReadMessage;
    struct WriteMessage;

    template <typename T>
    struct Syncable;

    struct StreamResult;
    struct StreamVisitor;

    struct SerializableUnitPtr;
    struct SerializableUnitConstPtr;
    struct SerializableDataPtr;
    struct SerializableDataConstPtr;

    struct Serializer;
    struct SyncFunction;
    struct SerializeTableCallbacks;

    struct Formatter;
    using Format = std::unique_ptr<Formatter>(*)();

    struct CompareStreamId;

    struct PendingRequest;

    struct GenericMessageReceiver;

    struct CreatorCategory;

    typedef int StateTransmissionFlags;

    typedef uint32_t ParticipantId;
    typedef uint32_t MessageId;
    typedef uint32_t UnitId;
    enum class UnitIdTag {
        NONE = 0,
        SYNCABLE = 1,
        SERIALIZABLE = 2
    };

    struct message_streambuf;

    struct noparent_deleter;

    struct SerializeTable;

    using SyncableUnitMap = std::map<UnitId, SyncableUnitBase *>;
    using SerializableUnitMap = std::map<SerializableDataConstPtr, uint32_t>;
    using SerializableUnitList = std::vector<SerializableDataPtr>;

    struct SerializableMapHolder;
    struct SerializableListHolder;

    ENUM(MessageType,
        STATE,
        ACTION,
        REQUEST,
        ERROR,
        FUNCTION_ACTION,
        FUNCTION_REQUEST,
        FUNCTION_ERROR)

    ENUM(MessageResult,
        OK,
        REJECTED,
        DATA_CORRUPTION,
        SERVER_ERROR)

    enum FunctionType {
        QUERY,
        CALL
    };

    enum Command {
        SET_ID,
        SEND_NAME_MAPPINGS
    };

    template <typename, typename... Configs>
    struct Operations;

    namespace __serialize_impl__ {
        template <typename T>
        struct SyncFunctionTable;
    }

    struct apply_map_t;
    struct set_synced_t;

    template <typename T, typename... Configs>
    void setActive(T &t, bool active, bool existenceChanged, CallerHierarchyBasePtr hierarchy = {});
    template <typename T, typename... Configs>
    StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);

}

struct Vector2;
struct Vector3;
struct Vector4;

struct NormalizedVector3;

struct Vector2i;
struct Vector3i;
struct Vector4i;

struct Color3;
struct Color4;

struct Rect2;
struct Rect2i;

struct Matrix3;
struct Matrix4;

struct Quaternion;

struct Line3;
struct Line2;

struct Ray2;
struct Ray3;

struct Frustum;
struct Sphere;
struct Plane;
struct AABB;
struct BoundingBox;
}
