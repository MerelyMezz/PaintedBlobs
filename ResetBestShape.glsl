R"(
#version 460

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
precision highp float;

layout(std430, binding = 2) buffer ShapeDataT
{
	vec2 Pos;
	vec2 Size;
	float Angle;
	mat3 Matrix;
	vec3 Color;
	int Score;
} BestShapeData;

void main()
{
	BestShapeData.Pos = vec2(0, 0);
	BestShapeData.Size = vec2(0, 0);
	BestShapeData.Angle = 0;
	BestShapeData.Matrix = mat3(0, 0, 0, 0, 0, 0, 0, 0, 0);
	BestShapeData.Color = vec3(0, 0, 0);
	BestShapeData.Score = 0;
}
)"