#ifndef _LIGHDATA_SL_
#define _LIGHDATA_SL_

struct DirectionalLight {
	float3 color;
	float _padding0;
	float3 dir;
	float _padding1;
};

struct PointLight {
	float3 position;
	float _padding0;
	float3 color;
	float _padding1;

	float constantFactor;
	float linearFactor;
	float squaredFactor;
	float _padding2;
};

struct ShadowCaster {
	float4x4 reprojectionMatrix;

	int shadowSamples;
	float _padding0;
	float _padding1;
	float _padding2;
};

struct DirectionalShadowLight {
	DirectionalLight light;

	ShadowCaster caster;
};

struct PointShadowLight {
	PointLight light;

	ShadowCaster caster;
};

#endif