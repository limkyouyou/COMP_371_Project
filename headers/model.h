#ifndef MODEL_H
#define MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shader.h>
#include <mesh.h>
#include <textureLoader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
using namespace std;


class Model {
public:
	// Mode data
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	// Model Coordinates
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	// Constructor, expects a filepath to a 3D model.
	Model(string const& path, bool gamma = false)
		: gammaCorrection(gamma)
	{
		loadModel(path);
	}

	// Draw the model
	void Draw(Shader& shader) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, scale);

		shader.setMat4("model", model);

		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(shader);
		}
	}

private:
	// Load a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const& path) {
		// read file via ASSIMP
		Assimp::Importer importer;
		//const aiScene* scene = importer.ReadFile(path,
		//	aiProcess_Triangulate |            // Ensure all faces are triangles
		//	aiProcess_GenSmoothNormals |       // Generate smooth normals if not provided
		//	aiProcess_FlipUVs |                // Flip V texture coordinate (OpenGL-style)
		//	aiProcess_CalcTangentSpace |       // Needed for normal/parallax mapping
		//	aiProcess_JoinIdenticalVertices |  // Combine duplicate vertices
		//	aiProcess_OptimizeMeshes |         // Combine small meshes
		//	aiProcess_ValidateDataStructure    // Check for correctness
		//);
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// chek for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}
		// Retrieve the directory path of the filepath
		// directory = path.substr(0, path.find_last_not_of('/'));
		directory = std::filesystem::path(path).parent_path().string();	// Supposed to be Mac ans Win compatible

		// Process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// Process a node in a recursive fashion. Processes each individual mesh located at the node and repeat this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene) {
		// Process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			// The node object only contains indices to index the actual objects in the scene.
			// The scene contains all the datam node is just to keep stuff organized (like relations between nodes).
			// string meshName = node->mName.C_Str(); // name of the current node
			
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			string meshName = mesh->mName.C_Str();
			meshes.push_back(processMesh(mesh, scene, meshName));
		}
		// After we've processed all the meshes (if any) we then recursively process each of the children nodes.
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene, const string& meshName) {
		// Data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;
			// Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// Normals
			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			// Texture coordinates
			if (mesh->mTextureCoords[0]) {	// Check if the mesh contains texture coordinates
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
				// Tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
				// Bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else {
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}
		// Walk through each of the mesh's faces and retrieve the corresponding vertex indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			// Retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// Process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// we assume a convention for sampler names in the shader. Each diffuse texture should be named
		// as 'texture_diffuseN' where N s a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
		// Same applies to other texture as the following list summarized:
		// diffuse: texture_diffuseN
		// specular: texure_specularN
		// normal: texture_normalN

		// DEBUG
		/*cout << "material index: " << mesh->mMaterialIndex << endl;
		cout << "within bounds: " << (mesh->mMaterialIndex < scene->mNumMaterials) << endl;
		cout << "scene has material: " << (scene->HasMaterials()) << endl;
		cout << "material is not null: " << (material != nullptr) << endl;*/
		// DEBUG: Print all available texture types
		/*for (int t = aiTextureType_NONE; t <= aiTextureType_UNKNOWN; ++t) {
			aiTextureType texType = static_cast<aiTextureType>(t);
			unsigned int texCount = material->GetTextureCount(texType);
			if (texCount > 0) {
				cout << "Texture type " << t << " (" << texCount << " textures)" << endl;
				aiString str;
				material->GetTexture(texType, 0, &str);
				cout << "Texture path: " << str.C_Str() << endl;
			}
		}*/
		/*cout << "Material has properties: " << material->mNumProperties << endl;
		for (unsigned int i = 0; i < material->mNumProperties; ++i) {
			aiMaterialProperty* prop = material->mProperties[i];
			cout << "Key: " << prop->mKey.C_Str() << ", Semantic: " << prop->mSemantic << ", Index: " << prop->mIndex << endl;
		}*/
		/*aiString name;
		material->Get(AI_MATKEY_NAME, name);
		std::cout << "Material name: " << name.C_Str() << std::endl;*/

		// 1. Diffuse maps
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. Specular maps
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. Normal maps
		vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. Height maps
		vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		// Return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures, meshName);
	}

	// Check all material textures of a given type and load the texture if they'r re not loaded yet.
	// The required info is returned as a texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector <Texture> textures;
		// cout << "get texture count " << static_cast<int>(mat->GetTextureCount(type)) << endl;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			// Check if texture was loaded before and if so, continue to next iteration.
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {
				// If texture hasn't been loaded already, loade it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}

		return textures;
	}
	
};


#endif