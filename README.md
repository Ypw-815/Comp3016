# COMP3016 - Immersive Game Technologies

A 3D treasure hunting game built with OpenGL 4.1, featuring advanced graphics techniques, physics simulation, and interactive gameplay mechanics.

## 🎮 Features

### Core Graphics

- **OpenGL 4.1 Core Profile** with modern shader pipeline
- **Advanced Lighting System** with Blinn-Phong lighting model
- **Shadow Mapping** with multiple filtering options (PCF, linear, nearest)
- **Post-Processing Effects** for enhanced visual quality
- **Terrain Generation** with procedural height mapping

### Gameplay Mechanics

- **Treasure Hunting Game**: Collect ancient keys and treasure chests
- **Ball Shooting Mini-Game**: Interactive target shooting with physics
- **3D Model Loading**: Support for OBJ/MTL files via ASSIMP
- **Physics Integration**: Realistic collision detection and physics simulation
- **Interactive GUI**: Real-time game status and technical information panels

### Technical Features

- **Performance Profiling**: Real-time performance monitoring
- **Event System**: Decoupled event-driven architecture
- **Resource Management**: Efficient texture and model loading
- **Multi-State Game System**: Menu, gameplay, settings, and victory states
- **Camera System**: First-person camera with mouse and keyboard controls
- **C++20 Standard**: Modern C++ features and optimizations

### Graphics Techniques

- **Vertex/Fragment Shaders**: Custom GLSL shaders for lighting and effects
- **Texture Mapping**: Diffuse and specular texture support
- **Normal Mapping**: Enhanced surface detail rendering
- **Shadow Mapping**: Real-time shadow rendering with configurable quality
- **Terrain Shading**: Procedural terrain with height-based coloring

## 🛠️ Dependencies

- **GLFW3**: Window management and input handling
- **GLAD**: OpenGL loading library
- **GLM**: Mathematics library for 3D graphics
- **ASSIMP**: 3D model loading (OBJ, MTL formats)
- **STB Image**: Image loading and processing
- **PhysX**: Physics simulation (optional)

## 📋 Requirements

### System Requirements

- **OS**: Windows 10/11 (64-bit)
- **Graphics**: OpenGL 4.1 compatible GPU
- **Memory**: 4GB RAM minimum
- **Storage**: 500MB available space

### Development Requirements

- **Visual Studio 2019/2022** or **Ninja Build System**
- **CMake 3.8+** (with presets support)
- **vcpkg** (automatically managed by build system)
- **Command Line Tools**: PowerShell or Command Prompt
- **Internet Connection**: Required for first-time vcpkg setup

## 🚀 Installation & Build

### Prerequisites

1. Install **Visual Studio 2019/2022** with C++ development tools
2. Install **CMake** (version 3.8 or higher)
3. Ensure you have an **OpenGL 4.1 compatible graphics card**
4. Open **PowerShell** or **Command Prompt** as administrator

### Building the Project

#### Method 1: Using CMake Presets (Recommended)

1. **Clone the repository**

   ```bash
   git clone https://github.com/Ypw-815/Comp3016.git
   cd comp3016
   ```

2. **Configure and build using presets**

   ```bash
   # Debug build with Ninja (fastest)
   cmake --preset x64-debug
   cmake --build --preset debug
   
   # Release build with Ninja
   cmake --preset x64-release
   cmake --build --preset release
   
   # Alternative: Visual Studio build
   cmake --preset x64-debug-vs
   cmake --build --preset debug-vs
   ```

#### Method 2: Manual CMake Configuration

1. **Configure the project**

   ```bash
   cmake -B build -S .
   ```

2. **Build the project**

   ```bash
   cmake --build build --config Debug
   ```

3. **Alternative Release build**

   ```bash
   cmake --build build --config Release
   ```

#### Available Build Presets

- **x64-debug**: Debug build with Ninja (recommended)
- **x64-release**: Release build with Ninja
- **x64-debug-vs**: Debug build with Visual Studio
- **x64-release-vs**: Release build with Visual Studio

### Build Output

The executable will be created in:

- `build/Debug/comp3016.exe` (Debug build)
- `build/Release/comp3016.exe` (Release build)

