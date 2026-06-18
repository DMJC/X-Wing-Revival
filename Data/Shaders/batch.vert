layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uModelView;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{
	gl_Position = uProjection * uModelView * vec4( aPos, 1.0 );
	vTexCoord = aTexCoord;
	vColor = aColor;
	gl_PointSize = 1.0;
}
