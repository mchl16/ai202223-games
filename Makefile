CC=g++
CFLAGS=-std=c++17 -O3
OUT=reversi

debug:
	g++ -g reversi4.cpp -o $(OUT)

one:
	$(CC) $(CFLAGS) reversi.cpp -o $(OUT)

two:
	$(CC) $(CFLAGS) reversi2.cpp -o $(OUT)

three:
	$(CC) $(CFLAGS) reversi3.cpp -o $(OUT)

four:
	$(CC) $(CFLAGS) reversi4.cpp -o $(OUT)

