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

assert 0 0
assert 42 42

assert 21 "5+20-4"
assert 48 "16+16+16-16+16"

assert 32 "16 + 16 - 0"
assert 0 "1 + 50     -1-     50"

echo OK
