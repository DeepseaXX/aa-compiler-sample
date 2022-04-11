#!/bin/sh
p_name=$1
echo $p_name
#cp ./step/parse04.c parse.c
make clean
make
./compiler ./sample/$p_name