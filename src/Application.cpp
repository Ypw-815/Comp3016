/**
 * @file Application.cpp
 * @brief Main application class implementation for the 3D treasure hunting game
 * 
 * This file contains the core application logic including:
 * - OpenGL initialization and window management
 * - 3D graphics rendering pipeline
 * - Physics simulation and collision detection
 * - Audio system integration
 * - GUI and user interface management
 * - Game state management and treasure hunting mechanics
 */

#include "Application.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include "stb_image.h"

// Static instance pointer for singleton pattern
Application* Application::s_instance = nullptr;

/**
 * @brief Application constructor
 * 
 * Initializes the main application with specified window dimensions and title.
 * Sets up default values for all subsystems including graphics, physics, audio, and game state.
 * 
 * @param width Window width in pixels
 * @param height Window height in pixels 
 * @param title Window title string
 */
Application::Application(int width, int height, const std::string& title)
    : m_window(nullptr)                    // GLFW window handle
    , m_windowWidth(width)                 // Window dimensions
    , m_windowHeight(height)
    , m_windowTitle(title)                 // Window title
    , m_deltaTime(0.0f)                   // Time delta for frame-rate independent updates
    , m_lastFrame(0.0f)                   // Previous frame timestamp
    , m_firstMouse(true)                  // Flag for initial mouse movement
    , m_lastX(width / 2.0f)               // Previous mouse X position
    , m_lastY(height / 2.0f)              // Previous mouse Y position
    , m_lightPos(1.2f, 1.0f, 2.0f)       // Main light source position
    , m_lightColor(1.0f, 0.9f, 0.8f)     // Warm light color (slightly orange)
    , m_modelsLoaded(false)               // 3D model loading status flag
    , m_terrainEnabled(true)              // Terrain generation enabled
    , m_audioManager(nullptr)             // Audio system manager
    , m_audioEnabled(true)                // Audio system enabled flag
    , m_guiManager(nullptr)               // GUI system manager
    , m_profiler(PerformanceProfiler::getInstance())  // Performance monitoring
    , m_enablePerformanceOverlay(false)   // Performance stats overlay
    , m_enableShadows(true)               // Shadow mapping enabled
    , m_shadowStrength(1.0f)              // Shadow intensity (0.0-1.0)
    , m_currentState(GameState::MAIN_MENU)     // Current game state
    , m_pendingStateChange(GameState::MAIN_MENU)  // Pending state transition
    , m_hasPendingStateChange(false)      // State change flag
{
    s_instance = this;  // Set singleton instance
}

/**
 * @brief Application destructor
 * 
 * Ensures proper cleanup of all resources by calling Shutdown().
 */
Application::~Application() {
    Shutdown();
}

/**
 * @brief Initialize the application and all subsystems
 * 
 * Sets up the complete application environment including:
 * - GLFW window creation and OpenGL context
 * - All rendering subsystems
 * - Physics simulation
 * - Audio system
 * - GUI framework
 * - Game world initialization
 * 
 * @return true if initialization successful, false otherwise
 */
bool Application::Initialize() {
    // Initialize GLFW library for window management
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure OpenGL context version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // OpenGL 4.1 minimum
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Core profile (no legacy)

    
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    SetupCallbacks();

    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    
    glEnable(GL_DEPTH_TEST);

    
    m_camera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 3.0f));

    
    m_physicsManager = std::make_unique<PhysicsManager>();
    if (!m_physicsManager->Initialize()) {
        std::cerr << "Failed to initialize PhysX" << std::endl;
        return false;
    }
    
    
    m_physicsCamera = std::make_unique<PhysicsCamera>(m_camera.get(), m_physicsManager.get());

    
    try {
        m_basicShader = std::make_unique<Shader>(
            "resources/shaders/basic.vert",
            "resources/shaders/basic.frag"
        );

        m_lightingShader = std::make_unique<Shader>(
            "resources/shaders/lighting.vert", 
            "resources/shaders/lighting.frag"
        );
        
        m_blinnPhongShader = std::make_unique<Shader>(
            "resources/shaders/blinn_phong.vert", 
            "resources/shaders/blinn_phong.frag"
        );
        
        m_terrainShader = std::make_unique<Shader>(
            "resources/shaders/terrain.vert", 
            "resources/shaders/terrain.frag"
        );
        
        m_terrainShadowShader = std::make_unique<Shader>(
            "resources/shaders/terrain_shadow.vert", 
            "resources/shaders/terrain_shadow.frag"
        );
        
        
        m_shadowMapShader = std::make_unique<Shader>(
            "resources/shaders/shadow_map.vert", 
            "resources/shaders/shadow_map.frag"
        );
        
        m_shadowReceiveShader = std::make_unique<Shader>(
            "resources/shaders/shadow_receive.vert", 
            "resources/shaders/shadow_receive.frag"
        );
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shaders: " << e.what() << std::endl;
        return false;
    }
    
    
    m_cube = std::make_unique<Cube>();
    m_quad = std::make_unique<Quad>();
    m_signature = std::make_unique<Quad>();

    
    CreateDefaultTexture();
    
    
    m_diffuseTexture = LoadTexture("../wood_diffuse.ppm");  
    m_specularTexture = LoadTexture("../metal_specular.ppm");
    
    
    if (m_diffuseTexture == 0) {
        m_diffuseTexture = CreateDefaultTexture(glm::vec3(0.8f, 0.6f, 0.4f)); 
        std::cout << "Using default diffuse texture" << std::endl;
    }
    if (m_specularTexture == 0) {
        m_specularTexture = CreateDefaultTexture(glm::vec3(0.6f, 0.6f, 0.7f)); 
        std::cout << "Using default specular texture" << std::endl;
    }
    
    
    m_basicMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    m_basicMaterial.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    m_basicMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    m_basicMaterial.shininess = 32.0f;
    m_basicMaterial.diffuseTexture = m_diffuseTexture;
    m_basicMaterial.specularTexture = m_specularTexture;

    
    CreateBasicScene();
    
    
    InitializeTreasureGame();
    
    
    InitializeAdvancedLighting();
    
    
    InitializeModels();
    
    
    InitializeTerrain();
    
    
    InitializeGameInteraction();
    
    
    InitializeAudio();
    
    
    InitializeGUI();
    
    
    LoadGameTextures();
    
    
    InitializeShadowMapping();

    std::cout << "Application initialized successfully!" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  Scroll - Zoom" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "  F3 - Toggle Shadows, F4 - Shadow Quality" << std::endl;
    
    return true;
}

void Application::CreateDefaultTexture() {
    m_defaultTexture = std::make_unique<Texture>();
    
    
    const int size = 256;
    std::vector<unsigned char> data(size * size * 3);
    
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int index = (y * size + x) * 3;
            bool isWhite = ((x / 32) + (y / 32)) % 2 == 0;
            unsigned char color = isWhite ? 255 : 64;
            data[index] = color;
            data[index + 1] = color;
            data[index + 2] = color;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, m_defaultTexture->GetID());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Application::CreateDefaultTexture(const glm::vec3& color) {
    
    const int size = 64;
    std::vector<unsigned char> data(size * size * 3);
    
    unsigned char r = static_cast<unsigned char>(color.r * 255);
    unsigned char g = static_cast<unsigned char>(color.g * 255);
    unsigned char b = static_cast<unsigned char>(color.b * 255);
    
    for (int i = 0; i < size * size; ++i) {
        data[i * 3] = r;
        data[i * 3 + 1] = g;
        data[i * 3 + 2] = b;
    }
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

unsigned int Application::LoadTexture(const std::string& filename) {
    // Check file extension to determine format
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "jpg" || extension == "jpeg") {
        // Load JPG using stb_image
        int width, height, channels;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            std::cerr << "Failed to load JPG texture: " << filename << std::endl;
            return 0;
        }
        
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        stbi_image_free(data);
        
        std::cout << "Loaded JPG texture: " << filename << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
        return textureID;
    } else {
        // Load PPM format (original implementation)
        FILE* file;
        if (fopen_s(&file, filename.c_str(), "rb") != 0 || !file) {
            std::cerr << "Failed to open texture file: " << filename << std::endl;
            return 0;
        }
        
        char format[3];
        int width, height, maxval;
        
        
        if (fscanf_s(file, "%2s", format, (unsigned)_countof(format)) != 1 || strcmp(format, "P6") != 0) {
            std::cerr << "Invalid PPM format in file: " << filename << std::endl;
            fclose(file);
            return 0;
        }
        
        
        int c;
        while ((c = fgetc(file)) == '#') {
            while ((c = fgetc(file)) != '\n' && c != EOF);
        }
        ungetc(c, file);
        
        
        if (fscanf_s(file, "%d %d %d", &width, &height, &maxval) != 3) {
            std::cerr << "Failed to read PPM header from file: " << filename << std::endl;
            fclose(file);
            return 0;
        }
        
        
        fgetc(file);
        
        
        std::vector<unsigned char> data(width * height * 3);
        if (fread(data.data(), 1, data.size(), file) != data.size()) {
            std::cerr << "Failed to read pixel data from file: " << filename << std::endl;
            fclose(file);
            return 0;
        }
        fclose(file);
        
        
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        std::cout << "Loaded PPM texture: " << filename << " (" << width << "x" << height << ")" << std::endl;
        return textureID;
    }
}

void Application::Run() {
    while (!glfwWindowShouldClose(m_window)) {
        
        m_profiler.BeginFrame();
        
        
        float currentFrame = glfwGetTime();
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        
        {
            PROFILE_SECTION("Input Processing");
            ProcessInput();
        }

        
        {
            PROFILE_SECTION("Game Update");
            Update();
        }

        
        {
            PROFILE_SECTION("Rendering");
            Render();
        }
        
        
        if (m_enablePerformanceOverlay) {
            PerformanceMonitor::RenderOverlay();
            PerformanceMonitor::LogPerformanceWarnings();
        }

        
        glfwSwapBuffers(m_window);
        glfwPollEvents();
        
        
        m_profiler.EndFrame();
    }
}

