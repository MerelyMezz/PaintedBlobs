R"(
#version 460

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
precision highp float;

layout(rgba32f, binding = 0) uniform image2D Canvas;
layout(location = 0) uniform vec4 FocusRectangle;

void main()
{
	ivec2 PPos = ivec2(gl_WorkGroupID.xy);
	vec2 Pos = vec2(PPos) / vec2(imageSize(Canvas));

	if (!(Pos.x > FocusRectangle.r &&
		Pos.x < FocusRectangle.g &&
		Pos.y > FocusRectangle.b &&
		Pos.y < FocusRectangle.a))
	{
		vec4 SourcePixel = imageLoad(Canvas, PPos);
		vec4 ExclusionColor = vec4(1.0, 0.5, 0.5, 0.25);
		vec4 ExcludedPixel = SourcePixel * (1-ExclusionColor.a) + ExclusionColor * ExclusionColor.a;

		imageStore(Canvas, PPos, ExcludedPixel);
	}
}
)"