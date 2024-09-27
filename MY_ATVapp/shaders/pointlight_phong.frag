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

uniform bool dayTime;
uniform bool pointLightsOn;
uniform bool spotLightsOn;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataIn;

vec4 calculatePointLights(PointLight light, vec3 normal, vec3 viewDir) {
	vec3 lightDir = normalize(vec3(light.position) - DataIn.eye);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    
    vec4 diffuse = diff * mat.diffuse;
    vec4 specular = spec * mat.specular;
    
    return (diffuse + specular);
}

void main() {

	vec4 spec = vec4(0.0);
	vec4 color = mat.ambient;

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(vec3(dirLight.direction));
	vec3 e = normalize(DataIn.eye);

	if (dayTime){
		float intensity = max(dot(n,l), 0.0);

	
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
	
		color += max(intensity * mat.diffuse + spec, mat.ambient);
	}

	//point lights
	if (pointLightsOn){
		for (int i = 0; i < 6; i++) {
		color+= calculatePointLights(pointLights[i], n, e);
		}
	}
	

	colorOut = color;
}