#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>
#include <functional>

using namespace std;



class Texture {
public:
	static GLuint framebuffer;
	static GLuint renderbuffer;

	unsigned int id;
	unsigned int width;
	unsigned int height;
	string name;
	GLenum type = GL_UNSIGNED_BYTE;
	GLint format;
	string path;
	GLenum target = GL_TEXTURE_2D;

	bool init = false;

	Texture() {
	}

	Texture(int id, string name, string path, GLenum target) {
		this->id = id;
		this->name = name;
		this->path = path;
		this->target = target;
	}

	Texture(int width, int height, GLenum type = GL_UNSIGNED_BYTE) {
		glGenTextures(1, &this->id);
		this->width = width;
		this->height = height;
		// this->format = options.format || gl.RGBA;
		this->format = GL_RGBA;
		// this->type = options.type || gl.UNSIGNED_BYTE;
		this->type = type;
		this->target = target;
		// var magFilter = options.filter || options.magFilter || gl.LINEAR;
		// var minFilter = options.filter || options.minFilter || gl.LINEAR;

		glBindTexture(GL_TEXTURE_2D, this->id);
		// glPixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, this->format, width, height, 0, this->format, this->type, NULL);
	
		
	}


	void bind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, this->id);
	}
	static void unbind(GLenum bind_unit)
	{
		glActiveTexture(GL_TEXTURE0 + bind_unit);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	bool canDrawTo() {
		if (!framebuffer) {
			glGenFramebuffers(1, &framebuffer);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->id, 0);
		auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		return result;
	}

	void drawTo(function<void()> callback) {

		int v[4];
		glGetIntegerv(GL_VIEWPORT, v);

		if (!framebuffer) {
			glGenFramebuffers(1, &framebuffer);
		}
		if (!renderbuffer) {
			glGenRenderbuffers(1, &renderbuffer);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);

		if (!init) {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, this->width, this->height);
			init = true;
		}
		//if (this->width != renderbuffer.width || this->height != renderbuffer.height) {
		//	renderbuffer.width = this->width;
		//	renderbuffer.height = this->height;
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width, this->height);
		//}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->id, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw exception("Rendering to this texture is not supported (incomplete framebuffer)");
		}
		glViewport(0, 0, this->width, this->height);

		callback();


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glViewport(v[0], v[1], v[2], v[3]);

	}

	void swapWith(Texture& other) {
		int temp;
		temp = other.id; other.id = this->id; this->id = temp;
		temp = other.width; other.width = this->width; this->width = temp;
		temp = other.height; other.height = this->height; this->height = temp;
		//string tmp;
		//tmp = other.type; other.type = this->type; this->type = tmp;
		//GLenum tmp2;
		//tmp2 = other.target; other.target = this->target; this->target = tmp2;
	}
};


