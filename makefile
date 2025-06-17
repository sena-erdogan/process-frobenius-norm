all:
	gcc -c -g processP.c -ansi
	gcc -o processP processP.o -lm
	gcc -c -g processR.c -ansi
	gcc -o processR processR.o -lm

clean:
	rm -rf *o processP
	rm -rf *o processR
