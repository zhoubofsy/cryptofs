#/bin/bash

rm -rf $PWD/mnt
mkdir $PWD/mnt

rm -rf $PWD/src
mkdir $PWD/src

./cryptofs -d -o debug $PWD/mnt/ $PWD/src
