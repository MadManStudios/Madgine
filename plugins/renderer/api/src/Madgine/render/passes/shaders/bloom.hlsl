#include "bloom.sl"



cbuffer Data : register(b0)
{
    BloomData data;
}

Texture2D input : register(t0, space2);
Texture2D blurred : register(t0, space3);

SamplerState texSampler : register(s0);


struct AppData
{
    float3 aPos : POSITION0;
};


struct FragmentData
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};


export FragmentData bloom_VS(AppData IN)
{
    FragmentData OUT;

    OUT.texCoord = (IN.aPos.xy * float2(1.0, -1.0) + float2(1.0, 1.0)) * 0.5;
    OUT.position = float4(IN.aPos, 1.0);

    return OUT;
}


float3 Uncharted2Tonemap(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}


export float4 bloom_PS(FragmentData IN)
{
    float W = 11.2;

    float3 hdrColor = input.Sample(texSampler, IN.texCoord).rgb;
    float3 bloomColor = blurred.Sample(texSampler, IN.texCoord).rgb;
    hdrColor += bloomColor; // additive blending
    // tone mapping

    hdrColor *= data.exposure; // Hardcoded Exposure Adjustment

    float ExposureBias = 2.0;
    float3 curr = Uncharted2Tonemap(ExposureBias * hdrColor);

    float3 whiteScale = 1.0 / Uncharted2Tonemap(W);
    float3 color = curr * whiteScale;
      

    return float4(color, 1.0);
}