### First-Time Setup Notes

- **vcpkg Auto-Download**: On first build, the system will automatically download and configure vcpkg
- **Internet Required**: Initial setup requires internet connection for dependency download
- **DLL Copying**: Required DLL files are automatically copied to output directories
- **Resource Files**: Game assets are automatically copied to the executable directory

## 🎯 Running the Application

### Launch the Game

```bash
# Debug version
build/Debug/comp3016.exe

# Release version
build/Release/comp3016.exe
```

### Controls

- **WASD**: Move
- **Mouse**: Look around
- **ESC**: Exit application
- **E**: Interact with objects 

## 🎮 Game 

### Treasure Hunting Game

- **Objective**: Find and collect ancient keys scattered across the ruins
- **Features**: 
  - Interactive 3D models (keys, treasure chests)
  - Physics-based collision detection
  - Progress tracking and scoring system
  - Real-time game status display

## 📁 Project Structure

```
comp3016/
├── src/                    # Source code
│   ├── Application.cpp     # Main application class
│   ├── Camera.cpp         # Camera system
│   ├── Shader.cpp         # Shader management
│   ├── Model.cpp          # 3D model loading
│   ├── PhysicsManager.cpp # Physics simulation
│   ├── TerrainGenerator.cpp # Procedural terrain
│   └── CustomGUI/         # GUI system
├── resources/             # Game assets
│   ├── shaders/          # GLSL shader files
│   ├── models/           # 3D models (OBJ/MTL)
│   └── textures/         # Texture files
├── external/             # External libraries
├── CMakePresets.json    # CMake presets configuration
└── CMakeLists.txt       # CMake configuration
```

## 🔧 Technical Implementation

### Graphics Pipeline

- **Vertex Shaders**: Transform vertices and pass data to fragment shaders
- **Fragment Shaders**: Calculate lighting, shadows, and final pixel colors
- **Shadow Mapping**: Depth-based shadow rendering with multiple filtering options
- **Post-Processing**: Screen-space effects for enhanced visual quality

### Physics System

- **Collision Detection**: Ray-casting for object interaction
- **Physics Simulation**: Realistic object movement and collision response
- **Terrain Integration**: Height-based terrain collision

### Game Architecture

- **Event-Driven Design**: Decoupled systems using event system
- **State Management**: Clean state transitions between game modes
- **Resource Management**: Efficient loading and caching of assets
- **Performance Monitoring**: Real-time profiling and optimization

## 🎨 Graphics Features

### Lighting System

- **Ambient Lighting**: Base illumination for all objects
- **Diffuse Lighting**: Directional light reflection
- **Specular Lighting**: Highlight reflections
- **Blinn-Phong Model**: Enhanced specular calculations

### Shadow System

- **Shadow Mapping**: Real-time shadow rendering
- **Multiple Filter Modes**: PCF, linear, and nearest neighbor filtering
- **Configurable Quality**: Adjustable shadow map resolution and bias
- **Frustum Culling**: Performance optimization for shadow rendering

### Terrain System

- **Procedural Generation**: Height-based terrain creation
- **Texture Blending**: Multiple texture layers
- **Height Mapping**: Realistic terrain elevation
- **Optimized Rendering**: Efficient terrain mesh generation

## 📝 Development Notes

### Code Style

- **C++20** standard compliance
- **Object-Oriented Design** with clear separation of concerns
- **Modern OpenGL** practices (no deprecated functions)
- **Comprehensive Error Handling** with detailed logging
- **CMake Presets**: Standardized build configurations

### Performance Considerations

- **Frustum Culling**: Reduces rendering overhead
- **Level-of-Detail**: Adaptive detail based on distance
- **Texture Compression**: Optimized texture storage
- **Shader Optimization**: Efficient GLSL code

## 🔗 Links

- **GitHub Repository**: [https://github.com/Ypw-815/Comp3016]
- **YouTube Demo Video**: [https://youtu.be/your-video-id](https://youtu.be/your-video-id)


*This project demonstrates advanced OpenGL programming techniques, modern game development practices, and comprehensive 3D graphics implementation.* 
