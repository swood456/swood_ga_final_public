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
 
uniform LightInfo Light;
uniform MaterialInfo Material;
//uniform sampler2D Tex;
 
void light(vec3 position, vec3 norm, out vec3 ambient, out vec3 diffuse, out vec3 spec )
{
	vec3 n = normalize( norm );
	vec3 s = normalize( Light[lightIndex].Position - position );
	vec3 v = normalize( -position );
	vec3 r = reflect( -s, n );
 
	ambient = Light.La * Material.Ka;
 
	float sDotN = max( dot( s, n ), 0.0 );
	diffuse = Light.Ld * Material.Kd * sDotN;
 
 
	spec = Light.Ls * Material.Ks * pow( max( dot(r,v) , 0.0 ), Material.Shininess ); 
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
		ambientSum += ambient;
		diffuseSum += diffuse;
		specSum += spec;
	}
	else
	{
		light(data.Position, -data.Normal, ambient, diffuse, spec );
		ambientSum += ambient;
		diffuseSum += diffuse;
		specSum += spec;
	}
	//ambientSum /= LIGHTCOUNT;
 
	//vec4 texColor = texture(Tex, data.TexCoord);
 
	FragColor = vec4( ambientSum + diffuseSum, 1 ) + vec4( specSum, 1 );
}