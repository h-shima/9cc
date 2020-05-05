#!/bin/bash

assert(){
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# 1つだけ渡された整数の表示
assert 0 'return 0;'
assert 42 'return 42;'
# 加算、減算、空白の削除
assert 21 'return 5+20-4;'
assert 41 'return 12   +34 - 5;'
# 乗算、除算、()による演算順の変更
assert 47 'return 5+6*7;'
assert 15 'return 5*(9-6);'
assert  4 'return (3+5)/2;'
# 単項演算子(+, -のみ)
assert 10 'return -10+20;'
assert  0 'return -10*+5+50;'
assert  8 'return +5-(6 / -2);'
assert 10 'return - -10;'
assert 10 'return - - +10;'
# 比較演算子(==, !=, <= <, >=, >)
assert  1 'return -1==-1;'
assert  0 'return -1 == 1;'
assert  0 'return -1!=-1;'
assert  1 'return -1!=1;'
assert  1 'return -1<=-1;'
assert  1 'return 1 <=2;'
assert  0 'return -1<-1;'
assert  1 'return 1 < 2;'
assert  1 'return -1>=-1;'
assert  1 'return -1 >= -2;'
assert  0 'return -1>-1;'
assert  1 'return -1 > -2;'
# セミコロンで文を分割できる
assert  3 '1; 2; return 3;'
# １文字のローカル変数(a〜z)を使用できる
assert  2 'return a=2;'
assert  2 'a=2; return a;'
assert 14 'a= 3; b = 5* 6-8; return a+b /2;'
assert  2 'a=b=c=2; a = a+b-c; return a;'
# 複数文字のローカル変数(アルファベットと数字)を使用できる
assert  2 'a10B=2; return a10B;'
assert 14 'foo =3; bar = 5*6-8; return foo+bar/2;'
assert  2 'hoge=fuga=piyo=2; hoge = hoge+fuga-piyo; return hoge;'
# return文が使用できる
assert 14 'foo =3; bar = 5*6-8; return foo+bar/2;'
assert 30 'a=30; return a;'
# シンプルはif文 if (A) B が使用できる
assert   5 'if (1) return 5;'
assert  10 'a = 1; b = 2; if (a + b < 4) return 10;'
assert   3 'a = 1; if (a) b = 2; return a + b;'
assert   1 'a = 1; if (0) a = 3; return a;'
assert   5 'if (1) a = 2; b = 3; return a +b;'
assert  10 'return 10; if (1) a = 2; return a;'
# elseを使ったif文が使用できる
assert  20 'if (0) a = 10; else a = 20; return a;'
assert   3 'a = 1; if (a) b = 2; else b = 3; return a + b;'
assert  20 'if (0) a = 10; else if (1) a = 20; return a;'
assert  30 'a = 0; if (a) b = 10; else if (a) b = 20; else b = 30; return b;'
# while文が使用できる
assert   0 'a = 2; while (a) a = a -1; return a;'
assert  10 'i=0; while(i<10) i=i+1; return i;'
# for文が使用できる
assert   5 'a=0; for(i = 5; i > 0; i = i - 1) a = a + 1; return a;'
assert  55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
assert   3 'for (;;) return 3; return 5;'

echo OK
