#version 410
uniform vec2 position;
uniform vec2 scale;
layout (location = 0) in float in_x;
layout (location = 1) in float left_y;
layout (location = 2) in float right_y;
void main(void) {
    gl_Position = vec4(((vec2(in_x + gl_InstanceID, gl_VertexID == 0 ? left_y : right_y) * scale) + position).xy, 0.0, 1.0);
}
