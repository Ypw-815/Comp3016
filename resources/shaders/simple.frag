#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 objectColor;

void main()
{
    // Simple fixed color output, no texture or lighting needed
    FragColor = vec4(objectColor, 1.0);
}
