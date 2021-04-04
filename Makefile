all:
	g++ main.cpp -o out -pthread -std=c++11 `pkg-config --cflags --libs opencv`
allMac:
	g++ main.cpp -o out -pthread -std=c++11 `pkg-config --cflags --libs opencv4`