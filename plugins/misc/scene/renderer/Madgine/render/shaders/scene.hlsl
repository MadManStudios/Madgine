
#include "light.hlsl"
#include "Madgine/render/shadinglanguage/memory.hlsl"


cbuffer ScenePerApplication : register(b0)
{
    float4x4 p;
}

cbuffer ScenePerObject : register(b2)
{
    float shininess;

    bool hasTexture;
    bool hasDistanceField;
}

struct SceneInstanceData
{
    row_major float4x4 mv;
    row_major float4x4 anti_mv;
    float4 diffuseColor;
    float4 specularColor;
	//ArrayPtr<float4x4> bones;
};

StructuredBuffer<SceneInstanceData> InstanceData : register(t0, space1);

struct AppData
{
    float3 aPos : POSITION0;
    float aW : POSITION1;
    float2 aPos2 : POSITION2;
    float3 aNormal : NORMAL;
    float4 aColor : COLOR;
    float2 aUV : TEXCOORD;
    int4 aBoneIDs : BONEINDICES;
    float4 aWeights : WEIGHTS;
    uint instanceId : SV_InstanceID;
};

struct FragmentData
{
    float4 color : COLOR;
    float4 viewPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 position : SV_Position;
};

export FragmentData scene_VS(AppData IN)
{
    FragmentData OUT;

    SceneInstanceData aInstance = InstanceData[IN.instanceId];
     
    float4 pos = float4(IN.aPos, IN.aW);
    
    float2 aPos2 = IN.aPos2;

    float4 effectivePos = pos;
    
    /*if (aInstance.bones.buffer() != 0) {
	    matrix BoneTransform = aInstance.bones[IN.aBoneIDs[0]] * IN.aWeights[0]
	    + aInstance.bones[IN.aBoneIDs[1]] * IN.aWeights[1]
	    + aInstance.bones[IN.aBoneIDs[2]] * IN.aWeights[2]
	    + aInstance.bones[IN.aBoneIDs[3]] * IN.aWeights[3];
	    effectivePos = mul(BoneTransform, effectivePos);
    }*/

    OUT.viewPos = mul(aInstance.mv, effectivePos);
    
    OUT.position = mul(p, OUT.viewPos + float4(aPos2, 0.0, 0.0));
    
    OUT.color = IN.aColor * aInstance.diffuseColor;

    OUT.normal = mul((float3x3) aInstance.anti_mv, IN.aNormal);

    OUT.uv = IN.aUV;

    return OUT;
}


Texture2D diffuseTex : register(t0, space2);
Texture2D emissiveTex : register(t1, space2);



float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

export LightingInput scene(FragmentData IN)
{
    LightingInput lightInput;
    
    lightInput.albedo = IN.color;
    lightInput.viewPos = IN.viewPos;
    lightInput.emissiveColor = float3(0.0, 0.0, 0.0);
    lightInput.shininess = shininess;

    lightInput.normal = normalize(IN.normal);

    if (hasTexture)
    {
        if (hasDistanceField)
        {
            float4 sample = diffuseTex.Sample(texSampler, IN.uv);
            float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
            float opacity = saturate(sigDist / fwidth(sigDist) + 0.5);
            lightInput.albedo = opacity * lightInput.albedo;
        }
        else
        {
            lightInput.emissiveColor = emissiveTex.Sample(texSampler, IN.uv).rgb;
            lightInput.albedo = diffuseTex.Sample(texSampler, IN.uv) * lightInput.albedo;
        }
    }
    
    return lightInput;
    
}
