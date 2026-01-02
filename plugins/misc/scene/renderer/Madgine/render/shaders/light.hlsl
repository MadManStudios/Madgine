
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


cbuffer LightPerFrame : register(b1)
{
    int pointLightCount;

    DirectionalShadowLight light;

    PointShadowLight pointLights[2];
}

SamplerState texSampler : register(s0);
SamplerState clampSampler : register(s1);

Texture2DMS<float> shadowDepthMap : register(t0, space3);
TextureCube<float> pointShadowDepthMaps0 : register(t1, space3);
TextureCube<float> pointShadowDepthMaps1 : register(t2, space3);

static const float ambientFactor = 0.5;

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

void castDirectionalShadowLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    DirectionalShadowLight light,
    float4 viewPos,
    float3 normal,
    Texture2DMS<float> shadowMap,
    float shininess)
{
    float bias = /* max(0.05 * (1.0 - dot(normal, light.light.dir)), */0.0005 /*)*/;
    
    
    float4 lightViewPosition = projectShadow(light.caster, viewPos); 
    
    float3 normalizedLightViewPosition = lightViewPosition.xyz / lightViewPosition.w;
    normalizedLightViewPosition.y *= -1;
    int2 lightTexCoord = int2(2048 * (normalizedLightViewPosition.xy * 0.5 + 0.5));
    
    float lightDepth = normalizedLightViewPosition.z - bias;

    float lightStrength = 1.0;

    for (int i = 0; i < light.caster.shadowSamples; ++i)
    {
        float shadowDepth = shadowMap.Load(lightTexCoord, i).r;
        lightStrength -= float(lightDepth > shadowDepth) / light.caster.shadowSamples;
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

    //diffuseIntensity = float3(lightDepth, shadowMap.Load(lightTexCoord, 0).r, lightTexCoord.y);
    //diffuseIntensity = normalizedLightViewPosition;
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

export HDRShaderOutput lighting(LightingInput IN)
{
    
    HDRShaderOutput OUT;
    
    float3 specularColor = float3(1.0, 1.0, 1.0);
	

    float3 lightDiffuseIntensity = ambientFactor.rrr;
    float3 lightSpecularIntensity = float3(0.0, 0.0, 0.0);
		
    castDirectionalShadowLight(
		lightDiffuseIntensity,
		lightSpecularIntensity,
		light,
		IN.viewPos,
		IN.normal,
		shadowDepthMap,
		IN.shininess
	);
    for (int i = 0; i < pointLightCount; ++i)
    {
        if (i == 0)
            castPointShadowLight(
				lightDiffuseIntensity,
				lightSpecularIntensity,
				pointLights[i],
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
				pointLights[i],
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