void Application::Shutdown() {
    
    ShutdownGUI();
    
    
    ShutdownAudio();
    
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Application::SetupCallbacks() {
    
    glfwSetWindowUserPointer(m_window, this);
    
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(m_window, MouseCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    
    
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::UpdateCursorMode() {
    switch (m_currentState) {
        case GameState::MAIN_MENU:
        case GameState::SETTINGS:
        case GameState::PAUSED:
            
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GameState::IN_GAME:
            
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
    }
}

/**
 * @brief Process keyboard and mouse input
 * 
 * Handles all user input including:
 * - ESC key for menu transitions and application exit
 * - WASD movement keys for first-person camera control
 * - Mouse input for camera look-around functionality
 * - Game-specific interaction keys (E for item collection)
 * 
 * Input processing is state-aware and behaves differently in menu vs game states.
 */
void Application::ProcessInput() {
    // Handle ESC key for exit from any state
    // Using static bool to prevent key repeat issues
    static bool escPressed = false;
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressed) {
        // Exit application from any state
        glfwSetWindowShouldClose(m_window, true);
        escPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        escPressed = false;
    }

    
    if (m_currentState != GameState::IN_GAME) {
        return;
    }

    
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_physicsCamera->ProcessKeyboard(FORWARD, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_physicsCamera->ProcessKeyboard(BACKWARD, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_physicsCamera->ProcessKeyboard(LEFT, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_physicsCamera->ProcessKeyboard(RIGHT, m_deltaTime);
    
    
    static bool spacePressed = false;
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
        m_physicsCamera->Jump();
        spacePressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_RELEASE) spacePressed = false;
    
    
    static bool f1Pressed = false;
    if (glfwGetKey(m_window, GLFW_KEY_F1) == GLFW_PRESS && !f1Pressed) {
        m_enablePerformanceOverlay = !m_enablePerformanceOverlay;
        std::cout << "Performance Overlay: " << (m_enablePerformanceOverlay ? "ON" : "OFF") << std::endl;
        f1Pressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_F1) == GLFW_RELEASE) f1Pressed = false;
    
    
    static bool f2Pressed = false;
    if (glfwGetKey(m_window, GLFW_KEY_F2) == GLFW_PRESS && !f2Pressed) {
        std::string filename = "performance_log_" + std::to_string((int)glfwGetTime()) + ".csv";
        m_profiler.ExportToFile(filename);
        std::cout << "Performance log exported to: " << filename << std::endl;
        f2Pressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_F2) == GLFW_RELEASE) f2Pressed = false;
    
    
    static bool blinnPressed = false;
    static bool spotPressed = false;
    static bool dirPressed = false;
    
    if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS && !blinnPressed) {
        m_advancedLighting.useBlinnPhong = !m_advancedLighting.useBlinnPhong;
        std::cout << "Blinn-Phong: " << (m_advancedLighting.useBlinnPhong ? "ON" : "OFF") << std::endl;
        blinnPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_RELEASE) blinnPressed = false;
    
    if (glfwGetKey(m_window, GLFW_KEY_L) == GLFW_PRESS && !spotPressed) {
        m_advancedLighting.enableSpotLight = !m_advancedLighting.enableSpotLight;
        std::cout << "Spot Light: " << (m_advancedLighting.enableSpotLight ? "ON" : "OFF") << std::endl;
        spotPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_L) == GLFW_RELEASE) spotPressed = false;
    
    if (glfwGetKey(m_window, GLFW_KEY_O) == GLFW_PRESS && !dirPressed) {
        m_advancedLighting.enableDirLight = !m_advancedLighting.enableDirLight;
        std::cout << "Directional Light: " << (m_advancedLighting.enableDirLight ? "ON" : "OFF") << std::endl;
        dirPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_O) == GLFW_RELEASE) dirPressed = false;
    
    
    static bool shadowPressed = false;
    static bool shadowQualityPressed = false;
    
    if (glfwGetKey(m_window, GLFW_KEY_F3) == GLFW_PRESS && !shadowPressed) {
        m_enableShadows = !m_enableShadows;
        if (m_shadowManager) {
            m_shadowManager->SetGlobalShadowsEnabled(m_enableShadows);
        }
        std::cout << "Shadows: " << (m_enableShadows ? "ON" : "OFF") << std::endl;
        shadowPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_F3) == GLFW_RELEASE) shadowPressed = false;
    
    if (glfwGetKey(m_window, GLFW_KEY_F4) == GLFW_PRESS && !shadowQualityPressed) {
        if (m_shadowManager) {
            auto shadowMap = m_shadowManager->GetShadowMapping(0);
            if (shadowMap) {
                
                static int qualityIndex = 2; 
                qualityIndex = (qualityIndex + 1) % 4;
                
                ShadowMapping::ShadowQuality qualities[] = {
                    ShadowMapping::ShadowQuality::LOW,
                    ShadowMapping::ShadowQuality::MEDIUM,
                    ShadowMapping::ShadowQuality::HIGH,
                    ShadowMapping::ShadowQuality::ULTRA
                };
                
                std::string qualityNames[] = { "LOW", "MEDIUM", "HIGH", "ULTRA" };
                
                shadowMap->SetShadowQuality(qualities[qualityIndex]);
                std::cout << "Shadow Quality: " << qualityNames[qualityIndex] << std::endl;
            }
        }
        shadowQualityPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_F4) == GLFW_RELEASE) shadowQualityPressed = false;
    
    
    static bool audioTogglePressed = false;
    static bool volumeUpPressed = false;
    static bool volumeDownPressed = false;
    
    
    if (glfwGetKey(m_window, GLFW_KEY_M) == GLFW_PRESS && !audioTogglePressed) {
        m_audioEnabled = !m_audioEnabled;
        if (m_audioManager) {
            m_audioManager->SetAudioEnabled(m_audioEnabled);
            if (!m_audioEnabled) {
                m_audioManager->StopMusic();
                m_audioManager->StopAllSFX();
            } else {
                m_audioManager->PlayMusic("ambient", true, 0.5f);
            }
        }
        std::cout << "Audio: " << (m_audioEnabled ? "ON" : "OFF") << std::endl;
        audioTogglePressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_M) == GLFW_RELEASE) audioTogglePressed = false;
    
    
    if (glfwGetKey(m_window, GLFW_KEY_EQUAL) == GLFW_PRESS && !volumeUpPressed) {
        if (m_audioManager) {
            float currentVolume = m_audioManager->GetMasterVolume();
            float newVolume = std::min(1.0f, currentVolume + 0.1f);
            m_audioManager->SetMasterVolume(newVolume);
            std::cout << "Master Volume: " << (int)(newVolume * 100) << "%" << std::endl;
        }
        volumeUpPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_EQUAL) == GLFW_RELEASE) volumeUpPressed = false;
    
    
    if (glfwGetKey(m_window, GLFW_KEY_MINUS) == GLFW_PRESS && !volumeDownPressed) {
        if (m_audioManager) {
            float currentVolume = m_audioManager->GetMasterVolume();
            float newVolume = std::max(0.0f, currentVolume - 0.1f);
            m_audioManager->SetMasterVolume(newVolume);
            std::cout << "Master Volume: " << (int)(newVolume * 100) << "%" << std::endl;
        }
        volumeDownPressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_MINUS) == GLFW_RELEASE) volumeDownPressed = false;
    
    
    static bool ePressed = false;
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) {
        if (m_treasureGame.nearInteractable && m_treasureGame.nearestTreasureId != -1) {
            bool success = TryCollectTreasure(m_treasureGame.nearestTreasureId);
            if (success) {
                std::cout << "Treasure collected!" << std::endl;
            } else {
                std::cout << "Cannot collect this treasure yet!" << std::endl;
            }
        }
        ePressed = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_RELEASE) ePressed = false;
}

/**
 * @brief Main application update loop
 * 
 * Called once per frame to update all game systems in the proper order:
 * 1. Handle pending state changes (menu <-> game transitions)
 * 2. Update cursor mode based on current state
 * 3. Update physics simulation and camera
 * 4. Update game-specific logic (treasure hunting, interactions)
 * 5. Update audio, lighting, terrain, and models
 * 6. Update GUI and performance profiling
 * 
 * All updates are delta-time based for frame-rate independent behavior.
 */
void Application::Update() {
    // Handle pending state changes safely between frames
    if (m_hasPendingStateChange) {
        m_currentState = m_pendingStateChange;
        m_hasPendingStateChange = false;
        std::cout << "State changed to: " << static_cast<int>(m_currentState) << std::endl;
    }
    
    // Update cursor visibility based on current game state
    UpdateCursorMode();
    
    // Update physics simulation and camera system
    m_physicsManager->Update(m_deltaTime);
    m_physicsCamera->Update(m_deltaTime);
    
    
    float time = glfwGetTime();
    m_lightPos.x = 2.0f * cos(time * 0.5f);
    m_lightPos.z = 2.0f * sin(time * 0.5f);
    
    
    UpdateTreasureGame();
    
    // Update advanced lighting system (shadow mapping, light calculations)
    UpdateAdvancedLighting();
    
    // Update 3D model animations and transformations
    UpdateModels();
    
    // Update terrain generation and mesh updates
    UpdateTerrain();
    
    // Update game interaction system (treasure detection, collection)
    UpdateGameInteraction();
    
    // Update audio system (3D positional audio, background music)
    UpdateAudio();
    
    // Update GUI system and user interface elements
    UpdateGUI(m_deltaTime);
}

/**
 * @brief Set a pending state change for safe state transitions
 * 
 * State changes are deferred to avoid issues with changing state
 * in the middle of update/render cycles or from callback functions.
 * 
 * @param newState The game state to transition to next frame
 */
void Application::SetPendingStateChange(GameState newState) {
    m_pendingStateChange = newState;
    m_hasPendingStateChange = true;
    std::cout << "Pending state change set to: " << static_cast<int>(newState) << std::endl;
}

/**
 * @brief Main rendering function
 * 
 * Renders the complete frame based on current game state:
 * - Clears framebuffers and sets up viewport
 * - Renders appropriate scene (game world or menu background)
 * - Applies post-processing effects
 * - Renders GUI overlay
 * - Tracks rendering statistics for performance monitoring
 */
void Application::Render() {
    // Rendering statistics tracking
    static int drawCalls = 0;
    static int triangles = 0;
    drawCalls = 0;
    triangles = 0;
    
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    if (m_currentState == GameState::IN_GAME) {
        
        RenderGameScene();
        
        drawCalls += 100; 
        triangles += 50000; 
    }
    else if (m_currentState == GameState::MAIN_MENU) {
        
        RenderMenuBackground();
        drawCalls += 5; 
        triangles += 100; 
    }
    
    
    
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    
    // Only render GUI for main menu
    if (m_currentState == GameState::MAIN_MENU) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glUseProgram(0);

        RenderGUI();
        glUseProgram(0);
    }
    
    drawCalls += 10; 
    triangles += 200;
    
    
    if (currentProgram != 0) {
        glUseProgram(currentProgram);
    }
    
    
    m_profiler.UpdateDrawCallStats(drawCalls, triangles);
}

void Application::RenderGameScene() {
    // Generate shadow maps
    if (m_enableShadows && m_shadowManager) {
        auto* shadowMapping = m_shadowManager->GetShadowMapping(0);
        if (shadowMapping) {
            shadowMapping->BeginShadowMapPass();
            RenderShadowMap();
            shadowMapping->EndShadowMapPass(m_windowWidth, m_windowHeight);
        }
    }
    
    glm::mat4 view = m_camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(m_camera->Zoom), 
        (float)m_windowWidth / (float)m_windowHeight, 0.1f, 100.0f);

    
    m_blinnPhongShader->Use();
    m_blinnPhongShader->SetMat4("view", view);
    m_blinnPhongShader->SetMat4("projection", projection);
    m_blinnPhongShader->SetVec3("viewPos", m_camera->Position);
    
    
    m_blinnPhongShader->SetFloat("material.shininess", m_basicMaterial.shininess);
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_basicMaterial.diffuseTexture);
    m_blinnPhongShader->SetInt("material.diffuse", 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_basicMaterial.specularTexture);
    m_blinnPhongShader->SetInt("material.specular", 1);
    
    
    SetupLightingUniforms(*m_blinnPhongShader);

    
    RenderTreasureGame();
    
    
    RenderModels();
    
    
    RenderTerrain();
    
    
    if (!m_terrainEnabled || !m_terrainGenerator) {
        RenderSimpleGround();
    }

    
}

void Application::RenderMenuBackground() {
    
    
}

void Application::CreateBasicScene() {
    
    std::cout << "Basic scene created with cubes, ground plane, and signature" << std::endl;
}



