#version 330 core

struct Material {
    sampler2D diffuse;
	sampler2D specular;
	int shininess;
};

struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform DirectionalLight dlight;

vec3 calcDirLight(DirectionalLight l, vec3 normal, vec3 viewDir) {
	// Ambient lighting
	vec3 ambient = l.ambient * texture(material.diffuse, TexCoords).rgb;

	// Diffuse lighting
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-l.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = l.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

	// Specular lighting
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = l.specular * spec * texture(material.specular, TexCoords).rgb;

	// Put it all together
	return ambient + diffuse + specular;
}

void main() {
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result = calcDirLight(dlight, norm, viewDir);
	FragColor = vec4(result, 0.0f);
}
