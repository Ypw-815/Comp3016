# CMakeList.txt: CMake project for comp3016, include source code and define
# project-specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable hot reload for MSVC compiler if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("comp3016")

# Check for system vcpkg first, then auto-download if not available
if(EXISTS "$ENV{VCPKG_ROOT}/vcpkg.exe")
    message(STATUS "Found system vcpkg at: $ENV{VCPKG_ROOT}")
    set(VCPKG_DIR "$ENV{VCPKG_ROOT}")
    set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows" ${CMAKE_PREFIX_PATH})
elseif(NOT EXISTS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
    message(STATUS "System vcpkg not found, attempting to download and setup...")
    
    # Check if we have internet connection (optional)
    set(HAVE_INTERNET TRUE)
    if(WIN32)
        execute_process(
            COMMAND ping -n 1 github.com
            OUTPUT_QUIET
            ERROR_QUIET
            RESULT_VARIABLE PING_RESULT
        )
        if(NOT PING_RESULT EQUAL 0)
            set(HAVE_INTERNET FALSE)
            message(WARNING "No internet connection detected. vcpkg auto-download may fail.")
        endif()
    endif()
    
    # Set vcpkg directory
    set(VCPKG_DIR "${CMAKE_SOURCE_DIR}/vcpkg")
    
    # Download vcpkg if not exists
    if(NOT EXISTS "${VCPKG_DIR}")
        if(HAVE_INTERNET)
            message(STATUS "Downloading vcpkg...")
            include(FetchContent)
            FetchContent_Declare(
                vcpkg_repo
                GIT_REPOSITORY https://github.com/Microsoft/vcpkg.git
                GIT_TAG master
            )
            FetchContent_GetProperties(vcpkg_repo)
            if(NOT vcpkg_repo_POPULATED)
                FetchContent_Populate(vcpkg_repo)
                # Copy vcpkg to our directory
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${vcpkg_repo_SOURCE_DIR}
                    ${VCPKG_DIR}
                    RESULT_VARIABLE COPY_RESULT
                )
                if(NOT COPY_RESULT EQUAL 0)
                    message(WARNING "Failed to copy vcpkg files")
                endif()
            endif()
        else()
            message(FATAL_ERROR "Cannot download vcpkg without internet connection. Please install vcpkg manually.")
        endif()
    endif()
    
    # Bootstrap vcpkg
    if(EXISTS "${VCPKG_DIR}")
        message(STATUS "Bootstrapping vcpkg...")
        if(WIN32)
            execute_process(
                COMMAND "${VCPKG_DIR}/bootstrap-vcpkg.bat"
                WORKING_DIRECTORY ${VCPKG_DIR}
                RESULT_VARIABLE BOOTSTRAP_RESULT
            )
        else()
            execute_process(
                COMMAND "${VCPKG_DIR}/bootstrap-vcpkg.sh"
                WORKING_DIRECTORY ${VCPKG_DIR}
                RESULT_VARIABLE BOOTSTRAP_RESULT
            )
        endif()
        
        if(BOOTSTRAP_RESULT EQUAL 0)
            message(STATUS "vcpkg bootstrap successful")
            
            # Install required packages from vcpkg.json
            message(STATUS "Installing required packages from vcpkg.json...")
            execute_process(
                COMMAND "${VCPKG_DIR}/vcpkg.exe" install --triplet=x64-windows
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE INSTALL_RESULT
            )
            
            if(INSTALL_RESULT EQUAL 0)
                message(STATUS "Package installation successful")
            else()
                message(WARNING "Package installation failed, trying manual installation...")
                # Fallback: install packages manually
                execute_process(
                    COMMAND "${VCPKG_DIR}/vcpkg.exe" install glfw3:x64-windows glad:x64-windows glm:x64-windows assimp:x64-windows stb:x64-windows physx:x64-windows
                    WORKING_DIRECTORY ${VCPKG_DIR}
                    RESULT_VARIABLE MANUAL_INSTALL_RESULT
                )
                if(MANUAL_INSTALL_RESULT EQUAL 0)
                    message(STATUS "Manual package installation successful")
                else()
                    message(WARNING "Manual package installation also failed, you may need to install packages manually")
                    message(STATUS "Please run: ${VCPKG_DIR}/vcpkg.exe install glfw3:x64-windows glad:x64-windows glm:x64-windows assimp:x64-windows stb:x64-windows physx:x64-windows")
                endif()
            endif()
        else()
            message(WARNING "vcpkg bootstrap failed, you may need to install vcpkg manually")
            message(STATUS "Please run: ${VCPKG_DIR}/bootstrap-vcpkg.bat")
        endif()
    else()
        message(FATAL_ERROR "vcpkg download failed. Please install vcpkg manually or check your internet connection.")
    endif()
endif()