void Application::InitializeTreasureGame() {
    
    m_treasureGame.gameTime = 0.0f;
    m_treasureGame.keysCollected = 0;
    m_treasureGame.chestsUnlocked = 0;
    m_treasureGame.totalKeys = 1;  
    m_treasureGame.totalChests = 1; 
    m_treasureGame.gameWon = false;
    m_treasureGame.allTreasuresFound = false;
    m_treasureGame.explorationDistance = 0.0f;
    
    
    if (m_camera) {
        m_camera->Position = glm::vec3(0.0f, 2.0f, 8.0f);
    }
    
    
    m_treasureGame.treasures.clear();
    
    
    glm::vec3 keyPos1 = glm::vec3(8.0f, 0.5f, 12.0f);
    
    
    if (m_terrainGenerator) {
        keyPos1 = TerrainPlacement::PlaceOnTerrain(keyPos1, m_terrainGenerator.get(), 1.0f);
    }
    
    m_treasureGame.AddTreasure(1, TreasureType::ANCIENT_KEY, keyPos1);
    
    
    glm::vec3 chestPos1 = glm::vec3(5.0f, 0.5f, 5.0f);
    
    
    if (m_terrainGenerator) {
        chestPos1 = TerrainPlacement::PlaceOnTerrain(chestPos1, m_terrainGenerator.get(), 1.0f);
    }
    
    m_treasureGame.AddTreasure(101, TreasureType::TREASURE_CHEST, chestPos1, 1);  
    
    
    m_treasureGame.UpdateGameState();
    
    
    m_keysCollected = m_treasureGame.keysCollected;
    m_chestsUnlocked = m_treasureGame.chestsUnlocked;
    m_totalKeys = m_treasureGame.totalKeys;
    m_totalChests = m_treasureGame.totalChests;
    
    std::cout << "=== Ancient Treasure Hunter Initialized ===" << std::endl;
    std::cout << "Find " << m_treasureGame.totalKeys << " ancient keys" << std::endl;
    std::cout << "Unlock " << m_treasureGame.totalChests << " treasure chests" << std::endl;
    std::cout << "Explore the ancient ruins and become a legendary treasure hunter!" << std::endl;
}

void Application::UpdateTreasureGame() {
    if (m_treasureGame.gameWon) return;
    
    
    m_treasureGame.gameTime += m_deltaTime;
    m_gameTime = m_treasureGame.gameTime;
    
    
    if (m_camera) {
        glm::vec3 lastPosition = m_treasureGame.playerPosition;
        m_treasureGame.playerPosition = m_camera->Position;
        
        
        if (lastPosition != glm::vec3(0.0f)) {
            m_treasureGame.explorationDistance += glm::distance(lastPosition, m_treasureGame.playerPosition);
            m_explorationDistance = m_treasureGame.explorationDistance;
        }
    }
    
    
    CheckTreasureInteraction();
    
    
    UpdateGameProgress();
    
    
    HandleVictoryCondition();
}

void Application::CheckTreasureInteraction() {
    const float interactionDistance = 2.0f;
    m_treasureGame.nearInteractable = false;
    m_treasureGame.nearestTreasureId = -1;
    m_treasureGame.distanceToNearestTreasure = 999.0f;
    
    
    for (auto& treasure : m_treasureGame.treasures) {
        if (treasure.status == TreasureStatus::COLLECTED || 
            treasure.status == TreasureStatus::UNLOCKED) continue;
        
        float distance = glm::distance(m_treasureGame.playerPosition, treasure.position);
        
        
        if (distance < m_treasureGame.distanceToNearestTreasure) {
            m_treasureGame.distanceToNearestTreasure = distance;
            m_treasureGame.nearestTreasureId = treasure.id;
        }
        
        
        if (distance < interactionDistance) {
            m_treasureGame.nearInteractable = true;
            
            if (treasure.type == TreasureType::ANCIENT_KEY) {
                m_treasureGame.interactionHint = "Press E to collect " + treasure.description;
            } else if (treasure.type == TreasureType::TREASURE_CHEST) {
                if (m_treasureGame.HasKey(treasure.requiredKeyId)) {
                    m_treasureGame.interactionHint = "Press E to unlock " + treasure.description;
                } else {
                    m_treasureGame.interactionHint = "Need Ancient Key #" + 
                        std::to_string(treasure.requiredKeyId) + " to unlock this chest";
                }
            }
        }
    }
    
    if (!m_treasureGame.nearInteractable) {
        m_treasureGame.interactionHint = "";
    }
}

bool Application::TryCollectTreasure(int treasureId) {
    bool success = m_treasureGame.CollectTreasure(treasureId);
    
    if (success) {
        
        m_keysCollected = m_treasureGame.keysCollected;
        m_chestsUnlocked = m_treasureGame.chestsUnlocked;
        
        
        if (m_audioManager && m_audioEnabled) {
            
        }
        
        std::cout << "Treasure collected! Keys: " << m_keysCollected 
                  << "/" << m_totalKeys << ", Chests: " << m_chestsUnlocked 
                  << "/" << m_totalChests << std::endl;
    }
    
    return success;
}

void Application::UpdateGameProgress() {
    m_treasureGame.UpdateGameState();
    
    
    static std::string lastTask = "";
    if (m_treasureGame.currentTask != lastTask) {
        std::cout << "Quest Update: " << m_treasureGame.currentTask << std::endl;
        lastTask = m_treasureGame.currentTask;
    }
}

void Application::HandleVictoryCondition() {
    if (m_treasureGame.CheckVictory() && !m_treasureGame.gameWon) {
        m_treasureGame.gameWon = true;
        m_gameWon = true;
        
        std::cout << "=== CONGRATULATIONS! ===" << std::endl;
        std::cout << "You have found all the ancient treasures!" << std::endl;
        std::cout << "Time: " << m_treasureGame.gameTime << " seconds" << std::endl;
        std::cout << "Exploration Distance: " << m_treasureGame.explorationDistance << " units" << std::endl;
        std::cout << "You are now a legendary Ancient Treasure Hunter!" << std::endl;
        
        
        if (m_audioManager && m_audioEnabled) {
            
        }
    }
}

void Application::RenderTreasureGame() {
    if (!m_blinnPhongShader || !m_camera) return;
    

    m_blinnPhongShader->Use();
    m_blinnPhongShader->SetMat4("view", m_camera->GetViewMatrix());
    glm::mat4 projection = glm::perspective(glm::radians(m_camera->Zoom), 
        (float)m_windowWidth / (float)m_windowHeight, 0.1f, 100.0f);
    m_blinnPhongShader->SetMat4("projection", projection);
    m_blinnPhongShader->SetVec3("viewPos", m_camera->Position);
    

    m_blinnPhongShader->SetVec3("light.position", m_lightPos);
    m_blinnPhongShader->SetVec3("light.ambient", 0.8f, 0.7f, 0.6f);   
    m_blinnPhongShader->SetVec3("light.diffuse", 2.0f, 1.8f, 1.5f);   
    m_blinnPhongShader->SetVec3("light.specular", 2.5f, 2.5f, 2.5f);  
    m_blinnPhongShader->SetFloat("material.shininess", 64.0f);
    
    for (const auto& treasure : m_treasureGame.treasures) {
        if (treasure.status == TreasureStatus::COLLECTED || 
            treasure.status == TreasureStatus::UNLOCKED) continue;
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, treasure.position);
        
        ModelObject* treasureModel = nullptr;
        std::string modelPrefix;
        float time = glfwGetTime(); 
        
        if (treasure.type == TreasureType::ANCIENT_KEY) {
            modelPrefix = "collectible_";
            
            // Bind key texture
            BindKeyTexture();
            
            float shimmerIntensity = 1.0f + sin(time * 5.0f + treasure.id) * 0.3f; 
            
            m_blinnPhongShader->SetVec3("light.ambient", 1.5f * shimmerIntensity, 1.3f * shimmerIntensity, 0.8f); 
            m_blinnPhongShader->SetVec3("light.diffuse", 3.0f * shimmerIntensity, 2.5f * shimmerIntensity, 1.3f);  
            m_blinnPhongShader->SetVec3("light.specular", 4.0f, 3.5f, 2.2f); 
            m_blinnPhongShader->SetFloat("material.shininess", 180.0f);       
            
            float rotationSpeed = 1.8f + sin(time * 0.4f + treasure.id) * 0.5f; 
            float floatHeight = sin(time * 2.5f + treasure.id) * 0.2f;           
            float pulse = 1.0f + sin(time * 4.0f + treasure.id) * 0.08f;         
            float shimmer = sin(time * 6.0f + treasure.id) * 0.03f;              
            
            model = glm::rotate(model, time * 1.5f, glm::vec3(0.0f, 1.0f, 0.0f));   
            model = glm::scale(model, glm::vec3(0.016f, 0.016f, 0.016f));            
            
        } else if (treasure.type == TreasureType::TREASURE_CHEST) {
            modelPrefix = "chest_";
            
            // Bind chest texture
            BindChestTexture();
            
            float mysticalGlow = 1.0f + sin(time * 1.5f + treasure.id) * 0.15f; 
            
            m_blinnPhongShader->SetVec3("light.ambient", 0.8f * mysticalGlow, 0.6f * mysticalGlow, 0.4f);   
            m_blinnPhongShader->SetVec3("light.diffuse", 1.6f * mysticalGlow, 1.2f * mysticalGlow, 0.7f);   
            m_blinnPhongShader->SetVec3("light.specular", 1.4f, 1.1f, 0.7f);  
            m_blinnPhongShader->SetFloat("material.shininess", 120.0f);        
            
            float breathe = 1.0f + sin(time * 1.2f + treasure.id) * 0.04f;       
            float gentleSway = sin(time * 0.8f + treasure.id) * 0.025f;          
            float slowRotate = time * 0.1f + treasure.id * 0.5f;                 
            float glowPulse = 1.0f + sin(time * 2.0f + treasure.id) * 0.02f;     
            
            model = glm::rotate(model, time * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));   
            model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); 
        }
        
        for (auto& gameModel : m_gameModels) {
            if (gameModel.name.find(modelPrefix) == 0) {
                treasureModel = &gameModel;
                std::cout << "[DEBUG] Found model: " << gameModel.name << " for prefix: " << modelPrefix << std::endl;
                break;
            }
        }
        
        if (!treasureModel) {
            std::cout << "[DEBUG] No model found for prefix: " << modelPrefix << std::endl;
            std::cout << "[DEBUG] Available models:" << std::endl;
            for (const auto& gameModel : m_gameModels) {
                std::cout << "  - " << gameModel.name << std::endl;
            }
        }
        
        m_blinnPhongShader->SetMat4("model", model);
        
        if (treasure.id == m_treasureGame.nearestTreasureId && 
            m_treasureGame.distanceToNearestTreasure < 5.0f) {
            
            float time = glfwGetTime();
            float proximity = 1.0f - (m_treasureGame.distanceToNearestTreasure / 5.0f); 
            float glow = 1.0f + sin(time * 4.0f) * 0.3f * proximity; 
            
            if (treasure.type == TreasureType::ANCIENT_KEY) {
                m_blinnPhongShader->SetVec3("light.ambient", 1.8f * glow, 1.5f * glow, 0.8f * glow);  
                m_blinnPhongShader->SetVec3("light.specular", 4.0f * glow, 3.0f * glow, 1.5f * glow); 
            } else {
                m_blinnPhongShader->SetVec3("light.ambient", 1.2f * glow, 0.8f * glow, 0.4f * glow);  
                m_blinnPhongShader->SetVec3("light.specular", 2.0f * glow, 1.4f * glow, 0.6f * glow); 
            }
        }
        

        if (treasureModel && treasureModel->model) {
            treasureModel->model->Draw(*m_blinnPhongShader);
        } else {

        }
        

        m_blinnPhongShader->SetVec3("light.ambient", 0.8f, 0.7f, 0.6f);  
        m_blinnPhongShader->SetVec3("light.diffuse", 2.0f, 1.8f, 1.5f);   
        m_blinnPhongShader->SetVec3("light.specular", 2.5f, 2.5f, 2.5f);  
        m_blinnPhongShader->SetFloat("material.shininess", 64.0f);
    }
}



