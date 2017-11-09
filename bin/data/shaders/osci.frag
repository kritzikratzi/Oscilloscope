#version 120
#define EPS 1E-6
#define TAU 6.283185307179586
#define TAUR 2.5066282746310002
#define SQRT2 1.4142135623730951
uniform float uSize;
uniform float uIntensity;
uniform float uHue;
uniform vec3 uRgb; 
uniform float uIntensityBase;
varying vec4 color;
varying float brightness;

float erf(float x) {
	float s = sign(x), a = abs(x);
	x = 1.0 + (0.278393 + (0.230389 + (0.000972 + 0.078108 * a) * a) * a) * a;
	x *= x;
	return s - s / (x * x);
}

void main (void)
{
	float len = color.z; // we pass in length ...
	vec2 xy = color.xy; // and xy through color
	float alpha;

	float sigma = uSize/(2.0+2.0*1000.0*uSize/50.0);
/*	if (len < EPS && ) {
		// If the beam segment is too short, just calculate intensity at the position.
		alpha = exp(-len*len/(2.0*sigma*sigma))/2.0/sqrt(uSize);
		alpha = 0.0; 
	} else {*/
		// Otherwise, use analytical integral for accumulated intensity.
		alpha = erf(xy.x/SQRT2/sigma) - erf((xy.x-len)/SQRT2/sigma);
		alpha *= exp(-xy.y*xy.y/(2.0*sigma*sigma))/2.0/len*uSize;
//	}

	alpha = pow(alpha,1.0-uIntensityBase)*(0.01+min(0.99,uIntensity*3.0));
	gl_FragColor = vec4(uRgb, alpha*brightness);
	
}
