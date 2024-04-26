#version 330 core

// Scalar effects
#define EFFECT_NONE    0
#define EFFECT_COLOR   1

// Point properties
uniform int   scalarEffect;

// Colormap to use if current effect is EFFECT_COLOR
uniform sampler2D colormap;

// Input variables
smooth in vec2 vVertex;
smooth in vec2 vPosition;
smooth in float vScalar;
smooth in vec3  vColor;

// Output color
out vec4 fragColor;

void main()
{
    // Set point color
    vec3 color = vColor;
    
    float dist = length(vVertex - vPosition);// * length(vVertex - vPosition) * 128;

    float a = smoothstep(0.002, 0.001, dist);
    //float a = smoothstep(0.015, 0.01, dist);

    if (scalarEffect == EFFECT_COLOR) {
        color = texture(colormap, vec2(vScalar, 1-vScalar)).rgb;
    }

    //fragColor = vec4(a, dist, dist, 1);
    fragColor = vec4(color, a);
}