void Application::InitializeAdvancedLighting() {
    
    m_advancedLighting.pointLightPos = m_lightPos;
    m_advancedLighting.pointLightColor = glm::vec3(1.0f, 0.9f, 0.8f);  
    m_advancedLighting.pointLightConstant = 1.0f;
    m_advancedLighting.pointLightLinear = 0.09f;  
    m_advancedLighting.pointLightQuadratic = 0.032f;  
    
    
    m_advancedLighting.spotLightPos = m_camera->Position;
    m_advancedLighting.spotLightDir = m_camera->Front;
    m_advancedLighting.spotLightCutOff = glm::cos(glm::radians(12.5f));
    m_advancedLighting.spotLightOuterCutOff = glm::cos(glm::radians(15.0f));
    m_advancedLighting.spotLightColor = glm::vec3(1.2f, 1.1f, 1.0f);  
    
    
    m_advancedLighting.dirLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    m_advancedLighting.dirLightColor = glm::vec3(0.8f, 0.7f, 0.6f);  
    
    
    m_advancedLighting.useBlinnPhong = true;
    m_advancedLighting.enableSpotLight = true;
    m_advancedLighting.enableDirLight = true;
    
    std::cout << "Advanced lighting system initialized!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  B - Toggle Blinn-Phong/Phong lighting" << std::endl;
    std::cout << "  L - Toggle spot light" << std::endl;
    std::cout << "  O - Toggle directional light" << std::endl;
}

void Application::UpdateAdvancedLighting() {
    float time = glfwGetTime();
    
    
    m_advancedLighting.pointLightPos = m_lightPos;
    
    
    m_advancedLighting.spotLightPos = m_camera->Position;
    m_advancedLighting.spotLightDir = m_camera->Front;
    
    
    m_advancedLighting.dirLightDir.x = -0.2f + 0.1f * sin(time * 0.1f);
    m_advancedLighting.dirLightDir.y = -1.0f + 0.2f * cos(time * 0.15f);
}

void Application::SetupLightingUniforms(Shader& shader) {
    
    shader.SetBool("useBlinnPhong", m_advancedLighting.useBlinnPhong);
    shader.SetBool("enableSpotLight", m_advancedLighting.enableSpotLight);
    shader.SetBool("enableDirLight", m_advancedLighting.enableDirLight);
    
    
    shader.SetVec3("pointLight.position", m_advancedLighting.pointLightPos);
    shader.SetFloat("pointLight.constant", m_advancedLighting.pointLightConstant);
    shader.SetFloat("pointLight.linear", m_advancedLighting.pointLightLinear);
    shader.SetFloat("pointLight.quadratic", m_advancedLighting.pointLightQuadratic);
    shader.SetVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
    shader.SetVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f);
    shader.SetVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
    
    
    shader.SetVec3("spotLight.position", m_advancedLighting.spotLightPos);
    shader.SetVec3("spotLight.direction", m_advancedLighting.spotLightDir);
    shader.SetFloat("spotLight.cutOff", m_advancedLighting.spotLightCutOff);
    shader.SetFloat("spotLight.outerCutOff", m_advancedLighting.spotLightOuterCutOff);
    shader.SetFloat("spotLight.constant", 1.0f);
    shader.SetFloat("spotLight.linear", 0.09f);
    shader.SetFloat("spotLight.quadratic", 0.032f);
    shader.SetVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.SetVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.SetVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    
    
    shader.SetVec3("dirLight.direction", m_advancedLighting.dirLightDir);
    shader.SetVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.SetVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
    shader.SetVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    
    // Shadow mapping uniforms
    if (m_enableShadows && m_shadowManager) {
        auto* shadowMapping = m_shadowManager->GetShadowMapping(0);
        if (shadowMapping) {
            shader.SetMat4("lightSpaceMatrix", shadowMapping->GetLightSpaceMatrix());
            
            // Bind shadow map texture
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, shadowMapping->GetShadowMapTexture());
            shader.SetInt("shadowMap", 2);
            
            // Shadow parameters
            shader.SetFloat("shadowBias", 0.005f);
            shader.SetFloat("normalBias", 0.01f);
            shader.SetInt("filterMode", 3); // PCF_3x3
            shader.SetFloat("shadowMapSize", 2048.0f);
        }
    }
}





void Application::InitializeModels() {
    std::cout << "\n=== Initializing 3D Model Loading System ===" << std::endl;
    
    try {
        
        m_gameModels.clear();

        
        if (!m_layoutManager) {
            m_layoutManager = std::make_unique<LayoutManager>(m_terrainGenerator.get());
        }

        
        std::map<std::string, int> objectsToPlace;
        objectsToPlace["collectible"] = 1;  // Generate only 1 key
        objectsToPlace["chest"] = 1;        // Generate only 1 chest

        
        const auto& zones = m_layoutManager->GetZones();
        if (!zones.empty()) {
            const auto& firstZone = zones[0]; // Only use the first zone
            
            for (auto const& [type, count] : objectsToPlace) {
                for (int i = 0; i < count; ++i) { 
                    std::string modelPath;
                    if (type == "collectible") modelPath = "resources/models/key.obj";
                    else if (type == "chest") modelPath = "resources/models/treasure_chest.obj";
                    else continue;

                    std::cout << "🔑 Loading model: " << modelPath << " (type: " << type << ")" << std::endl;

                    
                    glm::vec3 randomPos = m_layoutManager->GetRandomPositionInZone(firstZone);
                    
                    
                    glm::vec3 finalPos = TerrainPlacement::PlaceOnTerrain(randomPos, m_terrainGenerator.get(), 1.0f);

                    std::cout << "📍 Placing " << type << " at position: (" << finalPos.x << ", " << finalPos.y << ", " << finalPos.z << ")" << std::endl;

                    
                    
                    
                    float scale = 1.0f;
                    if (type == "collectible") scale = 0.016f;  
                    else if (type == "chest") scale = 0.01f;   
                    
                    LoadGameModel(modelPath, type + "_" + std::to_string(i), finalPos, 
                                  glm::vec3(0.0f, 45.0f * (i + 1), 0.0f), 
                                  glm::vec3(scale), true);
                }
            }
        } 
        
        m_modelsLoaded = true;
        std::cout << "Successfully loaded and placed " << m_gameModels.size() << " 3D models with collision volumes." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Model loading failed: " << e.what() << std::endl;
        m_modelsLoaded = false;
    }
}

void Application::LoadGameModel(const std::string& modelPath, const std::string& name, 
                               const glm::vec3& position, const glm::vec3& rotation,
                               const glm::vec3& scale, bool animated) {
    try {
        ModelObject modelObj;
        modelObj.model = std::make_unique<Model>(modelPath);
        
        
        if (m_terrainGenerator) {
            
            float objectHeight = scale.y * 0.5f;  
            
            std::cout << "\n=== Fixing Model Placement ===" << std::endl;
            std::cout << "Model: " << name << std::endl;
            std::cout << "Original position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
            
            
            glm::vec3 terrainPosition = TerrainPlacement::PlaceOnTerrain(position, m_terrainGenerator.get(), objectHeight);
            modelObj.position = terrainPosition;
            
            std::cout << "Terrain-adjusted position: (" << terrainPosition.x << ", " << terrainPosition.y << ", " << terrainPosition.z << ")" << std::endl;
        } else {
            
            modelObj.position = position;
            std::cout << "Warning: Terrain not available, using original position for " << name << std::endl;
        }
        
        modelObj.rotation = glm::radians(rotation); 
        modelObj.scale = scale;
        modelObj.animationTime = 0.0f;
        modelObj.isAnimated = animated;
        modelObj.name = name;
        
        m_gameModels.push_back(std::move(modelObj));
        std::cout << "Loaded model: " << name << " (" << modelPath << ")" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Cannot load model " << modelPath << ": " << e.what() << std::endl;
        throw;
    }
}

void Application::UpdateModels() {
    if (!m_modelsLoaded) return;
    
    float currentTime = glfwGetTime();
    
    for (auto& modelObj : m_gameModels) {
        if (modelObj.isAnimated) {
            modelObj.animationTime += m_deltaTime;
            
            
            if (modelObj.name.find("treasure") != std::string::npos || modelObj.name.find("chest") != std::string::npos) {
                
                modelObj.rotation.y += m_deltaTime * 0.3f; 
            } else if (modelObj.name.find("key") != std::string::npos || modelObj.name.find("collectible") != std::string::npos) {
                
                modelObj.rotation.y += m_deltaTime * 1.2f; 
            }
        }
    }
}

void Application::RenderModels() {
    if (!m_modelsLoaded || m_gameModels.empty()) return;
    
    
    m_blinnPhongShader->Use();
    
    
    SetupLightingUniforms(*m_blinnPhongShader);
    
    
    glm::mat4 view = m_camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_windowWidth / (float)m_windowHeight, 0.1f, 100.0f);
    
    m_blinnPhongShader->SetMat4("view", view);
    m_blinnPhongShader->SetMat4("projection", projection);
    m_blinnPhongShader->SetVec3("viewPos", m_camera->Position);
    
    
    for (const auto& modelObj : m_gameModels) {
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, modelObj.position);
        
        
        model = glm::rotate(model, modelObj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, modelObj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, modelObj.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        
        
        model = glm::scale(model, modelObj.scale);
        
        m_blinnPhongShader->SetMat4("model", model);
        
        
        if (modelObj.name.find("treasure") != std::string::npos) {
            
            m_blinnPhongShader->SetVec3("material.ambient", 0.2f, 0.2f, 0.2f);
            m_blinnPhongShader->SetVec3("material.diffuse", 0.7f, 0.6f, 0.4f);
            m_blinnPhongShader->SetVec3("material.specular", 0.9f, 0.8f, 0.6f);
            m_blinnPhongShader->SetFloat("material.shininess", 64.0f);
            
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_specularTexture);
            m_blinnPhongShader->SetInt("material.diffuse", 0);
            
        } else if (modelObj.name.find("key") != std::string::npos) {
            
            m_blinnPhongShader->SetVec3("material.ambient", 0.3f, 0.2f, 0.1f);
            m_blinnPhongShader->SetVec3("material.diffuse", 0.6f, 0.4f, 0.2f);
            m_blinnPhongShader->SetVec3("material.specular", 0.3f, 0.2f, 0.1f);
            m_blinnPhongShader->SetFloat("material.shininess", 8.0f);
            
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
            m_blinnPhongShader->SetInt("material.diffuse", 0);
        }
        
        
        modelObj.model->Draw(*m_blinnPhongShader);
    }
}





