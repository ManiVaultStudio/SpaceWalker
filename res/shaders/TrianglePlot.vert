#version 330 core

// Scalar effects
#define EFFECT_NONE    0
#define EFFECT_COLOR   1

// Point properties
uniform int  scalarEffect;
uniform mat3 orthoM;                            /** Projection matrix from bounds space to clip space */
uniform bool hasScalars;                        /** Whether a scalar buffer is used */
uniform vec3 colorMapRange;                     /** Color map scalar range */
uniform bool hasColors;                         /** Whether a color buffer is used */

layout(location = 0) in vec2    vertex;         /** Vertex input */
layout(location = 1) in vec2    position;       /** Sample position */
layout(location = 3) in float   scalar;         /** Point scalar */
layout(location = 4) in vec3    color;          /** Point color */

// Output variables
smooth out vec2 vVertex;
smooth out vec2 vPosition;
smooth out float vScalar;
smooth out vec3  vColor;

void main()
{
    // Transform position to clip space
    vec2 v = (orthoM * vec3(vertex, 1)).xy;

    vVertex = v;
    vPosition = (orthoM * vec3(position, 1)).xy;
    vScalar = hasScalars ? (scalar - colorMapRange.x) / colorMapRange.z : 0;
    
    vColor = hasColors ? color : vec3(0.5);

    // Move quad by position and output
    gl_Position = vec4(v, 0, 1);
}
