in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform bool uTextureEnabled;

out vec4 FragColor;

void main()
{
	vec4 texel = uTextureEnabled ? texture( uTexture, vTexCoord ) : vec4( 1.0 );
	FragColor = texel * vColor;
}
