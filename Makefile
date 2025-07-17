


all: game


game: main.cpp
	g++ $^ -lraylib -o $@ 


clean:
	rm -rfv *.o game 
