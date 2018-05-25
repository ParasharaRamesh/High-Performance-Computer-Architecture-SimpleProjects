compile: lib/imagelib/imagelib.c src/main.c
	mkdir -p build
	mkdir -p test/img_out
	gcc -o build/sequential lib/imagelib/imagelib.c src/main.c `libpng-config --ldflags` -lm -I.
	gcc -fopenmp -o build/parallel lib/imagelib/imagelib.c src/main.c `libpng-config --ldflags` -lm -I.
