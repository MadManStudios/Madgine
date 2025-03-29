
#include "Meta/math/matrix4.h"
#include "Madgine/render/ptr.h"

#define struct struct alignas(16)

#define int2 Engine::Vector2i
#define float2 Engine::Vector2
#define float3 Engine::Vector3
#define float4 Engine::Vector4
#define float4x4 Engine::Matrix4
#define bool uint32_t

#define row_major

#define ArrayPtr Engine::Render::GPUArrayPtr
