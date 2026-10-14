#version 120
varying vec2 uv;
uniform sampler2D meterTex;
uniform sampler2D exposureTex;
uniform float deltaTime;
uniform float exposureInitialized;

float decodeExposure(vec3 encoded)
{
    float highPart = floor(encoded.r * 31.0 + 0.5);
    float lowPart = floor(encoded.g * 31.0 + 0.5);
    return (highPart * 32.0 + lowPart) / 1023.0;
}

vec3 encodeExposure(float exposure)
{
    float value = floor(clamp(exposure, 0.0, 1.0) * 1023.0 + 0.5);
    float highPart = floor(value / 32.0);
    float lowPart = value - highPart * 32.0;
    return vec3(highPart / 31.0, lowPart / 31.0, exposure);
}

void main()
{
    vec4 metrics = texture2D(meterTex, vec2(0.5, 0.5));
    float averageLuminance = clamp(metrics.r, 0.0, 1.0);
    float highlightCoverage = clamp(metrics.g, 0.0, 1.0);
    float centreHighlights = clamp(metrics.b, 0.0, 1.0);

    float penalty = max(averageLuminance - 0.30, 0.0) * 0.48;
    penalty += max(highlightCoverage - 0.04, 0.0) * 0.55;
    penalty += max(centreHighlights - 0.04, 0.0) * 0.75;
    float targetExposure = clamp(1.0 - penalty, 0.68, 1.0);

    float exposure = targetExposure;
    if (exposureInitialized > 0.5) {
        float previousExposure = decodeExposure(texture2D(exposureTex, vec2(0.5, 0.5)).rgb);
        float timeConstant = targetExposure < previousExposure ? 0.28 : 0.85;
        float dt = clamp(deltaTime, 0.0, 0.10);
        float blend = 1.0 - exp(-dt / timeConstant);
        exposure = mix(previousExposure, targetExposure, blend);
    }

    gl_FragColor = vec4(encodeExposure(exposure), 1.0);
}
