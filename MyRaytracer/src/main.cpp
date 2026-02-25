#include "rtweekend.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include<iostream>
#include<thread>
#include<vector>
#include<mutex>
#include<atomic>


using namespace std;

color3 ray_color(const ray& r, const hittable& world, int depth)
{
	hit_record rec;

	if (depth <= 0)
		return color3(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec))
	{
		ray scattered;
		color3 attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color3(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color3(1.0, 1.0, 1.0) + t * color3(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
	hittable_list world;

	auto ground_material = make_shared<lambertian>(color3(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = 0; a < 2; a++) {
		for (int b = 0; b < 2; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color3::random() * color3::random();
					sphere_material = make_shared<lambertian>(albedo);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = color3::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color3(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color3(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

void render_tile(
	int row_start, int row_end,
	int col_start, int col_end,
	int image_width, int image_height,
	int samples_per_pixel, int max_depth,
	const camera& cam, const hittable& world,
	std::vector<std::vector<color3>>& framebuffer,
	std::atomic<int>& tiles_done,
	std::mutex& cerr_mutex)
{
	for (int j = row_start; j < row_end; j++)
	{
		for (int i = col_start; i < col_end; i++)
		{
			color3 pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			framebuffer[j][i] = pixel_color;
		}
	}

	int done = ++tiles_done;
	{
		std::lock_guard<std::mutex> lock(cerr_mutex);
		std::cerr << "\rTiles completed: " << done << "/16" << std::flush;
	}
}

int main()
{
	const auto aspect_ratio = 3.0 / 2.0;
	const int image_width = 1200;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 500;
	const int max_depth = 50;

	auto world = random_scene();

	// Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10;
	auto aperture = 0.1;
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	// Framebuffer: store all pixel colors, then write sequentially
	std::vector<std::vector<color3>> framebuffer(
		image_height, std::vector<color3>(image_width));

	// 4x4 tile grid → 16 threads
	const int tile_rows = 4;
	const int tile_cols = 4;

	std::vector<std::thread> threads;
	std::atomic<int> tiles_done(0);
	std::mutex cerr_mutex;

	std::cerr << "Rendering with " << tile_rows * tile_cols
	          << " threads (" << tile_rows << "x" << tile_cols << " tiles)..."
	          << std::endl;

	for (int ty = 0; ty < tile_rows; ty++)
	{
		int row_start = ty * image_height / tile_rows;
		int row_end = (ty + 1) * image_height / tile_rows;

		for (int tx = 0; tx < tile_cols; tx++)
		{
			int col_start = tx * image_width / tile_cols;
			int col_end = (tx + 1) * image_width / tile_cols;

			threads.emplace_back(render_tile,
				row_start, row_end, col_start, col_end,
				image_width, image_height,
				samples_per_pixel, max_depth,
				std::cref(cam), std::cref(world),
				std::ref(framebuffer),
				std::ref(tiles_done),
				std::ref(cerr_mutex));
		}
	}

	for (auto& t : threads)
		t.join();

	std::cerr << "\nWriting output..." << std::endl;

	// Write PPM output (top to bottom: j from image_height-1 down to 0)
	cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	for (int j = image_height - 1; j >= 0; j--)
	{
		for (int i = 0; i < image_width; i++)
		{
			write_color(std::cout, framebuffer[j][i], samples_per_pixel);
		}
	}

	std::cerr << "Done!" << std::endl;

	return 0;
}