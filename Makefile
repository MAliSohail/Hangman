all:
	g++ -I src/include -L src/lib -Dmain=SDL_main -g -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer -mwindows
