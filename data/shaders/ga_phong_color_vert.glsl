#version 430
 
layout(location = 0)in vec3 VertexPosition;
layout(location = 2)in vec3 VertexNormal;

out Data
{
	vec3 Position;
	vec3 Normal;
} data;

uniform mat4 u_mvp;

void main()
{
	data.Normal = normalize(VertexNormal);
	data.Position = VertexPosition;
 
	gl_Position = vec4(VertexPosition, 1.0) * u_mvp;
}