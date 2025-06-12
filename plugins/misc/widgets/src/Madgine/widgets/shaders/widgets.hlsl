#include "widgets.sl"

cbuffer PerApplication : register(b0)
{
    WidgetsPerApplication app;
}

cbuffer PerObject : register(b2)
{
    WidgetsPerObject object;
}


struct AppData
{
    float3 aPos : POSITION0;
    float aW : POSITION1; //Remove once dxc allows mapping semantic names to locations for SPIR-V
    float2 aPos2 : POSITION2; //-
    float3 aNormal : NORMAL; //-
    float4 aColor : COLOR;
    float2 aUV : TEXCOORD;
};

struct FragmentData
{
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float4 position : SV_Position;
};

export FragmentData widgets_VS(AppData IN)
{
    FragmentData OUT;



    OUT.position.xy = ((IN.aPos.xy / app.screenSize) * float2(2, -2)) - float2(1, -1);
    OUT.position.z = 1.0 / (IN.aPos.z + 1.0);
    OUT.position.w = 1.0;
    OUT.position = mul(app.c, OUT.position);
    OUT.color = IN.aColor;
    OUT.uv = IN.aUV;

    return OUT;
}



Texture2D tex : register(t0, space2);
SamplerState texSampler : register(s0);


float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

export float4 widgets_PS(FragmentData IN)
{
    float4 texColor;
    if (object.hasDistanceField)
    {
        float3 sample = tex.Sample(texSampler, IN.uv).rgb;
        float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
        float opacity = saturate(sigDist / fwidth(sigDist) + 0.5);
		
        if (opacity == 0.0)
        {
            discard;
        }
        texColor = opacity;
    }
    else if (object.hasTexture)
    {
        texColor = tex.Sample(texSampler, IN.uv);
    }
    else
    {
        texColor = 1;
    }
    return IN.color * texColor;
}
