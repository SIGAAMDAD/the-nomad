#ifndef __SG_SPRITES__
#define __SG_SPRITES__

#pragma once

#define SPR_NULL                    0

#define SPR_PLAYR_IDLE_R            0
#define SPR_PLAYR_IDLE_L            1
#define SPR_PLAYR_PARRY0_R          2
#define SPR_PLAYR_PARRY0_L          3
#define SPR_PLAYR_PARRY1_R          4
#define SPR_PLAYR_PARRY1_L          5
#define SPR_PLAYR_PARRY2_HIT0_R     6
#define SPR_PLAYR_PARRY2_HIT0_L     7
#define SPR_PLAYR_PARRY3_HIT1_R     8
#define SPR_PLAYR_PARRY3_HIT1_L     9
#define SPR_PLAYR_PARRY4_HIT2_R     10
#define SPR_PLAYR_PARRY4_HIT2_L     11
#define SPR_PLAYR_PARRY5_HIT3_R     12
#define SPR_PLAYR_PARRY5_HIT3_L     13
#define SPR_PLAYR_PARRY6_HIT4_R     14
#define SPR_PLAYR_PARRY6_HIT4_L     15
#define SPR_PLAYR_MURSTAR_IDLE_R    16
#define SPR_PLAYR_MURSTAR_IDLE_L    17
#define SPR_PLAYR_MURSTAR_FIRE_R    18
#define SPR_PLAYR_MURSTAR_FIRE_L    19
#define SPR_PLAYR_MURSTAR_RLD0_R    20
#define SPR_PLAYR_MURSTAR_RLD0_L    21
#define SPR_PLAYR_MURSTAR_RLD1_R    22
#define SPR_PLAYR_MURSTAR_RLD1_L    23
#define SPR_PLAYR_PLASMA_IDLE_R     24
#define SPR_PLAYR_PLASMA_IDLE_L     25
#define SPR_PLAYR_PLASMA_FIRE_R     26
#define SPR_PLAYR_PLASMA_FIRE_L     27
#define SPR_PLAYR_DASH_R            28
#define SPR_PLAYR_DASH_L            29
#define SPR_PLAYR_MOVE0_R           30
#define SPR_PLAYR_MOVE0_L           31
#define SPR_PLAYR_MOVE1_R           32
#define SPR_PLAYR_MOVE1_L           33
#define SPR_PLAYR_MOVE2_R           34
#define SPR_PLAYR_MOVE2_L           35
#define SPR_PLAYR_MOVE3_R           36
#define SPR_PLAYR_MOVE3_L           37
#define SPR_PLAYR_DEAD              38
#define SPR_PLAYR_LEGS0_7_R         39
#define SPR_PLAYR_LEGS0_7_L         40
#define SPR_PLAYR_LEGS1_7_R         41
#define SPR_PLAYR_LEGS1_7_L         42
#define SPR_PLAYR_LEGS2_7_R         43
#define SPR_PLAYR_LEGS2_7_L         44
#define SPR_PLAYR_LEGS3_7_R         45
#define SPR_PLAYR_LEGS3_7_L         46
#define SPR_PLAYR_LEGS0_5_R         47
#define SPR_PLAYR_LEGS0_5_L         48
#define SPR_PLAYR_LEGS1_5_R         49
#define SPR_PLAYR_LEGS1_5_L         50
#define SPR_PLAYR_LEGS2_5_R         51
#define SPR_PLAYR_LEGS2_5_L         52
#define SPR_PLAYR_LEGS3_5_R         53
#define SPR_PLAYR_LEGS3_5_L         54
#define SPR_PLAYR_HB_IDLE_R         56
#define SPR_PLAYR_HB_IDLE_L         57
#define SPR_PLAYR_HB_STAB_R         58
#define SPR_PLAYR_HB_STAB_L         59
#define SPR_PLAYR_HB_SLASH_R        60
#define SPR_PLAYR_HB_SLASH_L        61
#define SPR_PLAYR_LEGS0_4_R         62
#define SPR_PLAYR_LEGS0_4_L         63
#define SPR_PLAYR_LEGS1_4_R         64
#define SPR_PLAYR_LEGS1_4_L         65
#define SPR_PLAYR_LEGS2_4_R         66
#define SPR_PLAYR_LEGS2_4_L         67
#define SPR_PLAYR_LEGS3_4_R         68
#define SPR_PLAYR_LEGS3_4_L         69
#define NUMPLAYRSPRITES             70

#define SPR_PLAYR_PARRY_R           SPR_PLAYR_PARRY0_R
#define SPR_PLAYR_PARRY_L           SPR_PLAYR_PARRY0_L
#define SPR_PLAYR_PARRY_HIT_R       SPR_PLAYR_PARRY2_HIT0_R
#define SPR_PLAYR_PARRY_HIT_L       SPR_PLAYR_PARRY2_HIT0_L
#define SPR_PLAYR_MOVE_R            SPR_PLAYR_MOVE0_R
#define SPR_PLAYR_MOVE_L            SPR_PLAYR_MOVE0_L
#define SPR_PLAYR_LEGS_7_R          SPR_PLAYR_LEGS0_7_R
#define SPR_PLAYR_LEGS_7_L          SPR_PLAYR_LEGS0_7_L

// legs: 7, 5, 4, 6


#define SPR_GRUNT_FH_IDLE_R         0
#define SPR_GRUNT_FH_IDLE_L         1
#define SPR_GRUNT_FH_MOVE_R         2
#define SPR_GRUNT_FH_MOVE_L         3
#define SPR_GRUNT_FH_ATK0_R         4
#define SPR_GRUNT_FH_ATK0_L         5
#define SPR_GRUNT_FH_ATK1_R         6
#define SPR_GRUNT_FH_ATK1_L         7
#define SPR_GRUNT_HH_IDLE_R         8
#define SPR_GRUNT_HH_IDLE_L         9
#define SPR_GRUNT_HH_MOVE_R         10
#define SPR_GRUNT_HH_MOVE_L         11
#define SPR_GRUNT_HH_ATK0_R         12
#define SPR_GRUNT_HH_ATK0_L         13
#define SPR_GRUNT_HH_ATK1_R         14
#define SPR_GRUNT_HH_ATK1_L         15
#define SPR_GRUNT_DEAD_R            16
#define SPR_GRUNT_DEAD_L            17
#define NUMGRUNTSPRITES             18

#define NUMSPRITES (NUMGRUNTSPRITES+NUMPLAYRSPRITES)

#endif