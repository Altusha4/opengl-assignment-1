#version 150

in  vec4 vPosition;
in  vec4 vColor;
out vec4 color;

uniform float theta;
uniform float scale;
uniform vec2  offset;

void main()
{
    float c = cos(theta);
    float s = sin(theta);
    vec2 p = scale * vPosition.xy;
    p = vec2(c * p.x - s * p.y, s * p.x + c * p.y);

    color = vColor;
    gl_Position = vec4(p + offset, vPosition.z, 1.0);
}
