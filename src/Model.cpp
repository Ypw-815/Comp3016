/**
 * @file Model.cpp
 * @brief Implementation of 3D model loading and rendering system
 */

#include "Model.h"
#include "Texture.h"

/**
 * @brief Constructor - Initialize model and load from file
 * 
 * Creates a new Model object and immediately loads the specified 3D model file.
 * The gamma correction flag affects how textures are processed for proper lighting.
 * 
 * @param path File path to the 3D model file (supports OBJ, FBX, DAE, etc.)
 * @param gamma Whether to enable gamma correction for loaded textures
 */
Model::Model(const std::string& path, bool gamma) : gammaCorrection(gamma) {
    LoadModel(path);
}

/**
 * @brief Render all meshes in the model
 * 
 * Iterates through all meshes that make up this model and renders each one
 * using the provided shader program. The shader should already be activated
 * and have appropriate uniforms set before calling this function.
 * 
 * @param shader The shader program to use for rendering all meshes
 */
void Model::Draw(Shader& shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

/**
 * @brief Load 3D model from file using ASSIMP library
 * 
 * Uses ASSIMP to import the model file and convert it into renderable format.
 * Applies various post-processing steps to optimize the model data for OpenGL.
 * Provides detailed debugging information about the loaded model structure.
 * 
 * Post-processing steps applied:
 * - Triangulate: Convert all faces to triangles
 * - GenSmoothNormals: Generate smooth vertex normals
 * - FlipUVs: Flip texture coordinates for OpenGL coordinate system
 * - CalcTangentSpace: Calculate tangent and bitangent vectors for normal mapping
 * - JoinIdenticalVertices: Remove duplicate vertices to reduce memory usage
 * - ImproveCacheLocality: Optimize vertex order for better GPU cache performance
 * - FixInfacingNormals: Fix incorrectly oriented normals
 * - SortByPType: Sort primitives by type for better rendering efficiency
 * 
 * @param path File path to the 3D model file
 */
void Model::LoadModel(const std::string& path) {
    // Create ASSIMP importer and load the model with optimization flags
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate |           // Convert polygons to triangles
        aiProcess_GenSmoothNormals |      // Generate smooth normals if missing
        aiProcess_FlipUVs |               // Flip UV coordinates for OpenGL
        aiProcess_CalcTangentSpace |      // Calculate tangent space for normal mapping
        aiProcess_JoinIdenticalVertices | // Remove duplicate vertices
        aiProcess_ImproveCacheLocality |  // Optimize for GPU cache
        aiProcess_FixInfacingNormals |    // Fix incorrect normals
        aiProcess_SortByPType);           // Sort primitives by type
    
    // Check if model loading was successful
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        std::cerr << "Failed to load model: " << path << std::endl;
        return;
    }
    
    // Print successful loading information
    std::cout << "ASSIMP: Successfully loaded model: " << path << std::endl;
    std::cout << "ASSIMP: Meshes: " << scene->mNumMeshes << ", Materials: " << scene->mNumMaterials << std::endl;
    
    // Debug: Print detailed mesh information for troubleshooting
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        std::cout << "  Mesh " << i << ": " << mesh->mNumVertices << " vertices, " 
                  << mesh->mNumFaces << " faces" << std::endl;
        
        // Print vertex position range for debugging geometry issues
        if (mesh->mNumVertices > 0) {
            std::cout << "    First vertex: (" << mesh->mVertices[0].x << ", " 
                      << mesh->mVertices[0].y << ", " << mesh->mVertices[0].z << ")" << std::endl;
            std::cout << "    Last vertex: (" << mesh->mVertices[mesh->mNumVertices-1].x << ", " 
                      << mesh->mVertices[mesh->mNumVertices-1].y << ", " 
                      << mesh->mVertices[mesh->mNumVertices-1].z << ")" << std::endl;
        }
    }
    
    // Debug: Print material information for texture/lighting troubleshooting
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        std::cout << "  Material " << i << ": " << name.C_Str() << std::endl;
        
        // Extract and print material color properties
        aiColor3D diffuse, ambient, specular;
        float shininess;
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            std::cout << "    Diffuse: (" << diffuse.r << ", " << diffuse.g << ", " << diffuse.b << ")" << std::endl;
        }
        if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS) {
            std::cout << "    Ambient: (" << ambient.r << ", " << ambient.g << ", " << ambient.b << ")" << std::endl;
        }
        if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS) {
            std::cout << "    Specular: (" << specular.r << ", " << specular.g << ", " << specular.b << ")" << std::endl;
        }
        if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            std::cout << "    Shininess: " << shininess << std::endl;
        }
    }
    
    // Extract directory path for relative texture loading
    directory = path.substr(0, path.find_last_of('/'));

    // Start recursive processing from the root node
    ProcessNode(scene->mRootNode, scene);
}

