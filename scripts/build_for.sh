#!/bin/bash

ver="$1"
target="${2:-.}"
target="$(readlink -e "$target")"
cwd="$(dirname `readlink -e "$0"`)"
BUILD_CONFIG="${BUILD_CONFIG:-mysql_release}"

if [[ -z "$ver" ]]; then
  echo "usage: ./build_for.sh <mysql version>" >&2
  exit 1
fi

name='mdl_info'

cd "$target"
if [[ ! -e "mysql-$ver" ]]; then
  file="mysql-$ver.tar.gz"
  if [[ ! -e "$file" ]]; then
    wget "http://downloads.mysql.com/archives/get/file/$file"
  fi
  tar -xzvf "$file"
fi
if [[ ! -e "mysql-$ver/plugin/${name}" ]]; then
  cp -r "$cwd/../src" "mysql-$ver/plugin/${name}"
fi
cd "mysql-$ver"
if [[ ! -e plugin/${name}/${name}.so ]]; then
  cmake . -DBUILD_CONFIG="${BUILD_CONFIG}"
  ncpu=$( grep "processor" /proc/cpuinfo | wc -l )
  (( nproc=$ncpu*2 ))
  make -j $nproc "${name}"
fi
my_ver=$(gawk -F'[()" ]+' '$1=="SET"&&$2=="CPACK_PACKAGE_FILE_NAME"{print $3}' "CPackConfig.cmake")
ver="${my_ver#*-}"
cp plugin/${name}/${name}.so "$target/${name}_${ver}.so"

