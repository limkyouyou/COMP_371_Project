#ifndef MESH_H
#define MESH_H

#define GLEW_STATIC
#include <GL/glew.h>	// Holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>

#include <string>
#include <vector>
using namespace std;

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texture coordinates
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangnet
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};


class Mesh {
public:
	// Mesh data
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	// Constructor
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// Set the vertex buffer and its attribute pointers
		setupMesh();
	}

	// Render the mesh
	void Draw(Shader& shader) {
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);		// activate proper texture unit before binding
			// retreive texture number (the N in diffuse_textureN)
			string number;
			string name = textures[i].type;
			if (name == "texture_diffuse")
				number = to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = to_string(specularNr++);
			else if (name == "texture_normal")
				number = to_string(normalNr++);
			else if (name == "texture_height")
				number = to_string(heightNr++);

			// Set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
			// Bind the texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		// Draw Mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Set everything back to default
		glActiveTexture(GL_TEXTURE0);
	}

private:
	// Render data
	unsigned int VBO, EBO;

	// Initializes all the buffer objects/arrays
	void setupMesh() {
		// Create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// Struct memory layout is sequential.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translate to 3/2 floats which tanslates to a byte array
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		// Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// Vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// Vertex texture coordinates
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		// Vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		// Vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		glBindVertexArray(0);
	}
};


#endif