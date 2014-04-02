#!/bin/bash

ver="$1"
target="${2:-.}"
target="$(readlink -e "$target")"
cwd="$(dirname `readlink -e "$0"`)"

if [[ -z "$ver" ]]; then
  echo "usage: ./build_for.sh <mysql version>" >&2
  exit 1
fi
cd "$target"
file="mysql-$ver.tar.gz"
#wget "http://downloads.mysql.com/archives/get/file/$file"
#tar -xzvf "$file"
cp -r "$cwd/../src" "mysql-$ver/plugin/mdl_locks"
cd "mysql-$ver"
#cmake .
ncpu=$( grep "processor" /proc/cpuinfo | wc -l )
(( nproc=$ncpu*2 ))
#make -j $nproc mdl_locks
cp plugin/mdl_locks/mdl_locks.so "$target"



