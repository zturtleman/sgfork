#!/bin/sh

PLATFORM=`uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]'`
VMPAKNAME="sgfork-qvms.pk3"
CONFPAKNAME="sgfork-configs.pk3"
UIPAKNAME="sgfork-ui.pk3"
SCRIPTSPAKNAME="sgfork-scripts.pk3"
GFXPAKNAME="sgfork-gfx.pk3"
MENUPAKNAME="sgfork-menu.pk3"
CONFDIR="configs"
PWD=`pwd`
MAINBINARY=
DEDIBINARY=
HDIR=
SHDIR=

[ "$USE_DEDICATED" = "" ] && USE_DEDICATED=0
[ "$USE_CLIENT" = "" ] && USE_CLIENT=0
[ "$USE_SMP" = "" ] && USE_SMP=0
[ "$USE_QVMS" = "" ] && USE_QVMS=1
[ "$USE_GFX" = "" ] && USE_GFX=1
[ "$USE_SCRIPTS" = "" ] && USE_SCRIPTS=1
[ "$USE_UI" = "" ] && USE_UI=1
[ "$USE_MENU" = "" ] && USE_MENU=1

buildqvms( )
{
  make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=1 $*
  cd $BDIR/smokinguns/
  zip -r $VMPAKNAME vm/
  mv $VMPAKNAME "$HDIR"
  cd ../../..
}

buildclient( )
{
  make BUILD_CLIENT=1 BUILD_CLIENT_SMP=${USE_SMP} BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=0 $*
  cd $BDIR/
  cp $MAINBINARY "$SHDIR"
  cd ../..
}

buildded( )
{
  make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=1 BUILD_GAME_SO=0 BUILD_GAME_QVM=0 $*
  cd $BDIR/
  cp $DEDIBINARY "$SHDIR"
  cd ../..
}

if [ "$1" = "--help" ] || [ "$1" = "-h" ]
then
  echo "Usage: $0 [OPTIONS]"
  echo "" && echo "OPTIONS"
  echo "  --home home"
  echo "  --second_home second home"
  echo "  --dedonly"
  echo "  --clientonly"
  echo "  --qvmsonly"
  echo "  --pk3only"
  echo "  --smp"
  echo "  --buildall"
  exit
fi

ARCH=`uname -m | sed -e s/i.86/i386/`
if [ "$PLATFORM" = "mingw32" ]
then
  if [ "$ARCH" = "i386" ]
  then
    ARCH="x86"
  fi
fi

if [ "$PLATFORM" = "sunos" ] || [ "$PLATFORM" = "darwin" ]
then
  ARCH=`uname -p | sed -e s/i.86/i386/`
fi

for i in "$@"
do
  if [ "$i" = "--home" ]
  then
    shift
    HDIR=$i
  elif [ "$i" = "--second_home" ]
  then
    shift
    SHDIR=$HDIR
  elif [ "$i" = "--dedonly" ]
  then
    USE_DEDICATED=1
    USE_QVMS=0
    USE_CLIENT=0
    USE_PK3=0
    FINISH_LIMIT=`expr $FINISH_LIMIT + 1`
  elif [ "$i" = "--clientonly" ]
  then
    USE_QVMS=0
    USE_DEDICATED=0
    USE_PK3=0
    USE_CLIENT=1
    USE_PK3=0
    FINISH_LIMIT=`expr $FINISH_LIMIT + 1`
  elif [ "$i" = "--qvmsonly" ]
  then
    USE_QVMS=1
    USE_PK3=0
    FINISH_LIMIT=1
  elif [ "$i" = "--pk3only" ]
  then
    USE_DEDICATED=0
    USE_CLIENT=0
    USE_SMP=0
    USE_PK3=1
    USE_QVMS=0
    FINISH_LIMIT=4
  elif [ "$i" = "--smp" ]
  then
    USE_SMP=1
    FINISH_LIMIT=`expr $FINISH_LIMIT + 1`
  elif [ "$i" = "--buildall" ]
  then
    USE_DEDICATED=1
    USE_CLIENT=1
    USE_SMP=1
    USE_PK3=1
    FINISH_LIMIT=8
  fi
done

if [ "$USE_PK3" = "0" ]
then
  USE_GFX=0 USE_SCRIPTS=0 USE_UI=0 USE_MENU=0
else
  USE_GFX=1 USE_SCRIPTS=1 USE_UI=1 USE_MENU=1
fi

if [ "$PLATFORM" = "mingw32" ]
then
  BDIR=build/release-mingw32-x86
  if [ "$1" = "-home" ]
  then
    shift
    HDIR=$1
    SHDIR=$HDIR
    shift
  else
    HDIR=$PWD
    SHDIR=$HDIR
  fi
  MAINBINARY=smokinguns.x86.exe
  DEDIBINARY=smokinguns_dedicated.x86.exe
fi

if [ "$PLATFORM" = "linux" ]
then
  BDIR=build/release-linux-${ARCH}
  MAINBINARY=smokinguns.${ARCH}
  DEDIBINARY=smokinguns_dedicated.${ARCH}
  [ "$HDIR" = "" ] && HDIR=$HOME/.smokinguns/base
  [ "$SDIR" = "" ] && SDIR=$HOME/smokinguns
fi

if [ `basename $PWD` = "tools" ]
then
  cd ../
fi

make clean

FINISH=0
while [ "$FINISH" != "$FINISH_LIMIT" ]
do
  if [ "$USE_QVMS" = "1" ] && [ "$DID_QVMS" != "1" ]
  then
    buildqvms
    DID_QVMS=1
  elif [ "$USE_GFX" = "1" ] && [ "$DID_GFX" != "1" ]
  then
    cd base/
    zip -r $GFXPAKNAME gfx/
    mv $GFXPAKNAME "$HDIR"
    cd ..
    DID_GFX=1
  elif [ "$USE_UI" = "1" ] && [ "$DID_UI" != "1" ]
  then
    cd base/
    zip -r $UIPAKNAME ui/
    mv $UIPAKNAME "$HDIR"
    cd ..
    DID_UI=1
  elif [ "$USE_SCRIPTS" = "1" ] && [ "$DID_SCRIPTS" != "1" ]
  then
    cd base/
    zip -r $SCRIPTSPAKNAME scripts/
    mv $SCRIPTSPAKNAME "$HDIR"
    cd ..
    DID_SCRIPTS=1
  elif [ "$USE_MENU" = "1" ] && [ "$DID_MENU" != "1" ]
  then
    cd base/
    zip -r $MENUPAKNAME menu/
    mv $MENUPAKNAME "$HDIR"
    cd ..
    DID_MENU=1
  elif [ "$USE_CLIENT" = "1" ] && [ "$DID_CLIENT" != "1" ]
  then
    buildclient
    DID_CLIENT=1
  elif [ "$USE_DEDICATED" = "1" ] && [ "$DID_DED" != "1" ]
  then
    buildded
    DID_DED=1
  fi
  FINISH=`expr $FINISH + 1`
done

[ "$PLATFORM" = "mingw32" ] && cp misc/win32/dlls/SDL.dll "$HDIR/../"

cd base
[ -f  ui/main.menu.template ] && sed "s/SGFORK_RELEASE/SGFork release at `date`/" ui/main.menu.template > ui/main.menu
cd ../
cd $PWD

echo "<<<The game is successfully deployed!>>>"
