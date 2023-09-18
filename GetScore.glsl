R"(
#version 460

#define COLOR_MAX_DISTANCE 20

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
precision highp float;

layout(std430, binding = 0) buffer ShapeDataT	// 14 floats total
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
layout(rgba32f, binding = 3) readonly uniform image2D ShapeCanvas;

int PixelDifference(vec4 A, vec4 B)
{
	if (A.a == 0)
	{
		return B.a == 0 ? 0 : 442; //sqrt(3*255*255);
	}

	vec4 D = A - B;
	D = D * D;
	return int(sqrt(D.r+D.g+D.b)*255.0);
}

void main()
{
	ivec2 Pos = ivec2(gl_WorkGroupID.xy);

	vec3 USPos = vec3(vec2(Pos), 1.0) * ShapeData.Matrix;

	if (USPos.x*USPos.x+USPos.y*USPos.y < 1.0)
	{
		vec4 SourcePixel = imageLoad(SourceImage, Pos);

		if (SourcePixel.a > 0.0)
		{
			vec4 ShapeCanvasPixel = imageLoad(ShapeCanvas, Pos);

			int PScore = PixelDifference(SourcePixel, ShapeCanvasPixel) - PixelDifference(SourcePixel, vec4(ShapeData.Color, 1.0));
			atomicCounterAdd(Score, PScore);
		}
	}
}
)"