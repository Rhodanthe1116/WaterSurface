/************************************************************************
	 File:        WaterView.cpp

	 Author:
				  Michael Gleicher, gleicher@cs.wisc.edu

	 Modifier
				  Yu-Chi Lai, yu-chi@cs.wisc.edu

	 Comment:
						The WaterView is the window that actually shows the
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within
						a TrainWindow
						that is the outer window with all the widgets.
						The WaterView needs
						to be aware of the window - since it might need to
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know
						about it (beware circular references)

	 Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glu.h>

#include "WaterView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"
//#include "model.h"
#include "water.h"

GLuint Texture::framebuffer = NULL;
GLuint Texture::renderbuffer = NULL;

//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
WaterView::
WaterView(int x, int y, int w, int h, const char* l)
	: Fl_Gl_Window(x, y, w, h, l)
	//========================================================================
{
	mode(FL_RGB | FL_ALPHA | FL_DOUBLE | FL_STENCIL);

	resetArcball();
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void WaterView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 5, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int WaterView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event))
			return 1;

	// remember what button was used
	static int last_push;

	switch (event) {
		// Mouse button being pushed event
	case FL_PUSH:
		last_push = Fl::event_button();
		// if the left button be pushed is left mouse button
		if (last_push == FL_LEFT_MOUSE) {
			doPick();

			/*glReadBuffer(GL_COLOR_ATTACHMENT0);
			glm::vec3 uv;
			glReadPixels(Fl::event_x(), h() - Fl::event_y(), 1, 1, GL_RGB, GL_FLOAT, &uv[0]);

			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

			if (uv.b != 1.0) {
				cout << "drop : " << uv.x << ' ' << uv.y << endl;
				water->addDrop(uv.x, uv.z, 0.3, 0.1);
			}*/
			water->addDrop(0, 0, 0.03, 0.01);



			damage(1);
			return 1;
		};
		break;

		// Mouse button release event
	case FL_RELEASE: // button release
		damage(1);
		last_push = 0;
		return 1;

		// Mouse button drag event
	case FL_DRAG:

		// Compute the new control point position
		if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
			ControlPoint* cp = &m_pTrack->points[selectedCube];

			double r1x, r1y, r1z, r2x, r2y, r2z;
			getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

			double rx, ry, rz;
			mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
				static_cast<double>(cp->pos.x),
				static_cast<double>(cp->pos.y),
				static_cast<double>(cp->pos.z),
				rx, ry, rz,
				(Fl::event_state() & FL_CTRL) != 0);

			cp->pos.x = (float)rx;
			cp->pos.y = (float)ry;
			cp->pos.z = (float)rz;
			damage(1);
		}
		break;

		// in order to get keyboard events, we need to accept focus
	case FL_FOCUS:
		return 1;

		// every time the mouse enters this window, aggressively take focus
	case FL_ENTER:
		focus(this);
		break;

	case FL_KEYBOARD:
		int k = Fl::event_key();
		int ks = Fl::event_state();
		if (k == 'p') {
			// Print out the selected control point information
			if (selectedCube >= 0)
				printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
					selectedCube,
					m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					m_pTrack->points[selectedCube].orient.x,
					m_pTrack->points[selectedCube].orient.y,
					m_pTrack->points[selectedCube].orient.z);
			else
				printf("Nothing Selected\n");

			return 1;
		};
		break;
	}

	return Fl_Gl_Window::handle(event);
}
Model* wave = nullptr;



void  WaterView::prepareDevice() {
	//Tutorial: https://ffainelli.github.io/openal-example/
	this->device = alcOpenDevice(NULL);
	if (!this->device)
		puts("ERROR::NO_AUDIO_DEVICE");

	ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE)
		puts("Enumeration not supported");
	else
		puts("Enumeration supported");

	this->context = alcCreateContext(this->device, NULL);
	if (!alcMakeContextCurrent(context))
		puts("Failed to make context current");

	this->source_pos = glm::vec3(0.0f, 5.0f, 0.0f);

	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
	alListener3f(AL_VELOCITY, 0, 0, 0);
	alListenerfv(AL_ORIENTATION, listenerOri);

	alGenSources((ALuint)1, &this->source);
	alSourcef(this->source, AL_PITCH, 1);
	alSourcef(this->source, AL_GAIN, 1.0f);
	alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
	alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
	alSourcei(this->source, AL_LOOPING, AL_TRUE);

	alGenBuffers((ALuint)1, &this->buffer);

	ALsizei size, freq;
	ALenum format;
	ALvoid* data;
	ALboolean loop = AL_TRUE;

	//Material from: ThinMatrix
	alutLoadWAVFile((ALbyte*)PROJECT_DIR "/Audios/bounce.wav", &format, &data, &size, &freq, &loop);
	alBufferData(this->buffer, format, data, size, freq);
	alSourcei(this->source, AL_BUFFER, this->buffer);

	if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
		puts("TYPE::STEREO");
	else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
		puts("TYPE::MONO");

	alSourcePlay(this->source);

	// cleanup context
	//alDeleteSources(1, &source);
	//alDeleteBuffers(1, &buffer);
	//device = alcGetContextsDevice(context);
	//alcMakeContextCurrent(NULL);
	//alcDestroyContext(context);
	//alcCloseDevice(device);
}

