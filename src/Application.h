#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <string>
#include <iostream> 


class Camera;
class Shader;
class Model;
class Texture;
class Cube;
class Quad;
class PhysicsManager;
class PhysicsCamera;
class TerrainGenerator;
class LayoutManager;
class GameInteraction;
class ShadowMappingManager;
class PerformanceProfiler;


#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "Geometry.h"
#include "PhysicsManager.h"
#include "PhysicsCamera.h"
#include "TerrainGenerator.h"   
#include "LayoutManager.h"
#include "GameInteraction.h"
#include "ShadowMapping.h"
#include "PerformanceProfiler.h"
#include "GameState.h"
#include "CustomGUI/GUIManager.h"
#include "IrrklangAudioManager.h"
#include "TerrainPlacement.h"




#define GL_CHECK(call) \
    do { \
        call; \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            std::cerr << "✗ OpenGL Error: " << error << " in " << #call \
                      << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while(0)


#define FILE_CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "✗ File Error: " << message \
                      << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)


#define MEMORY_CHECK(ptr, message) \
    do { \
        if (!(ptr)) { \
            std::cerr << "✗ Memory Error: " << message \
                      << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)


struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    
    
    unsigned int diffuseTexture = 0;
    unsigned int specularTexture = 0;
};


struct AnimatedWall {
    glm::vec3 position;
    glm::vec3 scale;
    float rotationY;           
    float targetRotationY;     
    float height;              
    float targetHeight;        
    bool isAnimating;          
    float animationSpeed;      
    int wallType;              
    bool isActive;             
    void* physicsBody;         
    
    AnimatedWall() : physicsBody(nullptr) {}
};


struct MazeGame {
    static const int MAZE_SIZE = 5;
    std::vector<AnimatedWall> walls;
    glm::vec3 playerStartPos;
    glm::vec3 exitPos;
    float gameTime;
    float timeLimit;
    bool gameWon;
    bool gameStarted;
};


struct AdvancedLighting {
    
    glm::vec3 pointLightPos;
    glm::vec3 pointLightColor;
    float pointLightConstant;
    float pointLightLinear;
    float pointLightQuadratic;
    
    glm::vec3 spotLightPos;
    glm::vec3 spotLightDir;
    float spotLightCutOff;
    float spotLightOuterCutOff;
    glm::vec3 spotLightColor;
    
    glm::vec3 dirLightDir;
    glm::vec3 dirLightColor;
    
    bool enableSpotLight;
    bool enableDirLight;
    bool useBlinnPhong;
};

// Ball Shooting Game Structures
struct Target {
    int id;
    glm::vec3 position;
    float radius;
    bool isActive;
    float hitTime;
    float respawnTime;
    glm::vec3 color;
    int points;
};

struct BallProjectile {
    int id;
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    float lifetime;
    float age;
    glm::vec3 color;
};

struct ModelObject {
    std::unique_ptr<Model> model;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float animationTime;
    bool isAnimated;
    std::string name;
};

class Application {
public:

    Application(int width = 1200, int height = 800, const std::string& title = "COMP3016 OpenGL Application");
    

    ~Application();


    bool Initialize();
    

    void Run();
    

    void Shutdown();

    static Application* GetInstance() { return s_instance; }

private:

    int m_drawCalls = 0;
    int m_vertexCount = 0;
    int m_keysCollected = 0;      
    float m_explorationDistance = 0.0f; 
    bool m_enableLighting = true;
    bool m_gameWon = false;              
    
