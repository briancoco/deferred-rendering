#version 120
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

uniform sampler2D texture0;

varying vec4 cPos;
varying vec3 cNor;
varying vec3 cLight;
varying vec2 vTex0;
uniform int hasTex;
uniform int currLight;

uniform vec3 lights[10]; //light positions in world space
uniform vec3 lightColors[10]; //light colors

varying mat4 viewMat;

void main()
{
	vec3 n = normalize(cNor);
	vec3 e = normalize(vec3(0.0, 0.0, 0.0) - vec3(cPos));
	vec3 result = currLight != -1 ? lightColors[currLight] : vec3(0.0f, 0.0f, 0.0f);

	//traverse thru lights
	//for each light, compute diffuse and specular colors
	//add them together and multiply by given light's color
	//add to result

	for(int i = 0; i < 10; i++) {
		vec3 cLightPos = vec3(viewMat * vec4(lights[i], 1.0));
		vec3 l = normalize(cLightPos - vec3(cPos));
		vec3 h = normalize(e + l);

		vec3 cd = kd * max(0, dot(l, n));
		vec3 cs = ks * pow(max(0, dot(h, n)), s);

		vec3 color = lightColors[i] * (cd + cs);

		//calculate light fall off
		float r = sqrt(pow(cLightPos.x - cPos.x, 2) + pow(cLightPos.y - cPos.y, 2) + pow(cLightPos.z - cPos.z, 2))/10;
		float attenuation = 1.0 / (1.0 + 0.0429 * r + 0.9857 * pow(r, 2));

		result += attenuation * color;

	}
	
	gl_FragColor = vec4(result.r, result.g, result.b, 1.0f);
	
}
