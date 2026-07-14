#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader/shader.h>

#include <string>
#include <vector>
using namespace std;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture
{
	unsigned int id;
	string type;
	string path;
};

class Mesh
{
public:

	// mesh data
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}

	void Draw(Shader& shader)
	{
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			// activates texture unit before binding
			glActiveTexture(GL_TEXTURE0 + i);
			string number;
			string name = textures[i].type;
			if (name == "texture_diffuse")
			{
				number = to_string(diffuseNr++);
			}
			else if (name == "texture_specular")
			{
				number = to_string(specularNr++);
			}
			else if (name == "texture_normal")
			{
				number = to_string(normalNr++);
			}

			shader.setInt((name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

private:

	// render data
	unsigned int VAO, VBO, EBO;

	void setupMesh()
	{
		// generates one vertex array using the ID of the vertex array
		glGenVertexArrays(1, &VAO);
		// generates one vertex buffer using the ID of the vertex buffer
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// binds vertex array
		glBindVertexArray(VAO);
		// binds the vertex buffer to the GL_ARRAY_BUFFER target and copies data into it's memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		// binds the element buffer to the GL_ARRAY_BUFFER target and copies data into it's memory
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// instructs how to read vertex attributes from the vertex buffer then enables it
		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		// normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(1);

		// texture
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(2);

		// unbinds vertex buffer and vertex array 
		glBindVertexArray(0);
	}

};

#endif
