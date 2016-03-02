#version 130

in vec2 fragmentPosition;
in vec4 fragmentColor;

uniform sampler2D tex;
uniform float random;

out vec4 color;

void main()
{
	color = texture(tex, gl_PointCoord);
	color = vec4(fragmentColor.rg , fragmentColor.b * random, color.a);
}