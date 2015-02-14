//
//  util.h
//  audioOutputExample
//
//  Created by Hansi on 14.09.14.
//
//

#pragma once

template <typename valueType>
static std::vector <valueType> Split (valueType text, const valueType& delimiter)
{
    std::vector <valueType> tokens;
    size_t pos = 0;
    valueType token;
	
    while ((pos = text.find(delimiter)) != valueType::npos)
    {
        token = text.substr(0, pos);
        tokens.push_back (token);
        text.erase(0, pos + delimiter.length());
    }
    tokens.push_back (text);
	
    return tokens;
}

/* Coefficients for Matrix M */
#define M11	 0.0
#define M12	 1.0
#define M13	 0.0
#define M14	 0.0
#define M21	-0.5
#define M22	 0.0
#define M23	 0.5
#define M24	 0.0
#define M31	 1.0
#define M32	-2.5
#define M33	 2.0
#define M34	-0.5
#define M41	-0.5
#define M42	 1.5
#define M43	-1.5
#define M44	 0.5

double catmullRomSpline(float x, float v0,float v1,
						float v2,float v3) {
	
	double c1,c2,c3,c4;
	
	c1 =  	      M12*v1;
	c2 = M21*v0          + M23*v2;
	c3 = M31*v0 + M32*v1 + M33*v2 + M34*v3;
	c4 = M41*v0 + M42*v1 + M43*v2 + M44*v3;
	
	return(((c4*x + c3)*x +c2)*x + c1);
}



ofPoint catmullRomSpline( float t, ofPoint a, ofPoint b, ofPoint c, ofPoint d ){
	return ofPoint(
		   catmullRomSpline( t, a.x, b.x, c.x, d.x ),
		   catmullRomSpline( t, a.y, b.y, c.y, d.y )
	);
}


/// CMR code from
// http://stackoverflow.com/questions/9489736/catmull-rom-curve-with-no-cusps-and-no-self-intersections

struct CubicPoly
{
	float c0, c1, c2, c3;
	
	float eval(float t)
	{
		float t2 = t*t;
		float t3 = t2 * t;
		return c0 + c1*t + c2*t2 + c3*t3;
	}
	
};

struct CMRPoly{
	CubicPoly x;
	CubicPoly y;
	
	ofPoint eval( float t ){
		return ofPoint( x.eval( t ), y.eval( t ) );
	}
};

/*
 * Compute coefficients for a cubic polynomial
 *   p(s) = c0 + c1*s + c2*s^2 + c3*s^3
 * such that
 *   p(0) = x0, p(1) = x1
 *  and
 *   p'(0) = t0, p'(1) = t1.
 */
inline void InitCubicPoly(float x0, float x1, float t0, float t1, CubicPoly &p)
{
	p.c0 = x0;
	p.c1 = t0;
	p.c2 = -3*x0 + 3*x1 - 2*t0 - t1;
	p.c3 = 2*x0 - 2*x1 + t0 + t1;
}

// standard Catmull-Rom spline: interpolate between x1 and x2 with previous/following points x1/x4
// (we don't need this here, but it's for illustration)
inline void InitCatmullRom(float x0, float x1, float x2, float x3, CubicPoly &p)
{
	// Catmull-Rom with tension 0.5
	InitCubicPoly(x1, x2, 0.5f*(x2-x0), 0.5f*(x3-x1), p);
}

// compute coefficients for a nonuniform Catmull-Rom spline
inline void InitNonuniformCatmullRom(float x0, float x1, float x2, float x3, float dt0, float dt1, float dt2, CubicPoly &p)
{
	// compute tangents when parameterized in [t1,t2]
	float t1 = (x1 - x0) / dt0 - (x2 - x0) / (dt0 + dt1) + (x2 - x1) / dt1;
	float t2 = (x2 - x1) / dt1 - (x3 - x1) / (dt1 + dt2) + (x3 - x2) / dt2;
	
	// rescale tangents for parametrization in [0,1]
	t1 *= dt1;
	t2 *= dt1;
	
	InitCubicPoly(x1, x2, t1, t2, p);
}

inline float VecDistSquared(const ofPoint& p, const ofPoint& q)
{
	float dx = q.x - p.x;
	float dy = q.y - p.y;
	return dx*dx + dy*dy;
}

inline void InitCentripetalCR(const ofPoint& p0, const ofPoint& p1, const ofPoint& p2, const ofPoint& p3,
					   CubicPoly &px, CubicPoly &py)
{
	float dt0 = powf(VecDistSquared(p0, p1), 0.25f);
	float dt1 = powf(VecDistSquared(p1, p2), 0.25f);
	float dt2 = powf(VecDistSquared(p2, p3), 0.25f);
	
	// safety check for repeated points
	if (dt1 < 1e-4f)    dt1 = 1.0f;
	if (dt0 < 1e-4f)    dt0 = dt1;
	if (dt2 < 1e-4f)    dt2 = dt1;
	
	InitNonuniformCatmullRom(p0.x, p1.x, p2.x, p3.x, dt0, dt1, dt2, px);
	InitNonuniformCatmullRom(p0.y, p1.y, p2.y, p3.y, dt0, dt1, dt2, py);
}

inline CMRPoly catmullRomPoly( const ofPoint& p0, const ofPoint& p1, const ofPoint& p2, const ofPoint& p3 ){
	CMRPoly poly;
	InitCentripetalCR( p0, p1, p2, p3, poly.x, poly.y );
	return poly;
}
