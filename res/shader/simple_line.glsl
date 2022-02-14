#version 410

uniform vec2 position;
uniform vec2 scale;
uniform vec2 screen_dimensions;
uniform float radius;
layout (location = 0) in float in_x;
layout (location = 1) in float left_y;
layout (location = 2) in float right_y;
void main(void) {
    float angle = atan((left_y - right_y) * scale.y, scale.x);
    vec2 offset = radius * vec2(sin(angle), cos(angle)) / screen_dimensions;
    gl_Position = vec4(
        (
            (
                vec2(
                    in_x + gl_InstanceID,
                    (gl_VertexID < 2 ? left_y : right_y)
                ) * scale
            )
            + position
            + (offset * ((gl_VertexID & 1) == 1 ? 1 : -1))
        ).xy,
        0.0,
        1.0
    );
}
