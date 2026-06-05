VALUE_TYPE(Null, std::monostate, std::monostate)
VALUE_SEP
VALUE_TYPE(String, CoWString, const CoWString &, std::string_view, std::string)
VALUE_SEP
VALUE_TYPE(Bool, bool, bool)
VALUE_SEP
VALUE_TYPE(Int, int, int)
VALUE_SEP
VALUE_TYPE(UInt, uint64_t, uint64_t, uint32_t, uint16_t)
VALUE_SEP
VALUE_TYPE(Float, float, float)
VALUE_SEP
VALUE_TYPE(Scope, ScopePtr, const ScopePtr &)
VALUE_SEP
VALUE_TYPE(OwnedScope, OwnedScopePtr, const OwnedScopePtr &)
VALUE_SEP
VALUE_TYPE(Matrix3, CoW<Math::Matrix3>, const CoW<Math::Matrix3> &, const Math::Matrix3 &)
VALUE_SEP
VALUE_TYPE(Matrix4, CoW<Math::Matrix4>, const CoW<Math::Matrix4> &, const Math::Matrix4 &)
VALUE_SEP
VALUE_TYPE(Quaternion, Math::Quaternion, const Math::Quaternion &)
VALUE_SEP
VALUE_TYPE(Vector4, Math::Vector4, const Math::Vector4 &)
VALUE_SEP
VALUE_TYPE(Vector3, Math::Vector3, const Math::Vector3 &, Math::NormalizedVector3)
VALUE_SEP
VALUE_TYPE(Vector2, Math::Vector2, const Math::Vector2 &)
VALUE_SEP
VALUE_TYPE(Vector4i, Math::Vector4i, const Math::Vector4i &)
VALUE_SEP
VALUE_TYPE(Vector3i, Math::Vector3i, const Math::Vector3i &)
VALUE_SEP
VALUE_TYPE(Vector2i, Math::Vector2i, const Math::Vector2i &)
VALUE_SEP
VALUE_TYPE(Color3, Math::Color3, const Math::Color3 &)
VALUE_SEP
VALUE_TYPE(Color4, Math::Color4, const Math::Color4 &)
VALUE_SEP
VALUE_TYPE(AssociativeRange, AssociativeRange, const AssociativeRange &)
VALUE_SEP
VALUE_TYPE(SequenceRange, SequenceRange, const SequenceRange &)
VALUE_SEP
VALUE_TYPE(ApiFunction, ApiFunction, const ApiFunction &)
VALUE_SEP
VALUE_TYPE(BoundApiFunction, BoundApiFunction, const BoundApiFunction &)
VALUE_SEP
VALUE_TYPE(Function, Function, const Function &)
VALUE_SEP
VALUE_TYPE(Object, ObjectPtr, ObjectPtr)
VALUE_SEP
VALUE_TYPE(Enum, Enum, const Enum &)
VALUE_SEP
VALUE_TYPE(Flags, Flags, const Flags &)
VALUE_SEP
VALUE_TYPE(Sender, Sender, const Sender &)
VALUE_SEP
VALUE_TYPE(Duration, Duration64, Duration64, std::chrono::nanoseconds, std::chrono::microseconds)
VALUE_SEP
VALUE_TYPE(Type, ExtendedType, const ExtendedType &, const Type &)
VALUE_SEP
VALUE_TYPE(Binding, Binding, const Binding &)
VALUE_SEP
VALUE_TYPE(ScopeBinding, ScopeBinding, const ScopeBinding &)