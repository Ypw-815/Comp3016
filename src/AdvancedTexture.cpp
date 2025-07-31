#include "AdvancedTexture.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <stb_image.h>
#include <vector>





AdvancedTexture::AdvancedTexture() 
    : ID(0), type(TextureType::DIFFUSE), width(0), height(0), channels(0),
      format(TextureFormat::RGB8), filterMode(FilterMode::LINEAR), 
      wrapMode(WrapMode::REPEAT), compressed(false), anisotropyLevel(1.0f), m_loaded(false) {
    glGenTextures(1, &ID);
}

AdvancedTexture::AdvancedTexture(const std::string& path, TextureType type)
    : AdvancedTexture() {
    this->type = type;
    this->path = path;
    LoadFromFile(path);
}

AdvancedTexture::~AdvancedTexture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

bool AdvancedTexture::LoadFromFile(const std::string& path, bool flipY) {
    this->path = path;
    
    stbi_set_flip_vertically_on_load(flipY);
    
    int w, h, c;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 0);
    
    if (data) {
        width = w;
        height = h;
        channels = c;
        
        
        switch (channels) {
            case 1: format = TextureFormat::R8; break;
            case 2: format = TextureFormat::RG8; break;
            case 3: format = TextureFormat::RGB8; break;
            case 4: format = TextureFormat::RGBA8; break;
            default: format = TextureFormat::RGB8; break;
        }
        
        bool success = LoadFromMemory(data, width, height, channels, format);
        stbi_image_free(data);
        
        if (success) {
            std::cout << "Advanced texture loaded: " << path << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
        }
        
        return success;
    } else {
        std::cerr << "Failed to load texture: " << path << " - " << stbi_failure_reason() << std::endl;
        
        
        unsigned char defaultData[] = { 255, 255, 255, 255 };
        LoadFromMemory(defaultData, 1, 1, 4, TextureFormat::RGBA8);
        return false;
    }
}

bool AdvancedTexture::LoadFromMemory(const unsigned char* data, int w, int h, int c, TextureFormat fmt) {
    width = w;
    height = h;
    channels = c;
    format = fmt;
    
    glBindTexture(GL_TEXTURE_2D, ID);
    
    GLenum glFormat = GetGLFormat(format);
    GLenum glInternalFormat = GetGLInternalFormat(format);
    GLenum glType = GetGLType(format);
    
    
    if (compressed && (format == TextureFormat::RGB8 || format == TextureFormat::RGBA8)) {
        glInternalFormat = (format == TextureFormat::RGB8) ? GL_COMPRESSED_RGB : GL_COMPRESSED_RGBA;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glFormat, glType, data);
    
    SetTextureParameters();
    GenerateMipmaps();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    m_loaded = true;
    return true;
}

void AdvancedTexture::Bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void AdvancedTexture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AdvancedTexture::GenerateNoise(int w, int h, float scale, int octaves) {
    width = w;
    height = h;
    channels = 3;
    format = TextureFormat::RGB8;
    
    std::vector<unsigned char> noiseData(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float noiseValue = PerlinNoise2D(x * scale, y * scale, 0.5f, octaves);
            noiseValue = (noiseValue + 1.0f) * 0.5f; 
            noiseValue = std::max(0.0f, std::min(1.0f, noiseValue));
            
            unsigned char value = static_cast<unsigned char>(noiseValue * 255);
            int index = (y * width + x) * 3;
            
            noiseData[index] = value;     
            noiseData[index + 1] = value; 
            noiseData[index + 2] = value; 
        }
    }
    
    LoadFromMemory(noiseData.data(), width, height, 3, TextureFormat::RGB8);
    type = TextureType::NOISE;
    
    std::cout << "Generated noise texture: " << width << "x" << height << std::endl;
}

void AdvancedTexture::GeneratePerlinNoise(int w, int h, float frequency, int octaves) {
    width = w;
    height = h;
    channels = 3;
    format = TextureFormat::RGB8;
    
    std::vector<unsigned char> noiseData(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float noiseValue = PerlinNoise2D(x * frequency, y * frequency, 0.5f, octaves);
            noiseValue = (noiseValue + 1.0f) * 0.5f; 
            noiseValue = std::max(0.0f, std::min(1.0f, noiseValue));
            
            unsigned char value = static_cast<unsigned char>(noiseValue * 255);
            int index = (y * width + x) * 3;
            
            noiseData[index] = value;
            noiseData[index + 1] = value;
            noiseData[index + 2] = value;
        }
    }
    
    LoadFromMemory(noiseData.data(), width, height, 3, TextureFormat::RGB8);
    type = TextureType::NOISE;
}

