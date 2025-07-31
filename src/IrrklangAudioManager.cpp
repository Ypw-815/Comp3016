#include "IrrklangAudioManager.h"
#include <iostream>
#include <algorithm>

IrrklangAudioManager::IrrklangAudioManager() 
    : masterVolume(1.0f)
    , musicVolume(0.7f)
    , sfxVolume(0.8f)
    , audioEnabled(true)
    , initialized(false)
#ifdef IRRKLANG_ENABLED
    , soundEngine(nullptr)
    , backgroundMusic(nullptr)
#endif
{
}

IrrklangAudioManager::~IrrklangAudioManager() {
    Shutdown();
}

bool IrrklangAudioManager::Initialize() {
    if (initialized) {
        return true;
    }
    
#ifdef IRRKLANG_ENABLED
    std::cout << "Initializing Irrklang Audio Manager..." << std::endl;
    
    
    soundEngine = createIrrKlangDevice();
    if (!soundEngine) {
        std::cerr << "Failed to create Irrklang sound engine!" << std::endl;
        return false;
    }
    
    std::cout << "Irrklang sound engine created successfully" << std::endl;
    std::cout << "Audio driver: " << soundEngine->getDriverName() << std::endl;
    
    initialized = true;
    InitializeSoundPaths();
    
    return true;
#else
    std::cout << "Audio disabled - Irrklang not available" << std::endl;
    initialized = true;
    return true;
#endif
}

void IrrklangAudioManager::Shutdown() {
    if (!initialized) {
        return;
    }
    
#ifdef IRRKLANG_ENABLED
    if (soundEngine) {
        soundEngine->stopAllSounds();
        soundEngine->drop();
        soundEngine = nullptr;
    }
    
    activeSounds.clear();
    backgroundMusic = nullptr;
#endif
    
    initialized = false;
    std::cout << "Audio system shutdown complete" << std::endl;
}

void IrrklangAudioManager::PlaySFX(const std::string& soundName, float volume) {
    if (!initialized || !audioEnabled) {
        return;
    }
    
#ifdef IRRKLANG_ENABLED
    if (!soundEngine) {
        return;
    }
    
    auto it = soundPaths.find(soundName);
    if (it == soundPaths.end()) {
        std::cerr << "Sound file not found: " << soundName << std::endl;
        return;
    }
    
    float finalVolume = ApplyVolumeSettings(volume, false);
    ISound* sound = soundEngine->play2D(it->second.c_str(), false, false, true);
    
    if (sound) {
        sound->setVolume(finalVolume);
        activeSounds[soundName] = sound;
        std::cout << "Playing SFX: " << soundName << " at volume " << finalVolume << std::endl;
    }
#else
    std::cout << "Would play SFX: " << soundName << " (audio disabled)" << std::endl;
#endif
}

