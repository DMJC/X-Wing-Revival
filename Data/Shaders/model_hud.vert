#ifdef GL_ES
precision highp float;
#endif


// Vertex attribute inputs:
layout(location = 0) in vec4 aVertex;

// Matrix uniforms:
uniform mat4 uProjection;
uniform mat4 uModelView;

// Camera position:
uniform vec3 CamPos;

// Model properties:
uniform vec3 Pos;
uniform vec3 XVec;
uniform vec3 YVec;
uniform vec3 ZVec;


void main( void )
{
	// Calculate the vertex in worldspace and apply it to the rendering matrices.
	vec4 world_vertex = vec4( Pos.x + dot(aVertex.xyz,XVec), Pos.y + dot(aVertex.xyz,YVec), Pos.z + dot(aVertex.xyz,ZVec), aVertex.w );
	gl_Position = uProjection * uModelView * world_vertex;
}
