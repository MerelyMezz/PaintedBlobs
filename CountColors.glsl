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

layout(rgba32f, binding = 2) readonly uniform image2D SourceImage;

void main()
{
	ivec2 Pos = ivec2(gl_WorkGroupID.xy);
	vec3 USPos = vec3(vec2(Pos), 1.0) * ShapeData.Matrix;

	if (USPos.x*USPos.x+USPos.y*USPos.y < 1.0)
	{
		ivec4 SourcePixel = ivec4(imageLoad(SourceImage, Pos) * 255) ;

		if (SourcePixel.a == 0.0)
		{
			atomicCounterIncrement(CoverBad);
		}
		else
		{
			atomicCounterIncrement(CoverGood);
			atomicCounterAdd(SumR, SourcePixel.r);
			atomicCounterAdd(SumG, SourcePixel.g);
			atomicCounterAdd(SumB, SourcePixel.b);
		}
	}
}
)"