R"(
#version 330 core 

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 texCoords;

void main()
{
    texCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0, 1);
}

)"

