#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray
{
private:
	point3 orig;
	vec3 dir;
	double tm;

public:
	ray() {}
	ray(const point3& origin, const vec3& dir, double time = 0.0) : orig(origin), dir(dir), tm(time) {}
	point3 origin() const { return orig; }
	vec3 direction() const { return dir; }
	double time() const { return tm; }

	point3 at(double t) const
	{
		return orig + t * dir;
	}
};
#endif // !RAY_H
