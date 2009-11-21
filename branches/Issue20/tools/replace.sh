#!/bin/sh

PWD=`pwd`

if [ "$2" = "" ]
then
  echo "$0: usage: <str> <dest>"
  exit
fi

if [ "basename ${PWD}" = "tools" ]
then
cd ../
fi

for i in `find . -name "*.[ch]"`; \
do \
  cat $i | sed -e s/$1/$2/g > tmpsed; \
  cat tmpsed > $i; \
  rm tmpsed; \
done
