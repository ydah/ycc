#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  cc -c ./test/test_helper.c -o test_helper.o
  ./ycc "$input" > tmp.s
  cc -o tmp tmp.s test_helper.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 3 'a=3;'
assert 19 'a=3*5;a+4;'
assert 13 'foo=3;bar=5;foo+bar*2;'

assert 8 'return 8;'
assert 21 'a=3;return a*7;'
assert 14 'foo = 3;bar = 5 * 6 - 8;return foo + bar / 2;'

assert 3 'if (0) return 2; return 3;'
assert 2 'if (1-1) return 3; return 2;'
assert 3 'if (1) return 3; return 2;'
assert 3 'if (2-1) return 3; return 2;'

assert 10 'i=0;while(i<10)i=i+1;return i;'
assert 55 'i=0;j=0;while(i<10){i=i+1;j=j+i;}return j;'
assert 2 'i=0;for(i=0;i<3;i=i+1)j=i;return j;'
assert 5 'i=0;for(i=0;i<3;i=i+1)j=i;return j+i;'
assert 19 'for(i=0;i<10;i=i+1)j=i;return j+i;'

assert 5 'foo();'

echo OK
