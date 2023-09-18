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
} ShapeData;

layout(rgba32f, binding = 3) uniform image2D ShapeCanvas;

void main()
{
	if (ShapeData.Score > 0)
	{
		ivec2 Pos = ivec2(gl_WorkGroupID.xy);

		vec3 USPos = vec3(vec2(Pos), 1.0) * ShapeData.Matrix;

		if (USPos.x*USPos.x+USPos.y*USPos.y < 1.0)
		{
			imageStore(ShapeCanvas, Pos, vec4(ShapeData.Color, 1.0));
		}
	}
}
)"