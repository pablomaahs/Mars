#version 460

struct VertexData
{
	float x, y, z;
	float r, g, b, a;
};

layout(binding = 0) uniform UniformBuffer
{
	mat4 mp;
	float time;
} ubo;

layout(binding = 1) readonly buffer LinesBuffer
{
	VertexData data[];
} lines;

layout(location = 0) out vec4 outColor;

void main()
{
	VertexData v = lines.data[gl_VertexIndex];

	gl_Position = ubo.mp * vec4(v.x, v.y, v.z, 1.0);
	outColor = vec4(v.r, v.g, v.b, v.a);
}