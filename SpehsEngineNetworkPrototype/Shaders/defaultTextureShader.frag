#version 130

in vec2 fragmentPosition;
in vec2 texCoord;
in vec4 fragmentColor;

out vec4 color;

uniform sampler2D tex;

void main()
{
	color = texture(tex, texCoord);
	color =  color * fragmentColor;
}