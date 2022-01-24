#pragma once

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

#include "RenderUtilities/Texture.h"
#include "RenderUtilities/Vertex.h"

using namespace std;

class Mesh {
public:
	// mesh Data
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	unsigned int VAO;

	// constructor
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}

	static Mesh plane() {
		//options = options || {};
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;
		//Mesh mesh = new Mesh(options);
		// detailX = options.detailX || options.detail || 1;
		// detailY = options.detailY || options.detail || 1;
		int detailX = 1;
		int detailY = 1;

		for (unsigned int y = 0; y <= detailY; y++) {
			auto t = double(y) / double(detailY);
			for (unsigned int x = 0; x <= detailX; x++) {
				auto s = double(x) / double(detailX);
				vertices.push_back(
					make_vertex(2 * s - 1, 2 * t - 1, 0)
				);
				//if (mesh.coords) mesh.coords.push([s, t]);
				//if (mesh.normals) mesh.normals.push([0, 0, 1]);
				if (x < detailX && y < detailY) {
					unsigned int i = x + y * (detailX + 1);
					indices.insert(indices.end(), { i, i + 1, i + detailX + 1 });
					indices.insert(indices.end(), { i + detailX + 1, i + 1, i + detailX + 2 });
				}
			}
		}

		return Mesh{ vertices , indices , textures };
	}

	// render the mesh
	void Draw(Shader& shader)
	{
		//cout << "Draw mesh with vertices: " << vertices.size() << endl;

		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;

		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
			// retrieve texture number (the N in diffuse_textureN)
			string number = "";
			string name = textures[i].name;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to stream
			else if (name == "texture_normal")
				number = std::to_string(normalNr++); // transfer unsigned int to stream
			else if (name == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to stream

			//cout << i << " " << name + number << endl;
			// now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
			// and finally bind the texture
			glBindTexture(textures[i].target, textures[i].id);
		}

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	// render the mesh
	void simpleDraw(Shader& shader)
	{
		// draw mesh
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));


		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		//glActiveTexture(GL_TEXTURE0);
	}

private:
	// render data
	unsigned int VBO, EBO;

	// initializes all the buffer objects/arrays
	void setupMesh()
	{
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		//// vertex tangent
		//glEnableVertexAttribArray(3);
		//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		//// vertex bitangent
		//glEnableVertexAttribArray(4);
		//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		glBindVertexArray(0);
	}
};
#endif