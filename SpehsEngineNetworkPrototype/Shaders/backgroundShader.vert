#version 130

in vec2 vertexPosition;

out vec4 fragmentColor;
out vec2 coord;

uniform mat4 transformMatrix;
uniform mat4 cameraMatrix;
uniform vec4 polygonColor;

void main()
{
	gl_Position = cameraMatrix * (transformMatrix * vec4(vertexPosition.xy, 0.0f, 1.0));
	fragmentColor = polygonColor;
	coord = vertexPosition.xy;
}