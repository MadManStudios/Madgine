
#include "lightdata.sl"

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
    float3 pos,
    float3 normal,
    float ambientFactor,
    float diffuseFactor,
    float specularFactor,
    float shininess)
{ 
    float3 ambient = ambientFactor * light.color;
    
    float diff = max(dot(normal, -light.dir), 0.0);
    float3 diffuse = diffuseFactor * diff * light.color;

    diffuseIntensity += ambient + diffuse;

    float3 viewDir = normalize(/** -pos /*/ float3(0.0, 0.0, -1.0) /**/);
    float3 reflectDir = reflect(-light.dir, normal);
    float spec = pow(max(dot(viewDir, -reflectDir), 0.0), shininess);
    float3 specular = specularFactor * spec * light.color;

    specularIntensity += specular;
}

void castDirectionalShadowLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    DirectionalShadowLight light,
    float4 lightViewPosition,
    float3 pos,
    float3 normal,
    Texture2DMS<float> shadowMap,
    float ambientFactor,
    float diffuseFactor,
    float specularFactor,
    float shininess)
{
    float bias = /* max(0.05 * (1.0 - dot(normal, light.light.dir)), */0.0005/*)*/;  
    float3 normalizedLightViewPosition = lightViewPosition.xyz / lightViewPosition.w;    
    normalizedLightViewPosition.y *= -1;
    int2 lightTexCoord = int2(2048 * (normalizedLightViewPosition.xy * 0.5 + 0.5));
    
    float lightDepth = normalizedLightViewPosition.z - bias;

    float lightStrength = 1.0;

    for (int i = 0; i < light.caster.shadowSamples; ++i) {
        float shadowDepth = shadowMap.Load(lightTexCoord, i).r;
        lightStrength -= float(lightDepth > shadowDepth) / light.caster.shadowSamples;
    }

    castDirectionalLight(
        diffuseIntensity,
        specularIntensity, 
        light.light, 
        pos,
        normal, 
        ambientFactor, 
        diffuseFactor * lightStrength,
        specularFactor * lightStrength,
        shininess
    );

    //diffuseIntensity = float3(lightDepth, shadowMap.Load(lightTexCoord, 0).r, lightTexCoord.y);
    //diffuseIntensity = normalizedLightViewPosition;
}

void castPointLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    PointLight light,
    float3 pos,
    float3 normal,
    float ambientFactor,
    float diffuseFactor,
    float specularFactor,
    float shininess)
{
    float3 ambient = ambientFactor * light.color;

    float3 lightDir = normalize(light.position - pos);

    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diffuseFactor * diff * light.color;

    float distance = length(light.position - pos);
    float attenuation = 1.0 / (light.constantFactor + light.linearFactor * distance + light.squaredFactor * (distance * distance));

    diffuseIntensity += attenuation * (ambient + diffuse);
}

void castPointShadowLight(
    inout float3 diffuseIntensity,
    inout float3 specularIntensity,
    PointShadowLight light,
    float3 pos,
    float3 normal,
    TextureCube<float> shadowMap,
    SamplerState samplerState,
    float ambientFactor,
    float diffuseFactor,
    float specularFactor,
    float shininess)
{
    float bias = 0.001;
    float3 lightDir = projectShadow(light.caster, float4(pos - light.light.position, 0.0)).xyz;

    float lightDepth = (length(lightDir) - 0.01) / 99.99 - bias;

    float lightStrength = 1.0;
    
    float shadowDepth = shadowMap.Sample(samplerState, lightDir);
    lightStrength -= float(lightDepth > shadowDepth);
    
    castPointLight(
        diffuseIntensity,
        specularIntensity, 
        light.light, 
        pos, 
        normal, 
        ambientFactor, 
        diffuseFactor * lightStrength * 10.0,
        specularFactor * lightStrength * 10.0,
        shininess
    );
}
