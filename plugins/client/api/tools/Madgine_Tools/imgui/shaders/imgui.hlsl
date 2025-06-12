cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};
struct AppData
{
    float2 pos : POSITION2;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
};
            
struct FragmentData
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
};
            
export FragmentData imgui_VS(AppData input)
{
    FragmentData output;
    output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv = input.uv;
    return output;
}

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0, space2);
            
export float4 imgui_PS(FragmentData input) : SV_TARGET
{
    float4 out_col = float4(pow(input.col.xyz, 2.2), input.col.w) * texture0.Sample(sampler0, input.uv);
    return out_col;
}