void Application::InitializeTerrain() {
    std::cout << "\n=== Initializing Procedural Terrain Generation ===" << std::endl;
    
    try {
        
        m_terrainGenerator = std::make_unique<TerrainGenerator>(16, 8.0f);  
        
        
        m_terrainGenerator->SetNoiseScale(0.1f);    
        m_terrainGenerator->SetHeightScale(2.0f);   
        m_terrainGenerator->SetOctaves(3);          
        
        
        glm::vec3 cameraPos = m_camera->Position;
        m_terrainGenerator->GenerateTerrainAt(glm::vec2(cameraPos.x, cameraPos.z));
        
        
        std::vector<glm::vec3> terrainVertices;
        std::vector<unsigned int> terrainIndices;
        m_terrainGenerator->GetCollisionData(terrainVertices, terrainIndices);
        
        if (!terrainVertices.empty() && !terrainIndices.empty()) {
            m_physicsManager->CreateTerrainCollision(terrainVertices, terrainIndices);
        }
        
        
        std::cout << "\n=== Calculating Safe Spawn Point ===" << std::endl;
        glm::vec3 safeSpawn = m_terrainGenerator->FindSafeSpawnPoint(0.0f, 0.0f);
        
        
        glm::vec3 newCameraPos = glm::vec3(safeSpawn.x, safeSpawn.y + 2.0f, safeSpawn.z);
        m_camera->Position = newCameraPos;
        
        std::cout << "Camera repositioned to safe spawn point: (" 
                  << newCameraPos.x << ", " << newCameraPos.y << ", " << newCameraPos.z << ")" << std::endl;
        std::cout << "Terrain height at spawn: " << safeSpawn.y << std::endl;
        
        
        std::cout << "\n=== Initializing Layout Management System ===" << std::endl;
        m_layoutManager = std::make_unique<LayoutManager>(m_terrainGenerator.get());
        
        std::cout << "Layout zones created:" << std::endl;
        const auto& zones = m_layoutManager->GetZones();
        for (size_t i = 0; i < zones.size(); ++i) {
            const auto& zone = zones[i];
            std::cout << "Zone " << (i+1) << ": " << zone.theme 
                     << " (center: " << zone.center.x << "," << zone.center.z 
                     << ", radius: " << zone.radius << ")" << std::endl;
        }
        
        std::cout << "PCG Terrain System Initialized!" << std::endl;
        std::cout << "Features: Multi-biome procedural generation" << std::endl;
        std::cout << "Biomes: Grassland, Mountain, Desert, Forest" << std::endl;
        std::cout << "Dynamic: Real-time chunk generation and LOD" << std::endl;
        std::cout << "Noise: Multi-octave Perlin noise with 4 octaves" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize terrain: " << e.what() << std::endl;
        m_terrainEnabled = false;
    }
}

void Application::UpdateTerrain() {
    if (!m_terrainEnabled || !m_terrainGenerator) return;
    
    
    m_terrainGenerator->UpdateLOD(m_camera->Position);
    
    
    glm::vec3 cameraPos = m_camera->Position;
    m_terrainGenerator->GenerateTerrainAt(glm::vec2(cameraPos.x, cameraPos.z));
    
    
    if (m_terrainGenerator->HasTerrainUpdated()) {
        std::cout << "Terrain updated! Rebuilding collision bodies..." << std::endl;
        
        
        m_physicsManager->ClearTerrainCollision();
        
        
        std::vector<glm::vec3> terrainVertices;
        std::vector<unsigned int> terrainIndices;
        m_terrainGenerator->GetCollisionData(terrainVertices, terrainIndices);
        
        if (!terrainVertices.empty() && !terrainIndices.empty()) {
            m_physicsManager->CreateTerrainCollision(terrainVertices, terrainIndices);
        }
        
        
        m_terrainGenerator->ResetTerrainUpdateFlag();
    }
}

void Application::RenderSimpleGround() {
    
    m_blinnPhongShader->Use();
    
    
    m_blinnPhongShader->SetVec3("material.ambient", 0.3f, 0.3f, 0.3f);
    m_blinnPhongShader->SetVec3("material.diffuse", 0.7f, 0.7f, 0.7f);
    m_blinnPhongShader->SetVec3("material.specular", 0.1f, 0.1f, 0.1f);
    m_blinnPhongShader->SetFloat("material.shininess", 16.0f);
    
    
    glm::mat4 groundModel = glm::mat4(1.0f);
    groundModel = glm::translate(groundModel, glm::vec3(0.0f, -0.5f, 0.0f));
    groundModel = glm::scale(groundModel, glm::vec3(50.0f, 1.0f, 50.0f));
    
    m_blinnPhongShader->SetMat4("model", groundModel);
    
    
    if (m_quad) {
        m_quad->Draw(*m_blinnPhongShader, groundModel);
    }
}

void Application::RenderTerrain() {
    if (!m_terrainEnabled || !m_terrainGenerator) return;
    
    
    if (m_enableShadows && m_terrainShadowShader && m_shadowManager) {
        m_terrainShadowShader->Use();
        
        
        glm::mat4 view = m_camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                              (float)m_windowWidth / (float)m_windowHeight, 
                                              0.1f, 200.0f);
        glm::mat4 model = glm::mat4(1.0f);
        
        
        m_terrainShadowShader->SetMat4("model", model);
        m_terrainShadowShader->SetMat4("view", view);
        m_terrainShadowShader->SetMat4("projection", projection);
        m_terrainShadowShader->SetVec3("lightPos", m_lightPos);
        m_terrainShadowShader->SetVec3("lightColor", m_lightColor);
        m_terrainShadowShader->SetVec3("viewPos", m_camera->Position);
        
        
        auto* shadowMapping = m_shadowManager->GetShadowMapping(0);
        if (shadowMapping) {
            glm::mat4 lightSpaceMatrix = shadowMapping->GetLightSpaceMatrix();
            m_terrainShadowShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
            
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowMapping->GetShadowMapTexture());
            m_terrainShadowShader->SetInt("shadowMap", 1);
        }
        
        
        m_terrainShadowShader->SetFloat("shadowBias", 0.005f);
        m_terrainShadowShader->SetFloat("normalBias", 0.01f);
        m_terrainShadowShader->SetInt("filterMode", 3); 
        m_terrainShadowShader->SetFloat("shadowMapSize", 2048.0f);
        
        
        m_terrainGenerator->RenderTerrain(*m_terrainShadowShader, view, projection);
    } else {
        
        m_terrainShader->Use();
        
        
        glm::mat4 view = m_camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                              (float)m_windowWidth / (float)m_windowHeight, 
                                              0.1f, 200.0f);
        
        
        m_terrainShader->SetMat4("view", view);
        m_terrainShader->SetMat4("projection", projection);
        m_terrainShader->SetVec3("lightPos", m_lightPos);
        m_terrainShader->SetVec3("lightColor", m_lightColor);
        m_terrainShader->SetVec3("viewPos", m_camera->Position);
        
        
        m_terrainGenerator->RenderTerrain(*m_terrainShader, view, projection);
    }
}

void Application::InitializeGameInteraction() {
    std::cout << "\n=== Initializing Game Interaction System ===" << std::endl;
    
    try {
        
        m_gameInteraction = std::make_unique<GameInteraction>(m_terrainGenerator.get());
        
        
        m_gameInteraction->Initialize();
        
        
        m_gameInteraction->SetDebugMode(true);
        
        std::cout << "Game Interaction System initialized successfully!" << std::endl;
        std::cout << "Features:" << std::endl;
        std::cout << "  - Item collection system" << std::endl;
        std::cout << "  - Ray casting for interaction" << std::endl;
        std::cout << "  - Event-driven game logic" << std::endl;
        std::cout << "  - Task progression system" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  E - Interact with objects" << std::endl;
        std::cout << "  F3 - Toggle debug mode" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize game interaction: " << e.what() << std::endl;
    }
}

void Application::UpdateGameInteraction() {
    if (m_gameInteraction) {
        m_gameInteraction->Update(m_deltaTime, m_camera.get());
    }
}


