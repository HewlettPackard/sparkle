#!/bin/bash

if [ -z $1 ] || [ -z $2 ]; then
  echo "$0 <PROJECT_DIR> <DISTRIBUTION_DIR>"
  exit
fi

PROJECT_DIR=$1
DISTRIBUTION_DIR=$2

if [ -f ${DISTRIBUTION_DIR} ] || [ -f ${DISTRIBUTION_DIR} ]; then
  echo "Path ${DISTRIBUTION_DIR} already exists"
  exit
fi

# copy tree
cp -r ${PROJECT_DIR} ${DISTRIBUTION_DIR}

# cleanup tree
rm -f ${DISTRIBUTION_DIR}/.gitignore
rm -rf ${DISTRIBUTION_DIR}/.git

rm -f ${DISTRIBUTION_DIR}/TODO
rm -f ${DISTRIBUTION_DIR}/pegasus.yml
rm -rf ${DISTRIBUTION_DIR}/scripts
rm -rf ${DISTRIBUTION_DIR}/build
rm -rf ${DISTRIBUTION_DIR}/doc/dox
rm -rf ${DISTRIBUTION_DIR}/doc/ppt
rm -rf ${DISTRIBUTION_DIR}/test/integration
rm -rf ${DISTRIBUTION_DIR}/test/unit/slabheap
rm -rf ${DISTRIBUTION_DIR}/test/unit/roles.yml
rm -rf ${DISTRIBUTION_DIR}/test/unit/test_unit.yml

# remove code using The-Machine API 
sed '/ifdef __ARCH_FAM__/,/endif \/\/ __ARCH_FAM__/d' -i ${DISTRIBUTION_DIR}/src/pegasus/lfs_region_file.cc
sed '/ifdef __ARCH_FAM__/,/endif \/\/ __ARCH_FAM__/d' -i ${DISTRIBUTION_DIR}/src/pegasus/tmpfs_region_file.cc
sed '/ifdef __ARCH_FAM__/,/endif \/\/ __ARCH_FAM__/d' -i ${DISTRIBUTION_DIR}/src/globalheap/lease.cc


# add copyright notice
./add_copyright.sh ${DISTRIBUTION_DIR}
