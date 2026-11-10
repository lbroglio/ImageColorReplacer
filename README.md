# Image Color Replacer

Image Color Replacer is a command-line application that replaces pixels close to a chosen color in an image. It uses a WebGPU compute shader, via `wgpu-native`, to process the image on the GPU and writes the result as `output.png`.


## How it works

The application loads the input image as RGBA pixels, uploads it to a GPU texture, and dispatches a compute shader over the image. For each pixel, the shader calculates a weighted RGB distance from the target color:

```text
sqrt(0.299 * (red difference)^2
   + 0.587 * (green difference)^2
   + 0.114 * (blue difference)^2)
```

Colors are normalized to the range `[0, 1]`. Pixels whose distance is less than or equal to the threshold are replaced; all other pixels are preserved. Alpha is not part of the distance calculation. The replacement color is fully opaque.

## Requirements

- CMake 3.14 or newer; use a release that supports the project's C++26 standard setting
- A C++ compiler with support for the C++26 features used by the project
- `wgpu-native` development files: the `wgpu.h` header and native library must be discoverable by CMake
- A working WebGPU adapter/device supported by `wgpu-native`
- Network access during the first CMake configure if the stb image headers are not already present in the build directory (CMake downloads pinned copies)

The included Makefile uses `gcc-16` and `g++-16` by default. If your compiler uses different names or paths, configure the compiler explicitly as shown below.

## Build

### With Make

```sh
make
```

This configures the project in `build/` and builds `build/ImageColorReplacer`. To use different compiler commands:

```sh
make COMPILER_FLAGS="-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release"
```

### With CMake

```sh
cmake -S . -B build \
  -DCMAKE_C_COMPILER=gcc-16 \
  -DCMAKE_CXX_COMPILER=g++-16 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If CMake cannot locate `wgpu-native`, set its library and header locations explicitly:

```sh
cmake -S . -B build \
  -DWGPU_NATIVE_LIBRARY=/path/to/libwgpu_native.dylib \
  -DWGPU_NATIVE_INCLUDE_DIR=/path/to/wgpu-native/include
```

Use the library filename and paths appropriate to your operating system and installation.

## Usage

```sh
./build/ImageColorReplacer <image_path> <target_color> <replacement_color> [-d <distance_threshold>]
```

| Argument | Description |
| --- | --- |
| `image_path` | Path to the image to process. |
| `target_color` | RGB color to find, in `R,G,B` form. Each component is specified on the 0–255 scale. |
| `replacement_color` | RGB color to use for matching pixels, also in `R,G,B` form. |
| `-d distance_threshold` | Optional normalized color-distance threshold. Defaults to `0.05`. |
| `-h` | Print the command-line usage help. |

For example, replace pixels matching red with blue:

```sh
./build/ImageColorReplacer test-files/cornell-test.png 255,0,0 0,0,255
```

Use a wider match range to also replace colors near red:

```sh
./build/ImageColorReplacer test-files/cornell-test.png 255,0,0 0,0,255 -d 0.15
```

The threshold uses normalized, weighted RGB distance: `0` matches only an exact RGB color, and larger values include more nearby colors. The application always writes its result to `output.png` in the current working directory, replacing that file if it already exists.

## Project layout

```text
resources/ICRShader.wgsl       WebGPU compute shader
src/file-io/                  Image loading and PNG writing
src/graphics/                  WebGPU setup and compute pipeline
src/main/                      Command-line entry point
src/terminal-io/               Argument parsing and error reporting
test-files/                    Sample input image
```

## Notes

- The input is decoded to RGBA and the output is written as PNG. The output filename is currently fixed.
- The color parser expects three comma-separated components. Values are mapped into the 0–255 range; use integer RGB values for predictable results.
- Image processing requires a functioning WebGPU adapter. If initialization fails, check that `wgpu-native` is installed and that the system has a supported GPU/backend.
- The Makefile's `clean` target removes the configured build directory:

  ```sh
  make clean
  ```