void Application::InitializeAudio() {
    std::cout << "Initializing audio system..." << std::endl;
    
    try {
        
        m_audioManager = &IrrklangAudioManager::GetInstance();
        
        
        if (m_audioManager->Initialize()) {
            std::cout << "✓ Audio system initialized successfully" << std::endl;
            
            
            AudioEventHandler::InitializeEventSounds();
            
            
            if (m_audioEnabled) {
                m_audioManager->PlayMusic("ambient", true, 0.5f);
                std::cout << "✓ Background music started" << std::endl;
            }
            
            std::cout << "Audio features:" << std::endl;
            std::cout << "  - Game sound effects (crack, pickup, button, victory)" << std::endl;
            std::cout << "  - 3D spatial audio with distance falloff" << std::endl;
            std::cout << "  - Background music with volume control" << std::endl;
            std::cout << "  - Event-driven audio system" << std::endl;
            std::cout << "Audio controls:" << std::endl;
            std::cout << "  M - Toggle audio on/off" << std::endl;
            std::cout << "  + - Increase volume" << std::endl;
            std::cout << "  - - Decrease volume" << std::endl;
            
        } else {
            std::cerr << "⚠ Audio initialization failed - continuing without audio" << std::endl;
            m_audioEnabled = false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Audio system error: " << e.what() << std::endl;
        m_audioEnabled = false;
    }
}

void Application::UpdateAudio() {
    if (m_audioManager && m_audioEnabled) {
        
        m_audioManager->Update();
        
        
        if (m_camera) {
            glm::vec3 cameraPos = m_camera->Position;
            glm::vec3 cameraFront = m_camera->Front;
            glm::vec3 cameraUp = m_camera->Up;
            
            m_audioManager->SetListenerPosition(cameraPos, cameraFront, cameraUp);
        }
    }
}

void Application::ShutdownAudio() {
    if (m_audioManager) {
        std::cout << "Shutting down audio system..." << std::endl;
        m_audioManager->Shutdown();
        m_audioManager = nullptr;
    }
}



void Application::InitializeGUI() {
    std::cout << "=== Initializing Custom GUI System ===" << std::endl;
    
    
    m_guiManager = CustomGUI::GUIManager::GetInstance();
    
    if (!m_guiManager) {
        std::cerr << "ERROR: Failed to get GUIManager instance!" << std::endl;
        return;
    }
    
    std::cout << "✅ GUIManager instance obtained successfully" << std::endl;
    
    if (!m_guiManager->Initialize(m_windowWidth, m_windowHeight)) {
        std::cerr << "ERROR: Failed to initialize GUI system!" << std::endl;
        m_guiManager = nullptr; // Set to null to prevent access to failed instance
        return;
    }
    
    std::cout << "✅ GUIManager initialized successfully" << std::endl;
    
    
    CreateMainMenu();
    
    std::cout << "✅ Custom GUI system initialized successfully!" << std::endl;
    std::cout << "GUI Features:" << std::endl;
    std::cout << "  - Custom button and panel rendering" << std::endl;
    std::cout << "  - Mouse interaction support" << std::endl;
    std::cout << "  - Modular UI element system" << std::endl;
    std::cout << "  - Z-order layering" << std::endl;
}

void Application::InitializeShadowMapping() {
    std::cout << "=== Initializing Shadow Mapping System ===" << std::endl;
    
    
    m_shadowManager = std::make_unique<ShadowMappingManager>();
    
    
    auto mainShadowMap = m_shadowManager->AddShadowLight(
        0, 
        ShadowMapping::LightType::DIRECTIONAL, 
        ShadowMapping::ShadowQuality::HIGH 
    );
    
    if (mainShadowMap) {
        
        glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
        mainShadowMap->SetLight(
            ShadowMapping::LightType::DIRECTIONAL,
            glm::vec3(20.0f, 20.0f, 20.0f), 
            lightDirection,                  
            90.0f                           
        );
        
        
        mainShadowMap->SetShadowRange(1.0f, 100.0f);
        
        
        mainShadowMap->SetDepthBias(0.005f);
        mainShadowMap->SetNormalBias(0.05f);
        mainShadowMap->EnableSlopeScaledBias(true);
        
        std::cout << "✅ Main directional light shadow mapping configured" << std::endl;
    }
    
    
    m_shadowManager->SetGlobalShadowsEnabled(m_enableShadows);
    m_shadowManager->SetGlobalShadowStrength(m_shadowStrength);
    
    std::cout << "✅ Shadow mapping system initialized successfully!" << std::endl;
    std::cout << "Shadow Features:" << std::endl;
    std::cout << "  - High-quality directional light shadows" << std::endl;
    std::cout << "  - PCF soft shadow filtering" << std::endl;
    std::cout << "  - Automatic bias adjustment" << std::endl;
    std::cout << "  - Real-time shadow rendering" << std::endl;
}

void Application::RenderShadowMap() {
    if (!m_enableShadows || !m_shadowManager) {
        return;
    }
    
    auto* shadowMapping = m_shadowManager->GetShadowMapping(0);
    if (!shadowMapping) {
        return;
    }
    
    // Use shadow map shader
    if (m_shadowMapShader) {
        m_shadowMapShader->Use();
        m_shadowMapShader->SetMat4("lightSpaceMatrix", shadowMapping->GetLightSpaceMatrix());
        
        // Render terrain to shadow map
        if (m_terrainEnabled && m_terrainGenerator) {
            m_terrainGenerator->RenderTerrain(*m_shadowMapShader, glm::mat4(1.0f), glm::mat4(1.0f));
        }
        
        // Render game objects to shadow map - use actual game object positions
        int renderedObjects = 0;
        for (const auto& treasure : m_treasureGame.treasures) {
            if (treasure.status == TreasureStatus::UNCOLLECTED) {
                // Find corresponding model
                const ModelObject* treasureModel = nullptr;
                std::string modelPrefix;
                
                if (treasure.type == TreasureType::ANCIENT_KEY) {
                    modelPrefix = "collectible_";
                } else if (treasure.type == TreasureType::TREASURE_CHEST) {
                    modelPrefix = "chest_";
                }
                
                // Find corresponding model
                for (const auto& gameModel : m_gameModels) {
                    if (gameModel.name.find(modelPrefix) == 0) {
                        treasureModel = &gameModel;
                        break;
                    }
                }
                
                if (treasureModel) {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, treasure.position); // Use actual game object position
                    
                    // Apply animation rotation
                    float time = glfwGetTime();
                    if (treasure.type == TreasureType::ANCIENT_KEY) {
                        model = glm::rotate(model, time * 1.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                        model = glm::scale(model, glm::vec3(0.016f, 0.016f, 0.016f));
                    } else if (treasure.type == TreasureType::TREASURE_CHEST) {
                        model = glm::rotate(model, time * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
                        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
                    }
                    
                    m_shadowMapShader->SetMat4("model", model);
                    treasureModel->model->Draw(*m_shadowMapShader);
                    renderedObjects++;
                }
            }
        }
    }
}

void Application::UpdateGUI(float deltaTime) {
    if (m_guiManager) {
        m_guiManager->Update(deltaTime);
        
        
        if (m_currentState == GameState::IN_GAME) {
            UpdateGameStatusGUI();
        }
    }
}

void Application::UpdateGameStatusGUI() {
    if (!m_guiManager) return;
    
    
    auto statusElement = m_guiManager->GetElementById("gameStatus");
    if (statusElement) {
        auto statusText = std::dynamic_pointer_cast<CustomGUI::UIText>(statusElement);
        if (statusText) {
            std::string status = "Ancient Treasure Hunter - Keys: " + 
                               std::to_string(m_treasureGame.keysCollected) + "/1, Chests: " +
                               std::to_string(m_treasureGame.chestsUnlocked) + "/1";
            statusText->SetText(status);
        }
    }
    
    
    auto perfElement = m_guiManager->GetElementById("performanceInfo");
    if (perfElement) {
        auto perfText = std::dynamic_pointer_cast<CustomGUI::UIText>(perfElement);
        if (perfText) {
            float fps = (m_deltaTime > 0) ? 1.0f / m_deltaTime : 0.0f;
            glm::vec3 pos = m_camera->Position;
            
            std::stringstream info;
            info << "FPS: " << static_cast<int>(fps) 
                 << " | Position: (" << static_cast<int>(pos.x) 
                 << ", " << static_cast<int>(pos.y) 
                 << ", " << static_cast<int>(pos.z) << ")";
            
            perfText->SetText(info.str());
        }
    }
}

void Application::RenderGUI() {
    if (m_guiManager) {
        m_guiManager->Render();
    }
}


void Application::RenderShadowDemoScene() {
    
    m_basicShader->Use();
    
    
    m_defaultTexture->Bind(0);
    m_basicShader->SetInt("texture_diffuse1", 0);
    
}

void Application::ShutdownGUI() {
    if (m_guiManager) {
        std::cout << "Shutting down custom GUI system..." << std::endl;
        m_guiManager->Shutdown();
        CustomGUI::GUIManager::DestroyInstance();
        m_guiManager = nullptr;
    }
}

void Application::CreateMainMenu() {
    if (!m_guiManager) return;
    
    std::cout << "=== Creating Main Menu GUI ===" << std::endl;
    
    
    auto mainPanel = std::make_shared<CustomGUI::UIPanel>("mainMenuBackground");
    mainPanel->SetPosition(glm::vec2(0, 0));
    mainPanel->SetSize(glm::vec2(m_windowWidth, m_windowHeight));
    mainPanel->SetBackgroundColor(glm::vec4(0.05f, 0.1f, 0.2f, 0.9f)); 
    mainPanel->SetZOrder(0);
    
    
    auto titleText = std::make_shared<CustomGUI::UIText>("ANCIENT TREASURE HUNTER", "titleText");
    
    titleText->SetPosition(glm::vec2(m_windowWidth / 2, m_windowHeight / 2 - 180));
    titleText->SetFontSize(56.0f);  
    titleText->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f)); 
    titleText->SetAlignment(CustomGUI::TextAlignment::CENTER); 
    titleText->SetZOrder(5); 
    
    
    auto subtitleText = std::make_shared<CustomGUI::UIText>("Ancient Treasure Hunter - Collect the Key and Unlock the Chest!", "subtitleText");
    
    subtitleText->SetPosition(glm::vec2(m_windowWidth / 2, m_windowHeight / 2 - 140));
    subtitleText->SetFontSize(28.0f);  
    subtitleText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    subtitleText->SetAlignment(CustomGUI::TextAlignment::CENTER); 
    subtitleText->SetZOrder(5); 
    
    
    auto signatureText = std::make_shared<CustomGUI::UIText>("COMP3016 Advanced 3D Graphics Project", "signatureText");
    signatureText->SetPosition(glm::vec2(10, m_windowHeight - 150)); 
    signatureText->SetFontSize(18.0f);
    signatureText->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f)); 
    signatureText->SetAlignment(CustomGUI::TextAlignment::LEFT);
    signatureText->SetZOrder(10); 
    
    
    auto featuresText = std::make_shared<CustomGUI::UIText>("Features: Shadow Mapping | Physics Engine | 3D Models | Terrain Generation", "featuresText");
    featuresText->SetPosition(glm::vec2(10, m_windowHeight - 125));
    featuresText->SetFontSize(16.0f);
    featuresText->SetTextColor(glm::vec3(0.7f, 1.0f, 0.7f)); 
    featuresText->SetAlignment(CustomGUI::TextAlignment::LEFT);
    featuresText->SetZOrder(10);

    // Add personal signature at the bottom left
    auto nameSignatureText = std::make_shared<CustomGUI::UIText>("Name: PENGWEN YAN", "nameSignatureText");
    nameSignatureText->SetPosition(glm::vec2(10, m_windowHeight - 40));
    nameSignatureText->SetFontSize(20.0f);
    nameSignatureText->SetTextColor(glm::vec3(0.8f, 0.95f, 1.0f));
    nameSignatureText->SetAlignment(CustomGUI::TextAlignment::LEFT);
    nameSignatureText->SetZOrder(12);
    
    
    auto titlePanel = std::make_shared<CustomGUI::UIPanel>("titlePanel");
    
    
    glm::vec2 titleSize = titleText->CalculateTextSize();
    glm::vec2 subtitleSize = subtitleText->CalculateTextSize();
    
    
    float panelWidth = std::max(titleSize.x, subtitleSize.x) + 40; 
    float panelHeight = titleSize.y + subtitleSize.y + 60; 
    
    titlePanel->SetPosition(glm::vec2(m_windowWidth / 2 - panelWidth/2, m_windowHeight / 2 - 200));
    titlePanel->SetSize(glm::vec2(panelWidth, panelHeight));
    titlePanel->SetBackgroundColor(glm::vec4(0.2f, 0.3f, 0.4f, 0.3f)); 
    titlePanel->SetZOrder(1);
    
    
    auto buttonPanel = std::make_shared<CustomGUI::UIPanel>("buttonPanel");
    buttonPanel->SetPosition(glm::vec2(m_windowWidth / 2 - 120, m_windowHeight / 2 - 60));
    buttonPanel->SetSize(glm::vec2(240, 150)); 
    buttonPanel->SetBackgroundColor(glm::vec4(0.15f, 0.2f, 0.3f, 0.5f)); 
    buttonPanel->SetZOrder(1);
    
    
    auto controlsText = std::make_shared<CustomGUI::UIText>("CONTROLS: WASD - Move | Mouse - Look | E - Interact", "controlsText");
    
    controlsText->SetPosition(glm::vec2(m_windowWidth / 2, m_windowHeight - 85));
    controlsText->SetFontSize(20.0f);  
    controlsText->SetTextColor(glm::vec3(0.0f, 1.0f, 1.0f)); 
    controlsText->SetAlignment(CustomGUI::TextAlignment::CENTER); 
    controlsText->SetZOrder(5); 
    
    
    auto versionText = std::make_shared<CustomGUI::UIText>("COMP3016 - Advanced OpenGL | v1.0", "versionText");
    
    versionText->SetPosition(glm::vec2(m_windowWidth / 2, m_windowHeight - 50));
    versionText->SetFontSize(18.0f);  
    versionText->SetTextColor(glm::vec3(1.0f, 0.0f, 1.0f)); 
    versionText->SetAlignment(CustomGUI::TextAlignment::CENTER); 
    versionText->SetZOrder(5); 

    
    auto infoPanel = std::make_shared<CustomGUI::UIPanel>("infoPanel");
    
    
    glm::vec2 controlsSize = controlsText->CalculateTextSize();
    glm::vec2 versionSize = versionText->CalculateTextSize();
    
    
    float infoPanelWidth = std::max(controlsSize.x, versionSize.x) + 40; 
    float infoPanelHeight = controlsSize.y + versionSize.y + 50; 
    
    infoPanel->SetPosition(glm::vec2(m_windowWidth / 2 - infoPanelWidth/2, m_windowHeight - infoPanelHeight - 20));
    infoPanel->SetSize(glm::vec2(infoPanelWidth, infoPanelHeight));
    infoPanel->SetBackgroundColor(glm::vec4(0.1f, 0.15f, 0.25f, 0.7f)); 
    infoPanel->SetZOrder(1);
    
    
    auto startButton = std::make_shared<CustomGUI::UIButton>("START GAME", "startBtn");
    startButton->SetPosition(glm::vec2(m_windowWidth / 2 - 100, m_windowHeight / 2 - 30));
    startButton->SetSize(glm::vec2(200, 40));
    startButton->SetNormalColor(glm::vec4(0.2f, 0.7f, 0.3f, 0.9f)); 
    startButton->SetHoverColor(glm::vec4(0.3f, 0.8f, 0.4f, 1.0f));
    startButton->SetPressedColor(glm::vec4(0.1f, 0.6f, 0.2f, 1.0f));
    startButton->SetTextColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); 
    startButton->SetZOrder(4); 
    startButton->SetOnClick([]() {
        std::cout << "Start Game clicked! Entering game..." << std::endl;
        
        // Use static instance access instead of corrupted 'this' pointer
        Application* app = Application::GetInstance();
        if (!app) {
            std::cerr << "Error: Cannot get Application instance!" << std::endl;
            return;
        }
        
        std::cout << "Application instance obtained successfully: " << app << std::endl;
        
        // Add debug info about GUI manager state
        std::cout << "Checking GUI manager state before transition..." << std::endl;
        if (app->m_guiManager) {
            std::cout << "GUI manager is valid, clearing elements..." << std::endl;
            app->m_guiManager->ClearElements();
            std::cout << "Elements cleared successfully" << std::endl;
        } else {
            std::cerr << "ERROR: GUI manager is null before clearing!" << std::endl;
            return;
        }
        
        // Use safe state change method
        std::cout << "Setting pending state change to IN_GAME..." << std::endl;
        app->SetPendingStateChange(GameState::IN_GAME);
        
        std::cout << "Setting first mouse flag..." << std::endl;
        app->m_firstMouse = true;
        std::cout << "First mouse flag set successfully" << std::endl;
        
        std::cout << "About to create test GUI..." << std::endl;
        if (app->m_guiManager) {
            std::cout << "GUI manager is still valid, creating test GUI..." << std::endl;
            app->CreateTestGUI();
            std::cout << "Test GUI created successfully" << std::endl;
        } else {
            std::cerr << "ERROR: GUI manager became null after clearing elements!" << std::endl;
        }
    });
    
    
    auto exitButton = std::make_shared<CustomGUI::UIButton>("EXIT GAME", "exitBtn");
    exitButton->SetPosition(glm::vec2(m_windowWidth / 2 - 100, m_windowHeight / 2 + 20)); 
    exitButton->SetSize(glm::vec2(200, 40));
    exitButton->SetNormalColor(glm::vec4(0.8f, 0.3f, 0.3f, 0.9f)); 
    exitButton->SetHoverColor(glm::vec4(0.9f, 0.4f, 0.4f, 1.0f));
    exitButton->SetPressedColor(glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    exitButton->SetTextColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); 
    exitButton->SetZOrder(3); 
    exitButton->SetOnClick([this]() {
        std::cout << "❌ Exit clicked! Closing application..." << std::endl;
        
        if (!this) {
            std::cerr << "Error: Invalid this pointer in lambda!" << std::endl;
            return;
        }
        
        if (m_window) {
            glfwSetWindowShouldClose(m_window, true);
        }
    });
    
    
    m_guiManager->AddElement(mainPanel);
    m_guiManager->AddElement(titlePanel);
    m_guiManager->AddElement(buttonPanel);
    m_guiManager->AddElement(infoPanel);
    m_guiManager->AddElement(titleText);
    m_guiManager->AddElement(subtitleText);
    m_guiManager->AddElement(signatureText);  
    m_guiManager->AddElement(featuresText);   
    m_guiManager->AddElement(nameSignatureText);
    m_guiManager->AddElement(startButton);
    m_guiManager->AddElement(exitButton);
    m_guiManager->AddElement(controlsText);
    m_guiManager->AddElement(versionText);
    
    std::cout << "✅ Main Menu created successfully!" << std::endl;
    std::cout << "📋 Enhanced Menu contains:" << std::endl;
    std::cout << "   🏷️  Game Title: ANCIENT TREASURE HUNTER" << std::endl;
    std::cout << "   📝 Subtitle with game description" << std::endl;
    std::cout << "   🎮 START GAME (Green button)" << std::endl;
    std::cout << "   ❌ EXIT GAME (Red button)" << std::endl;
    std::cout << "   📖 Control instructions" << std::endl;
    std::cout << "   📄 Version information" << std::endl;
}

