#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"

Camera::Camera() :
	aspect(1.0f),
	fovy((float)(45.0*M_PI/180.0)),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 0.5f, -2.0f),
	rfactor(0.01f),
	tfactor(0.001f),
	sfactor(0.005f)
{
}


Camera::~Camera()
{
}

void Camera::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	if(shift) {
		state = Camera::TRANSLATE;
	} else if(ctrl) {
		state = Camera::SCALE;
	} else {
		state = Camera::ROTATE;
	}
}

void Camera::mouseMoved(float x, float y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;
	float absMaxPitch = glm::radians(60.0f);
	switch(state) {
		case Camera::ROTATE:
			rotations += rfactor * dv;
			if (rotations.y > absMaxPitch) {
				rotations.y = absMaxPitch;
			}
			else if (rotations.y < -absMaxPitch) {
				rotations.y = -absMaxPitch;
			}
			break;
		case Camera::TRANSLATE:
			translations.x -= translations.z * tfactor * dv.x;
			translations.y += translations.z * tfactor * dv.y;
			break;
		case Camera::SCALE:
			translations.z *= (1.0f - sfactor * dv.y);
			break;
	}
	mousePrev = mouseCurr;
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const
{
	// Modify provided MatrixStack
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const
{
	glm::vec3 eye = translations;
	glm::vec3 target = !isTD ? glm::vec3(sin(rotations.x), rotations.y, cos(rotations.x)) + eye : glm::vec3(0.0, -1.0, 0.0) + eye;
	glm::vec3 up = glm::vec3(0, 1, 0);
	MV->multMatrix(glm::lookAt(eye, target, up));
}

void Camera::applyCameraMatrix(std::shared_ptr<MatrixStack> MV) {
	glm::vec3 eye = translations;
	glm::vec3 target = glm::vec3(sin(rotations.x), rotations.y, cos(rotations.x)) + eye;
	glm::vec3 up = glm::vec3(0, 1, 0);
	MV->multMatrix(glm::inverse(glm::lookAt(eye, target, up)));
}
