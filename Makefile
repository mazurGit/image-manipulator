.PHONY: configure build run clean debug

build:
	cmake -S . -B build
	cmake --build build

run: build
	./build/playground