void IrrklangAudioManager::Play3DSFX(const std::string& soundName, const glm::vec3& position, float volume) {
    if (!initialized || !audioEnabled) {
        return;
    }
    
#ifdef IRRKLANG_ENABLED
    if (!soundEngine) {
        return;
    }
    
    auto it = soundPaths.find(soundName);
    if (it == soundPaths.end()) {
        std::cerr << "Sound file not found: " << soundName << std::endl;
        return;
    }
    
    float finalVolume = ApplyVolumeSettings(volume, false);
    vec3df irrPos(position.x, position.y, position.z);
    
    ISound* sound = soundEngine->play3D(it->second.c_str(), irrPos, false, false, true);
    
    if (sound) {
        sound->setVolume(finalVolume);
        activeSounds[soundName + "_3d"] = sound;
        std::cout << "Playing 3D SFX: " << soundName << " at position (" 
                  << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
#else
    std::cout << "Would play 3D SFX: " << soundName << " at position (" 
              << position.x << ", " << position.y << ", " << position.z << ") (audio disabled)" << std::endl;
#endif
}

void IrrklangAudioManager::PlayMusic(const std::string& musicName, bool loop, float volume) {
    if (!initialized || !audioEnabled) {
        return;
    }
    
    
    StopMusic();
    
#ifdef IRRKLANG_ENABLED
    if (!soundEngine) {
        return;
    }
    
    auto it = soundPaths.find(musicName);
    if (it == soundPaths.end()) {
        std::cerr << "Music file not found: " << musicName << std::endl;
        return;
    }
    
    float finalVolume = ApplyVolumeSettings(volume, true);
    backgroundMusic = soundEngine->play2D(it->second.c_str(), loop, false, true);
    
    if (backgroundMusic) {
        backgroundMusic->setVolume(finalVolume);
        std::cout << "Playing music: " << musicName << " (loop: " << (loop ? "yes" : "no") << ")" << std::endl;
    }
#else
    std::cout << "Would play music: " << musicName << " (audio disabled)" << std::endl;
#endif
}

void IrrklangAudioManager::StopMusic() {
#ifdef IRRKLANG_ENABLED
    if (backgroundMusic) {
        backgroundMusic->stop();
        backgroundMusic = nullptr;
        std::cout << "Background music stopped" << std::endl;
    }
#endif
}

void IrrklangAudioManager::SetListenerPosition(const glm::vec3& position, const glm::vec3& lookDirection, const glm::vec3& upVector) {
#ifdef IRRKLANG_ENABLED
    if (soundEngine) {
        vec3df irrPos(position.x, position.y, position.z);
        vec3df irrLook(lookDirection.x, lookDirection.y, lookDirection.z);
        vec3df irrUp(upVector.x, upVector.y, upVector.z);
        
        soundEngine->setListenerPosition(irrPos, irrLook, vec3df(0, 0, 0), irrUp);
    }
#endif
}

void IrrklangAudioManager::SetMasterVolume(float volume) {
    masterVolume = std::clamp(volume, 0.0f, 1.0f);
    
#ifdef IRRKLANG_ENABLED
    if (soundEngine) {
        soundEngine->setSoundVolume(masterVolume);
    }
#endif
}

void IrrklangAudioManager::SetMusicVolume(float volume) {
    musicVolume = std::clamp(volume, 0.0f, 1.0f);
    
#ifdef IRRKLANG_ENABLED
    if (backgroundMusic) {
        backgroundMusic->setVolume(ApplyVolumeSettings(1.0f, true));
    }
#endif
}

void IrrklangAudioManager::SetSFXVolume(float volume) {
    sfxVolume = std::clamp(volume, 0.0f, 1.0f);
}

void IrrklangAudioManager::SetAudioEnabled(bool enabled) {
    audioEnabled = enabled;
    
    if (!enabled) {
        StopAllSFX();
        StopMusic();
    }
}

void IrrklangAudioManager::StopAllSFX() {
#ifdef IRRKLANG_ENABLED
    if (soundEngine) {
        
        for (auto& pair : activeSounds) {
            if (pair.second) {
                pair.second->stop();
            }
        }
        activeSounds.clear();
    }
#endif
}

void IrrklangAudioManager::Update() {
    if (!initialized) {
        return;
    }
    
#ifdef IRRKLANG_ENABLED
    
    auto it = activeSounds.begin();
    while (it != activeSounds.end()) {
        if (!it->second || it->second->isFinished()) {
            it = activeSounds.erase(it);
        } else {
            ++it;
        }
    }
    
    
    if (backgroundMusic && backgroundMusic->isFinished()) {
        backgroundMusic = nullptr;
    }
#endif
}

void IrrklangAudioManager::LoadSoundFile(const std::string& soundName, const std::string& filePath) {
    soundPaths[soundName] = filePath;
    std::cout << "Loaded sound mapping: " << soundName << " -> " << filePath << std::endl;
}

IrrklangAudioManager& IrrklangAudioManager::GetInstance() {
    static IrrklangAudioManager instance;
    return instance;
}

void IrrklangAudioManager::InitializeSoundPaths() {
    
    LoadSoundFile("crack", "resources/audio/sfx/crack.wav");
    LoadSoundFile("pickup", "resources/audio/sfx/pickup.wav");
    LoadSoundFile("button", "resources/audio/sfx/button.wav");
    LoadSoundFile("victory", "resources/audio/sfx/victory.wav");
    LoadSoundFile("footstep", "resources/audio/sfx/footstep.wav");
    LoadSoundFile("error", "resources/audio/sfx/error.wav");
    LoadSoundFile("menu_click", "resources/audio/sfx/menu_click.wav");
    LoadSoundFile("menu_hover", "resources/audio/sfx/menu_hover.wav");
    
    
    LoadSoundFile("ambient", "resources/audio/music/ambient.ogg");
    LoadSoundFile("menu_music", "resources/audio/music/menu.ogg");
    LoadSoundFile("victory_music", "resources/audio/music/victory.ogg");
}

float IrrklangAudioManager::ApplyVolumeSettings(float baseVolume, bool isMusic) {
    float volume = baseVolume * masterVolume;
    if (isMusic) {
        volume *= musicVolume;
    } else {
        volume *= sfxVolume;
    }
    return std::clamp(volume, 0.0f, 1.0f);
}


void AudioEventHandler::PlayEventSound(AudioEvent event, const glm::vec3& position) {
    IrrklangAudioManager& audio = IrrklangAudioManager::GetInstance();
    
    if (position == glm::vec3(0.0f)) {
        
        switch (event) {
            case AudioEvent::BLOCK_DESTROYED:
                audio.PlaySFX("crack", 0.8f);
                break;
            case AudioEvent::ITEM_COLLECTED:
                audio.PlaySFX("pickup", 1.0f);
                break;
            case AudioEvent::BUTTON_ACTIVATED:
                audio.PlaySFX("button", 0.9f);
                break;
            case AudioEvent::GAME_VICTORY:
                audio.PlaySFX("victory", 1.0f);
                audio.PlayMusic("victory_music", false, 0.8f);
                break;
            case AudioEvent::ERROR_SOUND:
                audio.PlaySFX("error", 0.7f);
                break;
            case AudioEvent::MENU_CLICK:
                audio.PlaySFX("menu_click", 0.6f);
                break;
            case AudioEvent::MENU_HOVER:
                audio.PlaySFX("menu_hover", 0.4f);
                break;
            default:
                break;
        }
    } else {
        
        switch (event) {
            case AudioEvent::FOOTSTEP:
                audio.Play3DSFX("footstep", position, 0.5f);
                break;
            case AudioEvent::BLOCK_DESTROYED:
                audio.Play3DSFX("crack", position, 0.8f);
                break;
            case AudioEvent::ITEM_COLLECTED:
                audio.Play3DSFX("pickup", position, 1.0f);
                break;
            default:
                
                PlayEventSound(event, glm::vec3(0.0f));
                break;
        }
    }
}

void AudioEventHandler::InitializeEventSounds() {
    
    std::cout << "Audio event system initialized" << std::endl;
}
