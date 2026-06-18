#ifdef GL_ES
precision highp float;
#endif


// Fragment output:
out vec4 FragColor;

// Shader variable:
uniform vec3 AmbientLight;


void main( void )
{
	FragColor = vec4( AmbientLight, 1.0 );
}
