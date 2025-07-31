#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 objectColor;

void main()
{
    // Greatly enhanced ambient light
    float ambientStrength = 0.5;  // Increased from 0.3 to 0.5
    vec3 ambient = ambientStrength * lightColor;
    
    // Greatly enhanced diffuse light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 2.5;  // Enhanced from 1.5 to 2.5
    
    // Greatly enhanced specular light
    float specularStrength = 1.2;  // Increased from 0.8 to 1.2
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);  // Reduced shininess for more scattered lighting
    vec3 specular = specularStrength * spec * lightColor;
    
    // Sample texture
    vec4 texColor = texture(texture_diffuse1, TexCoord);
    
    // Ensure base color is not too dark and increase minimum brightness
    vec3 baseColor = max(objectColor, vec3(0.7, 0.7, 0.7));  // Increased from 0.5 to 0.7
    
    vec3 result = (ambient + diffuse + specular) * baseColor * max(texColor.rgb, vec3(0.5));  // Increased from 0.3 to 0.5
    
    // Greatly increase overall brightness
    result = result * 1.8;  // Increased from 1.2 to 1.8
    
    FragColor = vec4(result, 1.0);
}
