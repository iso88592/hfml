FLAGS := -I/hfml/cinatra/include 
CF := -Wall -Wextra -Werror

server: src/server.cpp src/translator.cpp src/hfml.o src/lex.o
	g++ -std=c++20 ${FLAGS} ${CF} -o server src/translator.cpp src/server.cpp src/hfml.o src/lex.o ${LINK}

src/lex.yy.c: src/hfml.l src/hfml.tab.h
	flex -o $@ $<

src/hfml.tab.c src/hfml.tab.h: src/hfml.y 
	bison -d -o src/hfml.tab.c $<

src/hfml.o: src/hfml.tab.c 
	gcc -c -o src/hfml.o src/hfml.tab.c 

src/lex.o: src/lex.yy.c
	gcc -c -o src/lex.o src/lex.yy.c

clean:
	-rm src/hfml.tab.c src/hfml.tab.h src/hfml.o src/lex.o src/lex.yy.c
