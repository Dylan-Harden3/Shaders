#version 120

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

uniform vec3 light1Pos;
uniform vec3 light2Pos;
uniform vec3 light1Color;
uniform vec3 light2Color;

varying vec3 vPos;
varying vec3 normal;

float quantize(float color) {

	if(color < 0.25f) return 0.0f;
	else if(color < 0.5f) return 0.25f;
	else if(color < 0.75f) return 0.5f;
	else if( color < 1.0f) return 0.75f;
	else return 1.0f;
	
}

void main()
{
	vec3 n = normalize(normal);
	vec3 e = normalize(vec3(0) - vPos);

	if(dot(n, e) < 0.3f) {
		gl_FragColor = vec4(vec3(0.0f), 1.0f);
	}
	else {
		vec3 color = vec3(0.0f);

		vec3 l1 = normalize(light1Pos - vPos);
		vec3 l2 = normalize(light2Pos - vPos);

		vec3 h1 = normalize(l1 + e);
		vec3 h2 = normalize(l2 + e);

		vec3 diffuse1 = kd * max(dot(l1, n),0);
		vec3 diffuse2 = kd * max(dot(l2, n),0);

		vec3 specular1 = ks * pow(max(dot(h1, n), 0), s);
		vec3 specular2 = ks * pow(max(dot(h2, n), 0), s);

		color.r = light1Color.r * (ka.r + diffuse1.r + specular1.r) + light2Color.r * (ka.r + diffuse2.r + specular2.r);
		color.g = light1Color.g * (ka.g + diffuse1.g + specular1.g) + light2Color.g * (ka.g + diffuse2.g + specular2.g);
		color.b = light1Color.b * (ka.b + diffuse1.b + specular1.b) + light2Color.b * (ka.b + diffuse2.b + specular2.b);

		color.r = quantize(color.r);
		color.g = quantize(color.g);
		color.b = quantize(color.b);

		gl_FragColor = vec4(color, 1.0);
	}
}