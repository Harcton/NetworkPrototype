#version 130

in vec4 fragmentColor;
in vec2 coord;

uniform float delta;
uniform vec2 mouse;
uniform float zoom;

out vec4 color;


void main()
{
	float mag = (8000.0 * zoom) / sqrt((mouse.x - gl_FragCoord.x)*(mouse.x - gl_FragCoord.x) + (mouse.y - gl_FragCoord.y)*(mouse.y - gl_FragCoord.y));
	if (mag > 0.8)
		mag = 0.8;

	float t = 1.0 + (delta * 0.0001);
	float x = coord.x / 40.0;
	float y = coord.y / 40.0;
	float SFMN = 0.4;


	float vv = 0.25 * (
			(0.5 + (0.5 * sin(x/14.0+t))) +
			(0.5 + (0.5 * sin(y/10.0-t))) +
			(0.5 + (0.5 * sin((x+y)/12.0-t))) +
			(0.5 + (0.5 * sin(sqrt(float(x*x + y*y))/15.0+t)))
			);

	color = fragmentColor;
	color = vec4((vv*0.05)*SFMN+color.r, (vv*0.8*cos(t)/6.0)*SFMN+color.g, (vv*0.5)*SFMN+color.b, color.a);
	color = vec4(color.rgb * mag, color.a);
	//color = fragmentColor; /*<< UNCOMMENT THAT TO DISABLE BACKGROUND EFFECT!*/
}