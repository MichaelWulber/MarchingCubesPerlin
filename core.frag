#version 330 core

in vec3 f_col;
in vec4 surf_norm;

out vec4 color;

void main() {
	vec4 lightDir = vec4(0.0, 1.0, 0.0, 1.0); 
	float diff = 0.5 * max( dot( surf_norm, lightDir ), 0.0 );
	vec4 diffuse = diff * vec4(1.0, 1.0, 1.0, 1.0);
	vec4 ambient = 0.2 * vec4(f_col, 1.0);
	vec4 temp = vec4(diffuse * vec4(f_col, 1.0) + ambient);
	color = vec4(temp.x, temp.y, temp.z, 1.0);
}