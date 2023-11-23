#!/bin/sh

mkdir -p vm
cd vm

CC="../lcc -I. -DQ3_VM -DSGAME -D_NOMAD_VERSION=1 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=0 -S -Wf-target=bytecode -Wf-g -I../../game -I../../engine -I../../allocator"

$CC ../sg_main.c
$CC ../sg_level.c
$CC ../../engine/n_shared.c
$CC ../../engine/n_math.c
$CC ../sg_mem.c
$CC ../sg_imgui.c
$CC ../sg_lib.c

../q3asm -f ../sgame

cd ..

cd ../../nomadmain; ./bff-tool -w bff0.bff entries.json; cd ../code/sgame
