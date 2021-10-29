#version 410
uniform vec4 in_color;
out vec4 out_color;
void main (void) {
    out_color = in_color.rgba;
}
