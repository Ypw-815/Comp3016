/**
 * @file IrrklangAudioManager.h
 * @brief Audio management system using irrKlang library
 * 
 * Provides comprehensive audio management for the game including:
 * - Background music playback
 * - Sound effects with 3D positioning
 * - Volume control (master, music, SFX)
 * - Audio resource management
 * - Safe fallback when irrKlang is not available
 * 
 * Features:
 * - Conditional compilation for irrKlang dependency
 * - Multiple audio stream management
 * - 3D positional audio support
 * - Dynamic volume adjustment
 * - Audio file caching and preloading
 */

#pragma once

#include <string>
#include <map>
#include <glm/glm.hpp>

#ifdef IRRKLANG_ENABLED
#include <irrKlang.h>
using namespace irrklang;
#endif

/**
 * @brief Audio management system for the game
 * 
 * Manages all audio playback including background music, sound effects,
 * and 3D positioned audio. Provides volume controls and graceful fallback
 * when the irrKlang library is not available.
 * 
 * Key features:
 * - Background music with loop support
 * - 3D positioned sound effects
 * - Separate volume controls for music and SFX
 * - Audio resource preloading and caching
 * - Safe initialization and cleanup
 */
class IrrklangAudioManager {
private:
#ifdef IRRKLANG_ENABLED
    ISoundEngine* soundEngine;         // Main irrKlang sound engine
    std::map<std::string, ISound*> activeSounds;  // Currently playing sounds
    ISound* backgroundMusic;           // Background music track
#endif
    
    std::map<std::string, std::string> soundPaths;  // Sound file path registry
    float masterVolume;                // Overall volume multiplier
    float musicVolume;                 // Background music volume
    float sfxVolume;                   // Sound effects volume
    bool audioEnabled;                 // Whether audio is currently enabled
    bool initialized;                  // Whether the audio system initialized successfully
    
public:
    /**
     * @brief Constructor - initializes audio manager
     */
    IrrklangAudioManager();
    
    /**
     * @brief Destructor - cleanup audio resources
     */
    ~IrrklangAudioManager();
    
    
    bool Initialize();
    void Shutdown();
    
    
    void PlaySFX(const std::string& soundName, float volume = 1.0f);
    void Play3DSFX(const std::string& soundName, const glm::vec3& position, float volume = 1.0f);
    void StopSFX(const std::string& soundName);
    void StopAllSFX();
    
    
    void PlayMusic(const std::string& musicName, bool loop = true, float volume = 1.0f);
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    
    
    void SetListenerPosition(const glm::vec3& position, const glm::vec3& lookDirection, const glm::vec3& upVector = glm::vec3(0, 1, 0));
    
    
    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetSFXVolume(float volume);
    float GetMasterVolume() const { return masterVolume; }
    float GetMusicVolume() const { return musicVolume; }
    float GetSFXVolume() const { return sfxVolume; }
    
    
    void SetAudioEnabled(bool enabled);
    bool IsAudioEnabled() const { return audioEnabled; }
    bool IsInitialized() const { return initialized; }
    
    
    void Update();
    
    
    void LoadSoundFile(const std::string& soundName, const std::string& filePath);
    
    
    static IrrklangAudioManager& GetInstance();
    
private:
    void InitializeSoundPaths();
    float ApplyVolumeSettings(float baseVolume, bool isMusic = false);
    
    
    IrrklangAudioManager(const IrrklangAudioManager&) = delete;
    IrrklangAudioManager& operator=(const IrrklangAudioManager&) = delete;
};


enum class AudioEvent {
    BLOCK_DESTROYED,
    ITEM_COLLECTED,
    BUTTON_ACTIVATED,
    GAME_VICTORY,
    FOOTSTEP,
    ERROR_SOUND,
    MENU_CLICK,
    MENU_HOVER
};


class AudioEventHandler {
public:
    static void PlayEventSound(AudioEvent event, const glm::vec3& position = glm::vec3(0.0f));
    static void InitializeEventSounds();
};
