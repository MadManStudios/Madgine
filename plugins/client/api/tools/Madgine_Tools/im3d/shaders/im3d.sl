
struct Im3DPerApplication {
	float4x4 p;
};

struct Im3DPerObject {
	float4x4 mv;
	
	bool hasTexture;
	bool hasDistanceField;
};