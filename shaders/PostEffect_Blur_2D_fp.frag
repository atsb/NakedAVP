#version 120
// Auto-translated from the supplied HPL1 Cg shader; texture-unit suffixes preserve TEXUNIT semantics.
varying vec4 hplColor0;
varying vec4 hplTexCoord0;
varying vec4 hplTexCoord1;
varying vec4 hplTexCoord2;
varying vec4 hplTexCoord3;
uniform sampler2D textureU0;

void main() {
    vec4 pos = gl_FragCoord;
    vec4 color = hplColor0;
    vec3 uv0 = hplTexCoord0.xyz;
    vec3 uv1 = hplTexCoord1.xyz;
    vec3 uv2 = hplTexCoord2.xyz;
    vec3 uv3 = hplTexCoord3.xyz;
    vec4 oColor = vec4(0.0);
    
vec4 col5 = texture2D(textureU0, ((uv0 + uv2)/2).xy) * 0.35;
	
	vec4 col1 = texture2D(textureU0, (uv0).xy) * 0.15;
	vec4 col3 = texture2D(textureU0, (uv2).xy) * 0.15;
	
	vec4 col6 = texture2D(textureU0, ((uv2 + uv3)/2).xy) * 0.1;
	vec4 col7 = texture2D(textureU0, ((uv0 + uv1)/2).xy) * 0.1;

	vec4 col2 = texture2D(textureU0, (uv1).xy) * 0.075;
	vec4 col4 = texture2D(textureU0, (uv3).xy) * 0.075;
	
	
	oColor = col1 + col2 + col3 + col4 + col5 + col6 + col7;
    gl_FragColor = oColor;
}
