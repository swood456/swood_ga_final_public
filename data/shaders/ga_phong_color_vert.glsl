#version 430
 
layout(location = 0)in vec3 VertexPosition;
layout(location = 2)in vec3 VertexNormal;

out Data
{
	vec3 Position;
	vec3 Normal;
} data;

//uniform mat4 ModelViewMatrix;
//uniform mat3 NormalMatrix;
uniform mat4 u_mvp;

void main()
{
	//data.Normal = normalize( NormalMatrix * VertexNormal );
	//data.Position = vec3( ModelViewMatrix * vec4( VertexPosition, 1 ) );

	data.Normal = normalize(VertexNormal);
	data.Position = VertexPosition;
 
	//gl_Position = u_mvp * vec4( VertexPosition, 1 );
	gl_Position = vec4(VertexPosition, 1.0) * u_mvp;
}