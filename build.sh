#!/bin/bash

debug=0

make clean

for i in $@
do
	if [ "$i" == "DEBUG" ];then
		debug=1
		break
	fi
done

if [ "$debug" == "1" ];then
	make CFLAG="-g -O0" CXXFLAG="-g -O0"
else
	make
fi