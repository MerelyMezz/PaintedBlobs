R"(
#version 460

#define PI 3.1415926535897932384626433832795

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
precision highp float;

layout(location = 0) uniform float RandomNumbers[5];

layout(std430, binding = 0) buffer ShapeDataT
{
	vec2 Pos;
	vec2 Size;
	float Angle;
	mat3 Matrix;
	vec3 Color;
	int Score;
} ShapeData;

layout (binding = 1, offset = 0) uniform atomic_uint SumR;
layout (binding = 1, offset = 4) uniform atomic_uint SumG;
layout (binding = 1, offset = 8) uniform atomic_uint SumB;
layout (binding = 1, offset = 12) uniform atomic_uint CoverGood;
layout (binding = 1, offset = 16) uniform atomic_uint CoverBad;
layout (binding = 1, offset = 20) uniform atomic_uint Score;

layout(rgba32f, binding = 2) readonly uniform image2D SourceImage;

float RandomFloat(int Seed, float Min, float Max)
{
	float R = RandomNumbers[Seed];
	return Min + (Max - Min) * R;
}

void main()
{
	ShapeData.Pos = vec2(RandomFloat(0, 0.0, 1.0), RandomFloat(1, 0.0, 1.0));
	ShapeData.Size = vec2(RandomFloat(2, 0.001, 0.3), RandomFloat(3, 0.001, 0.3));
	ShapeData.Angle = RandomFloat(4, 0.0, 2*PI);

	// Construct Shape matrix
	mat3 ScaleMatrix = mat3(	ShapeData.Size[0], 	0.0f, 				0.0f,
								0.0f,		 		ShapeData.Size[1], 	0.0f,
								0.0f, 				0.0f, 				1.0f);

	vec2 ScreenSize = vec2(imageSize(SourceImage));
	mat3 ScreenScaleMatrix = mat3(	ScreenSize.x, 		0.0f, 				0.0f,
									0.0f,		 		ScreenSize.y, 		0.0f,
									0.0f, 				0.0f, 				1.0f);

	float s = sin(ShapeData.Angle);
	float c = cos(ShapeData.Angle);
	mat3 RotMatrix = mat3(	c, 		-s,		0.0f,
							s, 		c, 		0.0f,
							0.0f, 	0.0f,	1.0f);

	mat3 TranslateMatrix = mat3(	1.0f, 	0.0f,	ShapeData.Pos[0],
									0.0f, 	1.0f,	ShapeData.Pos[1],
									0.0f, 	0.0f,	1.0f);

	mat3 UnitSquareToShapeTransform = ((ScaleMatrix * RotMatrix) * TranslateMatrix) * ScreenScaleMatrix;
	mat3 ShapeToUnitSquareTransform = inverse(UnitSquareToShapeTransform);

	ShapeData.Matrix = ShapeToUnitSquareTransform;

	//Reset atomic counters
	atomicCounterExchange(SumR, 0);
	atomicCounterExchange(SumG, 0);
	atomicCounterExchange(SumB, 0);
	atomicCounterExchange(CoverGood, 0);
	atomicCounterExchange(CoverBad, 0);
	atomicCounterExchange(Score, 0);
}
)"