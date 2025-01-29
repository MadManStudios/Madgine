#include "lightdata.sl"
#include "Madgine/render/shadinglanguage/memory.sl"

struct ScenePerApplication {
	float4x4 p;

	bool hasHDR;

	float ambientFactor;
	float diffuseFactor;
	float specularFactor;
};

struct ScenePerFrame {
	int pointLightCount;
	float _padding0;
	float _padding1;
	float _padding2;

	DirectionalShadowLight light;

	PointShadowLight pointLights[2];
};

struct SceneInstanceData{
	row_major float4x4 mv;
	row_major float4x4 anti_mv;	
	float4 diffuseColor;
	float4 specularColor;
	ArrayPtr<float4x4> bones;
	float _padding0;
	float _padding1;	
};

struct ScenePerObject {

	float shininess;

	bool hasLight;
	bool hasTexture;
	bool hasDistanceField;

};