void AdvancedTexture::GenerateWorleyNoise(int w, int h, int numPoints) {
    width = w;
    height = h;
    channels = 3;
    format = TextureFormat::RGB8;
    
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    std::vector<std::pair<float, float>> points(numPoints);
    for (int i = 0; i < numPoints; i++) {
        points[i] = { dis(gen), dis(gen) };
    }
    
    std::vector<unsigned char> noiseData(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float px = static_cast<float>(x) / width;
            float py = static_cast<float>(y) / height;
            
            float minDist = 1.0f;
            for (const auto& point : points) {
                float dx = px - point.first;
                float dy = py - point.second;
                float dist = std::sqrt(dx * dx + dy * dy);
                minDist = std::min(minDist, dist);
            }
            
            unsigned char value = static_cast<unsigned char>(minDist * 255);
            int index = (y * width + x) * 3;
            
            noiseData[index] = value;
            noiseData[index + 1] = value;
            noiseData[index + 2] = value;
        }
    }
    
    LoadFromMemory(noiseData.data(), width, height, 3, TextureFormat::RGB8);
    type = TextureType::NOISE;
}

void AdvancedTexture::GenerateNormalMap(const AdvancedTexture& heightMap, float strength) {
    if (!heightMap.m_loaded) {
        std::cerr << "Height map not loaded!" << std::endl;
        return;
    }
    
    width = heightMap.width;
    height = heightMap.height;
    channels = 3;
    format = TextureFormat::RGB8;
    
    
    std::vector<unsigned char> heightData = heightMap.GetPixelData();
    std::vector<unsigned char> normalData(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            int left = std::max(0, x - 1);
            int right = std::min(width - 1, x + 1);
            int top = std::max(0, y - 1);
            int bottom = std::min(height - 1, y + 1);
            
            float heightL = heightData[(y * width + left) * heightMap.channels] / 255.0f;
            float heightR = heightData[(y * width + right) * heightMap.channels] / 255.0f;
            float heightT = heightData[(top * width + x) * heightMap.channels] / 255.0f;
            float heightB = heightData[(bottom * width + x) * heightMap.channels] / 255.0f;
            
            
            float dx = (heightR - heightL) * strength;
            float dy = (heightB - heightT) * strength;
            
            
            float nx = -dx;
            float ny = -dy;
            float nz = 1.0f;
            
            
            float length = std::sqrt(nx * nx + ny * ny + nz * nz);
            nx /= length;
            ny /= length;
            nz /= length;
            
            
            int index = (y * width + x) * 3;
            normalData[index] = static_cast<unsigned char>((nx + 1.0f) * 0.5f * 255);     
            normalData[index + 1] = static_cast<unsigned char>((ny + 1.0f) * 0.5f * 255); 
            normalData[index + 2] = static_cast<unsigned char>((nz + 1.0f) * 0.5f * 255); 
        }
    }
    
    LoadFromMemory(normalData.data(), width, height, 3, TextureFormat::RGB8);
    type = TextureType::NORMAL;
    
    std::cout << "Generated normal map from height map: " << width << "x" << height << std::endl;
}

