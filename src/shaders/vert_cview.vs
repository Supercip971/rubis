#version 450

vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0),//1
    vec2(1.0, -1.0),//2
    vec2(1.0, 1.0),//3
    vec2(-1.0, -1.0),//1
    vec2(1.0, 1.0),//3
    vec2(-1.0, 1.0)//4
);

layout (location = 0) out vec2 fragCoord;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragCoord = vec2((positions[gl_VertexIndex].x + 1.0f)/2, (positions[gl_VertexIndex].y + 1.0f)/2);
}
