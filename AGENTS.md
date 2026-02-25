# MyRaytracer

A C++ ray tracing application based on the "Ray Tracing in One Weekend" book series.

## Cursor Cloud specific instructions

### Project overview

Single-file C++ console application that renders a scene with spheres (Lambertian, metal, dielectric materials) and outputs a PPM image to stdout.

- **Source**: All files live in `MyRaytracer/src/` — one `main.cpp` and 9 header files.
- **No external dependencies**: Pure C++ standard library only.
- **No package manager / build system on Linux**: The repo ships Visual Studio `.sln`/`.vcxproj` files (Windows). On Linux, compile directly with `g++`.

### Build

```bash
# Release build
g++ -std=c++17 -O2 -o raytracer MyRaytracer/src/main.cpp

# Debug build with warnings
g++ -std=c++17 -g -Wall -Wextra -o raytracer MyRaytracer/src/main.cpp
```

One expected warning: unused parameter `r_in` in `material.h` (`lambertian::scatter`). This is harmless.

### Run

```bash
./raytracer > output.ppm 2>progress.log
```

Output is PPM (`P3`) format to stdout; progress is printed to stderr. The default scene (500 SPP, 1200×800) takes a long time. For quick validation, create a small test program importing the same headers with lower `samples_per_pixel` (e.g. 10) and smaller `image_width` (e.g. 200).

### Lint / Static analysis

No linter or formatter is configured. Use compiler warnings (`-Wall -Wextra`) as a basic lint check.

### Tests

No automated test suite exists. Validate by compiling and confirming the program produces a valid PPM header (`P3\n<width> <height>\n255\n`).

### Converting output

```bash
ffmpeg -i output.ppm output.png
```
