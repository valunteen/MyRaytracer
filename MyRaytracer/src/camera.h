#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

class camera 
{
public:
    camera(point3 loolfrom,
           point3 lookat,
           vec3 vup,
           double vfov, 
           double aspect_ratio,
           double aperture,
           double focus_dist,
           double _time0 = 0.0,
           double _time1 = 0.0) 
    {
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2);
        auto viewport_height = h * 2.0;
        auto viewport_width = viewport_height * aspect_ratio;

        w = unit_vector(loolfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = unit_vector(cross(w, u));

        origin = loolfrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

        lens_radius = aperture / 2;

        time0 = _time0;
        time1 = _time1;
    }

    ray get_ray(double s, double t) const 
    {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x() + v * rd.y();

        return ray(origin + offset, 
                   lower_left_corner + s * horizontal + t * vertical - origin - offset,
                   random_double(time0, time1)
                   );
    }

private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    double lens_radius;
    //shutter open&close time
    double time0;
    double time1;
};
#endif