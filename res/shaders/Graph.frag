#version 330 core

uniform sampler2D colormap;

// Colors
uniform vec4 mainColor;
uniform vec4 selectedColor;
uniform vec4 primaryColor;
uniform vec4 secondaryColor;

// Selected lines
uniform int primaryLine;
uniform int secondaryLine;
uniform int lineHover;
uniform int lineSelection;

flat in int passLineId;
in vec4 passColor;

out vec4 fragColor;

void main()
{
    if (passLineId == lineSelection)
        fragColor = vec4(1, 0, 0, 1);
    else if (passLineId == lineHover)
        fragColor = vec4(0.5, 0.5, 1, 1);
    else if (passLineId == primaryLine)
        fragColor = primaryColor;
    else if (passLineId == secondaryLine)
        fragColor = secondaryColor;
    else
        fragColor = mainColor;
}
