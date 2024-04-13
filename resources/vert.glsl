#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 itMV;
uniform mat4 V;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space
attribute vec2 aTex;
uniform vec3 lightPos; // in world space
uniform int isSOR;
uniform float t;

varying vec4 cPos;
varying vec3 cNor;
varying vec2 vTex0;
varying mat4 viewMat;

void main()
{
	if(isSOR == 1) {
		// given x and theta, compute x, y, z for the surface of revolution
		vec4 pos = vec4(aPos.x, (cos(aPos.x + t) + 2) * cos(aPos.y), (cos(aPos.x + t) + 2) * sin(aPos.y), 1.0);
		gl_Position = P * (MV * pos);
		cPos = MV * pos;
		//compute normal 
		vec3 dpdx = vec3(1, -sin(aPos.x + t) * cos(aPos.y), -sin(aPos.x + t) * sin(aPos.y));
		vec3 dpdt = vec3(0, -(cos(aPos.x + t) + 2) * sin(aPos.y), (cos(aPos.x + t) + 2) * cos(aPos.y));
		vec3 n = normalize(cross(dpdt, dpdx));

		cNor = vec3(itMV * vec4(n, 0.0)); // Assuming MV contains only translations and rotations
		
	} else {
		gl_Position = P * MV * aPos;
		cPos = MV * aPos;
		cNor = vec3(itMV * vec4(aNor, 0.0));
		vTex0 = aTex;
	}
	viewMat = V;
	
}
