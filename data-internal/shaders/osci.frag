#define EPS 1E-6
#define TAU 6.283185307179586
#define TAUR 2.5066282746310002
#define SQRT2 1.4142135623730951
uniform float uSize;
uniform float uIntensity;
uniform float uHue;

varying vec3 texcoord;
varying vec2 pos;

float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (TAUR * sigma);
}

float erf(float x) {
    float s = sign(x), a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + (0.000972 + 0.078108 * a) * a) * a) * a;
    x *= x;
    return s - s / (x * x);
}

// http://stackoverflow.com/a/17897228/347508
vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main (void)
{
    float len = texcoord.z;
    vec2 xy = texcoord.xy;
    float alpha;

    float sigma = uSize/(2.0+2.0*1000.0*uSize/50.0+0.0*pow(uIntensity,2.0));
    if (len < EPS) {
    // If the beam segment is too short, just calculate intensity at the position.
        alpha = exp(-pow(length(xy),2.0)/(2.0*sigma*sigma))/2.0/sqrt(uSize);
    } else {
    // Otherwise, use analytical integral for accumulated intensity.
		alpha = erf(xy.x/SQRT2/sigma) - erf((xy.x-len)/SQRT2/sigma);
		alpha *= exp(-xy.y*xy.y/(2.0*sigma*sigma))/2.0/len*uSize;
    }

    //alpha *= uIntensity;
//	float maxIntensity = max(0.004,min(0.7, 1.0 - pow(1000.0*uSize/25.0,1.0)*5.0)); 
//	float intens = min(uIntensity,maxIntensity); 
	float intens = max(0.0,uIntensity-0.4)*0.7-1000.0*uSize/500.0; 
	alpha = pow(alpha,1.0-intens)*(0.01+min(0.99,uIntensity*3.0));
//	alpha = min(alpha,0.03); 
    //float afterglow = smoothstep(0.0, 0.33, uvl.w/2048.0);
    //alpha *= afterglow * 1.0;

	// green, going into white:
	// gl_FragColor = vec4(1./32., 1.0, 1./32., alpha);
	
	// full color (using hue)
	vec3 rgb = hsv2rgb(vec3(uHue/360.0,1.0,1.0));
	gl_FragColor = vec4(rgb, alpha);
	
}