void AdvancedTexture::SetFilterMode(FilterMode mode) {
    filterMode = mode;
    
    glBindTexture(GL_TEXTURE_2D, ID);
    
    switch (mode) {
        case FilterMode::NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case FilterMode::LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case FilterMode::MIPMAP_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case FilterMode::MIPMAP_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case FilterMode::ANISOTROPIC:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if (anisotropyLevel > 1.0f) {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropyLevel);
            }
            break;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AdvancedTexture::SetWrapMode(WrapMode mode) {
    wrapMode = mode;
    
    glBindTexture(GL_TEXTURE_2D, ID);
    
    GLenum glWrapMode;
    switch (mode) {
        case WrapMode::REPEAT: glWrapMode = GL_REPEAT; break;
        case WrapMode::MIRRORED_REPEAT: glWrapMode = GL_MIRRORED_REPEAT; break;
        case WrapMode::CLAMP_TO_EDGE: glWrapMode = GL_CLAMP_TO_EDGE; break;
        case WrapMode::CLAMP_TO_BORDER: glWrapMode = GL_CLAMP_TO_BORDER; break;
        default: glWrapMode = GL_REPEAT; break;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapMode);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AdvancedTexture::SetAnisotropy(float level) {
    anisotropyLevel = level;
    if (filterMode == FilterMode::ANISOTROPIC) {
        SetFilterMode(FilterMode::ANISOTROPIC); 
    }
}

void AdvancedTexture::EnableCompression(bool enable) {
    compressed = enable;
}

void AdvancedTexture::GenerateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, ID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AdvancedTexture::SetTextureParameters() {
    SetFilterMode(filterMode);
    SetWrapMode(wrapMode);
}

GLenum AdvancedTexture::GetGLFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::R8: return GL_RED;
        case TextureFormat::RG8: return GL_RG;
        case TextureFormat::RGB8: 
        case TextureFormat::RGB16F: 
        case TextureFormat::RGB32F: 
        case TextureFormat::COMPRESSED_RGB: return GL_RGB;
        case TextureFormat::RGBA8: 
        case TextureFormat::RGBA16F: 
        case TextureFormat::RGBA32F: 
        case TextureFormat::COMPRESSED_RGBA: return GL_RGBA;
        case TextureFormat::DEPTH24: 
        case TextureFormat::DEPTH32F: return GL_DEPTH_COMPONENT;
        default: return GL_RGB;
    }
}

GLenum AdvancedTexture::GetGLInternalFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::R8: return GL_R8;
        case TextureFormat::RG8: return GL_RG8;
        case TextureFormat::RGB8: return GL_RGB8;
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGB16F: return GL_RGB16F;
        case TextureFormat::RGBA16F: return GL_RGBA16F;
        case TextureFormat::RGB32F: return GL_RGB32F;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        case TextureFormat::DEPTH24: return GL_DEPTH_COMPONENT24;
        case TextureFormat::DEPTH32F: return GL_DEPTH_COMPONENT32F;
        case TextureFormat::COMPRESSED_RGB: return GL_COMPRESSED_RGB;
        case TextureFormat::COMPRESSED_RGBA: return GL_COMPRESSED_RGBA;
        default: return GL_RGB8;
    }
}

GLenum AdvancedTexture::GetGLType(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F: return GL_HALF_FLOAT;
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::DEPTH32F: return GL_FLOAT;
        default: return GL_UNSIGNED_BYTE;
    }
}

std::vector<unsigned char> AdvancedTexture::GetPixelData() const {
    std::vector<unsigned char> data(width * height * channels);
    
    glBindTexture(GL_TEXTURE_2D, ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GetGLFormat(format), GL_UNSIGNED_BYTE, data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return data;
}


std::shared_ptr<AdvancedTexture> AdvancedTexture::CreateColorTexture(int width, int height, float r, float g, float b, float a) {
    auto texture = std::make_shared<AdvancedTexture>();
    
    std::vector<unsigned char> data(width * height * 4);
    unsigned char red = static_cast<unsigned char>(r * 255);
    unsigned char green = static_cast<unsigned char>(g * 255);
    unsigned char blue = static_cast<unsigned char>(b * 255);
    unsigned char alpha = static_cast<unsigned char>(a * 255);
    
    for (int i = 0; i < width * height; i++) {
        data[i * 4] = red;
        data[i * 4 + 1] = green;
        data[i * 4 + 2] = blue;
        data[i * 4 + 3] = alpha;
    }
    
    texture->LoadFromMemory(data.data(), width, height, 4, TextureFormat::RGBA8);
    return texture;
}

std::shared_ptr<AdvancedTexture> AdvancedTexture::CreateCheckerboard(int width, int height, int checkSize) {
    auto texture = std::make_shared<AdvancedTexture>();
    
    std::vector<unsigned char> data(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool isWhite = ((x / checkSize) + (y / checkSize)) % 2 == 0;
            unsigned char value = isWhite ? 255 : 0;
            
            int index = (y * width + x) * 3;
            data[index] = value;
            data[index + 1] = value;
            data[index + 2] = value;
        }
    }
    
    texture->LoadFromMemory(data.data(), width, height, 3, TextureFormat::RGB8);
    return texture;
}


