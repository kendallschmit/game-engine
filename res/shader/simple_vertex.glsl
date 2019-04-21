#version 410
uniform mat4 projmat;
uniform mat4 viewmat;
uniform mat4 modelmat;
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 vert_uv;
out vec2 uv;
void main(void) {
    gl_Position = projmat * viewmat * modelmat * vec4(in_pos.xyz, 1.0);
    uv = vert_uv;
}
