#version 410 core
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 color;
uniform bool useTexture;
uniform sampler2D ourTexture;

void main()
{
    if (useTexture) {
        FragColor = texture(ourTexture, TexCoord) * color;
    } else {
        FragColor = color;
    }
}
