#/bin/bash -v

export PATH=/mingw32/bin:$PATH
export MINGW_BIN_DIR=/mingw32/bin
export WX_DIR=/home/User/wxWidgets-3.2.2.1

# dangerous; stash changes first
#git clean -x -d -f .

mingw32-make -f makefile.msys clean
mingw32-make -f makefile.msys git.c
rm libloc.c
mingw32-make -f makefile.msys MINGW_BIN_DIR=$MINGW_BIN_DIR WX_DIR=$WX_DIR install_win