tuple<double, double, double, double> WaterView::getLight() {
	ControlPoint lightPoint = this->m_pTrack->points[0];
	float lightDistanceScale = pow(10, tw->lightDistanceScale->value());
	float lightPower = pow(10, tw->lightPower->value());
	return make_tuple(
		lightPoint.pos.x * lightDistanceScale,
		lightPoint.pos.y * lightDistanceScale,
		lightPoint.pos.z * lightDistanceScale,
		lightPower
	);
}
unsigned int loadCubemap(vector<const GLchar*> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


Mesh* tiles_cube_mesh = nullptr;
Texture tiles_cube_texture;
void WaterView::initTiles() {
	// Load model
	const int tileNum = 12;
	// data to fill
	vector<Vertex> vertices = {
		// positions          
		make_vertex(-1.0f,  1.0f / tileNum * 2, -1.0f),
		make_vertex(-1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f,  1.0f / tileNum * 2, -1.0f),
		make_vertex(-1.0f,  1.0f / tileNum * 2, -1.0f),

		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(-1.0f, -1.0f, -1.0f),
		make_vertex(-1.0f,  1.0f / tileNum * 2, -1.0f),
		make_vertex(-1.0f,  1.0f / tileNum * 2, -1.0f),
		make_vertex(-1.0f,  1.0f / tileNum * 2,  1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),

		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f,  1.0f),
		make_vertex(1.0f,  1.0f / tileNum * 2,  1.0f),
		make_vertex(1.0f,  1.0f / tileNum * 2,  1.0f),
		make_vertex(1.0f,  1.0f / tileNum * 2, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),

		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(-1.0f,  1.0f / tileNum * 2,  1.0f),
		make_vertex(1.0f,  1.0f / tileNum * 2,  1.0f),
		make_vertex(1.0f,  1.0f / tileNum * 2,  1.0f),
		make_vertex(1.0f, -1.0f,  1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),

		// No Top ...

		make_vertex(-1.0f, -1.0f, -1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(1.0f, -1.0f,  1.0f)
	};
	vector<unsigned int> indices;
	vector<Texture> textures;

	// indicex
	// simple vertices without indexing
	for (int i = 0; i < vertices.size(); i++)
	{
		indices.push_back(i);
	}

	// textures
	vector<const GLchar*> faces = {
		"images/tiles.jpg",
		"images/tiles.jpg",
		"images/tiles.jpg",
		"images/tiles.jpg",
		"images/tiles.jpg",
		"images/tiles.jpg",
	};
	int texture_id = loadCubemap(faces);
	Texture texture{ texture_id, "tiles_cube", "images/tiles.jpg", GL_TEXTURE_CUBE_MAP };
	textures.push_back(texture);

	tiles_cube_mesh = new Mesh(vertices, indices, textures);

	tiles_cube_texture = texture;

}

void WaterView::drawTiles(glm::mat4 V, glm::mat4 P) {
	glm::mat4 scale_matrix = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
	glm::mat4 translate_matrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
	glm::mat4 M = translate_matrix * scale_matrix;

	// Shading	
	glEnable(GL_CULL_FACE);
	//glDepthMask(GL_FALSE);

	this->tiles_shader->Use();
	double lx, ly, lz, l;
	tie(lx, ly, lz, l) = getLight();
	glUniform3f(glGetUniformLocation(this->tiles_shader->Program, "LightPosition_worldspace"),
		lx, ly, lz
	);
	//glUniform1f(glGetUniformLocation(this->skybox_shader->Program, "skybox_texture"), skybox_texture_id);
	glUniformMatrix4fv(glGetUniformLocation(this->tiles_shader->Program, "M"), 1, GL_FALSE, &M[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->tiles_shader->Program, "V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->tiles_shader->Program, "P"), 1, GL_FALSE, &P[0][0]);

	//tiles_cube_texture.bind(0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(tiles_cube_texture.target, tiles_cube_texture.id);
	glUniform1i(glGetUniformLocation(this->tiles_shader->Program, (tiles_cube_texture.name).c_str()), 0);
	tiles_cube_mesh->simpleDraw(*this->tiles_shader);

	glDisable(GL_CULL_FACE);
}

Mesh* sky_mesh = nullptr;
Texture skybox_texture;
void WaterView::initSkybox() {
	// Load model
	// data to fill
	vector<Vertex> vertices = {
		// positions          
		// 
		make_vertex(-1.0f,  1.0f, -1.0f),
		make_vertex(-1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f,  1.0f, -1.0f),
		make_vertex(-1.0f,  1.0f, -1.0f),

		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(-1.0f, -1.0f, -1.0f),
		make_vertex(-1.0f,  1.0f, -1.0f),
		make_vertex(-1.0f,  1.0f, -1.0f),
		make_vertex(-1.0f,  1.0f,  1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),

		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f,  1.0f),
		make_vertex(1.0f,  1.0f,  1.0f),
		make_vertex(1.0f,  1.0f,  1.0f),
		make_vertex(1.0f,  1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),

		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(-1.0f,  1.0f,  1.0f),
		make_vertex(1.0f,  1.0f,  1.0f),
		make_vertex(1.0f,  1.0f,  1.0f),
		make_vertex(1.0f, -1.0f,  1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),

		// Top
		make_vertex(-1.0f,  1.0f, -1.0f),
		make_vertex(1.0f,  1.0f, -1.0f),
		make_vertex(1.0f,  1.0f,  1.0f),
		make_vertex(1.0f,  1.0f,  1.0f),
		make_vertex(-1.0f,  1.0f,  1.0f),
		make_vertex(-1.0f,  1.0f, -1.0f),

		make_vertex(-1.0f, -1.0f, -1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(1.0f, -1.0f, -1.0f),
		make_vertex(-1.0f, -1.0f,  1.0f),
		make_vertex(1.0f, -1.0f,  1.0f)
	};
	vector<unsigned int> indices;
	vector<Texture> textures;

	// indicex
	// simple vertices without indexing
	for (int i = 0; i < vertices.size(); i++)
	{
		indices.push_back(i);
	}

	// textures
	vector<const GLchar*> faces = {
		"images/skybox/right.jpg",
		"images/skybox/left.jpg",
		"images/skybox/top.jpg",
		"images/skybox/bottom.jpg",
		"images/skybox/back.jpg",
		"images/skybox/front.jpg",
	};
	int skybox_texture_id = loadCubemap(faces);
	Texture texture{ skybox_texture_id, "skybox_cube", "images/skybox", GL_TEXTURE_CUBE_MAP };
	textures.push_back(texture);

	sky_mesh = new Mesh(vertices, indices, textures);

	skybox_texture = texture;

}

void WaterView::drawSkybox(glm::mat4 V, glm::mat4 P) {


	// Shading	
	glDepthMask(GL_FALSE);

	glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
	this->skybox_shader->Use();
	//glUniform1f(glGetUniformLocation(this->skybox_shader->Program, "skybox_texture"), skybox_texture_id);
	glUniformMatrix4fv(glGetUniformLocation(this->skybox_shader->Program, "V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->skybox_shader->Program, "P"), 1, GL_FALSE, &P[0][0]);
	sky_mesh->Draw(*this->skybox_shader);
	glDepthFunc(GL_LESS); // Set depth function back to default

	glDepthMask(GL_TRUE);
}

void WaterView::drawWater(glm::mat4 V, glm::mat4 P) {

	//bind shader
	if (tw->waveBrowser->value() == WaveType::SineWave) {
		this->shader = this->sinewave_shader;
	}
	else if (tw->waveBrowser->value() == WaveType::HeightMap) {
		this->shader = this->height_map_shader;
	}
	this->shader->Use();


	if (tw->waveBrowser->value() == WaveType::SineWave) {
		// sinewave attr
		glUniform1f(glGetUniformLocation(this->shader->Program, "amplitude"), 0.15);
		glUniform1f(glGetUniformLocation(this->shader->Program, "wavelength"), 1);
		glUniform1f(glGetUniformLocation(this->shader->Program, "time"), this->cur_time);
		glUniform1f(glGetUniformLocation(this->shader->Program, "speed"), tw->speed->value());
	}
	else if (tw->waveBrowser->value() == WaveType::HeightMap) {
		glUniform1f(glGetUniformLocation(this->shader->Program, "amplitude"), 0.15);

	}


	glm::mat4 view_inv = glm::inverse(V);
	glm::vec3 eye_pos(view_inv[3][0], view_inv[3][1], view_inv[3][2]);
	glUniform3f(glGetUniformLocation(this->shader->Program, "EyePosition_worldspace"), eye_pos[0], eye_pos[1], eye_pos[2]);

	glm::mat4 scale_matrix = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
	glm::mat4 translate_matrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
	glm::mat4 wave_model = translate_matrix * scale_matrix;
	glUniformMatrix4fv(glGetUniformLocation(this->shader->Program, "M"), 1, GL_FALSE, &wave_model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->shader->Program, "V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->shader->Program, "P"), 1, GL_FALSE, &P[0][0]);

	double lx, ly, lz, lightPower;
	tie(lx, ly, lz, lightPower) = getLight();
	glUniform3f(glGetUniformLocation(this->shader->Program, "LightPosition_worldspace"),
		lx, ly, lz
	);
	glUniform1f(glGetUniformLocation(this->shader->Program, "LightPower"), lightPower);
	float ambientLight = tw->ambientLight->value();
	glUniform3f(glGetUniformLocation(this->shader->Program, "ambientLight"),
		ambientLight,
		ambientLight,
		ambientLight
	);
	float specularLight = tw->specularLight->value();
	glUniform3f(glGetUniformLocation(this->shader->Program, "specularLight"),
		specularLight,
		specularLight,
		specularLight
	);

	//glActiveTexture(GL_TEXTURE0 + 0);
	//glBindTexture(this->water->textureA.target, this->water->textureA.id);
	//glUniform1i(glGetUniformLocation(this->shader->Program, "texture_diffuse1"), 0);



	glActiveTexture(GL_TEXTURE0 + 0);
	glUniform1i(glGetUniformLocation(this->shader->Program, "skybox_cube"), 0);
	glBindTexture(skybox_texture.target, skybox_texture.id);
	glActiveTexture(GL_TEXTURE0 + 1);
	glUniform1i(glGetUniformLocation(this->shader->Program, "tiles_cube"), 1);
	glBindTexture(tiles_cube_texture.target, tiles_cube_texture.id);
	glActiveTexture(GL_TEXTURE0 + 2);
	glUniform1i(glGetUniformLocation(this->shader->Program, "causticTex"), 2);
	glBindTexture(GL_TEXTURE_2D, wave->height_maps[cur_frame]);

	if (tw->waveBrowser->value() == WaveType::SineWave) {
		// sinewave attr
	}
	else if (tw->waveBrowser->value() == WaveType::HeightMap) {
		//glActiveTexture(GL_TEXTURE0 + 3);
		//glUniform1i(glGetUniformLocation(this->shader->Program, "water"), 3);
		//glBindTexture(GL_TEXTURE_2D, wave->height_maps[cur_frame]);


		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, water->textureA.id);
		glUniform1i(glGetUniformLocation(this->shader->Program, "water"), 3);
	}
	wave->simpleDraw(*this->shader);

	cur_frame += 1;
	cur_frame %= frame_size;

	//wave->Draw(*this->shader, WaveType(tw->waveBrowser->value()), this->cur_time);
	//water->plane.Draw(*this->shader, WaveType(tw->waveBrowser->value()), this->cur_time);






	water->textureA.bind(0);
	this->simple_shader->Use();
	glUniform1i(glGetUniformLocation(this->simple_shader->Program, "water"), 0);
	glUniformMatrix4fv(glGetUniformLocation(this->simple_shader->Program, "M"), 1, GL_FALSE, &wave_model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->simple_shader->Program, "V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(this->simple_shader->Program, "P"), 1, GL_FALSE, &P[0][0]);
	water->plane.simpleDraw(*this->simple_shader);

	//water->dropShader->Use();
	//glUniformMatrix4fv(glGetUniformLocation(water->dropShader->Program, "M"), 1, GL_FALSE, &wave_model[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(water->dropShader->Program, "V"), 1, GL_FALSE, &V[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(water->dropShader->Program, "P"), 1, GL_FALSE, &P[0][0]);
	//water->plane.simpleDraw(*water->dropShader);

}

void WaterView::updateCommon() {
	// Set up the view port
	glViewport(0, 0, w(), h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0, 0, .3f, 0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Blayne prefers GL_DIFFUSE
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//glDisable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	}
	else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}
	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[] = { 0,1,1,0 }; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[] = { 1, 0, 0, 0 };
	GLfloat lightPosition3[] = { 0, -1, 0, 0 };
	GLfloat yellowLight[] = { 0.5f, 0.5f, .1f, 1.0 };
	GLfloat whiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	GLfloat blueLight[] = { .1f,.1f,.3f,1.0 };
	GLfloat grayLight[] = { .3f, .3f, .3f, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	//glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}
}


void WaterView::update(double seconds) {
	glDisable(GL_DEPTH_TEST);

	// Update the water simulation and graphics
	if (!this->water) {
		cout << "error" << endl;
		return;
	}

	this->water->stepSimulation();
	this->water->stepSimulation();
	//this->water->stepSimulation();
	this->water->updateNormals();
	// renderer.updateCaustics(water);
}


void WaterView::realDraw()
{
	updateCommon();
	// Prepare common matrix
	glm::mat4 projection;
	glm::mat4 view;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);
	glGetFloatv(GL_MODELVIEW_MATRIX, &view[0][0]);
	// Draw skybox
	// remove translation
	glm::mat4 skybox_view = glm::mat4(glm::mat3(view));
	drawSkybox(
		skybox_view,
		projection
	);

	// Draw tiles
	drawTiles(
		view,
		projection
	);

	drawWater(
		view,
		projection
	);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);

}
void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

bool is_init = false;

// animate
//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void WaterView::draw() {

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{

		if (!is_init) {
			is_init = true;
			
			// During init, enable debug output
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(MessageCallback, 0);

			//initiailize VAO, VBO, Shader...
			this->simple_shader = new
				Shader(
					PROJECT_DIR "/src/shaders/simple.vert",
					nullptr, nullptr, nullptr,
					PROJECT_DIR "/src/shaders/simple.frag");
			this->sinewave_shader = new
				Shader(
					PROJECT_DIR "/src/shaders/sinewave.vert",
					nullptr, nullptr, nullptr,
					PROJECT_DIR "/src/shaders/sinewave.frag");
			this->height_map_shader = new
				Shader(
					PROJECT_DIR "/src/shaders/water.vert",
					nullptr, nullptr, nullptr,
					PROJECT_DIR "/src/shaders/sinewave.frag");
		}


		if (!this->skybox_shader) {
			this->skybox_shader = new
				Shader(
					PROJECT_DIR "/src/shaders/skybox.vert",
					nullptr, nullptr, nullptr,
					PROJECT_DIR "/src/shaders/skybox.frag");
		}
		if (!sky_mesh) {
			initSkybox();
		}

		if (!this->tiles_shader) {
			this->tiles_shader = new
				Shader(
					PROJECT_DIR "/src/shaders/tiles.vert",
					nullptr, nullptr, nullptr,
					PROJECT_DIR "/src/shaders/tiles.frag");
		}
		if (!tiles_cube_mesh) {
			initTiles();
		}

		if (!wave) {
			wave = new Model("assets/models/water.obj");

			// Load height map once at beginning no matter what type of wave.
			wave->add_height_map_textures("assets/images/height_map");

			wave->meshes[0].textures.push_back(skybox_texture);

			cout << "wave model loaded" << endl;
		}

		if (!this->device) {
			prepareDevice();
		}

		if (!this->water) {
			this->water = new Water();

			if (!water->textureA.canDrawTo() || !water->textureB.canDrawTo()) {
				throw exception("Rendering to floating-point textures is required but not supported");
			}
		}
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");



	//var nextTime = new Date().getTime();
	//update((nextTime - prevTime) / 1000);
	realDraw();
	update(1);
	//prevTime = nextTime;
}


//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void WaterView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		}
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90, 1, 0, 0);
	}
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this, aspect);
#endif
}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void WaterView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if (((int)i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void WaterView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();

	// where is the mouse?
	int mx = Fl::event_x();
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)mx, (double)(viewport[3] - my),
		5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for (size_t i = 0; i < m_pTrack->points.size(); ++i) {
		glLoadName((GLuint)(i + 1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3] - 1;
	}
	else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n", selectedCube);
}

