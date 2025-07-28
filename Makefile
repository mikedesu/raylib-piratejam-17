all: game index.html

game: main.cpp
	g++ $^ -lraylib -o $@ 

index.html:
	emcc main.cpp -DWEB_BUILD=1 -o index.html -I /usr/local/include -I /home/darkmage/src/emsdk/upstream/emscripten/cache/sysroot/include -L. -L /home/darkmage/src/raylib/src -l:libraylib.web.a -s USE_GLFW=3 -s EXPORTED_RUNTIME_METHODS=ccall -s ALLOW_MEMORY_GROWTH --shell-file minshell.html --preload-file ./img --preload-file ./music --preload-file ./sfx --preload-file ./shaders -DPLATFORM_WEB -DWEB

clean:
	rm -rfv *.o game index.html index.wasm index.data index.js
