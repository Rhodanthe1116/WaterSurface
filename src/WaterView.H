/************************************************************************
	 File:        WaterView.H

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

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"

// Preclarify for preventing the compiler error
class TrainWindow;
class Water;
class Mesh;
class CTrack;


//#######################################################################
// TODO
// You might change the WaterView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The WaterView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

#include <AL/alut.h>
//#include <AL/al.h>
//#include <AL/alc.h>

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"

class WaterView : public Fl_Gl_Window
{
public:
	// note that we keep the "standard widget" constructor arguments
	WaterView(int x, int y, int w, int h, const char* l = 0);

	// overrides of important window things
	virtual int handle(int);
	void preparePlane();
	void prepareDevice();
	std::tuple<double, double, double, double> getLight();
	void initTiles();
	void drawTiles(glm::mat4, glm::mat4);
	void initSkybox();
	void drawSkybox(glm::mat4, glm::mat4);
	void drawWater(glm::mat4 V, glm::mat4 P);
	void updateCommon();
	void update(double seconds);
	void realDraw();
	virtual void draw();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated, since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows = false);

	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// Reset the Arc ball control
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick();

	//set ubo
	void setUBO();
public:
	ArcBallCam		arcball;			// keep an ArcBall for the UI
	int				selectedCube = -1;  // simple - just remember which cube is selected

	TrainWindow* tw;				// The parent of this display window
	CTrack* m_pTrack;		// The track of the entire scene

	Water* water = nullptr;
	Mesh* waterMesh;
	Shader* shader = nullptr;
	Shader* simple_shader = nullptr;
	Shader* sinewave_shader = nullptr;
	Shader* height_map_shader = nullptr;
	Shader* water_shader = nullptr;
	Shader* skybox_shader = nullptr;
	Shader* tiles_shader = nullptr;
	unsigned int skybox_texture_id;


	//OpenAL
	glm::vec3 source_pos;
	glm::vec3 listener_pos;
	ALCdevice* device = nullptr;
	ALCcontext* context = nullptr;
	ALuint source;
	ALuint buffer;

	int frame_size = 200;
	int cur_frame = 0;
	float cur_time = 0;



};