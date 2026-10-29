// GBufferRenderPass: writes the surface attributes of every pixel (sample) into the G-buffer.
//   target 0: albedo.rgb, 1
//   target 1: view space normal (octahedral encoded, xy), shininess, 1
//   target 2: emissive.rgb, 1
// Every alpha is 1: blending stays enabled globally, so each target must replace the old value.

#include "Madgine/render/shadinglanguage/distancefield.hlsl"
#include "Madgine/render/shadinglanguage/memory.hlsl"
#include "Madgine/render/shadinglanguage/normalencoding.hlsl"

struct GBufferPerApplication
{
    float4x4 p;
};

ConstantBuffer<GBufferPerApplication> app : register(b0);

struct GBufferPerObject
{
    bool hasTexture;
    bool hasDistanceField;
};

ConstantBuffer<GBufferPerObject> object : register(b2);

struct GBufferInstanceData
{
    row_major float4x4 mv;
    row_major float4x4 anti_mv;
    float4 diffuseColor;
    float4 emissiveColor;
};

StructuredBuffer<GBufferInstanceData> InstanceData : register(t0, space1);

// Same layout as the materials of the forward scene shader, so the material resource blocks are shared
struct GBufferMaterialData
{
    float shininess;
};

Texture2D diffuseTex : register(t0, space2);
Texture2D emissiveTex : register(t1, space2);

ConstantBuffer<GBufferMaterialData> material : register(b2, space2);

SamplerState texSampler : register(s0);

struct AppData
{
    float3 aPos : POSITION0;
    float aW : POSITION1;
    float2 aPos2 : POSITION2;
    float3 aNormal : NORMAL;
    float4 aColor : COLOR;
    float2 aUV : TEXCOORD;
    uint instanceId : SV_InstanceID;
};

struct FragmentData
{
    centroid float4 color : COLOR;
    centroid float3 emissive : COLOR1;
    centroid float3 normal : NORMAL;
    centroid float2 uv : TEXCOORD0;
    float4 position : SV_Position;
};

struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 emissive : SV_Target2;
};

export FragmentData gbuffer_VS(AppData IN)
{
    FragmentData OUT;

    GBufferInstanceData aInstance = InstanceData[IN.instanceId];

    float4 viewPos = mul(aInstance.mv, float4(IN.aPos, IN.aW));

    OUT.position = mul(app.p, viewPos + float4(IN.aPos2, 0.0, 0.0));
    OUT.color = IN.aColor * aInstance.diffuseColor;
    OUT.emissive = aInstance.emissiveColor.rgb;
    OUT.normal = mul((float3x3) aInstance.anti_mv, IN.aNormal);
    OUT.uv = IN.aUV;

    return OUT;
}

export GBufferOutput gbuffer_PS(FragmentData IN)
{
    float4 albedo = IN.color;
    float3 emissive = IN.emissive;

    if (object.hasTexture) {
        if (object.hasDistanceField) {
            float4 sample = diffuseTex.Sample(texSampler, IN.uv);
            float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
            albedo.a *= saturate(sigDist / fwidth(sigDist) + 0.5);
        } else {
            emissive += emissiveTex.Sample(texSampler, IN.uv).rgb;
            albedo *= diffuseTex.Sample(texSampler, IN.uv);
        }
    }

    // No blending in the G-buffer: (mostly) transparent surfaces are cut out
    if (albedo.a < 0.5)
        discard;

    GBufferOutput OUT;
    OUT.albedo = float4(albedo.rgb, 1.0);
    OUT.normal = float4(encodeNormal(normalize(IN.normal)), material.shininess, 1.0);
    OUT.emissive = float4(emissive, 1.0);
    return OUT;
}