/**
 * @brief Recursively process ASSIMP scene nodes
 * 
 * ASSIMP organizes loaded models in a tree structure where each node can contain
 * multiple meshes and have child nodes. This function recursively traverses this
 * tree structure and processes all meshes found in each node.
 * 
 * The recursive approach ensures that complex models with hierarchical structures
 * (like character models with bones) are properly processed.
 * 
 * @param node Current ASSIMP node being processed
 * @param scene Complete ASSIMP scene data containing all meshes and materials
 */
void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    // Process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // The node contains indices to meshes in the scene's mesh array
        // Retrieve the actual mesh and convert it to our format
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }
    
    // Recursively process each child node
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

/**
 * @brief Convert ASSIMP mesh to our Mesh format
 * 
 * Processes an individual mesh from ASSIMP and converts it into our own Mesh format
 * that can be rendered with OpenGL. This involves extracting vertex data, indices,
 * and material information.
 * 
 * For each vertex, we extract:
 * - Position coordinates (x, y, z)
 * - Normal vectors for lighting calculations
 * - Texture coordinates for texture mapping
 * - Tangent and bitangent vectors for normal mapping (if available)
 * 
 * @param mesh ASSIMP mesh to convert
 * @param scene Complete ASSIMP scene (needed for material access)
 * @return Converted Mesh object ready for OpenGL rendering
 */
Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    // Data containers for the mesh
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Process vertices - extract all vertex attribute data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector; // Temporary vector for data conversion
        
        // Extract vertex position coordinates
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        
        // Extract vertex normals (if available)
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        
        // Extract texture coordinates (UV mapping)
        if (mesh->mTextureCoords[0]) { // Check if texture coordinates exist
            glm::vec2 vec;
            // ASSIMP supports up to 8 texture coordinate sets, we use the first one
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            
            // Extract tangent and bitangent vectors for normal mapping
            if (mesh->HasTangentsAndBitangents()) {
                // Tangent vector
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                
                // Bitangent vector
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f); // Default UV coordinates

        vertices.push_back(vertex);
    }
    
    // Process indices - extract face/triangle information
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices for this face and add them to the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    
    // Process materials - extract texture information
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    
    // Load different types of textures for comprehensive material support
    
    // 1. Diffuse maps (base color/albedo textures)
    std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    
    // 2. Specular maps (shininess/reflection textures)
    std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    
    // 3. Normal maps (surface detail textures for advanced lighting)
    std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    
    // 4. Height maps (displacement/parallax mapping)
    std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // Create and return the completed mesh object
    return Mesh(vertices, indices, textures);
}

/**
 * @brief Load textures of a specific type from material
 * 
 * Extracts texture file paths from ASSIMP material and loads the actual texture files.
 * Implements texture caching to avoid loading the same texture multiple times,
 * which improves performance and reduces memory usage.
 * 
 * Supports various texture types:
 * - Diffuse: Base color textures
 * - Specular: Reflection/shininess textures  
 * - Normal: Surface detail for advanced lighting
 * - Height: Displacement/parallax effects
 * 
 * @param mat ASSIMP material containing texture file references
 * @param type ASSIMP texture type to extract (diffuse, specular, etc.)
 * @param typeName String identifier for the texture type (used in shaders)
 * @return Vector of loaded Texture objects
 */
std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    
    // Process all textures of the specified type in this material
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str); // Get texture file path
        
        // Check if this texture was already loaded (texture caching)
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]); // Reuse cached texture
                skip = true; // Skip loading since we already have it
                break;
            }
        }
        
        // Load new texture if not found in cache
        if (!skip) {   
            // Create new texture with full path (directory + filename)
            Texture texture(directory + "/" + str.C_Str(), typeName);
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // Add to cache for future use
        }
    }
    return textures;
}
