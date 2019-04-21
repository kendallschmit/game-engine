#version 410
uniform sampler2D tex_sampler;
in vec2 uv;
out vec4 out_color;
void main (void) {
    out_color = texture(tex_sampler, uv).rgba;
}
