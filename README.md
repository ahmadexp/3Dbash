## :black_large_square: retrocube :white_large_square:

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
---
Render 3D meshes in ASCII on the command line.
It runs on the standard C library.
```
                    +==
                ++======
             +============
          ==================
      ========================
   +===========================..
   ||+=====================.......
    ||+=================..........
    +||||===========...............
     +||||||==+.....................
      +||||||........................
       +||||||.......................
       +||||||~.......................
        +||||||....................~.%
           |||||...............~.
            ||||~..........~..
               ||.........
                ||....~

```

### 1. This implementation

In human language, the graphics are rendered more or less by the following algorithm:
```
rows <- terminal's height
columns <- terminal's width
// a face (surface) is a plane segment (x, y, z) restricted within 4 cube vertices
initialise a cube (6 faces)
for (r in rows):
    for (c in columns):
        z_rendered <- +inf
        have_intersection <- false
        pixel_to_draw <- (c, r)
        color_to_draw <- background
        for (surface in cube's faces):
            // from equation ax + by + cz + d = 0
            z <- surface.z(c, r)
            if (z < z_rendered) and ((c, r, z) in surface):
                z_rendered <- z
                color_to_draw <- surface.color
        draw(pixel_to_draw, color_to_draw)
```

### 2. Requirements

Currenctly there is no Windows support. You only need gcc and make:
1. **gcc**
2. **make**

### 3. Development and installation

#### 3.1 Development

##### 3.1.1 Compiling the project

The naming convention follows the one of [stb](https://github.com/nothings/stb).
Source files are found in `src` and headers in `include`.

When compiling the from project from a clean state, you need to specify where the mesh files
(those that specify how shapes are rendereed) shall be stored. You can do this by setting the
`PREFIX` variable to your directory of choice, e.g.:
```
make PREFIX=~/.config/retrocube
```
You can run the binary with (a list of command line arguments is provided in the next section):
```
./cube
```
You can delete the binary and object files with:
```
make clean
```
##### 3.1.2 Compiling the demos

Several demos that showcase various usages of the libraries are found in the `demos` directory.
These are compiled independently from their own file. To compile them you need to set the `PREFIX`
once again:
```
cd demos
make PREFIX=~/.config/retrocube
# then you will see some binaries and run the binary of your choice
```

#### 3.2 General installation

The `Makefile` includes an installation command. The binary will be installed at `/usr/bin/cube` as:
```
sudo make install
```
Similarly, you can uninstall it from `/usr/bin` as:
```
sudo make uninstall
```

#### 3.3 Installation as Nix package

On Nix (with flakes enabled) you don't need to install it and you can directly run it with:
```
nix run github:leonmavr/retrocube
```
Credits for the Nix packaging @pmarreck and @Quantenzitrone.

### 4. Usage

#### 4.1 Arguments

By default the program runs forever so you can stop it with `Ctr+C`. Below are the command line arguments it accepts.

Below are two examples of running the demo binary `./cube`:

#### 4.2 Tips

1. If the CPU usage is too high (it was low on my ancient laptop), you can reduce the fps e.g. to 15 by: `./cube -f 15` or `./cube --fps 15`.

### 5. Contributing

If you'd like to contribute, please follow the codiing guidelines (section 3.1) and make sure that it builds and runs.
I'll be happy to merge new changes.

Kudos to:
* [@pmarreck](https://github.com/pmarreck) - Nix packaging
* [@Quantenzitrone](https://github.com/Quantenzitrone) - Fixing and competing nix packaging, including the mesh text files properly, tidying up the make build
* [@IchikaZou](https://github.com/IchikaZou) - porting to Gentoo
* Anyone else who opened an issue for helping me make this project more robust.
