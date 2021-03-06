// You can format this file with the following one-liner:
// $ perl -i -pe 's{assert\((.*?), (.*), ".*"\);}{($a,$b)=($1,$2); (($c=$2) =~ s/([\\"])/\\\1/g); "assert($a, $b, \"$c\");"}ge' tests/tests.c

/*
 * This is a block comment.
 */

int g1, g2[4];

int assert(int expected, int actual, char *code) {
	if (expected == actual) {
		printf("%s => %d\n", code, actual);
	} else {
		printf("%s => %d expected but got %d\n", code, expected, actual);
		exit(1);
	}
}

int ret3() {
	return 3;
	return 5;
}

int add2(int x, int y) {
	return x + y;
}

int sub2(int x, int y) {
	return x - y;
}

int add5(int a, int b, int c, int d, int e) {
	return a + b + c + d + e;
}

int addx(int *x, int y) {
	return *x + y;
}

int sub_char(char a, char b, char c) {
	return a - b - c;
}

int fib(int x) {
	if (x<=1)
		return 1;
	return fib(x-1) + fib(x-2);
}

int main() {
	assert(0, 0, "0");
	assert(42, 42, "42");
	assert(21, 5+20-4, "5+20-4");
	assert(41,  12 + 34 - 5 , " 12 + 34 - 5 ");
	assert(47, 5+6*7, "5+6*7");
	assert(15, 5*(9-6), "5*(9-6)");
	assert(4, (3+5)/2, "(3+5)/2");
	assert(10, -10+20, "-10+20");
	assert(10, - -10, "- -10");
	assert(10, - - +10, "- - +10");

	assert(0, 0==1, "0==1");
	assert(1, 42==42, "42==42");
	assert(1, 0!=1, "0!=1");
	assert(0, 42!=42, "42!=42");

	assert(1, 0<1, "0<1");
	assert(0, 1<1, "1<1");
	assert(0, 2<1, "2<1");
	assert(1, 0<=1, "0<=1");
	assert(1, 1<=1, "1<=1");
	assert(0, 2<=1, "2<=1");

	assert(1, 1>0, "1>0");
	assert(0, 1>1, "1>1");
	assert(0, 1>2, "1>2");
	assert(1, 1>=0, "1>=0");
	assert(1, 1>=1, "1>=1");
	assert(0, 1>=2, "1>=2");

	assert(3, ({ int a; a=3; a; }), "({ int a; a=3; a; })");
	assert(3, ({ int a=3; a; }), "({ int a=3; a; })");
	assert(8, ({ int a=3; int z=5; a+z; }), "({ int a=3; int z=5; a+z; })");

	assert(3, ({ int a=3; a; }), "({ int a=3; a; })");
	assert(8, ({ int a=3; int z=5; a+z; }), "({ int a=3; int z=5; a+z; })");
	assert(6, ({ int a; int b; a=b=3; a+b; }), "({ int a; int b; a=b=3; a+b; })");
	assert(3, ({ int foo=3; foo; }), "({ int foo=3; foo; })");
	assert(8, ({ int foo123=3; int bar=5; foo123+bar; }), "({ int foo123=3; int bar=5; foo123+bar; })");

	assert(3, ({ int x; if (0) x=2; else x=3; x; }), "({ int x; if (0) x=2; else x=3; x; })");
	assert(3, ({ int x; if (1-1) x=2; else x=3; x; }), "({ int x; if (1-1) x=2; else x=3; x; })");
	assert(2, ({ int x; if (1) x=2; else x=3; x; }), "({ int x; if (1) x=2; else x=3; x; })");
	assert(2, ({ int x; if (2-1) x=2; else x=3; x; }), "({ int x; if (2-1) x=2; else x=3; x; })");

	assert(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }), "({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })");

	assert(10, ({ int i=0; while(i<10) i=i+1; i; }), "({ int i=0; while(i<10) i=i+1; i; })");

	assert(3, ({ 1; {2;} 3; }), "({ 1; {2;} 3; })");

	assert(10, ({ int i=0; while(i<10) i=i+1; i; }), "({ int i=0; while(i<10) i=i+1; i; })");
	assert(55, ({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; }), "({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; })");

	assert(3, ({ int x=3; *&x; }), "({ int x=3; *&x; })");
	assert(3, ({ int x=3; int *y=&x; int **z=&y; **z; }), "({ int x=3; int *y=&x; int **z=&y; **z; })");
	assert(5, ({ int x=3; int y=5; *(&x+1); }), "({ int x=3; int y=5; *(&x+1); })");
	assert(3, ({ int x=3; int y=5; *(&y-1); }), "({ int x=3; int y=5; *(&y-1); })");
	assert(5, ({ int x=3; int *y=&x; *y=5; x; }), "({ int x=3; int *y=&x; *y=5; x; })");
	assert(7, ({ int x=3; int y=5; *(&x+1)=7; y; }), "({ int x=3; int y=5; *(&x+1)=7; y; })");
	assert(7, ({ int x=3; int y=5; *(&y-1)=7; x; }), "({ int x=3; int y=5; *(&y-1)=7; x; })");
	assert(2, ({ int x=3; (&x+2)-&x; }), "({ int x=3; (&x+2)-&x; })");
	assert(8, ({ int x, y; x=3; y=5; x+y; }), "({ int x, y; x=3; y=5; x+y; })");
	assert(8, ({ int x=3, y=5; x+y; }), "({ int x=3, y=5; x+y; })");

	assert(3, ret3(), "ret3()");
	assert(8, add2(3, 5), "add2(3, 5)");
	assert(2, sub2(5, 3), "sub2(5, 3)");
	assert(15, add5(1,2,3,4,5), "add5(1,2,3,4,5)");

	assert(7, add2(3,4), "add2(3,4)");
	assert(1, sub2(4,3), "sub2(4,3)");
	assert(55, fib(9), "fib(9)");

	assert(3, ({ int x[2]; int *y=&x; *y=3; *x; }), "({ int x[2]; int *y=&x; *y=3; *x; })");

	assert(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; })");
	assert(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); })");
	assert(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }), "({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); })");

	assert(0, ({ int x[2][3]; int *y=x; *y=0; **x; }), "({ int x[2][3]; int *y=x; *y=0; **x; })");
	assert(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }), "({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); })");
	assert(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }), "({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); })");
	assert(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }), "({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); })");
	assert(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }), "({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); })");
	assert(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }), "({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); })");

	assert(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; })");
	assert(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); })");
	assert(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })");
	assert(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); })");
	assert(5, ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); }), "({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); })");

	assert(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }), "({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; })");
	assert(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; }), "({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; })");
	assert(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; }), "({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; })");
	assert(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }), "({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; })");
	assert(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; }), "({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; })");
	assert(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }), "({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; })");

	assert(8, ({ int x; sizeof(x); }), "({ int x; sizeof(x); })");
	assert(8, ({ int x; sizeof x; }), "({ int x; sizeof x; })");
	assert(8, ({ int *x; sizeof(x); }), "({ int *x; sizeof(x); })");
	assert(32, ({ int x[4]; sizeof(x); }), "({ int x[4]; sizeof(x); })");
	assert(96, ({ int x[3][4]; sizeof(x); }), "({ int x[3][4]; sizeof(x); })");
	assert(32, ({ int x[3][4]; sizeof(*x); }), "({ int x[3][4]; sizeof(*x); })");
	assert(8, ({ int x[3][4]; sizeof(**x); }), "({ int x[3][4]; sizeof(**x); })");
	assert(9, ({ int x[3][4]; sizeof(**x) + 1; }), "({ int x[3][4]; sizeof(**x) + 1; })");
	assert(9, ({ int x[3][4]; sizeof **x + 1; }), "({ int x[3][4]; sizeof **x + 1; })");
	assert(8, ({ int x[3][4]; sizeof(**x + 1); }), "({ int x[3][4]; sizeof(**x + 1); })");
	assert(8, ({ int x=1; sizeof(x=2); }), "({ int x=1; sizeof(x=2); })");
	assert(1, ({ int x=1; sizeof(x=2); x; }), "({ int x=1; sizeof(x=2); x; })");

	assert(0, g1, "g1");
	assert(3, ({ g1=3; g1; }), "({ g1=3; g1; })");
	assert(0, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; })");
	assert(1, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; })");
	assert(2, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; })");
	assert(3, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; }), "({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; })");

	assert(8, sizeof(g1), "sizeof(g1)");
	assert(32, sizeof(g2), "sizeof(g2)");

	assert(1, ({ char x=1; x; }), "({ char x=1; x; })");
	assert(1, ({ char x=1; char y=2; x; }), "({ char x=1; char y=2; x; })");
	assert(2, ({ char x=1; char y=2; y; }), "({ char x=1; char y=2; y; })");

	assert(1, ({ char x; sizeof(x); }), "({ char x; sizeof(x); })");
	assert(10, ({ char x[10]; sizeof(x); }), "({ char x[10]; sizeof(x); })");
	assert(1, ({ sub_char(7, 3, 3); }), "({ sub_char(7, 3, 3); })");

	assert(97, "abc"[0], "\"abc\"[0]");
	assert(98, "abc"[1], "\"abc\"[1]");
	assert(99, "abc"[2], "\"abc\"[2]");
	assert(0, "abc"[3], "\"abc\"[3]");
	assert(4, sizeof("abc"), "sizeof(\"abc\")");

	assert(7, "\a"[0], "\"\\a\"[0]");
	assert(8, "\b"[0], "\"\\b\"[0]");
	assert(9, "\t"[0], "\"\\t\"[0]");
	assert(10, "\n"[0], "\"\\n\"[0]");
	assert(11, "\v"[0], "\"\\v\"[0]");
	assert(12, "\f"[0], "\"\\f\"[0]");
	assert(13, "\r"[0], "\"\\r\"[0]");
	assert(27, "\e"[0], "\"\\e\"[0]");

	assert(106, "\j"[0], "\"\\j\"[0]");
	assert(107, "\k"[0], "\"\\k\"[0]");
	assert(108, "\l"[0], "\"\\l\"[0]");

	assert(7, "\ax\ny"[0], "\"\\ax\\ny\"[0]");
	assert(120, "\ax\ny"[1], "\"\\ax\\ny\"[1]");
	assert(10, "\ax\ny"[2], "\"\\ax\\ny\"[2]");
	assert(121, "\ax\ny"[3], "\"\\ax\\ny\"[3]");

	assert(0, "\0"[0], "\"\\0\"[0]");
	assert(16, "\20"[0], "\"\\20\"[0]");
	assert(65, "\101"[0], "\"\\101\"[0]");
	assert(104, "\1500"[0], "\"\\1500\"[0]");
	assert(0, "\x00"[0], "\"\\x00\"[0]");
	assert(119, "\x77"[0], "\"\\x77\"[0]");

	assert(2, ({ int x=2; { int x=3; } x; }), "({ int x=2; { int x=3; } x; })");
	assert(2, ({ int x=2; { int x=3; } int y=4; x; }), "({ int x=2; { int x=3; } int y=4; x; })");
	assert(3, ({ int x=2; { x=3; } x; }), "({ int x=2; { x=3; } x; })");

	printf("OK\n");
	return 0;
}
