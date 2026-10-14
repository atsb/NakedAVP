#version 120
varying vec4 hplColor0;
varying vec4 hplTex0;
varying vec4 hplTex1;
varying vec4 hplTexCoord0;
varying vec4 hplTexCoord1;
varying vec4 hplTexCoord2;
varying vec4 hplTexCoord3;
uniform float amount;
uniform float xOffset;
uniform float yOffset;
uniform mat4 worldViewProj;

void main() {
    vec4 position = gl_Vertex;
    vec3 color = gl_Color.rgb;
    vec2 uv = gl_MultiTexCoord0.xy;
    vec4 oPosition = vec4(0.0);
    vec3 oColor = vec3(0.0);
    vec2 oUv0 = vec2(0.0);
    vec2 oUv1 = vec2(0.0);
    vec2 oUv2 = vec2(0.0);
    vec2 oUv3 = vec2(0.0);
    
oPosition = (worldViewProj * position);
	oColor = color;
		 	
	oUv0 = uv + vec2(xOffset,yOffset)		* amount;
	oUv1 = uv + vec2(xOffset,yOffset)*2	* amount;
	oUv2 = uv - vec2(xOffset,yOffset)		* amount;
	oUv3 = uv - vec2(xOffset,yOffset)*2	* amount;
    gl_Position = oPosition;
    hplColor0 = vec4(oColor, 1.0);
    hplTexCoord0 = vec4(oUv0, 0.0, 1.0);
    hplTexCoord1 = vec4(oUv1, 0.0, 1.0);
    hplTexCoord2 = vec4(oUv2, 0.0, 1.0);
    hplTexCoord3 = vec4(oUv3, 0.0, 1.0);
    hplTex0 = gl_MultiTexCoord0;
    hplTex1 = gl_MultiTexCoord1;

    // Preserve original Cg COLOR/TEXCOORD semantics for fixed-function passes.
    gl_FrontColor = hplColor0;
    gl_TexCoord[0] = hplTexCoord0;
    gl_TexCoord[1] = hplTexCoord1;
    gl_TexCoord[2] = hplTexCoord2;
    gl_TexCoord[3] = hplTexCoord3;
}