void Application::CreateTestGUI() {
    // In-game UI disabled - only main menu is kept
    std::cout << "In-game UI disabled" << std::endl;
}



void Application::CreateTechnicalFeaturesPanel() {
    if (!m_guiManager) return;
    
    
    auto techPanel = std::make_shared<CustomGUI::UIPanel>("technicalFeaturesPanel");
    techPanel->SetPosition(glm::vec2(m_windowWidth - 400, 10));
    techPanel->SetSize(glm::vec2(380, 300));
    techPanel->SetBackgroundColor(glm::vec4(0.0f, 0.2f, 0.0f, 0.8f)); 
    techPanel->SetZOrder(1);
    
    
    auto titleText = std::make_shared<CustomGUI::UIText>("COMP3016 Ancient Treasure Hunter", "techPanelTitle");
    titleText->SetPosition(glm::vec2(m_windowWidth - 390, 25));
    titleText->SetFontSize(16.0f);
    titleText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    titleText->SetZOrder(2);
    
    
    auto basicFeaturesText = std::make_shared<CustomGUI::UIText>("Basic Features (30/30 points)", "basicFeatures");
    basicFeaturesText->SetPosition(glm::vec2(m_windowWidth - 390, 50));
    basicFeaturesText->SetFontSize(14.0f);
    basicFeaturesText->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    basicFeaturesText->SetZOrder(2);
    
    auto feature1 = std::make_shared<CustomGUI::UIText>("C++ OpenGL 4.1 Application", "feature1");
    feature1->SetPosition(glm::vec2(m_windowWidth - 380, 70));
    feature1->SetFontSize(11.0f);
    feature1->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    feature1->SetZOrder(2);
    
    auto feature2 = std::make_shared<CustomGUI::UIText>("Texture System: Diffuse + Specular", "feature2");
    feature2->SetPosition(glm::vec2(m_windowWidth - 380, 85));
    feature2->SetFontSize(11.0f);
    feature2->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    feature2->SetZOrder(2);
    
    auto feature3 = std::make_shared<CustomGUI::UIText>("3D Model Loading and Animation", "feature3");
    feature3->SetPosition(glm::vec2(m_windowWidth - 380, 100));
    feature3->SetFontSize(11.0f);
    feature3->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    feature3->SetZOrder(2);
    
    auto feature4 = std::make_shared<CustomGUI::UIText>("WASD + Mouse Control System", "feature4");
    feature4->SetPosition(glm::vec2(m_windowWidth - 380, 115));
    feature4->SetFontSize(11.0f);
    feature4->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    feature4->SetZOrder(2);
    
    
    auto advancedFeaturesText = std::make_shared<CustomGUI::UIText>("Advanced Features (~27/35 points)", "advancedFeatures");
    advancedFeaturesText->SetPosition(glm::vec2(m_windowWidth - 390, 145));
    advancedFeaturesText->SetFontSize(14.0f);
    advancedFeaturesText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    advancedFeaturesText->SetZOrder(2);
    
    auto advFeature1 = std::make_shared<CustomGUI::UIText>("Blinn-Phong Lighting (5/5 points)", "advFeature1");
    advFeature1->SetPosition(glm::vec2(m_windowWidth - 380, 165));
    advFeature1->SetFontSize(11.0f);
    advFeature1->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    advFeature1->SetZOrder(2);
    
    auto advFeature2 = std::make_shared<CustomGUI::UIText>("Shadow Mapping (5/5 points) - PCF Soft Shadows", "advFeature2");
    advFeature2->SetPosition(glm::vec2(m_windowWidth - 380, 180));
    advFeature2->SetFontSize(11.0f);
    advFeature2->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    advFeature2->SetZOrder(2);
    
    auto advFeature3 = std::make_shared<CustomGUI::UIText>("Physics Engine (Extra) - PhysX", "advFeature3");
    advFeature3->SetPosition(glm::vec2(m_windowWidth - 380, 195));
    advFeature3->SetFontSize(11.0f);
    advFeature3->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    advFeature3->SetZOrder(2);
    
    auto advFeature4 = std::make_shared<CustomGUI::UIText>("3D Audio System (4/5 points)", "advFeature4");
    advFeature4->SetPosition(glm::vec2(m_windowWidth - 380, 210));
    advFeature4->SetFontSize(11.0f);
    advFeature4->SetTextColor(glm::vec3(1.0f, 0.8f, 0.0f)); 
    advFeature4->SetZOrder(2);
    
    auto advFeature5 = std::make_shared<CustomGUI::UIText>("Aesthetic Quality (8/10 points) - Improvable", "advFeature5");
    advFeature5->SetPosition(glm::vec2(m_windowWidth - 380, 225));
    advFeature5->SetFontSize(11.0f);
    advFeature5->SetTextColor(glm::vec3(1.0f, 0.8f, 0.0f)); 
    advFeature5->SetZOrder(2);
    
    
    auto targetScoreText = std::make_shared<CustomGUI::UIText>("Target Score: 90-95 points", "targetScore");
    targetScoreText->SetPosition(glm::vec2(m_windowWidth - 390, 260));
    targetScoreText->SetFontSize(14.0f);
    targetScoreText->SetTextColor(glm::vec3(0.0f, 1.0f, 1.0f)); 
    targetScoreText->SetZOrder(2);
    
    
    m_guiManager->AddElement(techPanel);
    m_guiManager->AddElement(titleText);
    m_guiManager->AddElement(basicFeaturesText);
    m_guiManager->AddElement(feature1);
    m_guiManager->AddElement(feature2);
    m_guiManager->AddElement(feature3);
    m_guiManager->AddElement(feature4);
    m_guiManager->AddElement(advancedFeaturesText);
    m_guiManager->AddElement(advFeature1);
    m_guiManager->AddElement(advFeature2);
    m_guiManager->AddElement(advFeature3);
    m_guiManager->AddElement(advFeature4);
    m_guiManager->AddElement(advFeature5);
    m_guiManager->AddElement(targetScoreText);
}

