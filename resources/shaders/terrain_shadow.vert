#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 VertexColor;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    // Calculate world space coordinates
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal to world space
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass texture coordinates and vertex color
    TexCoord = aTexCoord;
    VertexColor = aColor;
    
    // Calculate position in light space
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    
    // Calculate final position
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
