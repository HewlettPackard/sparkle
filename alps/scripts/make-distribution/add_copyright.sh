#!/bin/bash

if [ -z $1 ]; then
  echo "$0 <DISTRIBUTION_DIR>"
  exit
fi

DISTRIBUTION_DIR=$1
COPYRIGHT_C_FILE=copyright.c.txt
COPYRIGHT_SH_FILE=copyright.sh.txt
COPYRIGHT_MD_FILE=copyright.md.txt

DIRS="${DISTRIBUTION_DIR} -name third-party -prune -o -name build -prune"

# Header with C style comment
for i in $(find ${DIRS} -o -name "*.cc" -o -name "*.hh" -o -name "*.h" -o -name "*.c" -o -name "*.xmacro")
do
  if [ -f $i ] && [ ! `grep -q Copyright $i` ];
  then
    echo 'Adding copyright notice to' $i
    cat ${COPYRIGHT_C_FILE} $i >$i.new && mv $i.new $i
  fi
done


# Header with Shell style comment
for i in $(find ${DIRS} -o -name "*.py" -o -name "CMakeLists.txt" -o -name "*.cmake" -o -name "Doxyfile")
do
  if [ -f $i ] && [ ! `grep -q Copyright $i` ];
  then
    echo 'Adding copyright notice to' $i
    cat ${COPYRIGHT_SH_FILE} $i >$i.new && mv $i.new $i
  fi
done


# Header with Markdown style comment
for i in $(find ${DIRS} -o -name "*.md")
do
  if [ -f $i ] && [ ! `grep -q Copyright $i` ];
  then
    echo 'Adding copyright notice to' $i
    cat ${COPYRIGHT_MD_FILE} $i >$i.new && mv $i.new $i
  fi
done
