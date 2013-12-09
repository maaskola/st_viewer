varying vec4 out_color;
varying vec4 out_texture;
varying vec2 out_options;

varying float out_values;
varying float out_features;
varying float out_references;

uniform int in_colorMode;
uniform int in_geneMode;
uniform int in_hitCountMin;
uniform int in_hitCountMax;
uniform int in_hitCountSum;

// bandpass smooth filter   __/  \__
float smoothband(float lo, float hi, float e, float t) {
    return (lo < hi) ?
                smoothstep(lo-e, lo+e, t) - smoothstep(hi-e, hi+e, t) :
                1.0 - (smoothstep(hi-e, hi+e, t) - smoothstep(lo-e, lo+e, t));
}

vec4 createHeatMapColor(float wavelength)
{
    float gamma = 0.8;
    // clamp input value
    float cwavelength = clamp(wavelength, 380.0, 780.0);
    
    // define colors according to wave lenght spectra
    float red;
    float green;
    float blue;
    if (380.0 <= cwavelength && cwavelength < 440.0) {
        red = -(cwavelength - 440.0) / (440.0 - 380.0);
        green = 0.0;
        blue = 1.0;
    } else if (440.0 <= cwavelength && cwavelength < 490.0) {
        red = 0.0;
        green = (cwavelength - 440.0) / (490.0 - 440.0);
        blue = 1.0;
    } else if (490.0 <= cwavelength && cwavelength < 510.0) {
        red = 0.0;
        green = 1.0;
        blue = -(cwavelength - 510.0) / (510.0 - 490.0);
    } else if (510.0 <= cwavelength && cwavelength < 580.0) {
        red = (cwavelength - 510.0) / (580.0 - 510.0);
        green = 1.0;
        blue = 0.0;
    } else if (580.0 <= cwavelength && cwavelength < 645.0) {
        red = 1.0;
        green = -(cwavelength - 645.0) / (645.0 - 580.0);
        blue = 0.0;
    } else if (645.0 <= cwavelength && cwavelength <= 780.0) {
        red = 1.0;
        green = 0.0;
        blue = 0.0;
    } else {
        red = 0.0;
        green = 0.0;
        blue = 0.0;
    }
    // float the intensity fall off near the vision limits
    float factor;
    if (380.0 <= cwavelength && cwavelength < 420.0) {
        factor = 0.3 + 0.7 * (cwavelength - 380.0) / (420.0 - 380.0);
    } else if (420.0 <= cwavelength && cwavelength < 700.0) {
        factor = 1.0;
    } else if (700.0 <= cwavelength && cwavelength <= 780.0) {
        factor = 0.3 + 0.7 * (780.0 - cwavelength) / (780.0 - 700.0);
    } else {
        factor = 0.3;
    }
    // Gamma adjustments (clamp to [0.0, 1.0])
    red = clamp(pow(red * factor, gamma), 0.0, 1.0);
    green = clamp(pow(green * factor, gamma), 0.0, 1.0);
    blue = clamp(pow(blue * factor, gamma), 0.0, 1.0);
    // return color
    return vec4(red, green, blue, 1.0);
}

float norm(float v, float t0, float t1)
{
    float vh = clamp(v, t0, t1);
    return (vh - t0 / t1 - t0);
}

float denorm(float nv, float t0, float t1)
{
    float vh = clamp(nv, 0.0, 1.0);
    return (vh * (t1 - t0) + t0);
}

void main(void)
{
    // colors
    const vec4 cNone = vec4(0.0,0.0,0.0,0.0);
    const vec4 cWhite = vec4(1.0,1.0,1.0,1.0);
	
    // input options
	bool selected = (out_options.x > 0.5);
	bool complex = (out_options.y > 0.5);
    
    // input parameters
    float min_value = float(in_hitCountMin);
    float max_value = float(in_hitCountMax);
    float sum_value = float(in_hitCountSum);
    int geneMode = in_geneMode;
    int colorMode = in_colorMode;
    float value = out_values;
    float references = out_references;
    float features = out_features;
    
    // calculate distance from center
    vec2 pos = mod(out_texture.xy, vec2(1.0)) - vec2(.5);
    float dist = length(pos);
    // radii of circle
    float radii = (selected) ? 0.3 : 0.5;
    
    // derive circle color
    vec4 fragColor = out_color;
    
    //adjust color for globalMode
    if (geneMode == 1) {
        if (colorMode == 0) {
            fragColor = out_color;
        }
        else if (colorMode == 1) {
            fragColor.a = value / max_value;
        }
        else if (colorMode == 2) {
            float nv = norm(value, min_value, max_value);
            float nt = denorm(sqrt(clamp(nv, 0.0, 1.0)), 380.0, 780.0);
            fragColor = createHeatMapColor(nt);
        }
        else {
            fragColor = out_color;
        }
    }
    
    fragColor = mix(fragColor, cNone, smoothstep(radii-0.02, radii, dist));
    if (selected) {
        fragColor = mix(fragColor, cWhite, smoothband(radii+0.02, 0.49, 0.01, dist));
    }
    gl_FragColor = fragColor;
}
