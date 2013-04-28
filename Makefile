compiler = -Wall -g
objects = glew.o main.o pixel.o structs.o

ifeq ($(box), win32)
	libs = -lmingw32 -lSDLmain -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lopengl32 -lglu32 -lGdi32
else
	libs = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lGL -lGLU
endif

ifeq ($(build), release)
	compiler = -Wall -O3
endif

all: $(objects)
	gcc $(objects) -o depixel-gl.exe $(libs)
     
main.o:
	gcc $(compiler) -c main.c
	
glew.o:
	gcc $(compiler) -c glew.c
	
pixel.o:
	gcc $(compiler) -c pixel.c
	
structs.o:
	gcc $(compiler) -c structs.c

clean:
	rm $(objects) depixel-gl.exe
