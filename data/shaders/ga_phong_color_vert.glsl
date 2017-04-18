#version 430
 
in vec3 VertexPosition;
in vec3 VertexNormal;

out Data
{
	vec3 Position;
	vec3 Normal;
} data;
 
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
 
void main()
{
	data.Normal = normalize( NormalMatrix * VertexNormal );
	data.Position = vec3( ModelViewMatrix * vec4( VertexPosition, 1 ) );
 
	gl_Position = MVP * vec4( VertexPosition, 1 );
}