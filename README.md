ml purge
ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0 # pouze na Barbore
cd Assignment
mkdir build && cd build
CC=icx CXX=icpx cmake ..
make -j

salloc -A DD-24-108 -p qcpu -N 1 -t 1:0:0 --comment 'use:vtune=2022.2.0' --x11