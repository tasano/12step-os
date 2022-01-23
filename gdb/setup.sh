#!/bin/bash

GMP_VER="gmp-6.2.1"
GMP_FILE_NAME="${GMP_VER}.tar.xz"
GDB_VER="gdb-11.2"
GDB_FILE_NAME="${GDB_VER}.tar.gz"
SCRIPT_DIR=$(cd $(dirname $0) && pwd)
GMP_DIR=${SCRIPT_DIR}/${GMP_VER}
GDB_DIR=${SCRIPT_DIR}/${GDB_VER}
PATCH_DIR=${SCRIPT_DIR}/patch
H8300_SIM_DIR=${GDB_DIR}/sim/h8300

#------------------------------------------------
# Install GMP
#------------------------------------------------
if [ ! -e ${GMP_DIR} ]; then
    sudo apt install m4

    cd ${SCRIPT_DIR}
    wget https://gmplib.org/download/gmp/${GMP_FILE_NAME}
    tar Jxvf ${GMP_FILE_NAME}
    rm ${GMP_FILE_NAME}

    cd ${GMP_DIR}
    ./configure --enable-cxx
    make
    make check

    sudo make install
fi

#------------------------------------------------
# Install GDB
#------------------------------------------------
if [ ! -e ${GDB_DIR} ]; then
    cd ${SCRIPT_DIR}
    wget https://ftp.jaist.ac.jp/pub/GNU/gdb/${GDB_FILE_NAME}
    tar zxvf ${GDB_FILE_NAME}
    rm ${GDB_FILE_NAME}

    cd ${GDB_DIR}
    ./configure --target=h8300-elf --disable-nls
    make

    cp ${PATCH_DIR}/* ${H8300_SIM_DIR}
    cd ${H8300_SIM_DIR}
    patch < Makefile.patch
    patch < Makefile.in.patch
    patch < compile.c.patch

    cd ${GDB_DIR}
    make
fi
