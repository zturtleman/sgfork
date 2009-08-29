#!/bin/sh

PLATFORM=`uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]'`
VMPAKNAME="sgfork-qvms.pk3"
CONFPAKNAME="sgfork-configs.pk3"
UIPAKNAME="sgfork-ui.pk3"
GFXPAKNAME="sgfork-gfx.pk3"
CONFDIR="configs"
PWD=`pwd`
MAINBINARY=
DEDIBINARY=

if [ "$PLATFORM" = "mingw32" ]
then
  BDIR=build/release-mingw32-x86
  if [ "$1" = "-home" ]
  then
    shift
    HDIR=$1
    shift
  else
    HDIR=$PWD
  fi
  MAINBINARY=smokinguns.x86.exe
  DEDIBINARY=smokinguns_dedicated.x86.exe
fi

if [ "$PLATFORM" = "linux" ]
then
  BDIR=build/release-linux-i386
  HDIR=$HOME/.smokinguns/base
  MAINBINARY=smokinguns.x86.exe
  DEDIBINARY=smokinguns_dedicated.x86.exe
fi

if [ `basename $PWD` = "tools" ]
then
  cd ../
fi

make clean
if [ "$1" = "-buildall" ]
then
  shift
  make
  cd $BDIR
  mv $MAINBINARY "$HDIR/../"
  mv $DEDIBINARY "$HDIR/../"
  cd ../../
else
  make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=1 $*
fi

cd $BDIR/smokinguns/
zip -r $VMPAKNAME vm/
mv $VMPAKNAME "$HDIR"
cd ../../..
cd base
zip -r $UIPAKNAME ui/
zip -r $GFXPAKNAME gfx/
mv $UIPAKNAME $GFXPAKNAME "$HDIR"
cd ../
cd $PWD
