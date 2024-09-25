#version 430

uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

struct PointLight {
	vec4 position;
};

struct DirectionalLight {
	vec4 direction;
};

struct SpotLight {
	vec4 position;
	float angle;
	vec3 direction;
};

uniform Materials mat;
uniform PointLight pointLights[6];
uniform SpotLight spotLights[2];
uniform DirectionalLight dirLight;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataIn;

void main() {

	vec4 spec = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(vec3(dirLight.direction));
	vec3 e = normalize(DataIn.eye);

	float intensity = max(dot(n,l), 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}
	
	colorOut = max(intensity * mat.diffuse + spec, mat.ambient);
}