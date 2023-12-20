#!/bin/sh

mkdir -p vm
cd vm

CC="../lcc -I. -DQ3_VM -DSGAME -D_NOMAD_VERSION=1 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=0 -S -Wf-target=bytecode -Wf-g -I../../game -I../../engine -I../../allocator"

$CC ../sg_main.c
$CC ../sg_level.c
$CC ../sg_mem.c
$CC ../sg_entity.c
$CC ../sg_info.c
$CC ../sg_playr.c
$CC ../sg_mthink.c
$CC ../sg_archive.c
$CC ../sg_cmds.c
$CC ../sg_gameinfo.c
$CC ../sg_draw.c
$CC ../sg_imgui.c
$CC ../sg_lib.c
$CC ../../engine/n_math.c
$CC ../../engine/n_shared.c

../q3asm -f ../sgame

cd ..