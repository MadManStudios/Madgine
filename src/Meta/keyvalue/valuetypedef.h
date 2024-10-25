VALUETYPE_TYPE(Null, std::monostate, std::monostate)
VALUETYPE_SEP
VALUETYPE_TYPE(String, CoWString, const CoWString &, std::string_view, std::string)
VALUETYPE_SEP
VALUETYPE_TYPE(Bool, bool, bool)
VALUETYPE_SEP
VALUETYPE_TYPE(Int, int, int)
VALUETYPE_SEP
VALUETYPE_TYPE(UInt, uint64_t, uint64_t, uint32_t, uint16_t)
VALUETYPE_SEP
VALUETYPE_TYPE(Float, float, float)
VALUETYPE_SEP
VALUETYPE_TYPE(Scope, ScopePtr, const ScopePtr &)
VALUETYPE_SEP
VALUETYPE_TYPE(OwnedScope, OwnedScopePtr, const OwnedScopePtr &)
VALUETYPE_SEP
VALUETYPE_TYPE(Matrix3, CoW<Matrix3>, const CoW<Matrix3>&, const Matrix3 &)
VALUETYPE_SEP
VALUETYPE_TYPE(Matrix4, CoW<Matrix4>, const CoW<Matrix4> &, const Matrix4 &)
VALUETYPE_SEP
VALUETYPE_TYPE(Quaternion, Quaternion, const Quaternion &)
VALUETYPE_SEP
VALUETYPE_TYPE(Vector4, Vector4, const Vector4 &)
VALUETYPE_SEP
VALUETYPE_TYPE(Vector3, Vector3, const Vector3 &, NormalizedVector3)
VALUETYPE_SEP
VALUETYPE_TYPE(Vector2, Vector2, const Vector2 &)
VALUETYPE_SEP
VALUETYPE_TYPE(Vector4i, Vector4i, const Vector4i &)
VALUETYPE_SEP
VALUETYPE_TYPE(Vector3i, Vector3i, const Vector3i &)
VALUETYPE_SEP
VALUETYPE_TYPE(Vector2i, Vector2i, const Vector2i &)
VALUETYPE_SEP
VALUETYPE_TYPE(Color3, Color3, const Color3 &)
VALUETYPE_SEP
VALUETYPE_TYPE(Color4, Color4, const Color4 &)
VALUETYPE_SEP
VALUETYPE_TYPE(KeyValueVirtualAssociativeRange, KeyValueVirtualAssociativeRange, const KeyValueVirtualAssociativeRange &)
VALUETYPE_SEP
VALUETYPE_TYPE(KeyValueVirtualSequenceRange, KeyValueVirtualSequenceRange, const KeyValueVirtualSequenceRange &)
VALUETYPE_SEP
VALUETYPE_TYPE(ApiFunction, ApiFunction, const ApiFunction &)
VALUETYPE_SEP
VALUETYPE_TYPE(BoundApiFunction, BoundApiFunction, const BoundApiFunction &)
VALUETYPE_SEP
VALUETYPE_TYPE(Function, KeyValueFunction, const KeyValueFunction &)
VALUETYPE_SEP
VALUETYPE_TYPE(Object, ObjectPtr, const ObjectPtr &)
VALUETYPE_SEP
VALUETYPE_TYPE(Enum, EnumHolder, const EnumHolder &)
VALUETYPE_SEP
VALUETYPE_TYPE(Flags, FlagsHolder, const FlagsHolder &)
VALUETYPE_SEP
VALUETYPE_TYPE(Sender, KeyValueSender, const KeyValueSender &)
VALUETYPE_SEP
VALUETYPE_TYPE(Duration, SINGLE_ARG(std::chrono::duration<uint64_t, std::nano>), std::chrono::nanoseconds, std::chrono::microseconds)
VALUETYPE_SEP
VALUETYPE_TYPE(Type, ExtendedValueTypeDesc, const ExtendedValueTypeDesc &, const ValueTypeDesc &)