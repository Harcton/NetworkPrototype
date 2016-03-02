#version 130

in vec2 vertexPosition;

out vec2 fragmentPosition;
out vec4 fragmentColor;

uniform mat4 transformMatrix;
uniform vec4 polygonColor;
uniform mat4 cameraMatrix;
uniform float zoom;
uniform float random;

void main()
{
	gl_Position = cameraMatrix * (transformMatrix * vec4(vertexPosition.xy, 0.0f, 1.0));
	gl_PointSize = zoom;
	fragmentPosition = vertexPosition;
	fragmentColor = vec4(polygonColor.x, polygonColor.y, polygonColor.z, polygonColor.w);
}