#include "im3d.sl"

cbuffer PerApplication : register(b0)
{
    Im3DPerApplication app;
}

cbuffer PerObject : register(b2)
{
    Im3DPerObject object;
}


Texture2D tex : register(t0, space2);

SamplerState texSampler : register(s0);


struct AppData
{
    float3 aPos : POSITION0;
    float aW : POSITION1;
    float2 aPos2 : POSITION2;
    float3 aNormal : NORMAL;
    float4 aColor : COLOR;
    float2 aUV : TEXCOORD;
};

struct FragmentData
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

export FragmentData im3d_VS(AppData IN)
{
    FragmentData OUT;

    float2 aPos2 = IN.aPos2;
    
    OUT.position = mul(app.p, mul(object.mv, float4(IN.aPos, IN.aW)) + float4(aPos2, 0.0, 0.0));
    OUT.position.z += OUT.position.w;
    OUT.position.z /= 2;

    
    OUT.color = IN.aColor;

    OUT.uv = IN.aUV;

    return OUT;
}

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

export float4 im3d_PS(FragmentData IN) : SV_TARGET
{
    float4 colorAcc = IN.color;

    if (object.hasTexture)
    {
        if (object.hasDistanceField)
        {
            float2 msdfUnit = 4.0 / float2(512.0, 512.0);
            float4 sample = tex.Sample(texSampler, IN.uv);
            float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
			//sigDist *= dot(msdfUnit, float2(0.5));
            sigDist *= 4.0;
            float opacity = saturate(sigDist + 0.5);
            colorAcc = lerp(float4(0, 0, 0, 0), colorAcc, opacity);
        }
        else
        {
            colorAcc = colorAcc * tex.Sample(texSampler, IN.uv);
        }
    }
	
    return colorAcc;
}