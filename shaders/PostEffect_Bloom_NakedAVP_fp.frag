#version 120
varying vec2 uv;
uniform sampler2D sceneTex;
uniform sampler2D blurTex;
uniform sampler2D exposureTex;
uniform float bloomStrength;

float decodeExposure(vec3 encoded)
{
    float highPart = floor(encoded.r * 31.0 + 0.5);
    float lowPart = floor(encoded.g * 31.0 + 0.5);
    return (highPart * 32.0 + lowPart) / 1023.0;
}

void main()
{
    vec3 sceneColour = texture2D(sceneTex, uv).rgb;
    vec3 blurColour = texture2D(blurTex, uv).rgb;
    float exposure = decodeExposure(texture2D(exposureTex, vec2(0.5, 0.5)).rgb);
    float sceneLuminance = dot(sceneColour, vec3(0.30, 0.58, 0.12));

    float highlightWeight = smoothstep(0.12, 0.82, sceneLuminance);
    float exposureWeight = mix(0.48, 1.0, highlightWeight);
    vec3 exposedScene = sceneColour * mix(1.0, exposure, exposureWeight);

    float exposedLuminance = dot(exposedScene, vec3(0.30, 0.58, 0.12));
    float shoulder = 1.0 / (1.0 + max(exposedLuminance - 0.60, 0.0) * 0.35);
    exposedScene *= shoulder;

    blurColour *= blurColour;
    vec3 bloom = blurColour * dot(blurColour, vec3(0.30, 0.58, 0.12));
    float mappedLuminance = dot(exposedScene, vec3(0.30, 0.58, 0.12));
    float highlightProtection = 1.0 - smoothstep(0.34, 0.80, mappedLuminance);
    bloom *= bloomStrength * exposure * exposure * highlightProtection;

    vec3 headroom = max(vec3(1.0) - exposedScene, vec3(0.0));
    bloom = min(bloom, headroom * 0.40);
    gl_FragColor = vec4(clamp(exposedScene + bloom, 0.0, 1.0), 1.0);
}