# Set vcpkg path - use system vcpkg if available, otherwise use local
if(EXISTS "$ENV{VCPKG_ROOT}/installed/x64-windows")
    set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows" ${CMAKE_PREFIX_PATH})
    set(CMAKE_MODULE_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows/share/stb" ${CMAKE_MODULE_PATH})
else()
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows" ${CMAKE_PREFIX_PATH})
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/share/stb" ${CMAKE_MODULE_PATH})
endif()

# Find necessary packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)

# Irrklang audio library configuration (manual integration)
set(IRRKLANG_ROOT "${CMAKE_SOURCE_DIR}/external/irrklang")
set(IRRKLANG_INCLUDE_DIR "${IRRKLANG_ROOT}/include")

# Choose appropriate library files based on platform
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit
    set(IRRKLANG_LIBRARY "${IRRKLANG_ROOT}/lib/irrKlang.lib")
    set(IRRKLANG_DLL "${IRRKLANG_ROOT}/bin/irrKlang.dll")
    set(IRRKLANG_MP3_DLL "${IRRKLANG_ROOT}/bin/ikpMP3.dll")
else()
    # 32-bit (fallback)
    set(IRRKLANG_LIBRARY "${IRRKLANG_ROOT}/lib/Win32-visualStudio/irrKlang.lib")
    set(IRRKLANG_DLL "${IRRKLANG_ROOT}/bin/win32-visualStudio/irrKlang.dll")
    set(IRRKLANG_MP3_DLL "${IRRKLANG_ROOT}/bin/win32-visualStudio/ikpMP3.dll")
endif()

# Check if Irrklang files exist
set(IRRKLANG_AVAILABLE FALSE)
if(EXISTS "${IRRKLANG_INCLUDE_DIR}/irrKlang.h" AND EXISTS "${IRRKLANG_LIBRARY}")
    set(IRRKLANG_AVAILABLE TRUE)
    message(STATUS "Found Irrklang audio library")
else()
    message(WARNING "Irrklang library not found. Audio features will be disabled. Run setup_irrklang.ps1 to install.")
endif()

# Collect all source files, but exclude problematic advanced texture files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
# Exclude deleted unused files
list(REMOVE_ITEM SOURCES 
    "${CMAKE_SOURCE_DIR}/src/AdvancedTexture.cpp"
    "${CMAKE_SOURCE_DIR}/src/AdvancedTextureDemo.cpp"
    "${CMAKE_SOURCE_DIR}/src/Application_old.cpp"
    "${CMAKE_SOURCE_DIR}/src/ProceduralGeometry.cpp"
    "${CMAKE_SOURCE_DIR}/src/Enhanced3DModelLoader.h"
    "${CMAKE_SOURCE_DIR}/src/TextureManager.h"
)
list(REMOVE_ITEM SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AdvancedTextureDemo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AdvancedTextureDemo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AdvancedTexture.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AdvancedTexture.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Application_old.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProceduralGeometry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ProceduralGeometry.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Enhanced3DModelLoader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TextureManager.h"
)
# FontRenderer functionality has been integrated into UIText, no longer need separate files

# Manually add performance profiler files to ensure inclusion
list(APPEND SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PerformanceProfiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PerformanceProfiler.h"
)

# Manually add shadow mapping system files
list(APPEND SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ShadowMapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ShadowMapping.h"
)

set(SOURCES ${SOURCES} "comp3016.cpp" "comp3016.h")

# Add source code to this project's executable.
add_executable (comp3016 ${SOURCES})

# Add debug programs - temporarily disable non-existent files
# add_executable (debug_blackscreen debug_blackscreen.cpp)
# add_executable (embedded_test embedded_test.cpp)

# Advanced texture technology demo program - temporarily disabled to fix compilation issues
# add_executable (advanced_texture_test advanced_texture_test.cpp
#     src/AdvancedTexture.cpp
#     src/AdvancedTexture.h
#     src/AdvancedTextureDemo.cpp
#     src/AdvancedTextureDemo.h
#     src/Shader.cpp
#     src/Shader.h
# )

# Simplified advanced texture demo
# Simple texture demo program - temporarily disable non-existent files
# add_executable (simple_texture_demo simple_texture_demo.cpp)

# Link libraries
target_link_libraries(comp3016 PRIVATE 
    glfw 
    glad::glad 
    glm::glm 
    assimp::assimp
    unofficial::omniverse-physx-sdk::sdk
)

# Add Irrklang support (if available)
if(IRRKLANG_AVAILABLE)
    target_include_directories(comp3016 PRIVATE ${IRRKLANG_INCLUDE_DIR})
    target_link_libraries(comp3016 PRIVATE ${IRRKLANG_LIBRARY})
    target_compile_definitions(comp3016 PRIVATE IRRKLANG_ENABLED)
    
    # Copy DLL to output directory
    add_custom_command(TARGET comp3016 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${IRRKLANG_DLL}"
        $<TARGET_FILE_DIR:comp3016>)
        
    add_custom_command(TARGET comp3016 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${IRRKLANG_MP3_DLL}"
        $<TARGET_FILE_DIR:comp3016>)
        
    message(STATUS "Irrklang audio support enabled")
else()
    message(STATUS "Building without audio support")
endif()


# Include header file directories
target_include_directories(comp3016 PRIVATE 
    "src"
    ${Stb_INCLUDE_DIR}
)



if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET comp3016 PROPERTY CXX_STANDARD 20)
endif()

# Copy resource files to output directory (post-build)
add_custom_command(TARGET comp3016 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:comp3016>/resources
)

# Copy required DLL files to output directory
if(WIN32)
    # Copy vcpkg Release DLL files
    file(GLOB VCPKG_RELEASE_DLLS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/bin/*.dll")
    file(COPY ${VCPKG_RELEASE_DLLS} DESTINATION ${CMAKE_BINARY_DIR}/Release)
    
    # Copy vcpkg Debug DLL files
    file(GLOB VCPKG_DEBUG_DLLS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/debug/bin/*.dll")
    file(COPY ${VCPKG_DEBUG_DLLS} DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    
    # If Irrklang is available, copy its DLL files
    if(IRRKLANG_AVAILABLE)
        file(COPY ${IRRKLANG_DLL} DESTINATION ${CMAKE_BINARY_DIR}/Release)
        file(COPY ${IRRKLANG_MP3_DLL} DESTINATION ${CMAKE_BINARY_DIR}/Release)
        file(COPY ${IRRKLANG_DLL} DESTINATION ${CMAKE_BINARY_DIR}/Debug)
        file(COPY ${IRRKLANG_MP3_DLL} DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    endif()
endif()
