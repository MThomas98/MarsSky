#version 330 core

struct Material {
    sampler2D diffuse;
	sampler2D specular;
	int shininess;
};

struct PointLight {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
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
uniform PointLight light;
uniform DirectionalLight dlight;
uniform float alpha = 1.0f;

vec3 calcDirLight(DirectionalLight l, vec3 normal, vec3 viewDir) {
	vec3 ambient = l.ambient * texture(material.diffuse, TexCoords).rgb;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-l.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = l.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = l.specular * spec * texture(material.specular, TexCoords).rgb;

	return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLight l, vec3 norm, vec3 fragPos, vec3 viewDir) {
	vec3 ambient = l.ambient * texture(material.diffuse, TexCoords).rgb;

	vec3 lightDir = normalize(l.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = l.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = l.specular * spec * texture(material.specular, TexCoords).rgb;

	float distance = length(l.position - FragPos);
	float attenuation = 1.0 / (l.constant + l.linear * distance + l.quadratic * (distance * distance));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return ambient + diffuse + specular;
}


void main() {
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result = calcPointLight(light, norm, FragPos, viewDir);
	result += calcDirLight(dlight, norm, viewDir);
	FragColor = vec4(result, alpha);
}
