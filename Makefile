edge_detector: edge_detector.o
	gcc -g -o edge_detector edge_detector.o

edge_detector.o: edge_detector.c
	gcc -g -c edge_detector.c

clean:
	rm -f edge_detector
	rm -f *.o *.exe laplacian*.ppm
