
#version 300 es
precision mediump float;

in vec3 a_Position;
in vec4 a_Color;

out vec4 vColor;

uniform mat4 u_MVPMatrix;

void main() {
	vColor = a_Color;
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
}
