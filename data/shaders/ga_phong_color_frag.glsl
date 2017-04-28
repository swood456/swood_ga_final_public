#version 430
 
struct LightInfo
{
	vec3 Position;	//Light Position in eye-coords
	vec3 La;		//Ambient light intensity
	vec3 Ld;		//Diffuse light intensity
	vec3 Ls;		//Specular light intensity
};
 
struct MaterialInfo
{
	vec3 Ka;			//Ambient reflectivity
	vec3 Kd;			//Diffuse reflectivity
	vec3 Ks;			//Specular reflectivity
	float Shininess;	//Specular shininess factor
};

in Data
{
	vec3 Position;
	vec3 Normal;
} data;

out vec4 FragColor;

uniform vec3 LightPos;
uniform vec3 LightAmbient;
uniform vec3 LightDiffuse;
uniform vec3 LightSpecular;

uniform vec3 MaterialAmbient;
uniform vec3 MaterialDiffuse;
uniform vec3 MaterialSpecular;

uniform vec3 BackMaterialAmbient;
uniform vec3 BackMaterialDiffuse;
uniform vec3 BackMaterialSpecular;

 
void light(vec3 position, vec3 norm, out vec3 ambient, out vec3 diffuse, out vec3 spec )
{
	vec3 n = normalize( norm );
	vec3 s = normalize( LightPos - position );
	vec3 v = normalize( -position );
	vec3 r = reflect( -s, n );
 
	ambient = LightAmbient * MaterialAmbient;
 
	float sDotN = max( dot( s, n ), 0.0 );
	diffuse = LightDiffuse * MaterialDiffuse * sDotN;
 
	spec = LightSpecular * MaterialSpecular * pow( max( dot(r,v) , 0.0 ), 0.1 ); 
}

void back_light(vec3 position, vec3 norm, out vec3 ambient, out vec3 diffuse, out vec3 spec )
{
	vec3 n = normalize( norm );
	vec3 s = normalize( LightPos - position );
	vec3 v = normalize( -position );
	vec3 r = reflect( -s, n );
 
	ambient = LightAmbient * BackMaterialAmbient;
 
	float sDotN = max( dot( s, n ), 0.0 );
	diffuse = LightDiffuse * BackMaterialDiffuse * sDotN;
 
	spec = LightSpecular * BackMaterialSpecular * pow( max( dot(r,v) , 0.0 ), 0.1 ); 
}

void main()
{
 
	vec3 ambientSum = vec3(0);
	vec3 diffuseSum = vec3(0);
	vec3 specSum = vec3(0);
	vec3 ambient, diffuse, spec;

	if ( gl_FrontFacing )
	{
		light(data.Position, data.Normal, ambient, diffuse, spec );
		ambientSum = ambient;
		diffuseSum = diffuse;
		specSum = spec;
	} else
	{
		back_light(data.Position, data.Normal, ambient, diffuse, spec );
		ambientSum = ambient;
		diffuseSum = diffuse;
		specSum = spec;
	}

	FragColor = vec4( ambientSum + diffuseSum, 1 ) + vec4( specSum, 1 );
}