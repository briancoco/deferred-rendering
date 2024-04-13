#ifndef ITEM_CPP
#define ITEM_CPP

#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include <vector>;


class Item {
public:
	glm::vec3 translate = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::mat4 shear = glm::mat4(1.0f);
	glm::vec3 rotateDirection = glm::vec3(1, 1, 1);
	float rotateAngle = 0;
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float s;
	std::shared_ptr<Shape> shape;
	bool pulse = false;
	bool hasTex = false;
	bool isBunny = false;
	bool isTeapot = false;
	bool isSphere = false;
	bool isSOR = false;
	std::vector<float> norBuf;
	std::vector<float> posBuf;
	std::vector<unsigned int> indBuf;

	Item() {};

	void sendBuffers(std::shared_ptr<Program> prog) {
		GLuint posBufID;
		GLuint norBufID;
		GLuint indBufID;

		//send position buffer to GPU
		glGenBuffers(1, &posBufID);
		glBindBuffer(GL_ARRAY_BUFFER, posBufID);
		glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_STATIC_DRAW);
		
		//send normal buffer to GPU
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_STATIC_DRAW);

		//send index buffer to GPU
		glGenBuffers(1, &indBufID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size() * sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//Bind position buffer
		int h_pos = prog->getAttribute("aPos");
		glEnableVertexAttribArray(h_pos);
		glBindBuffer(GL_ARRAY_BUFFER, posBufID);
		glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

		// Bind normal buffer
		int h_nor = prog->getAttribute("aNor");
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

		// Bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);


	}

	void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog) {
		//Apply transforms to MV matrix
		MV->pushMatrix();

		double t = glfwGetTime();
		if (isSphere) {
			float y = 1.3 * (0.5 * sin((2 * M_PI / 1.7) * (t + 0.9)) + 0.5);
			float s = -0.5 * (0.5 * cos((4 * M_PI / 1.7) * (t + 0.9)) + 0.5) + 1;
			MV->translate(0, y, 0);
			MV->translate(translate.x, translate.y, translate.z);
			MV->scale(s, 1, s);
			MV->scale(scale);

		}
		else {
			MV->translate(translate.x, translate.y, translate.z);
			MV->scale(scale);
		}

		if (isTeapot) {
			shear[1][0] = cos(t);
			MV->multMatrix(shear);
		}
		MV->rotate(rotateAngle, rotateDirection);

		if (isBunny) {
			//rotate
			MV->rotate(t, 0, 1, 0);
		}


		//Send uniform vars to GPU
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(MV->topMatrix()))));
		glUniform3f(prog->getUniform("ka"), ka.x, ka.y, ka.z);
		glUniform3f(prog->getUniform("kd"), kd.x, kd.y, kd.z);
		glUniform3f(prog->getUniform("ks"), ks.x, ks.y, ks.z);
		glUniform1f(prog->getUniform("s"), s);
		glUniform1i(prog->getUniform("hasTex"), (int) hasTex);
		glUniform1i(prog->getUniform("isSOR"), (int) isSOR);
		glUniform1f(prog->getUniform("t"), t);

		//draw
		if (isSOR) {
			//send buffers to GPU
			sendBuffers(prog);
			int count = (int) indBuf.size(); // number of indices to be rendered
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}	
		else {
			shape->draw(prog);
		}
		MV->popMatrix();


	}
};

#endif