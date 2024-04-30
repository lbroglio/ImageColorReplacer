imageColorReplacer: bin/ppmIO.o bin/main.o
	g++ -ggdb -o bin/imageColorReplacer bin/ppmIO.o bin/main.o -lGL

bin/ppmIO.o: src/file-io/ppmIO.cpp bin
	g++ -c -ggdb -o bin/ppmIO.o src/file-io/ppmIO.cpp 

bin/main.o: src/main/imageColorReplace.cpp bin
	g++ -c -ggdb -o bin/main.o src/main/imageColorReplace.cpp 

bin:
	mkdir bin

clean:
	rm bin/*