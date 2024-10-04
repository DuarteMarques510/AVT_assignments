#version 430

uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

out vec4 colorOut;

uniform int texMode;

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
	vec4 direction;
};

uniform Materials mat;
uniform PointLight pointLights[6];
uniform SpotLight spotLights[2];
uniform DirectionalLight dirLight;

uniform bool dayTime;
uniform bool pointLightsOn;
uniform bool spotLightsOn;
uniform bool fogOn;

uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);  // Color of the fog
uniform float fogDensity = 0.03;               // Fog density to control thickness
uniform int depthFog; 

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
} DataIn;

in vec4 pos;

void main() {

	vec4 spec = vec4(0.0);
	vec4 color = mat.ambient;

	vec4 texel;
	vec4 texel1;

	vec3 n = normalize(DataIn.normal);
	vec3 l = normalize(vec3(dirLight.direction));
	vec3 e = normalize(DataIn.eye);
	float intensity = max(dot(n,l), 0.0);

	if (dayTime){

	
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
	
		color += intensity * mat.diffuse + spec;
	}

	//point lights
	if (pointLightsOn){
		for (int i = 0; i < 6; i++) {
			l = normalize(vec3(pointLights[i].position) + DataIn.eye);
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
			}
			color += vec4(0.2, 0.2, 0.2, 1.0)*intensity * mat.diffuse + spec;
		}
	}

	if (spotLightsOn){
		for (int i=0; i<2; i++){
			l = normalize(vec3(spotLights[i].position) + DataIn.eye);
			float theta = dot(l, normalize(vec3(spotLights[i].direction)));
			if (theta > cos(radians(spotLights[i].angle))){
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = mat.specular * pow(intSpec, mat.shininess);
				}
				color += intensity * mat.diffuse + spec;
			}
		}
	}
	
	if (fogOn) {
		float dist;
		if (depthFog == 0) {
			// Plane-based fog (using depth along the z-axis)
			dist = abs(pos.z);
		} else {
			// Range-based fog (using Euclidean distance from the camera)
			dist = length(pos);
		}
		float fogAmount = exp(-dist * fogDensity); // Exponential fog based on distance
		fogAmount = clamp(fogAmount, 0.0, 1.0);    // Ensure it's between 0 and 1
		vec3 finalColor = mix(fogColor, vec3(color), fogAmount);
		finalColor = max(finalColor, vec3(mat.ambient));
		colorOut = vec4(finalColor, mat.diffuse.a); // Output the final color with alpha
	}
	else {
		colorOut = vec4(max(color, mat.ambient).rgb, mat.diffuse.a);
	}

	if (texMode==1){
		texel = texture(texmap2, DataIn.tex_coord);
		texel1 = texture(texmap1, DataIn.tex_coord);
		colorOut = max(colorOut*texel1*texel, texel*texel1*0.07);
	}
}
