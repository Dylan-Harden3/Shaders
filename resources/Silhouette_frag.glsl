#version 120

varying vec3 vPos;
varying vec3 normal;

void main()
{
	vec3 n = normalize(normal);
	vec3 e = normalize(vec3(0) - vPos);

	if(dot(n, e) < 0.3f){
		gl_FragColor = vec4(vec3(0.0f), 1.0f);
	}
	else {
		gl_FragColor = vec4(vec3(1.0f), 1.0f);
	}
}