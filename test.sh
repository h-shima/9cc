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
assert  3 '1; 2; 3;'
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

echo OK
