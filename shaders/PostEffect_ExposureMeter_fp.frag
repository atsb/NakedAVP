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

    for (y = 0; y < 7; ++y) {
        for (x = 0; x < 12; ++x) {
            vec2 coord = (vec2(float(x), float(y)) + vec2(0.5)) / vec2(12.0, 7.0);
            float lum = luminanceAt(coord);
            globalLuminance += lum;
            globalHighlights += highlightAt(lum);
        }
    }
    globalLuminance /= 84.0;
    globalHighlights /= 84.0;

    for (y = 0; y < 5; ++y) {
        for (x = 0; x < 5; ++x) {
            vec2 coord = vec2(0.38, 0.38) +
                         (vec2(float(x), float(y)) + vec2(0.5)) * vec2(0.048, 0.048);
            float lum = luminanceAt(coord);
            centreHighlights += highlightAt(lum);
        }
    }
    centreHighlights /= 25.0;

    float weightedLuminance = globalLuminance;
    float weightedHighlights = mix(globalHighlights, centreHighlights, 0.42);
    gl_FragColor = vec4(weightedLuminance, weightedHighlights,
                        centreHighlights, 1.0);
}
