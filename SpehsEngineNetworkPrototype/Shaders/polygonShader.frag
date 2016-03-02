#version 130

in vec2 fragmentPosition;
in vec2 texCoord;
in vec4 fragmentColor;

uniform vec2 mouse;

out vec4 color;

void main()
{
	float mag = 200.0 / sqrt((mouse.x - gl_FragCoord.x)*(mouse.x - gl_FragCoord.x) + (mouse.y - gl_FragCoord.y)*(mouse.y - gl_FragCoord.y));
	if (mag > 1.0)
		mag = 1.0;

	color = vec4(fragmentColor.rgb * mag, fragmentColor.a);
}