FLAGS := -I/hfml/cinatra/include -DFULL_LEAK_CHECK -DNO_YY_DEBUG
CF := -Wall -Wextra -Werror

server: src/server.cpp src/translator.cpp src/hfml.o src/lex.o src/mystr.o
	g++ -std=c++20 ${FLAGS} ${CF} -o server src/translator.cpp src/server.cpp src/hfml.o src/lex.o src/mystr.o ${LINK}

src/lex.yy.c: src/hfml.l src/hfml.tab.h
	flex -o $@ $<

src/hfml.tab.c src/hfml.tab.h: src/hfml.y
	bison -v --report-file=bison.log -d -o src/hfml.tab.c $<

src/hfml.o: src/hfml.tab.c
	gcc -c -o src/hfml.o src/hfml.tab.c -std=c99

src/mystr.o: src/mystr.c
	gcc -c -o src/mystr.o src/mystr.c -std=c99

src/lex.o: src/lex.yy.c
	gcc -c -o src/lex.o src/lex.yy.c -std=c99

splicestrtest: src/splicestr.h src/splicestr.c src/splicestrtest.c
	g++ -O3 -o splicestrtest src/splicestr.c src/splicestrtest.c

test: splicestrtest
	./splicestrtest

clean:
	-rm src/hfml.tab.c src/hfml.tab.h src/hfml.o src/lex.o src/lex.yy.c src/mystr.o splicestrtest
