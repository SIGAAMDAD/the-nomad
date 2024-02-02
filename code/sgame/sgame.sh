#!/bin/sh

mkdir -p vm
cd vm

#if [ ! -f q3rcc.exe ]; then ln -s ../q3rcc.exe; fi
#if [ ! -f q3cpp.exe ]; then ln -s ../q3cpp.exe; fi

CC="../q3lcc -I. -DQ3_VM -DSGAME -D_NOMAD_VERSION=1 -D_NOMAD_VERSION_UPDATE=1 -D_NOMAD_VERSION_PATCH=0 -S -Wf-target=bytecode -Wf-g -I../../game -I../../engine -I../../allocator"

$CC ../sg_main.c
$CC ../sg_level.c
$CC ../sg_mem.c
$CC ../sg_entity.c
$CC ../sg_info.c
$CC ../sg_playr.c
$CC ../sg_items.c
$CC ../sg_mob.c
$CC ../sg_cmds.c
$CC ../sg_draw.c
$CC ../sg_imgui.c
$CC ../sg_lib.c
$CC ../sg_math.c
$CC ../sg_hud.c
$CC ../../engine/n_shared.c

../q3asm -f ../sgame

cd ..
