build:
	gcc -Wall -std=c99 ./src/*.c -lSDL2 -lm -o ./bin/renderer
debug:
	gcc -ggdb -Wall -std=c99 ./src/*.c -lSDL2 -lm -o ./bin/renderer

run:
	./bin/renderer

clean:
	rm ./bin/*
	
