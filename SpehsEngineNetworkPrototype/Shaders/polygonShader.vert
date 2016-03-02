#version 130

in vec2 vertexPosition;
in vec2 textureCoords;

out vec2 fragmentPosition;
out vec2 texCoord;
out vec4 fragmentColor;

uniform mat4 transformMatrix;
uniform vec4 polygonColor;
uniform mat4 cameraMatrix;

void main()
{
	gl_Position = cameraMatrix * (transformMatrix * vec4(vertexPosition.xy, 0.0f, 1.0));
	fragmentPosition = vertexPosition;
	fragmentColor = polygonColor;
	texCoord = textureCoords;
}