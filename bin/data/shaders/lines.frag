#version 150
// based on http://m1el.github.io/woscope-how/

#define EPS 1E-6
#define TAU 6.283185307179586
#define TAUR 2.5066282746310002
#define SQRT2 1.4142135623730951

uniform float uSize;
uniform float uIntensity;
uniform float uHue; 

in vec4 vertex_color;
in vec4 uvl;

out vec4 frag_color;


// A standard gaussian function, used for weighting samples
// http://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
float gaussian(float x, float sigma) {
	return exp(-(x * x) / (2.0 * sigma * sigma)) / (TAUR * sigma);
}

// This approximates the error function, needed for the gaussian integral
// http://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
float erf(float x) {
	float s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
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


void main()
{
	float len = uvl.z;
	vec2 xy = uvl.xy;
	float alpha;

	float sigma = uSize/4.0;
	if (len < EPS) {
		// If the beam segment is too short, just calculate intensity at the position.
		alpha = exp(-pow(length(xy),2.0)/(2.0*sigma*sigma))/2.0/sqrt(uSize);
	}
	else {
		// Otherwise, use analytical integral for accumulated intensity.
		alpha = erf(xy.x/SQRT2/sigma) - erf((xy.x-len)/SQRT2/sigma);
		alpha *= exp(-xy.y*xy.y/(2.0*sigma*sigma))/2.0/len*uSize;
	}

	float afterglow = smoothstep(0.0, 0.33, uvl.w/2048.0);
	alpha *= afterglow * uIntensity*20;
	// this will need some tweaking
	alpha *= 100000;
	vec3 rgb = hsv2rgb(vec3(uHue/360.0,1.0,1.0));
	frag_color = vec4(rgb, alpha);
	
}