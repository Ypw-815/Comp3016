#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <vector> 

Texture::Texture() : ID(0), m_loaded(false) {
    glGenTextures(1, &ID);
}

Texture::Texture(const std::string& path, const std::string& type) 
    : ID(0), type(type), path(path), m_loaded(false) {
    glGenTextures(1, &ID);
    LoadTexture(path);
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

bool Texture::LoadTexture(const std::string& path, bool flip) {
    this->path = path;
    
    
    stbi_set_flip_vertically_on_load(flip);
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        GenerateTexture(data, width, height, nrChannels);
        stbi_image_free(data);
        m_loaded = true;
        std::cout << "Texture loaded successfully: " << path << std::endl;
        return true;
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        
        
        unsigned char whitePixel[] = { 255, 255, 255, 255 };
        GenerateTexture(whitePixel, 1, 1, 4);
        m_loaded = false;
        return false;
    }
}

void Texture::GenerateTexture(unsigned char* data, int width, int height, int nrChannels) {
    GLenum format = GL_RGB;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::generateCheckerboard(int width, int height) {
    
    std::vector<unsigned char> data(width * height * 3); 
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            int checkSize = 8;
            bool isWhite = ((x / checkSize) + (y / checkSize)) % 2 == 0;
            
            int index = (y * width + x) * 3;
            if (isWhite) {
                data[index] = 255;     
                data[index + 1] = 255; 
                data[index + 2] = 255; 
            } else {
                data[index] = 0;       
                data[index + 1] = 0;   
                data[index + 2] = 0;   
            }
        }
    }
    
    
    GenerateTexture(data.data(), width, height, 3);
    m_loaded = true;
}


void Texture::bind(unsigned int slot) const {
    Bind(slot); 
}
