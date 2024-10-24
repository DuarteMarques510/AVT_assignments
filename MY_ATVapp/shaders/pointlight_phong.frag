#version 430

uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;
uniform sampler2D texmap4;
uniform sampler2D texmap5;
uniform sampler2D texmap6;
uniform samplerCube cubeMap;

out vec4 colorOut;

uniform sampler2D texUnitDiff;
uniform sampler2D texUnitDiff1; //supports two diffuse textures
uniform sampler2D texUnitSpec;
uniform sampler2D texUnitNormalMap;

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
uniform bool normalMap;
uniform bool specularMap;
uniform uint diffMapCount;
uniform bool cubeMapping;
uniform bool bumpMapping;

uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);  // Color of the fog
uniform float fogDensity = 0.03;               // Fog density to control thickness
uniform int depthFog; 
uniform mat4 m_View;
uniform int reflect_perFrag;

vec4 diff, auxSpec;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
	vec3 skyboxTexCoord;
	vec3 reflected;
} DataIn;

const float reflect_factor = 1.0;

in vec4 pos;

void main() {

	vec4 spec = vec4(0.0);
	vec4 color = mat.ambient;
	vec3 n;

	vec4 texel;
	vec4 texel1;
	vec4 cube_texel;

	if (bumpMapping) {
		n= normalize(2.0*texture(texmap6, DataIn.tex_coord).rgb - 1.0);
	}
	else{
		n= normalize(DataIn.normal);
	}
	n=normalize(DataIn.normal);

	vec3 l = normalize(vec3(dirLight.direction));
	vec3 e = normalize(DataIn.eye);
	float intensity = 0; //max(dot(n,l), 0.0);

	if(mat.texCount==0){
		diff =mat.diffuse;
		auxSpec= mat.specular;
	}
	else{
		if(diffMapCount==0){
			diff=mat.diffuse;
		}
		else if(diffMapCount==1){
			diff=mat.diffuse*texture(texUnitDiff, DataIn.tex_coord);
		}
		else{
			diff=mat.diffuse*texture(texUnitDiff, DataIn.tex_coord)*texture(texUnitDiff1, DataIn.tex_coord);
		}
		if (specularMap){
			auxSpec=mat.specular*texture(texUnitSpec, DataIn.tex_coord);
		}
		else{
			auxSpec=mat.specular;
		}
	}

	if (dayTime){
		intensity=max(dot(n,l), 0.0);
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = auxSpec * pow(intSpec, mat.shininess);
		}
	
		color += intensity * diff + spec;
	}

	//point lights
	if (pointLightsOn){
		for (int i = 0; i < 6; i++) {
			intensity=max(dot(n,l), 0.0);
			l = normalize(vec3(pointLights[i].position) + DataIn.eye);
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = auxSpec * pow(intSpec, mat.shininess);
			}
			color += vec4(0.2, 0.2, 0.2, 1.0)*intensity * diff + spec;
		}
	}

	if (spotLightsOn){
		for (int i=0; i<2; i++){
			intensity=max(dot(n,l), 0.0);
			l = normalize(vec3(spotLights[i].position) + DataIn.eye);
			float theta = dot(l, normalize(vec3(spotLights[i].direction)));
			if (theta > cos(radians(spotLights[i].angle))){
				if (intensity > 0.0) {
					vec3 h = normalize(l + e);
					float intSpec = max(dot(h,n), 0.0);
					spec = auxSpec * pow(intSpec, mat.shininess);
				}
				color += vec4(0.5, 0.5, 0.5, 1.0)* intensity * diff + spec;
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
		colorOut = vec4(finalColor, diff.a); // Output the final color with alpha
	}
	else {
		colorOut = vec4(max(color, mat.ambient).rgb, diff.a);
	}

	if (texMode==1){
		texel = texture(texmap2, DataIn.tex_coord);
		texel1 = texture(texmap1, DataIn.tex_coord);
		colorOut = max(colorOut*texel1*texel, texel*texel1*0.07);
	}
	else if(texMode==2){ //particle shading
		texel=texture(texmap3, DataIn.tex_coord);
		colorOut = max(mat.diffuse*texel, texel*0.07);
	}
	else if (texMode==3){
		texel=texture(texmap4, DataIn.tex_coord);
		if(texel.a == 0.0) discard;
		else
			colorOut = vec4(max(intensity*texel.rgb + vec3(spec), 0.1*texel.rgb), texel.a);
	}
	else if (texMode==4){
		texel = texture(texmap5, DataIn.tex_coord);  //texel from element flare texture
		if((texel.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
		else
			colorOut = mat.diffuse * texel;
	}
	else if(texMode==5){
		colorOut = texture(cubeMap, DataIn.skyboxTexCoord);
		//colorOut = texture(texmap1, DataIn.tex_coord);
	}
	else if (cubeMapping){
		if(reflect_perFrag == 1) {  //reflected vector calculated here
			vec3 reflected1 = vec3 (transpose(m_View) * vec4 (vec3(reflect(-e, n)), 0.0)); //reflection vector in world coord
			reflected1.x= -reflected1.x;   
			cube_texel = texture(cubeMap, reflected1);
		}
		else
			cube_texel = texture(cubeMap, DataIn.reflected); //use interpolated reflected vector calculated in vertex shader

		texel = texture(texmap0, DataIn.tex_coord);  // texel from lighwood.tga
		vec4 aux_color = mix(texel, cube_texel, reflect_factor);
		aux_color = max(intensity*aux_color + spec, 0.1*aux_color);
	    colorOut = vec4(aux_color.rgb, 1.0); 
	  //colorOut = vec4(cube_texel.rgb, 1.0);
	}
	else if (bumpMapping){
		texel = texture(texmap0, DataIn.tex_coord);  // texel from stone.tga
		colorOut = vec4((max(intensity*texel + spec, 0.2*texel)).rgb, 1.0);
	}

}