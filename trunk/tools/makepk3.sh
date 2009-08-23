#!/bin/sh

PLATFORM=`uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]'`
PAKNAME="pak2.pk3"
PWD=`pwd`
PREF=

if [ "$PLATFORM" = "mingw32" ]
then
  BDIR=build/release-mingw32-x86
  HDIR=
fi

if [ "$PLATFORM" = "linux" ]
then
  BDIR=build/release-linux-i386
  HDIR=$HOME/.smokinguns/base
fi

if [ `basename $PWD` = "tools" ]
then
  cd ..
fi

make clean
make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=1 $*

cd $BDIR/smokinguns/
zip -r $PAKNAME vm/
mv $PAKNAME $HDIR
cd $PWD

