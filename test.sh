#!/bin/zsh

assert() {
  expected="${1}"
  input="${2}"

  ./9cc "${input}" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "${actual}" = "${expected}" ]; then
    echo "${input} => ${actual}"
  else
    echo "${input} => ${expected} expected, but got ${actual}"
    exit 1
  fi
}

echo "\n単項(0~255)のコンパイル"
assert 0 0
assert 42 42

echo "\n空白の内、足し引きの式"
assert 21 "5+20-4"
assert 48 "16+16+16-16+16"

echo "\n空白が入った足し引きの式"
assert 32 "16 + 16 - 0"
assert 0 "1 + 50     -1-     50"

echo "\nビット演算(&, |, ^)の式"
assert 7 "4 |3"
assert 16 "31& 16"
assert 3 "1   ^   2"

echo "\nTEST OK"