float AdvancedTexture::PerlinNoise2D(float x, float y, float persistence, int octaves) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++) {
        total += InterpolateNoise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

float AdvancedTexture::InterpolateNoise(float x, float y) const {
    int intX = FastFloor(x);
    int intY = FastFloor(y);
    float fracX = x - intX;
    float fracY = y - intY;
    
    float v1 = SmoothNoise(intX, intY);
    float v2 = SmoothNoise(intX + 1, intY);
    float v3 = SmoothNoise(intX, intY + 1);
    float v4 = SmoothNoise(intX + 1, intY + 1);
    
    float i1 = Lerp(fracX, v1, v2);
    float i2 = Lerp(fracX, v3, v4);
    
    return Lerp(fracY, i1, i2);
}

float AdvancedTexture::SmoothNoise(float x, float y) const {
    float corners = (Noise(x-1, y-1) + Noise(x+1, y-1) + Noise(x-1, y+1) + Noise(x+1, y+1)) / 16.0f;
    float sides = (Noise(x-1, y) + Noise(x+1, y) + Noise(x, y-1) + Noise(x, y+1)) / 8.0f;
    float center = Noise(x, y) / 4.0f;
    
    return corners + sides + center;
}

float AdvancedTexture::Noise(int x, int y) const {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float AdvancedTexture::Fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float AdvancedTexture::Lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

int AdvancedTexture::FastFloor(float x) const {
    return (x >= 0) ? static_cast<int>(x) : static_cast<int>(x) - 1;
}

float AdvancedTexture::Grad(int hash, float x, float y) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}





TextureManager& TextureManager::Instance() {
    static TextureManager instance;
    return instance;
}

std::shared_ptr<AdvancedTexture> TextureManager::LoadTexture(const std::string& path, TextureType type) {
    
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    
    auto texture = std::make_shared<AdvancedTexture>(path, type);
    if (texture) {
        texture->EnableCompression(m_compressionEnabled);
        m_textures[path] = texture;
        
        
        m_memoryUsage += texture->width * texture->height * texture->channels;
    }
    
    return texture;
}

std::shared_ptr<AdvancedTexture> TextureManager::GetTexture(const std::string& name) {
    auto it = m_textures.find(name);
    return (it != m_textures.end()) ? it->second : nullptr;
}

void TextureManager::RegisterTexture(const std::string& name, std::shared_ptr<AdvancedTexture> texture) {
    m_textures[name] = texture;
}

void TextureManager::UnloadTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        if (it->second) {
            m_memoryUsage -= it->second->width * it->second->height * it->second->channels;
        }
        m_textures.erase(it);
    }
}

void TextureManager::UnloadAll() {
    m_textures.clear();
    m_memoryUsage = 0;
}

size_t TextureManager::GetTotalMemoryUsage() const {
    return m_memoryUsage;
}





TextureAnimator::TextureAnimator() 
    : m_currentFrame(0), m_currentTime(0.0f), m_speed(1.0f), 
      m_loop(true), m_playing(true) {
}

TextureAnimator::~TextureAnimator() = default;

void TextureAnimator::AddFrame(std::shared_ptr<AdvancedTexture> texture, float duration) {
    m_frames.push_back({texture, duration});
}

void TextureAnimator::Update(float deltaTime) {
    if (!m_playing || m_frames.empty()) return;
    
    m_currentTime += deltaTime * m_speed;
    
    if (m_currentFrame < m_frames.size()) {
        if (m_currentTime >= m_frames[m_currentFrame].duration) {
            m_currentTime = 0.0f;
            m_currentFrame++;
            
            if (m_currentFrame >= m_frames.size()) {
                if (m_loop) {
                    m_currentFrame = 0;
                } else {
                    m_currentFrame = m_frames.size() - 1;
                    m_playing = false;
                }
            }
        }
    }
}

std::shared_ptr<AdvancedTexture> TextureAnimator::GetCurrentTexture() const {
    if (m_frames.empty() || m_currentFrame >= m_frames.size()) {
        return nullptr;
    }
    return m_frames[m_currentFrame].texture;
}

void TextureAnimator::Reset() {
    m_currentFrame = 0;
    m_currentTime = 0.0f;
}
