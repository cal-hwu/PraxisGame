# JUMPER GAME raylib
A simple endless runner for hours of fun.
## How to compile
1. Make sure raylib library is installed ([raylib download](https://raylib.com))

### Windows (MinGW)
* Make sure some form of C compiler (GCC recommended) is installed
*  Compile using this code
`  gcc main.c -o game -lraylib -lopengl32 -lgdi32 -lwinmm `

### Linux
*  Compile using this code
` gcc main.c -o game -lraylib -lm -lpthread -ldl -lrt -lX11 `

### MacOS
*  Compile using this code
` gcc main.c -o game -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `

## How to run
Run ./game in the correct directory
