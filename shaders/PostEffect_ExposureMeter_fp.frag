#version 120
varying vec2 uv;
uniform sampler2D sceneTex;

float luminanceAt(vec2 coord)
{
    vec3 colour = texture2D(sceneTex, coord).rgb;
    return dot(colour, vec3(0.30, 0.58, 0.12));
}

float highlightAt(float luminance)
{
    return smoothstep(0.72, 0.94, luminance);
}

void main()
{
    float globalLuminance = 0.0;
    float globalHighlights = 0.0;
    float centreHighlights = 0.0;
    int x;
    int y;

    for (y = 0; y < 5; ++y) {
        for (x = 0; x < 8; ++x) {
            vec2 coord = (vec2(float(x), float(y)) + vec2(0.5)) / vec2(8.0, 5.0);
            float lum = luminanceAt(coord);
            globalLuminance += lum;
            globalHighlights += highlightAt(lum);
        }
    }
    globalLuminance /= 40.0;
    globalHighlights /= 40.0;

    for (y = 0; y < 3; ++y) {
        for (x = 0; x < 3; ++x) {
            vec2 coord = vec2(0.44, 0.44) +
                         vec2(float(x), float(y)) * vec2(0.06, 0.06);
            float lum = luminanceAt(coord);
            centreHighlights += highlightAt(lum);
        }
    }
    centreHighlights /= 9.0;

    float weightedLuminance = globalLuminance;
    float weightedHighlights = mix(globalHighlights, centreHighlights, 0.42);
    gl_FragColor = vec4(weightedLuminance, weightedHighlights,
                        centreHighlights, 1.0);
}
