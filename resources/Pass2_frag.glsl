#version 120
    
uniform sampler2D textureA; //pos
uniform sampler2D textureB; //nor
uniform sampler2D textureC; //ke
uniform sampler2D textureD; //kd
uniform vec2 windowSize;
uniform mat4 V;
    
uniform vec3 lights[10]; //light positions in world space
uniform vec3 lightColors[10]; //light colors
uniform int isBlur;

varying mat4 viewMat;

vec2 poissonDisk[] = vec2[](
    vec2(-0.220147, 0.976896),
    vec2(-0.735514, 0.693436),
    vec2(-0.200476, 0.310353),
    vec2( 0.180822, 0.454146),
    vec2( 0.292754, 0.937414),
    vec2( 0.564255, 0.207879),
    vec2( 0.178031, 0.024583),
    vec2( 0.613912,-0.205936),
    vec2(-0.385540,-0.070092),
    vec2( 0.962838, 0.378319),
    vec2(-0.886362, 0.032122),
    vec2(-0.466531,-0.741458),
    vec2( 0.006773,-0.574796),
    vec2(-0.739828,-0.410584),
    vec2( 0.590785,-0.697557),
    vec2(-0.081436,-0.963262),
    vec2( 1.000000,-0.100160),
    vec2( 0.622430, 0.680868)
);

vec3 sampleTextureArea(sampler2D texture, vec2 tex0)
{
    const int N = 18; // [1-18]
    const float blur = 0.005;
    vec3 val = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < N; i++) {
        val += texture2D(texture, tex0.xy + poissonDisk[i]*blur).rgb;
    }
    val /= N;
    return val;
}

void main()
{
	
	float s = 200;

    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;
    
    // Fetch shading data
    vec3 pos = isBlur == 1 ? sampleTextureArea(textureA, tex).rbg : texture2D(textureA, tex).rbg;
    vec3 nor = isBlur == 1 ? sampleTextureArea(textureB, tex).rbg : texture2D(textureB, tex).rbg;
    vec3 ke = isBlur == 1 ? sampleTextureArea(textureC, tex).rbg : texture2D(textureC, tex).rbg;
    vec3 kd = isBlur == 1 ? sampleTextureArea(textureD, tex).rbg : texture2D(textureD, tex).rbg;
    
    
    // Calculate lighting here
    vec3 n = nor;
	vec3 e = normalize(vec3(0.0, 0.0, 0.0) - vec3(pos));
	vec3 result = ke;

	//traverse thru lights
	//for each light, compute diffuse and specular colors
	//add them together and multiply by given light's color
	//add to result

	for(int i = 0; i < 10; i++) {
		vec3 cLightPos = vec3(V * vec4(lights[i], 1.0));
		vec3 l = normalize(cLightPos - vec3(pos));
		vec3 h = normalize(e + l);

		vec3 cd = kd * max(0, dot(l, n));
		vec3 cs = vec3(1.0, 1.0, 1.0) * pow(max(0, dot(h, n)), s);

		vec3 color = lightColors[i] * (cd + cs);

		//calculate light fall off
		float r = sqrt(pow(cLightPos.x - pos.x, 2) + pow(cLightPos.y - pos.y, 2) + pow(cLightPos.z - pos.z, 2))/10;
		float attenuation = 1.0 / (1.0 + 0.0429 * r + 0.9857 * pow(r, 2));

		result += attenuation * color;

	}
	
	
	gl_FragColor = vec4(result, 1.0f);
}