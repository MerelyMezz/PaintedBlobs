R"(
#version 460

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
precision highp float;

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

void main()
{
	ShapeData.Color = vec3(atomicCounter(SumR), atomicCounter(SumG), atomicCounter(SumB)) / (float(atomicCounter(CoverGood)) * 255.0);
}
)"