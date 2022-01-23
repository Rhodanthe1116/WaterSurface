
#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;


class Water
{
public:
	Texture textureA;
	Texture textureB;
	// dropShader

	void addDrop(float x, float y, float radius, float strength) {
		auto this_ = this;

		auto f = [=](Water* this_) {
			this_->textureA.bind();
			/*this_.dropShader.uniforms({
			  center: [x, y] ,
			  radius : radius,
			  strength : strength
				}).draw(this_.plane);*/
		};

		this->textureB.drawTo(f);
		//this.textureB.swapWith(this.textureA);
	}
	// private:

};
