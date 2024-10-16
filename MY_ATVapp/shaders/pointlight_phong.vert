#version 430

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos;
uniform bool normalMap;

in vec3 position;
in vec3 normal, tangent, bitangent;    //por causa do gerador de geometria
in vec2 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
} DataOut;

out vec4 pos;

void main () {

	vec3 n, t, b;
	vec3 lightDir, eyeDir;
	vec3 aux;

	pos = m_viewModel * vec4(position,1.0);

	n=normalize(m_normal * normal.xyz);
	eyeDir = vec3(-pos);
	lightDir = vec3(l_pos - pos);

	if (normalMap){
		t = normalize(m_normal * tangent.xyz);
		b = normalize(m_normal * bitangent.xyz);

		aux.x = dot(lightDir, t);
		aux.y = dot(lightDir, b);
		aux.z = dot(lightDir, n);
		lightDir = normalize(aux);

		aux.x = dot(eyeDir, t);
		aux.y = dot(eyeDir, b);
		aux.z = dot(eyeDir, n);
		eyeDir = normalize(aux);
	}

	DataOut.normal = n;
	DataOut.lightDir = lightDir;
	DataOut.eye = eyeDir;
	DataOut.tex_coord = vec2(texCoord);

	gl_Position = m_pvm * vec4(position, 1.0);	
}
