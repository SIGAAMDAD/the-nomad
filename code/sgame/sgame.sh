#!/bin/sh

mkdir -p vm
cd vm

CC="../lcc -DQ3_VM -DSGAME -D_NOMAD_VERSION=0 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=1 -S -Wf-target=bytecode -Wf-g -I../../game -I../../engine -I../../allocator"

$CC ../sg_main.c
$CC ../sg_mem.c
$CC ../../engine/n_shared.c
$CC ../../engine/n_math.c
$CC ../../game/bg_lib.c

../q3asm -f ../sgame

cd ..