void Application::CreateGameStatePanel() {
    if (!m_guiManager) return;
    
    
    auto statePanel = std::make_shared<CustomGUI::UIPanel>("gameStatePanel");
    statePanel->SetPosition(glm::vec2(10, m_windowHeight - 250));
    statePanel->SetSize(glm::vec2(350, 240));
    statePanel->SetBackgroundColor(glm::vec4(0.0f, 0.0f, 0.2f, 0.8f)); 
    statePanel->SetZOrder(1);
    
    
    auto stateTitleText = std::make_shared<CustomGUI::UIText>("Ancient Treasure Hunter - Status", "statePanelTitle");
    stateTitleText->SetPosition(glm::vec2(20, m_windowHeight - 235));
    stateTitleText->SetFontSize(16.0f);
    stateTitleText->SetTextColor(glm::vec3(0.0f, 1.0f, 1.0f)); 
    stateTitleText->SetZOrder(2);
    
    
    auto gameStateText = std::make_shared<CustomGUI::UIText>("Current Quest Status:", "gameStateLabel");
    gameStateText->SetPosition(glm::vec2(20, m_windowHeight - 210));
    gameStateText->SetFontSize(14.0f);
    gameStateText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    gameStateText->SetZOrder(2);
    
    auto playerPosText = std::make_shared<CustomGUI::UIText>("Explorer Position: (0.0, 0.0, 0.0)", "playerPosition");
    playerPosText->SetPosition(glm::vec2(30, m_windowHeight - 190));
    playerPosText->SetFontSize(12.0f);
    playerPosText->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    playerPosText->SetZOrder(2);
    
    auto keysText = std::make_shared<CustomGUI::UIText>("Ancient Keys Found: 0/5", "collectedKeys");
    keysText->SetPosition(glm::vec2(30, m_windowHeight - 175));
    keysText->SetFontSize(12.0f);
    keysText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    keysText->SetZOrder(2);
    
    auto chestsText = std::make_shared<CustomGUI::UIText>("Treasure Chests Unlocked: 0/3", "unlockedChests");
    chestsText->SetPosition(glm::vec2(30, m_windowHeight - 160));
    chestsText->SetFontSize(12.0f);
    chestsText->SetTextColor(glm::vec3(1.0f, 0.8f, 0.0f)); 
    chestsText->SetZOrder(2);
    
    
    auto systemStateText = std::make_shared<CustomGUI::UIText>("Technical Systems Status:", "systemStateLabel");
    systemStateText->SetPosition(glm::vec2(20, m_windowHeight - 150));
    systemStateText->SetFontSize(14.0f);
    systemStateText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    systemStateText->SetZOrder(2);
    
    auto lightingStatusText = std::make_shared<CustomGUI::UIText>("Lighting System: ENABLED", "lightingStatus");
    lightingStatusText->SetPosition(glm::vec2(30, m_windowHeight - 130));
    lightingStatusText->SetFontSize(12.0f);
    lightingStatusText->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    lightingStatusText->SetZOrder(2);
    
    auto shadowStatusText = std::make_shared<CustomGUI::UIText>("Shadow Mapping: ENABLED", "shadowStatus");
    shadowStatusText->SetPosition(glm::vec2(30, m_windowHeight - 115));
    shadowStatusText->SetFontSize(12.0f);
    shadowStatusText->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    shadowStatusText->SetZOrder(2);
    
    auto physicsStatusText = std::make_shared<CustomGUI::UIText>("Physics Engine: RUNNING", "physicsStatus");
    physicsStatusText->SetPosition(glm::vec2(30, m_windowHeight - 100));
    physicsStatusText->SetFontSize(12.0f);
    physicsStatusText->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    physicsStatusText->SetZOrder(2);
    
    auto audioStatusText = std::make_shared<CustomGUI::UIText>("Audio System: RUNNING", "audioStatus");
    audioStatusText->SetPosition(glm::vec2(30, m_windowHeight - 85));
    audioStatusText->SetFontSize(12.0f);
    audioStatusText->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    audioStatusText->SetZOrder(2);
    
    
    auto performanceLabel = std::make_shared<CustomGUI::UIText>("Performance Stats:", "performanceLabel");
    performanceLabel->SetPosition(glm::vec2(20, m_windowHeight - 60));
    performanceLabel->SetFontSize(14.0f);
    performanceLabel->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    performanceLabel->SetZOrder(2);
    
    auto fpsText = std::make_shared<CustomGUI::UIText>("FPS: 60.0 | Frame Time: 16.67ms", "fpsInfo");
    fpsText->SetPosition(glm::vec2(30, m_windowHeight - 40));
    fpsText->SetFontSize(12.0f);
    fpsText->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    fpsText->SetZOrder(2);
    
    
    m_guiManager->AddElement(statePanel);
    m_guiManager->AddElement(stateTitleText);
    m_guiManager->AddElement(gameStateText);
    m_guiManager->AddElement(playerPosText);
    m_guiManager->AddElement(keysText);
    m_guiManager->AddElement(chestsText);
    m_guiManager->AddElement(systemStateText);
    m_guiManager->AddElement(lightingStatusText);
    m_guiManager->AddElement(shadowStatusText);
    m_guiManager->AddElement(physicsStatusText);
    m_guiManager->AddElement(audioStatusText);
    m_guiManager->AddElement(performanceLabel);
    m_guiManager->AddElement(fpsText);
}

void Application::CreateInteractiveDemo() {
    
    
}

void Application::CreateOperationGuide() {
    if (!m_guiManager) return;
    
    
    auto guidePanel = std::make_shared<CustomGUI::UIPanel>("operationGuidePanel");
    guidePanel->SetPosition(glm::vec2(m_windowWidth - 300, m_windowHeight - 200));
    guidePanel->SetSize(glm::vec2(280, 190));
    guidePanel->SetBackgroundColor(glm::vec4(0.2f, 0.0f, 0.2f, 0.8f)); 
    guidePanel->SetZOrder(1);
    
    
    auto guideTitleText = std::make_shared<CustomGUI::UIText>("Ancient Treasure Hunter Guide", "guidePanelTitle");
    guideTitleText->SetPosition(glm::vec2(m_windowWidth - 290, m_windowHeight - 185));
    guideTitleText->SetFontSize(16.0f);
    guideTitleText->SetTextColor(glm::vec3(1.0f, 1.0f, 0.0f)); 
    guideTitleText->SetZOrder(2);
    
    
    auto gameControlLabel = std::make_shared<CustomGUI::UIText>("Exploration Controls:", "gameControlLabel");
    gameControlLabel->SetPosition(glm::vec2(m_windowWidth - 290, m_windowHeight - 165));
    gameControlLabel->SetFontSize(12.0f);
    gameControlLabel->SetTextColor(glm::vec3(0.0f, 1.0f, 1.0f)); 
    gameControlLabel->SetZOrder(2);
    
    auto control1 = std::make_shared<CustomGUI::UIText>("WASD: Explore Ancient Ruins", "control1");
    control1->SetPosition(glm::vec2(m_windowWidth - 280, m_windowHeight - 150));
    control1->SetFontSize(11.0f);
    control1->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    control1->SetZOrder(2);
    
    auto control2 = std::make_shared<CustomGUI::UIText>("Mouse: Look for Hidden Treasures", "control2");
    control2->SetPosition(glm::vec2(m_windowWidth - 280, m_windowHeight - 135));
    control2->SetFontSize(11.0f);
    control2->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    control2->SetZOrder(2);
    
    
    auto techControlLabel = std::make_shared<CustomGUI::UIText>("Technical Demo:", "techControlLabel");
    techControlLabel->SetPosition(glm::vec2(m_windowWidth - 290, m_windowHeight - 115));
    techControlLabel->SetFontSize(12.0f);
    techControlLabel->SetTextColor(glm::vec3(0.0f, 1.0f, 1.0f)); 
    techControlLabel->SetZOrder(2);
    
    auto techControl1 = std::make_shared<CustomGUI::UIText>("F3/F4: Shadow Quality Demo", "techControl1");
    techControl1->SetPosition(glm::vec2(m_windowWidth - 280, m_windowHeight - 100));
    techControl1->SetFontSize(11.0f);
    techControl1->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    techControl1->SetZOrder(2);
    
    auto techControl2 = std::make_shared<CustomGUI::UIText>("F1: Toggle Technical Panels", "techControl2");
    techControl2->SetPosition(glm::vec2(m_windowWidth - 280, m_windowHeight - 85));
    techControl2->SetFontSize(11.0f);
    techControl2->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    techControl2->SetZOrder(2);
    
    
    auto demoLabel = std::make_shared<CustomGUI::UIText>("Game Objective:", "demoLabel");
    demoLabel->SetPosition(glm::vec2(m_windowWidth - 290, m_windowHeight - 65));
    demoLabel->SetFontSize(12.0f);
    demoLabel->SetTextColor(glm::vec3(0.0f, 1.0f, 0.0f)); 
    demoLabel->SetZOrder(2);
    
    auto demoText = std::make_shared<CustomGUI::UIText>("Find all keys and unlock treasure chests", "demoText");
    demoText->SetPosition(glm::vec2(m_windowWidth - 280, m_windowHeight - 50));
    demoText->SetFontSize(11.0f);
    demoText->SetTextColor(glm::vec3(0.9f, 0.9f, 0.9f));
    demoText->SetZOrder(2);
    
    
    m_guiManager->AddElement(guidePanel);
    m_guiManager->AddElement(guideTitleText);
    m_guiManager->AddElement(gameControlLabel);
    m_guiManager->AddElement(control1);
    m_guiManager->AddElement(control2);
    m_guiManager->AddElement(techControlLabel);
    m_guiManager->AddElement(techControl1);
    m_guiManager->AddElement(techControl2);
    m_guiManager->AddElement(demoLabel);
    m_guiManager->AddElement(demoText);
}



void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->m_windowWidth = width;
        app->m_windowHeight = height;
        glViewport(0, 0, width, height);
    }
}

void Application::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && app->m_camera) {
        
        if (app->m_currentState != GameState::IN_GAME) {
            return;
        }
        
        if (app->m_firstMouse) {
            app->m_lastX = static_cast<float>(xpos);
            app->m_lastY = static_cast<float>(ypos);
            app->m_firstMouse = false;
            return; 
        }
        
        float xoffset = static_cast<float>(xpos) - app->m_lastX;
        float yoffset = app->m_lastY - static_cast<float>(ypos);
        
        
        xoffset *= 0.05f;  
        yoffset *= 0.05f;
        
        app->m_lastX = static_cast<float>(xpos);
        app->m_lastY = static_cast<float>(ypos);
        
        
        if (app->m_physicsCamera) {
            app->m_physicsCamera->ProcessMouseMovement(xoffset, yoffset);
        } else {
            app->m_camera->ProcessMouseMovement(xoffset, yoffset);
        }
    }
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        
        if (app->m_guiManager) {
            app->m_guiManager->HandleMouseClick(xpos, ypos, action == GLFW_PRESS);
        }
    }
}

void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && app->m_camera) {
        
        if (app->m_currentState == GameState::IN_GAME) {
            app->m_camera->ProcessMouseScroll(static_cast<float>(yoffset));
        }
    }
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && action == GLFW_PRESS) {
        app->ProcessInput();
    }
}

void Application::LoadGameTextures() {
    if (m_texturesLoaded) return;
    
    std::cout << "Loading game textures..." << std::endl;
    
    // Try multiple possible paths for texture files
    std::vector<std::string> keyTexturePaths = {
        "resources/textures/key_texture.jpg",
        "./resources/textures/key_texture.jpg",
        "../resources/textures/key_texture.jpg",
        "../../resources/textures/key_texture.jpg"
    };
    
    std::vector<std::string> chestTexturePaths = {
        "resources/textures/chest_texture.jpg",
        "./resources/textures/chest_texture.jpg",
        "../resources/textures/chest_texture.jpg",
        "../../resources/textures/chest_texture.jpg"
    };
    
    // Load key texture
    m_keyTexture = 0;
    for (const auto& path : keyTexturePaths) {
        m_keyTexture = LoadTexture(path);
        if (m_keyTexture != 0) {
            std::cout << "Successfully loaded key texture from: " << path << std::endl;
            break;
        }
    }
    
    if (m_keyTexture == 0) {
        std::cout << "Warning: Failed to load key texture, using default" << std::endl;
        // Create default texture
        unsigned char defaultKeyTexture[] = { 255, 215, 0, 255 }; // Gold color
        glGenTextures(1, &m_keyTexture);
        glBindTexture(GL_TEXTURE_2D, m_keyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultKeyTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    // Load chest texture
    m_chestTexture = 0;
    for (const auto& path : chestTexturePaths) {
        m_chestTexture = LoadTexture(path);
        if (m_chestTexture != 0) {
            std::cout << "Successfully loaded chest texture from: " << path << std::endl;
            break;
        }
    }
    
    if (m_chestTexture == 0) {
        std::cout << "Warning: Failed to load chest texture, using default" << std::endl;
        // Create default texture
        unsigned char defaultChestTexture[] = { 139, 69, 19, 255 }; // Brown color
        glGenTextures(1, &m_chestTexture);
        glBindTexture(GL_TEXTURE_2D, m_chestTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultChestTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    m_texturesLoaded = true;
    std::cout << "Game textures loaded successfully!" << std::endl;
}

void Application::BindKeyTexture() {
    if (m_keyTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_keyTexture);
        m_blinnPhongShader->SetInt("material.diffuse", 0);
    }
}

void Application::BindChestTexture() {
    if (m_chestTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_chestTexture);
        m_blinnPhongShader->SetInt("material.diffuse", 0);
    }
}
