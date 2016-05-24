user-sh : bison.tab.o execute.o lex.yy.o
	cc -o user-sh bison.tab.o execute.o lex.yy.o
bison.tab.o lex.yy.o : bison.tab.c  global.h bison.tab.h lex.yy.c
	cc -c bison.tab.c lex.yy.c
execute.o : execute.c global.h
	cc -c execute.c
bison.tab.c  bison.tab.h: bison.y
	bison -d bison.y
lex.yy.c: myShell.lex
	lex myShell.lex
clean :
	rm user-sh bison.tab.o execute.o bison.tab.c lex.yy.c bison.tab.h lex.yy.o
