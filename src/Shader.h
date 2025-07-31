/**
 * @file Shader.h
 * @brief OpenGL shader program management system
 * 
 * Provides a comprehensive system for loading, compiling, and managing
 * OpenGL shader programs. Supports vertex and fragment shaders with
 * convenient uniform variable setting methods.
 * 
 * Features:
 * - Automatic shader compilation and linking
 * - Error checking and reporting
 * - Convenient uniform setters for common data types
 * - Resource management with automatic cleanup
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief OpenGL Shader Program wrapper class
 * 
 * Manages the lifecycle of OpenGL shader programs including:
 * - Loading shader source code from files
 * - Compiling vertex and fragment shaders
 * - Linking shaders into a program
 * - Setting uniform variables
 * - Error handling and debugging
 * 
 * The class provides both legacy and modern naming conventions
 * for methods to maintain compatibility with different coding styles.
 */
class Shader {
public:
    unsigned int ID;  // OpenGL shader program ID

    /**
     * @brief Constructors for loading shader from files
     * 
     * Creates a shader program by loading, compiling, and linking
     * vertex and fragment shaders from the specified file paths.
     * 
     * @param vertexPath Path to vertex shader source file
     * @param fragmentPath Path to fragment shader source file
     */
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader(const std::string& vertexPath, const std::string& fragmentPath); 
    
    /**
     * @brief Destructor - cleanup OpenGL resources
     */
    ~Shader();

    /**
     * @brief Activate this shader program for rendering
     * 
     * Makes this shader program the active one for subsequent
     * OpenGL draw calls. Both naming conventions provided.
     */
    void use() const;     // Legacy naming
    void Use() const;     // Modern naming

    // Convenience uniform setters (legacy naming)
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setFloat(const std::string& name, float value) const;
    
    // Comprehensive uniform setters (modern naming)
    /**
     * @brief Set boolean uniform variable
     * @param name Uniform variable name in shader
     * @param value Boolean value to set
     */
    void SetBool(const std::string& name, bool value) const;
    
    /**
     * @brief Set integer uniform variable
     * @param name Uniform variable name in shader
     * @param value Integer value to set
     */
    void SetInt(const std::string& name, int value) const;
    
    /**
     * @brief Set float uniform variable
     * @param name Uniform variable name in shader
     * @param value Float value to set
     */
    void SetFloat(const std::string& name, float value) const;
    
    /**
     * @brief Set 2D vector uniform variable
     * @param name Uniform variable name in shader
     * @param value 2D vector value to set
     */
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec2(const std::string& name, float x, float y) const;
    
    /**
     * @brief Set 3D vector uniform variable
     * @param name Uniform variable name in shader
     * @param value 3D vector value to set
     */
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    
    /**
     * @brief Set 4D vector uniform variable
     * @param name Uniform variable name in shader
     * @param value 4D vector value to set
     */
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    
    /**
     * @brief Set matrix uniform variables
     * @param name Uniform variable name in shader
     * @param mat Matrix value to set
     */
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;

private:
    /**
     * @brief Check for shader compilation/linking errors
     * 
     * Examines OpenGL error logs and reports any issues encountered
     * during shader compilation or program linking. Essential for
     * debugging shader code problems.
     * 
     * @param shader OpenGL shader/program ID to check
     * @param type Type of check ("VERTEX", "FRAGMENT", or "PROGRAM")
     */
    void CheckCompileErrors(unsigned int shader, const std::string& type);
};
