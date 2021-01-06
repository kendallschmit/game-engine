#version 410
uniform mat4 projmat;
uniform mat4 viewmat;
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 vert_uv;
layout (location = 2) in vec3 inst_offset;
out vec2 uv;
void main(void) {
    gl_Position = projmat * viewmat * vec4(in_pos.xyz + inst_offset, 1.0);
    uv = vert_uv;
}
