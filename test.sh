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
assert 0 0
assert 42 42
# 加算、減算、空白の削除
assert 21 '5+20-4'
assert 41 ' 12   +34 - 5'
# 乗算、除算、()による演算順の変更
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert  4 '(3+5)/2'
# 単項演算子(+, -のみ)
assert 10 '-10+20'
assert  0 '-10*+5+50'
assert  8 '+5-(6 / -2)'
assert 10 '- -10'
assert 10 '- - +10'
# 比較演算子(==, !=, <= <, >=, >)
assert  1 '-1==-1'
assert  0  '-1 == 1'
assert  0  '-1!=-1'
assert  1  '-1!=1'
assert  1  '-1<=-1'
assert  1  '1 <=2'
assert  0  '-1<-1'
assert  1  '1 < 2'
assert  1  '-1>=-1'
assert  1  '-1 >= -2'
assert  0  '-1>-1'
assert  1  '-1 > -2'

echo OK
