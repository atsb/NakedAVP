#version 120
varying vec2 uv;
uniform sampler2D sceneTex;
uniform sampler2D exposureTex;
uniform float threshold;
uniform float softKnee;

float decodeExposure(vec3 encoded)
{
    float highPart = floor(encoded.r * 31.0 + 0.5);
    float lowPart = floor(encoded.g * 31.0 + 0.5);
    return (highPart * 32.0 + lowPart) / 1023.0;
}

void main()
{
    float exposure = decodeExposure(texture2D(exposureTex, vec2(0.5, 0.5)).rgb);
    vec3 colour = texture2D(sceneTex, uv).rgb * exposure;
    float luminance = dot(colour, vec3(0.30, 0.58, 0.12));
    float knee = max(softKnee, 0.0001);

    float contribution = smoothstep(threshold - knee, threshold + knee, luminance);

    gl_FragColor = vec4(colour * contribution, 1.0);
}
