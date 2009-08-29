#!/bin/sh

PLATFORM=`uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]'`
VMPAKNAME="sgfork-vms.pk3"
CONFPAKNAME="sgfork-configs.pk3"
CONFDIR="configs"
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
  PREF=..
else
  PREF=.
fi

make clean
make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=1 $*

cd $PREF/$BDIR/smokinguns/
zip -r $VMPAKNAME vm/
mv $VMPAKNAME $HDIR
#cd $PWD
cd ../../..

