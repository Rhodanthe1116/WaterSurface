#pragma once
#ifndef WATER_H
#define WATER_H



#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


#include "RenderUtilities/Shader.h"
#include "model.h"

using namespace std;


class Water
{
public:
	Mesh* plane = Mesh::plane();
	//Model plane{ "assets/models/water.obj" };
	int filter = GL_LINEAR;
	//Texture textureA{ 2048, 2048, GL_FLOAT };
	Texture textureA{ 256, 256, GL_FLOAT };
	Texture textureB{ 256, 256, GL_FLOAT };
	Shader* simpleShader = new
		Shader(
			PROJECT_DIR "/src/shaders/simple.vert",
			nullptr, nullptr, nullptr,
			PROJECT_DIR "/src/shaders/simple.frag");
	Shader* dropShader = new
		Shader(
			PROJECT_DIR "/src/shaders/vertex.vert",
			nullptr, nullptr, nullptr,
			PROJECT_DIR "/src/shaders/drop.frag");
	Shader* updateShader = new
		Shader(
			PROJECT_DIR "/src/shaders/vertex.vert",
			nullptr, nullptr, nullptr,
			PROJECT_DIR "/src/shaders/update.frag");
	Shader* normalShader = new
		Shader(
			PROJECT_DIR "/src/shaders/vertex.vert",
			nullptr, nullptr, nullptr,
			PROJECT_DIR "/src/shaders/normal.frag");

	void addDrop(double x, double y, double radius, double strength) {
		auto f = [this, x, y, radius, strength]() {
			textureA.bind(0);

			dropShader->Use();
			glUniform1i(glGetUniformLocation(dropShader->Program, "water"), 0);
			glUniform2f(glGetUniformLocation(dropShader->Program, "center"),
				x,
				y
			);
			glUniform1f(glGetUniformLocation(dropShader->Program, "radius"),
				radius
			);
			glUniform1f(glGetUniformLocation(dropShader->Program, "strength"),
				strength
			);
			plane->simpleDraw(*dropShader);
		};
		this->textureB.drawTo(f);
		this->textureB.swapWith(this->textureA);
		
	}

	void stepSimulation() {
		//auto draw = [this]() {
		//	textureA.bind(0);
		//	//plane.meshes[0].textures[0].bind(0);
		//	simpleShader->Use();
		//	glUniform1i(glGetUniformLocation(simpleShader->Program, "water"), 0);
		//	// Prepare common matrix
		//	glm::mat4 P;
		//	glm::mat4 V;
		//	glGetFloatv(GL_PROJECTION_MATRIX, &P[0][0]);
		//	glGetFloatv(GL_MODELVIEW_MATRIX, &V[0][0]);
		//	glm::mat4 scale_matrix = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
		//	glm::mat4 translate_matrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
		//	glm::mat4 M = translate_matrix * scale_matrix;
		//	glUniformMatrix4fv(glGetUniformLocation(simpleShader->Program, "M"), 1, GL_FALSE, &M[0][0]);
		//	glUniformMatrix4fv(glGetUniformLocation(simpleShader->Program, "V"), 1, GL_FALSE, &V[0][0]);
		//	glUniformMatrix4fv(glGetUniformLocation(simpleShader->Program, "P"), 1, GL_FALSE, &P[0][0]);
		//	plane.simpleDraw(*simpleShader);
		//};



		auto f = [this]() {
			textureA.bind(0);
			updateShader->Use();
			glUniform2f(glGetUniformLocation(this->updateShader->Program, "delta"),
				1.0 / double(textureA.width),
				1.0 / double(textureA.height)
			);
			glUniform1i(glGetUniformLocation(this->updateShader->Program, "water"), 0);
			plane->simpleDraw(*updateShader);
		};
		this->textureB.drawTo(f);
		this->textureB.swapWith(this->textureA);

	}

	void updateNormals() {

		auto f = [this]() {
			textureA.bind(0);
			normalShader->Use();
			glUniform2f(glGetUniformLocation(normalShader->Program, "delta"),
				1.0 / double(textureA.width),
				1.0 / double(textureA.height)
			);
			glUniform1i(glGetUniformLocation(normalShader->Program, "water"), 0);
			plane->simpleDraw(*normalShader);
		};
		this->textureB.drawTo(f);
		this->textureB.swapWith(this->textureA);

	}
	// private:

};
#endif