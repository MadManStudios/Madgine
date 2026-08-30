#pragma once

#include "Generic/enum.h"
#include "Generic/tag_invocable_view.h"

namespace Engine {

namespace Type {
    struct TypeName;
    struct StorageOps;

    struct BaseStorage;
    struct StorageDeleter;

    struct InlineStorage;
}

namespace Reflect {
    struct Value;
    struct Result;

    template <typename T, typename Base>
    struct VirtualScope;
    template <typename Base>
    struct VirtualScopeBase;
    struct MetaTable;
    struct ScopeIterator;
    struct Accessor;
    using AccessorFlags = uint32_t;
    struct ScopeField;
    struct ScopePtr;
    struct OwnedValue;
    struct ApiFunction;
    struct BoundApiFunction;
    template <auto f>
    struct TypedBoundApiFunction;
    struct FunctionTable;
    struct FunctionArgument;
    struct Function;
    struct Sender;
    struct Enum;
    struct Flags;

    struct ArgumentList;

    struct ExtendedType;
    struct TypeIndex;
    struct Type;

    template <typename TypeInfo>
    struct BindingBase;
    using Binding = BindingBase<TypeIndex>;
    using ScopeBinding = BindingBase<const MetaTable *>;

    struct ObjectInstance;
    struct ObjectPtr;

    template <bool isReferenceWrapped>
    struct convert_Value_t;

    struct VirtualRangeHelper;
    using AssociativeIterator = Containers::VirtualIterator<const Value &, const Value &>;
    using AssociativeRange = Containers::VirtualRange<VirtualRangeHelper, const Value &, const Value &>;
    using SequenceIterator = Containers::VirtualIterator<const Value &>;
    using SequenceRange = Containers::VirtualRange<VirtualRangeHelper, const Value &>;

    using Duration64 = std::chrono::duration<uint64_t, std::nano>;

    struct get_reflect_contextual_t;

    using ContextPtr = tag_invocable_view<get_reflect_contextual_t, Result (Value &, const MetaTable *)>;
}

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

    template <typename T>
    struct NoParent;

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
    using Format = std::unique_ptr<Formatter> (*)();

    struct CompareStreamId;

    struct PendingRequest;

    struct GenericMessageReceiver;

    struct CreatorCategory;

    typedef uint32_t ParticipantId;
    typedef uint32_t MessageId;
    typedef uint32_t UnitId;
    enum class UnitIdTag {
        NONE = 0,
        SYNCABLE = 1,
        SERIALIZABLE = 2
    };
    constexpr ParticipantId sLocalMasterParticipantId = 1;

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

    enum class CallbackTiming {
        BEFORE,
        AFTER
    };

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

    struct set_parent_t;
    struct apply_map_t;
    struct set_synced_t;
    template <typename...>
    struct set_active_t;

    template <typename T, typename... Configs>
    StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth = 0);

    struct get_serialize_contextual_t;

    using ContextPtr = tag_invocable_view<get_serialize_contextual_t, void *(const SerializeTable *)>;

    template <typename T, typename... Configs, typename Context = ContextPtr>
    void write(FormattedSerializeStream &out, const T &t, const char *name, Context &&context = {});
    template <typename T, typename... Configs, typename Context = ContextPtr>
    StreamResult read(FormattedSerializeStream &in, T &t, const char *name, Context &&context = {});
}

namespace Math {
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
}
