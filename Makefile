all:
	g++ -O3 reversi.cpp -o reversi_cpp
	g++ -O3 reversi2.cpp -o reversi2_cpp
	g++ -O3 reversi3.cpp -o reversi3_cpp

one:
	g++ -O3 reversi.cpp -o reversi_cpp

two:
	g++ -O3 reversi2.cpp -o reversi2_cpp

three:
	g++ -O3 reversi3.cpp -o reversi3_cpp

four:
	g++ -O3 reversi4.cpp -o reversi4_cpp

debug:
	g++ -g reversi4.cpp -o reversi4_cpp