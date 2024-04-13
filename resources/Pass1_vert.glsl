#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;
uniform int isSOR;
uniform float t;

attribute vec4 aPos;
attribute vec3 aNor;

varying vec3 vPos; // in camera space
varying vec3 vNor; // in camera space

void main()
{
	mat4 rotX = mat4(1.0f);
	rotX[1][1] = cos(radians(90.0));
	rotX[1][2] = -sin(radians(90.0));
	rotX[2][1] = sin(radians(90.0));
	rotX[2][2] = cos(radians(90.0));

	if(isSOR == 1) {
		// given x and theta, compute x, y, z for the surface of revolution
		vec4 pos = vec4(aPos.x, (cos(aPos.x + t) + 2) * cos(aPos.y), (cos(aPos.x + t) + 2) * sin(aPos.y), 1.0);
		gl_Position = P * MV * pos;
		vPos = vec3(MV * pos);

		//compute normal 
		vec3 dpdx = vec3(1, -sin(aPos.x + t) * cos(aPos.y), -sin(aPos.x + t) * sin(aPos.y));
		vec3 dpdt = vec3(0, -(cos(aPos.x + t) + 2) * sin(aPos.y), (cos(aPos.x + t) + 2) * cos(aPos.y));
		vec3 n = normalize(cross(dpdt, dpdx));

		vNor = normalize(vec3(MVit * vec4(n, 0.0))); // Assuming MV contains only translations and rotations
	}
	else {
		gl_Position = P * MV * aPos;
		vPos = vec3(MV * aPos);
		vNor = normalize(vec3( MVit * vec4(aNor, 0.0)));
	}
	
}