    GLFWwindow* m_window;
    int m_windowWidth, m_windowHeight;
    std::string m_windowTitle;
    
    
    std::unique_ptr<Camera> m_camera;
    
    
    std::unique_ptr<PhysicsManager> m_physicsManager;
    std::unique_ptr<PhysicsCamera> m_physicsCamera;
    
    
    CustomGUI::GUIManager* m_guiManager;
    
    
    bool m_enablePerformanceOverlay;
    
    
    std::unique_ptr<ShadowMappingManager> m_shadowManager;
    bool m_enableShadows;
    float m_shadowStrength;
    
    
    
    
    std::unique_ptr<Shader> m_basicShader;
    std::unique_ptr<Shader> m_lightingShader;
    std::unique_ptr<Shader> m_blinnPhongShader;
    std::unique_ptr<Shader> m_terrainShader;
    std::unique_ptr<Shader> m_terrainShadowShader;  
    std::unique_ptr<Shader> m_shadowMapShader;
    std::unique_ptr<Shader> m_shadowReceiveShader;

    
    std::unique_ptr<Quad> m_quad;
    std::unique_ptr<Quad> m_signature;
    std::unique_ptr<Sphere> m_sphere;

    
    std::unique_ptr<Texture> m_defaultTexture;
    
    
    Material m_basicMaterial;
    unsigned int m_diffuseTexture;
    unsigned int m_specularTexture;
    
    
    AncientTreasureGameData m_treasureGame;
    
    
    // Ball Shooting Game Variables
    std::vector<Target> m_targets;
    std::vector<BallProjectile> m_projectiles;
    int m_gameScore = 0;
    int m_ballsShot = 0;
    int m_targetsHit = 0;
    float m_gameTime = 0.0f;
    bool m_gameActive = false;
    
    
    AdvancedLighting m_advancedLighting;
    
    
    std::vector<ModelObject> m_gameModels;
    bool m_modelsLoaded;
    
    
    std::unique_ptr<TerrainGenerator> m_terrainGenerator;
    std::unique_ptr<LayoutManager> m_layoutManager;
    std::unique_ptr<GameInteraction> m_gameInteraction;
    bool m_terrainEnabled;
    
    
    IrrklangAudioManager* m_audioManager;
    bool m_audioEnabled;
    
    
    bool m_showTechnicalPanel = true;
    bool m_showGameStatePanel = true;
    bool m_showInteractiveDemo = true;
    
    
    float m_ambientStrength = 0.3f;
    float m_diffuseStrength = 1.0f;
    float m_specularStrength = 0.8f;
    float m_shininess = 32.0f;
    float m_metallic = 0.1f;
    float m_roughness = 0.8f;
    float m_reflectance = 0.04f;
    
    
    bool m_showAdvancedTextures;

    
    glm::vec3 m_lightPos;
    glm::vec3 m_lightColor;

    
    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;

    
    float m_lastX = 0.0f, m_lastY = 0.0f;

    // Game state variables
    GameState m_currentState;
    GameState m_pendingStateChange;
    bool m_hasPendingStateChange;
    
    // Mouse and input variables
    bool m_firstMouse;
    
    // Treasure game variables
    int m_chestsUnlocked = 0;
    int m_totalKeys = 5;
    int m_totalChests = 3;
    
    // Performance profiler
    PerformanceProfiler& m_profiler;
    
    // Cube for rendering
    std::unique_ptr<Cube> m_cube;
    
    // Texture related
    unsigned int m_keyTexture = 0;
    unsigned int m_chestTexture = 0;
    bool m_texturesLoaded = false;
    
    // Game state related
    
    // Static instance
    static Application* s_instance;

    
    void SetupCallbacks();
    void ProcessInput();
    void Update();
    void Render();
    void RenderGameScene();
    void RenderLightCube(const glm::mat4& view, const glm::mat4& projection);
    void RenderMenuBackground();
    void CreateBasicScene();
    void CreateDefaultTexture();
    unsigned int CreateDefaultTexture(const glm::vec3& color);  
    
    unsigned int LoadTexture(const std::string& filename);
    
    
    void InitializeTreasureGame();
    void UpdateTreasureGame();
    void RenderTreasureGame();
    void CheckTreasureInteraction();
    bool TryCollectTreasure(int treasureId);
    void UpdateGameProgress();
    void HandleVictoryCondition();
    
    // Ball Shooting Game Functions
    void InitializeBallShootingGame();
    void InitializeTargets();
    void ShootBall();
    void UpdateBallShootingGame();
    void RenderBallShootingGame();
    
    
    void InitializeAdvancedLighting();
    void UpdateAdvancedLighting();
    void SetupLightingUniforms(Shader& shader);
    
    
    void InitializeModels();
    void LoadGameModel(const std::string& modelPath, const std::string& name, 
                      const glm::vec3& position, const glm::vec3& rotation = glm::vec3(0.0f),
                      const glm::vec3& scale = glm::vec3(1.0f), bool animated = false);
    void UpdateModels();
    void RenderModels();
    
    
    void UpdateTerrain();
    void RenderTerrain();
    void RenderSimpleGround();
    
    
    void InitializeGameInteraction();
    void UpdateGameInteraction();
    
    
    void InitializeAudio();
    void UpdateAudio();
    void ShutdownAudio();
    
    
    void InitializeGUI();
    void UpdateGUI(float deltaTime);
    void UpdateGameStatusGUI();  
    void RenderShadowDemoScene();  
    void ShutdownGUI();
    void CreateMainMenu();
    void CreateTestGUI();
    
    
    void CreateTechnicalFeaturesPanel();
    void CreateGameStatePanel();
    void CreateInteractiveDemo();
    void CreateOperationGuide();
    
    
    void InitializeShadowMapping();
    void RenderShadowMap();
    
    // State management methods
    void SetPendingStateChange(GameState newState);
    void UpdateCursorMode();
    void RenderGUI();
    
    // Terrain initialization
    void InitializeTerrain();
    
    // Texture loading functions
    void LoadGameTextures();
    void BindKeyTexture();
    void BindChestTexture();
    
    
    void RenderCube(const glm::vec3& position, float size);

    
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
