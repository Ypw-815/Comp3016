#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Material properties
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

// Light source properties
struct Light {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // Greatly enhanced ambient light
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb * 2.5;  // Increased from 1.5 to 2.5
    
    // Greatly enhanced diffuse light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb * 3.0;  // Increased from 1.8 to 3.0
    
    // Greatly enhanced specular light
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb * 2.0;  // Increased from 1.2 to 2.0
    
    vec3 result = ambient + diffuse + specular;
    
    // Greatly increase overall brightness and prevent complete black
    result = max(result * 2.0, vec3(0.2));  // Increased from 1.3 and 0.1 to 2.0 and 0.2 respectively
    
    FragColor = vec4(result, 1.0);
}
