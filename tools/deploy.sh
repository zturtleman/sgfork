#!/bin/sh

# The main SGFork deployment file

PLATFORM=`uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]'`
VMPAKNAME="sgfork-qvms.pk3"
UIPAKNAME="sgfork-ui.pk3"
SCRIPTSPAKNAME="sgfork-scripts.pk3"
GFXPAKNAME="sgfork-gfx.pk3"
MENUPAKNAME="sgfork-menu.pk3"
PWD=`pwd`
MAINBINARY=
DEDIBINARY=
BASE_HOME_DIR=
BINARY_HOME_DIR=

[ "$USE_DEDICATED" = "" ] && USE_DEDICATED=0
[ "$USE_CLIENT" = "" ]    && USE_CLIENT=0
[ "$USE_SMP" = "" ]       && USE_SMP=0
[ "$USE_QVMS" = "" ]      && USE_QVMS=0
[ "$USE_PK3" = "" ]       && USE_PK3=1


usage( )
{
  echo "Usage: $0 [OPTIONS]"
  echo "" && echo "OPTIONS"
  echo "  --base_home path"
  echo "  --binary_home path"
  echo "  --ded"
  echo "  --client"
  echo "  --qvms"
  echo "  --pk3"
  echo "  --smp"
  echo "  --buildall"
  exit 1
}

ExitOnError( )
{
  if [ "$1" != "" ]
  then
    echo "<<<ERROR: Operation $1 failed!>>>"
  else
    echo "<<<ERROR: Failed!>>>"
  fi
  exit 1
}

buildqvms( )
{
  make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=1 $*
  [ $? -eq 0 ] || ExitOnError "QVMS building"
  cd $BUILD_DIR/smokinguns/
  zip -r $VMPAKNAME vm/
  mv $VMPAKNAME "$BASE_HOME_DIR"
  cd ../../..
}

buildclient( )
{
  make BUILD_CLIENT=1 BUILD_CLIENT_SMP=${USE_SMP} BUILD_SERVER=0 BUILD_GAME_SO=0 BUILD_GAME_QVM=0 $*
  [ $? -eq 0 ] || ExitOnError "Client building"
  cd $BUILD_DIR/
  cp $MAINBINARY "$BINARY_HOME_DIR"
  cd ../..
  [ "$PLATFORM" = "mingw32" ] && cp misc/win32/dlls/SDL.dll "$BINARY_HOME_DIR"
}

buildded( )
{
  make BUILD_CLIENT=0 BUILD_CLIENT_SMP=0 BUILD_SERVER=1 BUILD_GAME_SO=0 BUILD_GAME_QVM=0 $*
  [ $? -eq 0 ] || ExitOnError "Dedicated building"
  cd $BUILD_DIR/
  cp $DEDIBINARY "$BINARY_HOME_DIR"
  cd ../..
}

# ---=== Read options ===---

if [ $# -ne 0 ]
then
  while [ "${1}" != "" ]
  do
    case "${1}" in
      --base_home)   BASE_HOME_DIR=${2}   && shift && shift ;;
      --binary_home) BINARY_HOME_DIR=${2} && shift && shift;;
      --smp)         USE_SMP=1            && shift ;;
      --ded)         USE_DEDICATED=1      && shift ;;
      --client)      USE_CLIENT=1         && shift ;;
      --qvms)        USE_QVMS=1           && shift ;;
      --pk3)         USE_PK3=1            && shift ;;
      --buildall)
        USE_DEDICATED=1
        USE_CLIENT=1
        USE_QVMS=1
        USE_PK3=1
        shift ;;
      --help) usage ;;
      --h) usage ;;
      --usage) usage ;;
	  *) break ;;
	  esac
  done
fi

if [ "$USE_PK3" = "0" ]
then
  USE_GFX=0 USE_SCRIPTS=0 USE_UI=0 USE_MENU=0
else
  USE_GFX=1 USE_SCRIPTS=1 USE_UI=1 USE_MENU=1
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

if [ "$PLATFORM" = "mingw32" ]
then
  BUILD_DIR=build/release-mingw32-${ARCH}
  [ "$BASE_HOME_DIR" = "" ] && BASE_HOME_DIR=$PWD
  [ "$SDIR" = "" ] && BINARY_HOME_DIR=$BASE_HOME_DIR
  MAINBINARY="smokinguns.x86.exe"
  DEDIBINARY="smokinguns_dedicated.x86.exe"
elif [ "$PLATFORM" = "linux" ]
then
  BUILD_DIR=build/release-linux-${ARCH}
  MAINBINARY="smokinguns.${ARCH}"
  DEDIBINARY="smokinguns_dedicated.${ARCH}"
  [ "$BASE_HOME_DIR" = "" ] && BASE_HOME_DIR=$HOME/.smokinguns/base
  [ "$SDIR" = "" ] && SDIR=$HOME/smokinguns
fi

if [ `basename $PWD` = "tools" ]
then
  cd ../
fi

# ---=== Clean ===---
echo "<<<Cleaning all previous builds>>>"
make clean

# ---=== Build and deploy ===---
echo "<<<Building and deploying>>>"

[ "$USE_DEDICATED" = "1" ] && buildded $*
[ "$USE_CLIENT" = "1" ] && buildclient $*
[ "$USE_QVMS" = "1" ] && buildqvms $*
if [ "$USE_GFX" = "1" ]
then
  cd base/
  zip -r $GFXPAKNAME gfx/
  mv $GFXPAKNAME "$BASE_HOME_DIR"
  cd ..
fi
if [ "$USE_UI" = "1" ]
then
  cd base/
  #Set current date to main manu special field
  [ -f  ui/main.menu.template ] && sed "s/SGFORK_RELEASE/SGFork release at `date`/" ui/main.menu.template > ui/main.menu
  zip -r $UIPAKNAME ui/
  mv $UIPAKNAME "$BASE_HOME_DIR"
  cd ..
fi
if [ "$USE_SCRIPTS" = "1" ]
then
  cd base/
  zip -r $SCRIPTSPAKNAME scripts/
  mv $SCRIPTSPAKNAME "$BASE_HOME_DIR"
  cd ..
fi
if [ "$USE_MENU" = "1" ]
then
  cd base/
  zip -r $MENUPAKNAME menu/
  mv $MENUPAKNAME "$BASE_HOME_DIR"
  cd ..
fi

cd $PWD

echo "<<<The game is successfully deployed!>>>"
