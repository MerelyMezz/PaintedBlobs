R"(
#version 460

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
precision highp float;

layout(location = 0) uniform mat3 Matrix;
layout(location = 1) uniform vec3 Color;

layout(rgba32f, binding = 0) uniform image2D ShapeCanvas;

void main()
{
	ivec2 Pos = ivec2(gl_WorkGroupID.xy);
	vec3 USPos = vec3(vec2(Pos), 1.0) * Matrix;

	if (USPos.x*USPos.x+USPos.y*USPos.y < 1.0)
	{
		imageStore(ShapeCanvas, Pos, vec4(Color, 1.0));
	}
}
)"