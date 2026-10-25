
struct DirectionalLight
{
    float3 color;
    float3 dir;
    bool orthographic;
};

struct PointLight
{
    float3 position;
    float3 color;

    float constantFactor;
    float linearFactor;
    float squaredFactor;
};

struct ShadowCaster
{
    float4x4 reprojectionMatrix;

    int shadowSamples;
};

struct DirectionalShadowLight
{
    DirectionalLight light;

    ShadowCaster caster;
};

struct PointShadowLight
{
    PointLight light;

    ShadowCaster caster;
};

struct LightingInput
{
    float4 albedo;
    float4 viewPos;
    float3 normal;
    float shininess;
    float3 emissiveColor;
};

struct HDRShaderOutput
{
    float4 fragColor : SV_Target0;
    float4 brightColor : SV_Target1;
};


struct LightPerFrame
{
    float3 ambient;
    
    int pointLightCount;

    DirectionalShadowLight light;

    PointShadowLight pointLights[2];
};

// Shaders that only need the light structures and functions (e.g. a deferred lighting pass) define
// LIGHT_NO_RESOURCES before including this file
#ifndef LIGHT_NO_RESOURCES
ConstantBuffer<LightPerFrame> frame : register(b1);

SamplerState texSampler : register(s0);
SamplerState clampSampler : register(s1);

Texture2DMS<float> shadowDepthMap : register(t0, space3);
TextureCube<float> pointShadowDepthMaps0 : register(t1, space3);
TextureCube<float> pointShadowDepthMaps1 : register(t2, space3);
#endif

float4 projectShadow(
    ShadowCaster caster,
    float4 pos)
{
    return mul(caster.reprojectionMatrix, pos);
}

void castDirectionalLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    DirectionalLight light,
    float4 viewPos,
    float3 normal,
    float factor,
    float shininess)
{    
    float diff = max(dot(normal, -light.dir), 0.0);
    float3 diffuse = factor * diff * light.color;

    diffuseIntensity += diffuse;

    float3 viewDir = normalize(light.orthographic ? float3(0.0, 0.0, -1.0) : -viewPos.xyz);
    float3 reflectDir = reflect(-light.dir, normal);
    float spec = pow(max(dot(viewDir, -reflectDir), 0.0), shininess);
    float3 specular = factor * spec * light.color;

    specularIntensity += specular;
}

static const float shadowMapSize = 2048.0;

// Fraction of the depth samples of one shadow map texel that do not occlude the given depth
float shadowTexel(Texture2DMS<float> shadowMap, int2 coord, float depth, int samples)
{
    if (any(coord < 0) || any(coord >= int(shadowMapSize)))
        return 1.0;
    float lit = 0.0;
    for (int i = 0; i < samples; ++i)
        lit += float(depth <= shadowMap.Load(coord, i).r);
    return lit / samples;
}

void castDirectionalShadowLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    DirectionalShadowLight light,
    float4 viewPos,
    float3 normal,
    Texture2DMS<float> shadowMap,
    float shininess)
{
    float lightStrength = 0.0;

    float NdotL = dot(normal, -light.light.dir);
    if (NdotL > 0.0)
    {
        // Size of a shadow map texel and the light space depth per unit, both taken from the reprojection
        float3 side = normalize(cross(light.light.dir, abs(light.light.dir.y) < 0.9 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0)));
        float texelSize = 2.0 / (length(projectShadow(light.caster, float4(side, 0.0)).xy) * shadowMapSize);
        float depthPerUnit = abs(projectShadow(light.caster, float4(light.light.dir, 0.0)).z);

        // Normal offset and slope scaled bias against shadow acne
        float tanTheta = min(sqrt(max(1.0 - NdotL * NdotL, 0.0)) / NdotL, 4.0);
        float4 offsetPos = float4(viewPos.xyz / viewPos.w + normal * texelSize * (0.6 + 0.8 * (1.0 - NdotL)), 1.0);
        float bias = depthPerUnit * texelSize * (0.5 + 1.2 * tanTheta);

        float4 lightViewPosition = projectShadow(light.caster, offsetPos);
        float3 normalizedLightViewPosition = lightViewPosition.xyz / lightViewPosition.w;
        normalizedLightViewPosition.y *= -1;

        float lightDepth = normalizedLightViewPosition.z - bias;

        if (any(abs(normalizedLightViewPosition.xy) > 1.0) || lightDepth > 1.0)
        {
            lightStrength = 1.0;
        }
        else
        {
            // 2x2 texel box filter evaluated bilinearly (3x3 texels): soft edges that move smoothly instead of
            // jumping from texel to texel
            float2 texelPos = shadowMapSize * (normalizedLightViewPosition.xy * 0.5 + 0.5);
            int2 base = int2(floor(texelPos));
            float2 f = texelPos - floor(texelPos);
            float weightX[3] = { 1.0 - f.x, 1.0, f.x };
            float weightY[3] = { 1.0 - f.y, 1.0, f.y };
            for (int y = 0; y < 3; ++y)
            {
                for (int x = 0; x < 3; ++x)
                {
                    lightStrength += weightX[x] * weightY[y] * shadowTexel(shadowMap, base + int2(x - 1, y - 1), lightDepth, light.caster.shadowSamples);
                }
            }
            lightStrength /= 4.0;
        }
    }

    castDirectionalLight(
        diffuseIntensity,
        specularIntensity,
        light.light,
        viewPos,
        normal,
        lightStrength,
        shininess
    );
}

void castPointLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    PointLight light,
    float3 viewPos,
    float3 normal,
    float factor,
    float shininess)
{
    float3 lightDir = normalize(light.position - viewPos);

    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = factor * diff * light.color;

    float distance = length(light.position - viewPos);
    float attenuation = 1.0 / (light.constantFactor + light.linearFactor * distance + light.squaredFactor * (distance * distance));

    diffuseIntensity += attenuation * diffuse;
}

void castPointShadowLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    PointShadowLight light,
    float3 viewPos,
    float3 normal,
    TextureCube<float> shadowMap,
    SamplerState samplerState,
    float shininess)
{
    float bias = 0.001;
    float3 lightDir = projectShadow(light.caster, float4(viewPos - light.light.position, 0.0)).xyz;

    float lightDepth = (length(lightDir) - 0.01) / 99.99 - bias;

    float lightStrength = 1.0;
    
    float shadowDepth = shadowMap.Sample(samplerState, lightDir);
    lightStrength -= float(lightDepth > shadowDepth);
    
    castPointLight(
        diffuseIntensity,
        specularIntensity,
        light.light,
        viewPos,
        normal,
        lightStrength,
        shininess
    );
}

#ifndef LIGHT_NO_RESOURCES
export HDRShaderOutput lighting(LightingInput IN)
{
    
    HDRShaderOutput OUT;
    
    float3 specularColor = float3(1.0, 1.0, 1.0);
	

    float3 lightDiffuseIntensity = frame.ambient;
    float3 lightSpecularIntensity = float3(0.0, 0.0, 0.0);
		
    castDirectionalShadowLight(
		lightDiffuseIntensity,
		lightSpecularIntensity,
		frame.light,
		IN.viewPos,
		IN.normal,
		shadowDepthMap,
		IN.shininess
	);
    for (int i = 0; i < frame.pointLightCount; ++i)
    {
        if (i == 0)
            castPointShadowLight(
				lightDiffuseIntensity,
				lightSpecularIntensity,
				frame.pointLights[i],
				IN.viewPos.xyz / IN.viewPos.w,
				IN.normal,
				pointShadowDepthMaps0,
				texSampler,
				IN.shininess
			);
        else
            castPointShadowLight(
				lightDiffuseIntensity,
				lightSpecularIntensity,
				frame.pointLights[i],
				IN.viewPos.xyz / IN.viewPos.w,
				IN.normal,
				pointShadowDepthMaps1,
				texSampler,
				IN.shininess
			);
    }

    OUT.fragColor =
		float4(
			lightDiffuseIntensity * IN.albedo.xyz +
			lightSpecularIntensity * specularColor.xyz,
			IN.albedo.a
		);
    

    OUT.brightColor = float4(IN.emissiveColor, 1.0);
    float brightness = dot(OUT.fragColor.rgb, float3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        OUT.brightColor.rgb += OUT.fragColor.rgb;

    OUT.fragColor.rgb += IN.emissiveColor;
	
    return OUT;
}
#endif
