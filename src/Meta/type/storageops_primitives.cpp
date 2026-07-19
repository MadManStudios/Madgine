#include "../metalib.h"

#include "storageops_impl.h"

#include "../math/matrix4.h"

STORAGEOPS_BEGIN(std::monostate)
CONSTRUCTOR()
STORAGEOPS_END(std::monostate)

STORAGEOPS_BEGIN(float)
CONSTRUCTOR()
STORAGEOPS_END(float)

STORAGEOPS_BEGIN(bool)
CONSTRUCTOR()
STORAGEOPS_END(bool)

STORAGEOPS_BEGIN(std::string)
CONSTRUCTOR()
STORAGEOPS_END(std::string)

STORAGEOPS_BEGIN(Engine::Reflect::Duration64)
CONSTRUCTOR()
STORAGEOPS_END(Engine::Reflect::Duration64)

STORAGEOPS_BEGIN(std::chrono::nanoseconds)
CONSTRUCTOR()
STORAGEOPS_END(std::chrono::nanoseconds)

STORAGEOPS_BEGIN(Engine::Math::Vector3)
CONSTRUCTOR()
STORAGEOPS_END(Engine::Math::Vector3)

STORAGEOPS_BEGIN(Engine::Math::Vector4)
CONSTRUCTOR()
STORAGEOPS_END(Engine::Math::Vector4)

STORAGEOPS_BEGIN(Engine::Math::Matrix3)
CONSTRUCTOR()
STORAGEOPS_END(Engine::Math::Matrix3)

STORAGEOPS_BEGIN(Engine::Math::Matrix4)
CONSTRUCTOR()
STORAGEOPS_END(Engine::Math::Matrix4)
