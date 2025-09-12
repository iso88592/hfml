.PHONY: all

all: backend backend-debug

backend-debug: backend.c hfml.h hfml.c
	gcc -O0 -g -o backend-debug backend.c hfml.c

backend: backend.c hfml.h hfml.c
	gcc -O3 -o backend backend.c hfml.c