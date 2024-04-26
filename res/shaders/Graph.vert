#version 330 core

uniform mat4 projMatrix;
uniform vec2 xRange;
uniform vec2 yRange;
uniform int numVerticesPerLine;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

flat out int passLineId;
out vec4 passColor;

void main()
{
    vec2 pos = vec2((position.x - xRange.x) / xRange.y, (position.y - yRange.x) / yRange.y);

    gl_Position = projMatrix * vec4(pos, 0, 1);
    
    passLineId = gl_VertexID / numVerticesPerLine;
    passColor = color;
}
