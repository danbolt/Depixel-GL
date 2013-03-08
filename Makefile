compiler = -Wall -g
objects = glew.o main.o pixel.o

ifeq ($(box), win32)
	libs = -lmingw32 -lSDLmain -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lopengl32 -lglu32
else
	libs = -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lGL -lGLU
endif

all: $(objects)
	gcc $(objects) -o depixel-gl.exe $(libs)
     
main.o:
	gcc $(compiler) -c main.c
	
glew.o:
	gcc $(compiler) -c glew.c
	
pixel.o:
	gcc $(compiler) -c pixel.c

clean:
	rm $(objects)
