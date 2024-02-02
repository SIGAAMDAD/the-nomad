export P_GiveWeapon
code
proc P_GiveWeapon 12 0
file "../sg_playr.c"
line 53
;1:#include "sg_local.h"
;2:
;3:#define LEGS_JUMP 0
;4:#define LEGS_JUMPB 1
;5:#define JUMP_VELOCITY 4.0f
;6:#define PLAYR_WIDTH 1.15
;7:#define MELEE_RANGE 5
;8:
;9:#define WEAPON_SLOT_MELEE   0
;10:#define WEAPON_SLOT_SHOTGUN 1
;11:#define WEAPON_SLOT_RIFLE   2
;12:#define WEAPON_SLOT_ARM     3
;13:
;14:// DO NOT CHANGE THESE, THESE VALUES ARE USED FOR CAMERA MOVEMENT!!!!!!!!!!!!!!!
;15:#define PMOVE_CLAMP_BORDER_HORZ		-0.2f
;16:#define PMOVE_CLAMP_BORDER_VERT		0.0f
;17:
;18:typedef enum {
;19:    kbMelee,
;20:    kbDash,
;21:    kbGrenade,
;22:    kbCrouch,
;23:    kbAltFire,
;24:} bindnum_t;
;25:
;26:#define PMF_WALKING         (unsigned)0x0001
;27:#define PMF_SLIDING         (unsigned)0x0002
;28:#define PMF_JUMP_HELD       (unsigned)0x0004
;29:#define PMF_BACKWARDS_JUMP  (unsigned)0x0008
;30:#define PMOVE_MAXSPEED 10
;31:
;32:typedef struct {
;33:	vec3_t vel;
;34:	vec3_t grapplePoint;
;35:	float forward, backward, right, left;
;36:	qboolean rightmove;
;37:	qboolean leftmove;
;38:	qboolean backwardmove;
;39:	qboolean forwardmove;
;40:	int waterlevel;
;41:	int velDir;
;42:	int velDirInverse;
;43:	int frametime;
;44:	int wallTime;
;45:	int flags;
;46:	dirtype_t movementDir;
;47:	qboolean wallHook;
;48:	qboolean groundPlane;
;49:	qboolean walking;
;50:} pmove_t;
;51:
;52:qboolean P_GiveWeapon( weapontype_t type )
;53:{
line 56
;54:    int slot;
;55:
;56:    switch (type) {
ADDRLP4 4
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 0
LTI4 $92
ADDRLP4 4
INDIRI4
CNSTI4 21
GTI4 $92
ADDRLP4 4
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $98
ADDP4
INDIRP4
JUMPV
data
align 4
LABELV $98
address $95
address $95
address $95
address $92
address $92
address $92
address $92
address $92
address $92
address $92
address $96
address $96
address $96
address $96
address $96
address $97
address $97
address $97
address $97
address $97
address $97
address $97
code
LABELV $95
line 60
;57:    case WT_SHOTTY_3B_PUMP:
;58:    case WT_SHOTTY_AUTO:
;59:    case WT_SHOTTY_DB:
;60:        slot = WEAPON_SLOT_SHOTGUN;
ADDRLP4 0
CNSTI4 1
ASGNI4
line 61
;61:        break;
ADDRGP4 $93
JUMPV
LABELV $96
line 67
;62:    case WT_MELEE_BO:
;63:    case WT_MELEE_BS:
;64:    case WT_MELEE_FISTS:
;65:    case WT_MELEE_KANTANA:
;66:    case WT_MELEE_STAR:
;67:        slot = WEAPON_SLOT_MELEE;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 68
;68:        break;
ADDRGP4 $93
JUMPV
LABELV $97
line 76
;69:    case WT_ARM_BLADE:
;70:    case WT_ARM_FT:
;71:    case WT_ARM_GRAPPLE:
;72:    case WT_ARM_HANDCANNON:
;73:    case WT_ARM_MP:
;74:    case WT_ARM_SD:
;75:    case WT_ARM_TT:
;76:        slot = WEAPON_SLOT_ARM;
ADDRLP4 0
CNSTI4 3
ASGNI4
line 77
;77:        break;
LABELV $92
LABELV $93
line 78
;78:    };
line 80
;79:
;80:    return qtrue;
CNSTI4 1
RETI4
LABELV $91
endproc P_GiveWeapon 12 0
proc P_SetLegsAnim 0 0
line 87
;81:}
;82:
;83:static int key_dash;
;84:static int key_melee;
;85:
;86:static void P_SetLegsAnim( int anim )
;87:{
line 88
;88:	sg.playr.foot_frame = anim;
ADDRGP4 sg+61620+48
ADDRFP4 0
INDIRI4
ASGNI4
line 89
;89:}
LABELV $99
endproc P_SetLegsAnim 0 0
proc PM_Friction 56 4
line 92
;90:
;91:static void PM_Friction( pmove_t *pm )
;92:{
line 96
;93:	float speed;
;94:	vec3_t v;
;95:
;96:	VectorCopy( v, pm->vel );
ADDRLP4 16
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 16
INDIRP4
INDIRF4
ASGNF4
ADDRLP4 0+4
ADDRLP4 16
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ASGNF4
ADDRLP4 0+8
ADDRFP4 0
INDIRP4
CNSTI4 8
ADDP4
INDIRF4
ASGNF4
line 98
;97:
;98:	speed = VectorLength( &v );
ADDRLP4 0
ARGP4
ADDRLP4 20
ADDRGP4 VectorLength
CALLF4
ASGNF4
ADDRLP4 12
ADDRLP4 20
INDIRF4
ASGNF4
line 99
;99:	if ( speed <= 0.0f ) {
ADDRLP4 12
INDIRF4
CNSTF4 0
GTF4 $105
line 100
;100:		return;
ADDRGP4 $102
JUMPV
LABELV $105
line 103
;101:	}
;102:
;103:	if ( pm->vel.x < 0 ) {
ADDRFP4 0
INDIRP4
INDIRF4
CNSTF4 0
GEF4 $107
line 104
;104:		pm->vel.x = MIN( 0, pm->vel.x + pm_groundFriction.f );
ADDRLP4 28
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 28
INDIRP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
ADDF4
GEF4 $112
ADDRLP4 24
CNSTF4 0
ASGNF4
ADDRGP4 $113
JUMPV
LABELV $112
ADDRLP4 24
ADDRFP4 0
INDIRP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
ADDF4
ASGNF4
LABELV $113
ADDRLP4 28
INDIRP4
ADDRLP4 24
INDIRF4
ASGNF4
line 105
;105:	} else if ( pm->vel.x > 0 ) {
ADDRGP4 $108
JUMPV
LABELV $107
ADDRFP4 0
INDIRP4
INDIRF4
CNSTF4 0
LEF4 $114
line 106
;106:		pm->vel.x = MAX( 0, pm->vel.x - pm_groundFriction.f );
ADDRLP4 28
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 28
INDIRP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
LEF4 $119
ADDRLP4 24
CNSTF4 0
ASGNF4
ADDRGP4 $120
JUMPV
LABELV $119
ADDRLP4 24
ADDRFP4 0
INDIRP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
ASGNF4
LABELV $120
ADDRLP4 28
INDIRP4
ADDRLP4 24
INDIRF4
ASGNF4
line 107
;107:	}
LABELV $114
LABELV $108
line 109
;108:
;109:	if ( pm->vel.y < 0 ) {
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
CNSTF4 0
GEF4 $121
line 110
;110:		pm->vel.y = MIN( 0, pm->vel.y + pm_groundFriction.f );
ADDRLP4 28
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 28
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
ADDF4
GEF4 $126
ADDRLP4 24
CNSTF4 0
ASGNF4
ADDRGP4 $127
JUMPV
LABELV $126
ADDRLP4 24
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
ADDF4
ASGNF4
LABELV $127
ADDRLP4 28
INDIRP4
CNSTI4 4
ADDP4
ADDRLP4 24
INDIRF4
ASGNF4
line 111
;111:	} else if ( pm->vel.x > 0 ) {
ADDRGP4 $122
JUMPV
LABELV $121
ADDRFP4 0
INDIRP4
INDIRF4
CNSTF4 0
LEF4 $128
line 112
;112:		pm->vel.y = MAX( 0, pm->vel.y - pm_groundFriction.f );
ADDRLP4 28
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 28
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
LEF4 $133
ADDRLP4 24
CNSTF4 0
ASGNF4
ADDRGP4 $134
JUMPV
LABELV $133
ADDRLP4 24
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
ASGNF4
LABELV $134
ADDRLP4 28
INDIRP4
CNSTI4 4
ADDP4
ADDRLP4 24
INDIRF4
ASGNF4
line 113
;113:	}
LABELV $128
LABELV $122
line 115
;114:
;115:	pm->forward = MAX( 0, pm->forward - pm_groundFriction.f );
ADDRLP4 28
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 28
INDIRP4
CNSTI4 24
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
LEF4 $138
ADDRLP4 24
CNSTF4 0
ASGNF4
ADDRGP4 $139
JUMPV
LABELV $138
ADDRLP4 24
ADDRFP4 0
INDIRP4
CNSTI4 24
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
ASGNF4
LABELV $139
ADDRLP4 28
INDIRP4
CNSTI4 24
ADDP4
ADDRLP4 24
INDIRF4
ASGNF4
line 116
;116:	pm->backward = MAX( 0, pm->backward - pm_groundFriction.f );
ADDRLP4 36
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 36
INDIRP4
CNSTI4 28
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
LEF4 $143
ADDRLP4 32
CNSTF4 0
ASGNF4
ADDRGP4 $144
JUMPV
LABELV $143
ADDRLP4 32
ADDRFP4 0
INDIRP4
CNSTI4 28
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
ASGNF4
LABELV $144
ADDRLP4 36
INDIRP4
CNSTI4 28
ADDP4
ADDRLP4 32
INDIRF4
ASGNF4
line 117
;117:	pm->left = MAX( 0, pm->left - pm_groundFriction.f );
ADDRLP4 44
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 44
INDIRP4
CNSTI4 36
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
LEF4 $148
ADDRLP4 40
CNSTF4 0
ASGNF4
ADDRGP4 $149
JUMPV
LABELV $148
ADDRLP4 40
ADDRFP4 0
INDIRP4
CNSTI4 36
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
ASGNF4
LABELV $149
ADDRLP4 44
INDIRP4
CNSTI4 36
ADDP4
ADDRLP4 40
INDIRF4
ASGNF4
line 118
;118:	pm->right = MAX( 0, pm->right - pm_groundFriction.f );
ADDRLP4 52
ADDRFP4 0
INDIRP4
ASGNP4
CNSTF4 0
ADDRLP4 52
INDIRP4
CNSTI4 32
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
LEF4 $153
ADDRLP4 48
CNSTF4 0
ASGNF4
ADDRGP4 $154
JUMPV
LABELV $153
ADDRLP4 48
ADDRFP4 0
INDIRP4
CNSTI4 32
ADDP4
INDIRF4
ADDRGP4 pm_groundFriction+256
INDIRF4
SUBF4
ASGNF4
LABELV $154
ADDRLP4 52
INDIRP4
CNSTI4 32
ADDP4
ADDRLP4 48
INDIRF4
ASGNF4
line 119
;119:}
LABELV $102
endproc PM_Friction 56 4
proc PM_Accelerate 32 0
line 122
;120:
;121:static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel, pmove_t *pm )
;122:{
line 125
;123:	float addspeed, accelspeed, speed;
;124:	
;125:	speed = DotProduct( pm->vel, wishdir );
ADDRLP4 12
ADDRFP4 12
INDIRP4
ASGNP4
ADDRLP4 16
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 8
ADDRLP4 12
INDIRP4
INDIRF4
ADDRLP4 16
INDIRP4
INDIRF4
MULF4
ADDRLP4 12
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ADDRLP4 16
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
MULF4
ADDF4
ADDRLP4 12
INDIRP4
CNSTI4 8
ADDP4
INDIRF4
ADDRLP4 16
INDIRP4
CNSTI4 8
ADDP4
INDIRF4
MULF4
ADDF4
ASGNF4
line 126
;126:	addspeed = wishspeed - speed;
ADDRLP4 4
ADDRFP4 4
INDIRF4
ADDRLP4 8
INDIRF4
SUBF4
ASGNF4
line 127
;127:	if ( addspeed <= 0 ) {
ADDRLP4 4
INDIRF4
CNSTF4 0
GTF4 $156
line 128
;128:		return;
ADDRGP4 $155
JUMPV
LABELV $156
line 130
;129:	}
;130:	accelspeed = accel * pm->frametime * wishspeed;
ADDRLP4 0
ADDRFP4 8
INDIRF4
ADDRFP4 12
INDIRP4
CNSTI4 68
ADDP4
INDIRI4
CVIF4 4
MULF4
ADDRFP4 4
INDIRF4
MULF4
ASGNF4
line 131
;131:	if ( accelspeed > addspeed ) {
ADDRLP4 0
INDIRF4
ADDRLP4 4
INDIRF4
LEF4 $158
line 132
;132:		accelspeed = addspeed;
ADDRLP4 0
ADDRLP4 4
INDIRF4
ASGNF4
line 133
;133:	}
LABELV $158
line 135
;134:
;135:	pm->vel.x += accelspeed * wishdir.x;
ADDRLP4 20
ADDRFP4 12
INDIRP4
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRF4
ADDRLP4 0
INDIRF4
ADDRFP4 0
INDIRP4
INDIRF4
MULF4
ADDF4
ASGNF4
line 136
;136:	pm->vel.y += accelspeed * wishdir.y;
ADDRLP4 24
ADDRFP4 12
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
ADDRLP4 24
INDIRP4
ADDRLP4 24
INDIRP4
INDIRF4
ADDRLP4 0
INDIRF4
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
MULF4
ADDF4
ASGNF4
line 137
;137:	pm->vel.z += accelspeed * wishdir.z;
ADDRLP4 28
ADDRFP4 12
INDIRP4
CNSTI4 8
ADDP4
ASGNP4
ADDRLP4 28
INDIRP4
ADDRLP4 28
INDIRP4
INDIRF4
ADDRLP4 0
INDIRF4
ADDRFP4 0
INDIRP4
CNSTI4 8
ADDP4
INDIRF4
MULF4
ADDF4
ASGNF4
line 138
;138:}
LABELV $155
endproc PM_Accelerate 32 0
proc PM_ClipVelocity 32 0
line 141
;139:
;140:static void PM_ClipVelocity( vec3_t vel, float *forward, float *left, float *right, float *backward )
;141:{
line 142
;142:	if ( vel.x > PMOVE_MAXSPEED ) {
ADDRFP4 0
INDIRP4
INDIRF4
CNSTF4 1092616192
LEF4 $161
line 143
;143:		vel.x = PMOVE_MAXSPEED;
ADDRFP4 0
INDIRP4
CNSTF4 1092616192
ASGNF4
line 144
;144:	} else if ( vel.x < -PMOVE_MAXSPEED ) {
ADDRGP4 $162
JUMPV
LABELV $161
ADDRFP4 0
INDIRP4
INDIRF4
CNSTF4 3240099840
GEF4 $163
line 145
;145:		vel.x = -PMOVE_MAXSPEED;
ADDRFP4 0
INDIRP4
CNSTF4 3240099840
ASGNF4
line 146
;146:	}
LABELV $163
LABELV $162
line 148
;147:
;148:	if ( vel.y > PMOVE_MAXSPEED ) {
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
CNSTF4 1092616192
LEF4 $165
line 149
;149:		vel.y = PMOVE_MAXSPEED;
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
CNSTF4 1092616192
ASGNF4
line 150
;150:	} else if ( vel.y < -PMOVE_MAXSPEED ) {
ADDRGP4 $166
JUMPV
LABELV $165
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
CNSTF4 3240099840
GEF4 $167
line 151
;151:		vel.y = -PMOVE_MAXSPEED;
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
CNSTF4 3240099840
ASGNF4
line 152
;152:	}
LABELV $167
LABELV $166
line 154
;153:
;154:	if ( vel.z > 10 ) {
ADDRFP4 0
INDIRP4
CNSTI4 8
ADDP4
INDIRF4
CNSTF4 1092616192
LEF4 $169
line 155
;155:		vel.z = 10;
ADDRFP4 0
INDIRP4
CNSTI4 8
ADDP4
CNSTF4 1092616192
ASGNF4
line 156
;156:	}
LABELV $169
line 158
;157:
;158:	*forward = MIN( *forward, PMOVE_MAXSPEED );
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
ADDRLP4 4
INDIRP4
INDIRF4
CNSTF4 1092616192
GEF4 $172
ADDRLP4 0
ADDRFP4 4
INDIRP4
INDIRF4
ASGNF4
ADDRGP4 $173
JUMPV
LABELV $172
ADDRLP4 0
CNSTF4 1092616192
ASGNF4
LABELV $173
ADDRLP4 4
INDIRP4
ADDRLP4 0
INDIRF4
ASGNF4
line 159
;159:	*backward = MIN( *backward, PMOVE_MAXSPEED );
ADDRLP4 12
ADDRFP4 16
INDIRP4
ASGNP4
ADDRLP4 12
INDIRP4
INDIRF4
CNSTF4 1092616192
GEF4 $175
ADDRLP4 8
ADDRFP4 16
INDIRP4
INDIRF4
ASGNF4
ADDRGP4 $176
JUMPV
LABELV $175
ADDRLP4 8
CNSTF4 1092616192
ASGNF4
LABELV $176
ADDRLP4 12
INDIRP4
ADDRLP4 8
INDIRF4
ASGNF4
line 160
;160:	*left = MIN( *left, PMOVE_MAXSPEED );
ADDRLP4 20
ADDRFP4 8
INDIRP4
ASGNP4
ADDRLP4 20
INDIRP4
INDIRF4
CNSTF4 1092616192
GEF4 $178
ADDRLP4 16
ADDRFP4 8
INDIRP4
INDIRF4
ASGNF4
ADDRGP4 $179
JUMPV
LABELV $178
ADDRLP4 16
CNSTF4 1092616192
ASGNF4
LABELV $179
ADDRLP4 20
INDIRP4
ADDRLP4 16
INDIRF4
ASGNF4
line 161
;161:	*right = MIN( *right, PMOVE_MAXSPEED );
ADDRLP4 28
ADDRFP4 12
INDIRP4
ASGNP4
ADDRLP4 28
INDIRP4
INDIRF4
CNSTF4 1092616192
GEF4 $181
ADDRLP4 24
ADDRFP4 12
INDIRP4
INDIRF4
ASGNF4
ADDRGP4 $182
JUMPV
LABELV $181
ADDRLP4 24
CNSTF4 1092616192
ASGNF4
LABELV $182
ADDRLP4 28
INDIRP4
ADDRLP4 24
INDIRF4
ASGNF4
line 162
;162:}
LABELV $160
endproc PM_ClipVelocity 32 0
export P_MeleeThink
proc P_MeleeThink 36 16
line 165
;163:
;164:void P_MeleeThink( sgentity_t *self )
;165:{
line 169
;166:	bbox_t bounds;
;167:	sgentity_t *ent;
;168:	
;169:	SG_BuildBounds( &bounds, PLAYR_WIDTH, MELEE_RANGE, &ent->origin );
ADDRLP4 4
ARGP4
CNSTF4 1066611507
ARGF4
CNSTF4 1084227584
ARGF4
ADDRLP4 0
INDIRP4
CNSTI4 64
ADDP4
ARGP4
ADDRGP4 SG_BuildBounds
CALLV
pop
line 171
;170:	
;171:	ent = Ent_CheckEntityCollision( self );
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 28
ADDRGP4 Ent_CheckEntityCollision
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 28
INDIRP4
ASGNP4
line 172
;172:	if ( !ent ) {
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $184
line 173
;173:		return;
ADDRGP4 $183
JUMPV
LABELV $184
line 180
;174:	}
;175:	
;176:	//
;177:	// check for parry
;178:	//
;179:	
;180:	if ( ent->flags & EF_PARRY ) {
ADDRLP4 0
INDIRP4
CNSTI4 148
ADDP4
INDIRI4
CNSTI4 2
BANDI4
CNSTI4 0
EQI4 $186
line 182
;181:		// its a projectile
;182:		trap_Snd_PlaySfx( sg.media.player_parry );
ADDRGP4 sg+24
INDIRI4
ARGI4
ADDRGP4 trap_Snd_PlaySfx
CALLV
pop
line 183
;183:		Ent_SetState( sg.playr.ent, S_PLAYR_PARRY );
ADDRGP4 sg+61620
INDIRP4
ARGP4
CNSTI4 5
ARGI4
ADDRGP4 Ent_SetState
CALLI4
pop
line 184
;184:	} else if ( ent->flags & EF_FIGHTING && ent->flags & EF_PARRY ) {
ADDRGP4 $187
JUMPV
LABELV $186
ADDRLP4 0
INDIRP4
CNSTI4 148
ADDP4
INDIRI4
CNSTI4 32
BANDI4
CNSTI4 0
EQI4 $190
ADDRLP4 0
INDIRP4
CNSTI4 148
ADDP4
INDIRI4
CNSTI4 2
BANDI4
CNSTI4 0
EQI4 $190
line 186
;185:		// its an attack
;186:		trap_Snd_PlaySfx( sg.media.player_parry );
ADDRGP4 sg+24
INDIRI4
ARGI4
ADDRGP4 trap_Snd_PlaySfx
CALLV
pop
line 187
;187:		Ent_SetState( sg.playr.ent, S_PLAYR_PARRY );
ADDRGP4 sg+61620
INDIRP4
ARGP4
CNSTI4 5
ARGI4
ADDRGP4 Ent_SetState
CALLI4
pop
line 188
;188:	}
LABELV $190
LABELV $187
line 189
;189:}
LABELV $183
endproc P_MeleeThink 36 16
proc P_ClipOrigin 36 8
line 230
;190:
;191:/*
;192:static void P_SetMovementDir( pmove_t *pm )
;193:{
;194:	if ( pm->rightmove > pm->forwardmove ) {
;195:		pm->velDir = 0;
;196:		pm->velDirInverse = 1;
;197:	} else if ( pm->rightmove < pm->forwardmove ) {
;198:		pm->velDir = 1;
;199:		pm->velDirInverse = 0;
;200:	} else {
;201:		pm->velDir = 0;
;202:	}
;203:	
;204:	if ( pm->rightmove < 0 && pm->forwardmove > 0 ) {
;205:		pm->movementDir = DIR_NORTH_WEST;
;206:	} else if ( pm->rightmove == 0 && pm->forwardmove > 0 ) {
;207:		pm->movementDir = DIR_NORTH;
;208:	} else if ( pm->rightmove > 0 && pm->forwardmove > 0 ) {
;209:		pm->movementDir = DIR_NORTH_EAST;
;210:	} else if ( pm->rightmove > 0 && pm->forwardmove == 0 ) {
;211:		pm->movementDir = DIR_EAST;
;212:	} else if ( pm->rightmove > 0 && pm->forwardmove < 0 ) {
;213:		pm->movementDir = DIR_SOUTH_EAST;
;214:	} else if ( pm->rightmove == 0 && pm->forwardmove < 0 ) {
;215:		pm->movementDir = DIR_SOUTH;
;216:	} else if ( pm->rightmove < 0 && pm->forwardmove < 0 ) {
;217:		pm->movementDir = DIR_SOUTH_WEST;
;218:	} else if ( pm->rightmove < 0 && pm->forwardmove == 0 ) {
;219:		pm->movementDir = DIR_WEST;
;220:	}
;221:}
;222:*/
;223:
;224:static pmove_t pm;
;225:
;226:/*
;227:* P_ClipOrigin: returns qtrue if the player's origin was clipped
;228:*/
;229:static qboolean P_ClipOrigin( sgentity_t *self )
;230:{
line 234
;231:	vec3_t origin;
;232:	sgentity_t *ent;
;233:
;234:	VectorCopy( origin, self->origin );
ADDRLP4 16
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 16
INDIRP4
CNSTI4 64
ADDP4
INDIRF4
ASGNF4
ADDRLP4 0+4
ADDRLP4 16
INDIRP4
CNSTI4 68
ADDP4
INDIRF4
ASGNF4
ADDRLP4 0+8
ADDRFP4 0
INDIRP4
CNSTI4 72
ADDP4
INDIRF4
ASGNF4
line 236
;235:
;236:	if ( origin.x > sg.mapInfo.width - 1 ) {
ADDRLP4 0
INDIRF4
ADDRGP4 sg+4268392+23616
INDIRI4
CNSTI4 1
SUBI4
CVIF4 4
LEF4 $197
line 237
;237:		origin.x = sg.mapInfo.width - 1;
ADDRLP4 0
ADDRGP4 sg+4268392+23616
INDIRI4
CNSTI4 1
SUBI4
CVIF4 4
ASGNF4
line 238
;238:	} else if ( origin.x < PMOVE_CLAMP_BORDER_HORZ ) {
ADDRGP4 $198
JUMPV
LABELV $197
ADDRLP4 0
INDIRF4
CNSTF4 3192704205
GEF4 $203
line 239
;239:		origin.x = PMOVE_CLAMP_BORDER_HORZ;
ADDRLP4 0
CNSTF4 3192704205
ASGNF4
line 240
;240:	}
LABELV $203
LABELV $198
line 242
;241:
;242:	if ( origin.y > sg.mapInfo.height - 1 ) {
ADDRLP4 0+4
INDIRF4
ADDRGP4 sg+4268392+23620
INDIRI4
CNSTI4 1
SUBI4
CVIF4 4
LEF4 $205
line 243
;243:		origin.y = sg.mapInfo.height - 1;
ADDRLP4 0+4
ADDRGP4 sg+4268392+23620
INDIRI4
CNSTI4 1
SUBI4
CVIF4 4
ASGNF4
line 244
;244:	} else if ( origin.y < PMOVE_CLAMP_BORDER_VERT ) {
ADDRGP4 $206
JUMPV
LABELV $205
ADDRLP4 0+4
INDIRF4
CNSTF4 0
GEF4 $213
line 245
;245:		origin.y = PMOVE_CLAMP_BORDER_VERT;
ADDRLP4 0+4
CNSTF4 0
ASGNF4
line 246
;246:	}
LABELV $213
LABELV $206
line 248
;247:
;248:	if ( origin.z > 10 ) {
ADDRLP4 0+8
INDIRF4
CNSTF4 1092616192
LEF4 $217
line 249
;249:		origin.z = 10;
ADDRLP4 0+8
CNSTF4 1092616192
ASGNF4
line 250
;250:	}
LABELV $217
line 252
;251:
;252:	if ( !VectorCompare( &self->origin, &origin ) ) { // clip it at map boundaries
ADDRFP4 0
INDIRP4
CNSTI4 64
ADDP4
ARGP4
ADDRLP4 0
ARGP4
ADDRLP4 20
ADDRGP4 VectorCompare
CALLI4
ASGNI4
ADDRLP4 20
INDIRI4
CNSTI4 0
NEI4 $221
line 253
;253:		VectorCopy( self->origin, origin );
ADDRFP4 0
INDIRP4
CNSTI4 64
ADDP4
ADDRLP4 0
INDIRF4
ASGNF4
ADDRFP4 0
INDIRP4
CNSTI4 68
ADDP4
ADDRLP4 0+4
INDIRF4
ASGNF4
ADDRFP4 0
INDIRP4
CNSTI4 72
ADDP4
ADDRLP4 0+8
INDIRF4
ASGNF4
line 254
;254:		VectorClear( pm.vel );
ADDRLP4 24
CNSTF4 0
ASGNF4
ADDRGP4 pm+8
ADDRLP4 24
INDIRF4
ASGNF4
ADDRGP4 pm+4
ADDRLP4 24
INDIRF4
ASGNF4
ADDRGP4 pm
ADDRLP4 24
INDIRF4
ASGNF4
line 255
;255:		return qtrue;
CNSTI4 1
RETI4
ADDRGP4 $194
JUMPV
LABELV $221
line 256
;256:	} else if ( Ent_CheckWallCollision( self ) || Ent_CheckEntityCollision( self ) ) { // hit a solid entity
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 24
ADDRGP4 Ent_CheckWallCollision
CALLI4
ASGNI4
ADDRLP4 24
INDIRI4
CNSTI4 0
NEI4 $229
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 28
ADDRGP4 Ent_CheckEntityCollision
CALLP4
ASGNP4
ADDRLP4 28
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $227
LABELV $229
line 257
;257:		VectorCopy( self->origin, origin );
ADDRFP4 0
INDIRP4
CNSTI4 64
ADDP4
ADDRLP4 0
INDIRF4
ASGNF4
ADDRFP4 0
INDIRP4
CNSTI4 68
ADDP4
ADDRLP4 0+4
INDIRF4
ASGNF4
ADDRFP4 0
INDIRP4
CNSTI4 72
ADDP4
ADDRLP4 0+8
INDIRF4
ASGNF4
line 258
;258:		VectorClear( pm.vel );
ADDRLP4 32
CNSTF4 0
ASGNF4
ADDRGP4 pm+8
ADDRLP4 32
INDIRF4
ASGNF4
ADDRGP4 pm+4
ADDRLP4 32
INDIRF4
ASGNF4
ADDRGP4 pm
ADDRLP4 32
INDIRF4
ASGNF4
line 259
;259:		return qtrue;
CNSTI4 1
RETI4
ADDRGP4 $194
JUMPV
LABELV $227
line 262
;260:	}
;261:
;262:	return qfalse;
CNSTI4 0
RETI4
LABELV $194
endproc P_ClipOrigin 36 8
proc Pmove 68 20
line 266
;263:}
;264:
;265:static void Pmove( sgentity_t *self )
;266:{
line 270
;267:	int i;
;268:	float tmp;
;269:
;270:	if ( pm.left > pm.right ) {
ADDRGP4 pm+36
INDIRF4
ADDRGP4 pm+32
INDIRF4
LEF4 $235
line 271
;271:		self->facing = 1;
ADDRFP4 0
INDIRP4
CNSTI4 124
ADDP4
CNSTI4 1
ASGNI4
line 272
;272:	} else if ( pm.right > pm.left ) {
ADDRGP4 $236
JUMPV
LABELV $235
ADDRGP4 pm+32
INDIRF4
ADDRGP4 pm+36
INDIRF4
LEF4 $239
line 273
;273:		self->facing = 0;
ADDRFP4 0
INDIRP4
CNSTI4 124
ADDP4
CNSTI4 0
ASGNI4
line 274
;274:	}
LABELV $239
LABELV $236
line 276
;275:
;276:	pm.vel.x += pm.right;
ADDRLP4 8
ADDRGP4 pm
ASGNP4
ADDRLP4 8
INDIRP4
ADDRLP4 8
INDIRP4
INDIRF4
ADDRGP4 pm+32
INDIRF4
ADDF4
ASGNF4
line 277
;277:	pm.vel.x -= pm.left;
ADDRLP4 12
ADDRGP4 pm
ASGNP4
ADDRLP4 12
INDIRP4
ADDRLP4 12
INDIRP4
INDIRF4
ADDRGP4 pm+36
INDIRF4
SUBF4
ASGNF4
line 279
;278:
;279:	pm.vel.y -= pm.forward;
ADDRLP4 16
ADDRGP4 pm+4
ASGNP4
ADDRLP4 16
INDIRP4
ADDRLP4 16
INDIRP4
INDIRF4
ADDRGP4 pm+24
INDIRF4
SUBF4
ASGNF4
line 280
;280:	pm.vel.y += pm.backward;
ADDRLP4 20
ADDRGP4 pm+4
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRF4
ADDRGP4 pm+28
INDIRF4
ADDF4
ASGNF4
line 282
;281:
;282:	pm.backwardmove = pm.backward == 0;
ADDRGP4 pm+28
INDIRF4
CNSTF4 0
NEF4 $252
ADDRLP4 24
CNSTI4 1
ASGNI4
ADDRGP4 $253
JUMPV
LABELV $252
ADDRLP4 24
CNSTI4 0
ASGNI4
LABELV $253
ADDRGP4 pm+48
ADDRLP4 24
INDIRI4
ASGNI4
line 283
;283:	pm.forwardmove = pm.forward == 0;
ADDRGP4 pm+24
INDIRF4
CNSTF4 0
NEF4 $257
ADDRLP4 28
CNSTI4 1
ASGNI4
ADDRGP4 $258
JUMPV
LABELV $257
ADDRLP4 28
CNSTI4 0
ASGNI4
LABELV $258
ADDRGP4 pm+52
ADDRLP4 28
INDIRI4
ASGNI4
line 284
;284:	pm.leftmove = pm.left == 0;
ADDRGP4 pm+36
INDIRF4
CNSTF4 0
NEF4 $262
ADDRLP4 32
CNSTI4 1
ASGNI4
ADDRGP4 $263
JUMPV
LABELV $262
ADDRLP4 32
CNSTI4 0
ASGNI4
LABELV $263
ADDRGP4 pm+44
ADDRLP4 32
INDIRI4
ASGNI4
line 285
;285:	pm.rightmove = pm.right == 0;
ADDRGP4 pm+32
INDIRF4
CNSTF4 0
NEF4 $267
ADDRLP4 36
CNSTI4 1
ASGNI4
ADDRGP4 $268
JUMPV
LABELV $267
ADDRLP4 36
CNSTI4 0
ASGNI4
LABELV $268
ADDRGP4 pm+40
ADDRLP4 36
INDIRI4
ASGNI4
line 287
;286:
;287:	PM_ClipVelocity( pm.vel, &pm.forward, &pm.left, &pm.right, &pm.backward );
ADDRLP4 40
ADDRGP4 pm
INDIRB
ASGNB 12
ADDRLP4 40
ARGP4
ADDRGP4 pm+24
ARGP4
ADDRGP4 pm+36
ARGP4
ADDRGP4 pm+32
ARGP4
ADDRGP4 pm+28
ARGP4
ADDRGP4 PM_ClipVelocity
CALLV
pop
line 288
;288:	PM_Friction( &pm );
ADDRGP4 pm
ARGP4
ADDRGP4 PM_Friction
CALLV
pop
line 290
;289:
;290:	self->origin.x += pm.vel.x;
ADDRLP4 52
ADDRFP4 0
INDIRP4
CNSTI4 64
ADDP4
ASGNP4
ADDRLP4 52
INDIRP4
ADDRLP4 52
INDIRP4
INDIRF4
ADDRGP4 pm
INDIRF4
ADDF4
ASGNF4
line 291
;291:	self->origin.y += pm.vel.y;
ADDRLP4 56
ADDRFP4 0
INDIRP4
CNSTI4 68
ADDP4
ASGNP4
ADDRLP4 56
INDIRP4
ADDRLP4 56
INDIRP4
INDIRF4
ADDRGP4 pm+4
INDIRF4
ADDF4
ASGNF4
line 293
;292:
;293:	if ( P_ClipOrigin( sg.playr.ent ) ) {
ADDRGP4 sg+61620
INDIRP4
ARGP4
ADDRLP4 60
ADDRGP4 P_ClipOrigin
CALLI4
ASGNI4
ADDRLP4 60
INDIRI4
CNSTI4 0
EQI4 $274
line 294
;294:		VectorClear( pm.vel );
ADDRLP4 64
CNSTF4 0
ASGNF4
ADDRGP4 pm+8
ADDRLP4 64
INDIRF4
ASGNF4
ADDRGP4 pm+4
ADDRLP4 64
INDIRF4
ASGNF4
ADDRGP4 pm
ADDRLP4 64
INDIRF4
ASGNF4
line 295
;295:	}
LABELV $274
line 297
;296:
;297:	sg.cameraPos.x = self->origin.x - ( sg.cameraPos.x / 2 );
ADDRGP4 sg+4268384
ADDRFP4 0
INDIRP4
CNSTI4 64
ADDP4
INDIRF4
ADDRGP4 sg+4268384
INDIRF4
CNSTF4 1056964608
MULF4
SUBF4
ASGNF4
line 298
;298:	sg.cameraPos.y = -self->origin.y;
ADDRGP4 sg+4268384+4
ADDRFP4 0
INDIRP4
CNSTI4 68
ADDP4
INDIRF4
NEGF4
ASGNF4
line 299
;299:}
LABELV $234
endproc Pmove 68 20
export P_Thinker
proc P_Thinker 12 12
line 302
;300:
;301:void P_Thinker( sgentity_t *self )
;302:{
line 306
;303:	int i;
;304:	float f;
;305:
;306:	Pmove( self );
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 Pmove
CALLV
pop
line 308
;307:
;308:	ImGui_BeginWindow( "Player Move Metrics", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize );
ADDRGP4 $284
ARGP4
CNSTP4 0
ARGP4
CNSTI4 66
ARGI4
ADDRGP4 ImGui_BeginWindow
CALLI4
pop
line 310
;309:
;310:	for ( i = 0; i < 3; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
LABELV $285
line 311
;311:		f = vec3_get( &self->origin, i );
ADDRFP4 0
INDIRP4
CNSTI4 64
ADDP4
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 8
ADDRGP4 vec3_get
CALLF4
ASGNF4
ADDRLP4 4
ADDRLP4 8
INDIRF4
ASGNF4
line 312
;312:		ImGui_Text( "self->origin[%i]: %f", i, f );
ADDRGP4 $289
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
INDIRF4
ARGF4
ADDRGP4 ImGui_Text
CALLV
pop
line 313
;313:	}
LABELV $286
line 310
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 3
LTI4 $285
line 315
;314:
;315:	ImGui_Text( "pm.forward: %f", pm.forward );
ADDRGP4 $290
ARGP4
ADDRGP4 pm+24
INDIRF4
ARGF4
ADDRGP4 ImGui_Text
CALLV
pop
line 316
;316:	ImGui_Text( "pm.backward: %f", pm.backward );
ADDRGP4 $292
ARGP4
ADDRGP4 pm+28
INDIRF4
ARGF4
ADDRGP4 ImGui_Text
CALLV
pop
line 317
;317:	ImGui_Text( "pm.left: %f", pm.left );
ADDRGP4 $294
ARGP4
ADDRGP4 pm+36
INDIRF4
ARGF4
ADDRGP4 ImGui_Text
CALLV
pop
line 318
;318:	ImGui_Text( "pm.right: %f", pm.right );
ADDRGP4 $296
ARGP4
ADDRGP4 pm+32
INDIRF4
ARGF4
ADDRGP4 ImGui_Text
CALLV
pop
line 320
;319:
;320:	ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 321
;321:}
LABELV $283
endproc P_Thinker 12 12
export SG_InitPlayer
proc SG_InitPlayer 0 12
line 324
;322:
;323:void SG_InitPlayer( void )
;324:{    
line 326
;325:    // initialize player state
;326:    memset( &sg.playr, 0, sizeof(sg.playr) );
ADDRGP4 sg+61620
ARGP4
CNSTI4 0
ARGI4
CNSTU4 56
ARGU4
ADDRGP4 memset
CALLP4
pop
line 328
;327:
;328:	sg.playr.foot_frame = 0;
ADDRGP4 sg+61620+48
CNSTI4 0
ASGNI4
line 329
;329:	sg.playr.foot_sprite = SPR_PLAYR_LEGS0_7_R;
ADDRGP4 sg+61620+44
CNSTI4 39
ASGNI4
line 332
;330:
;331:    // mark as allocated
;332:    sg.playrReady = qtrue;
ADDRGP4 sg+61676
CNSTI4 1
ASGNI4
line 333
;333:}
LABELV $298
endproc SG_InitPlayer 0 12
export SG_KeyEvent
proc SG_KeyEvent 24 0
line 336
;334:
;335:void SG_KeyEvent( int key, qboolean down )
;336:{
line 337
;337:	if ( down ) {
ADDRFP4 4
INDIRI4
CNSTI4 0
EQI4 $307
line 338
;338:		switch ( key ) {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 115
EQI4 $315
ADDRLP4 0
INDIRI4
CNSTI4 115
GTI4 $328
LABELV $327
ADDRLP4 4
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 97
EQI4 $319
ADDRLP4 4
INDIRI4
CNSTI4 100
EQI4 $323
ADDRGP4 $309
JUMPV
LABELV $328
ADDRFP4 0
INDIRI4
CNSTI4 119
EQI4 $311
ADDRGP4 $309
JUMPV
LABELV $311
line 340
;339:		case KEY_W:
;340:			pm.forward += pm_baseSpeed.f * pm_baseAccel.f;
ADDRLP4 8
ADDRGP4 pm+24
ASGNP4
ADDRLP4 8
INDIRP4
ADDRLP4 8
INDIRP4
INDIRF4
ADDRGP4 pm_baseSpeed+256
INDIRF4
ADDRGP4 pm_baseAccel+256
INDIRF4
MULF4
ADDF4
ASGNF4
line 341
;341:			break;
ADDRGP4 $310
JUMPV
LABELV $315
line 343
;342:		case KEY_S:
;343:			pm.backward += pm_baseSpeed.f * pm_baseAccel.f;
ADDRLP4 12
ADDRGP4 pm+28
ASGNP4
ADDRLP4 12
INDIRP4
ADDRLP4 12
INDIRP4
INDIRF4
ADDRGP4 pm_baseSpeed+256
INDIRF4
ADDRGP4 pm_baseAccel+256
INDIRF4
MULF4
ADDF4
ASGNF4
line 344
;344:			break;
ADDRGP4 $310
JUMPV
LABELV $319
line 346
;345:		case KEY_A:
;346:			pm.left += pm_baseSpeed.f * pm_baseAccel.f;
ADDRLP4 16
ADDRGP4 pm+36
ASGNP4
ADDRLP4 16
INDIRP4
ADDRLP4 16
INDIRP4
INDIRF4
ADDRGP4 pm_baseSpeed+256
INDIRF4
ADDRGP4 pm_baseAccel+256
INDIRF4
MULF4
ADDF4
ASGNF4
line 347
;347:			break;
ADDRGP4 $310
JUMPV
LABELV $323
line 349
;348:		case KEY_D:
;349:			pm.right += pm_baseSpeed.f * pm_baseAccel.f;
ADDRLP4 20
ADDRGP4 pm+32
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRF4
ADDRGP4 pm_baseSpeed+256
INDIRF4
ADDRGP4 pm_baseAccel+256
INDIRF4
MULF4
ADDF4
ASGNF4
line 350
;350:			break;
LABELV $309
LABELV $310
line 351
;351:		};
line 352
;352:	}
LABELV $307
line 353
;353:}
LABELV $306
endproc SG_KeyEvent 24 0
bss
align 4
LABELV pm
skip 96
align 4
LABELV key_melee
skip 4
align 4
LABELV key_dash
skip 4
import Cvar_VariableStringBuffer
import Cvar_Set
import Cvar_Update
import Cvar_Register
import trap_FS_Printf
import trap_FS_FileTell
import trap_FS_FileLength
import trap_FS_FileSeek
import trap_FS_GetFileList
import trap_FS_Read
import trap_FS_Write
import trap_FS_FClose
import trap_FS_FOpenRead
import trap_FS_FOpenWrite
import trap_FS_FOpenFile
import Sys_GetGPUConfig
import RE_AddSpriteToScene
import RE_AddPolyToScene
import RE_RenderScene
import RE_ClearScene
import RE_LoadWorldMap
import RE_RegisterSprite
import RE_RegisterSpriteSheet
import RE_RegisterShader
import trap_Snd_ClearLoopingTrack
import trap_Snd_SetLoopingTrack
import trap_Snd_StopSfx
import trap_Snd_PlaySfx
import trap_Snd_QueueTrack
import trap_Snd_RegisterTrack
import trap_Snd_RegisterSfx
import trap_Key_ClearStates
import trap_Key_GetKey
import trap_Key_GetCatcher
import trap_Key_SetCatcher
import trap_Milliseconds
import trap_CheckWallHit
import G_SoundRecursive
import G_CastRay
import G_SetActiveMap
import G_LoadMap
import G_SetCameraData
import trap_MemoryRemaining
import trap_RemoveCommand
import trap_AddCommand
import trap_SendConsoleCommand
import trap_LoadVec4
import trap_LoadVec3
import trap_LoadVec2
import trap_LoadString
import trap_LoadFloat
import trap_LoadInt
import trap_LoadUInt
import trap_GetSaveSection
import trap_WriteVec4
import trap_WriteVec3
import trap_WriteVec2
import trap_WriteFloat
import trap_WriteString
import trap_WriteUInt
import trap_WriteInt
import trap_WriteChar
import trap_EndSaveSection
import trap_BeginSaveSection
import trap_Args
import trap_Argv
import trap_Argc
import trap_Error
import trap_Print
import P_GiveItem
import P_ParryTicker
import P_MeleeTicker
import P_Ticker
import SG_SendUserCmd
import SG_MouseEvent
import SG_OutOfMemory
import SG_ClearToMemoryMark
import SG_MakeMemoryMark
import SG_MemInit
import SG_MemAlloc
import String_Alloc
import SG_SpawnMob
import SG_Spawn
import Ent_SetState
import SG_InitEntities
import Ent_BuildBounds
import SG_BuildBounds
import SG_FreeEntity
import SG_AllocEntity
import Ent_RunTic
import Ent_CheckEntityCollision
import Ent_CheckWallCollision
import SG_PickupWeapon
import SG_SpawnWeapon
import SG_SpawnItem
import SG_LoadLevelData
import SG_SaveLevelData
import SG_EndLevel
import SG_StartLevel
import SG_UpdateCvars
import G_Printf
import G_Error
import SG_Printf
import SG_Error
import SG_BuildMoveCommand
import SGameCommand
import SG_DrawFrame
import pm_wallTime
import pm_wallrunAccelMove
import pm_wallrunAccelVertical
import pm_airAccel
import pm_baseSpeed
import pm_baseAccel
import pm_waterAccel
import pm_airFriction
import pm_waterFriction
import pm_groundFriction
import sg_memoryDebug
import sg_numSaves
import sg_savename
import sg_levelIndex
import sg_gibs
import sg_decalDetail
import sg_printLevelStats
import sg_mouseAcceleration
import sg_mouseInvert
import sg_paused
import sg_debugPrint
import ammoCaps
import mobinfo
import iteminfo
import weaponinfo
import sg
import sg_entities
import inversedirs
import dirvectors
import stateinfo
import ImGui_CloseCurrentPopup
import ImGui_OpenPopup
import ImGui_EndPopup
import ImGui_BeginPopupModal
import ImGui_ColoredText
import ImGui_Text
import ImGui_ColoredTextUnformatted
import ImGui_TextUnformatted
import ImGui_SameLine
import ImGui_ProgressBar
import ImGui_Separator
import ImGui_SeparatorText
import ImGui_NewLine
import ImGui_PopColor
import ImGui_PushColor
import ImGui_GetCursorScreenPos
import ImGui_SetCursorScreenPos
import ImGui_GetCursorPos
import ImGui_SetCursorPos
import ImGui_GetFontScale
import ImGui_Button
import ImGui_Checkbox
import ImGui_ArrowButton
import ImGui_ColorEdit4
import ImGui_ColorEdit3
import ImGui_SliderInt4
import ImGui_SliderInt3
import ImGui_SliderInt2
import ImGui_SliderInt
import ImGui_SliderFloat4
import ImGui_SliderFloat3
import ImGui_SliderFloat2
import ImGui_SliderFloat
import ImGui_InputInt4
import ImGui_InputInt3
import ImGui_InputInt2
import ImGui_InputInt
import ImGui_InputFloat4
import ImGui_InputFloat3
import ImGui_InputFloat2
import ImGui_InputFloat
import ImGui_InputTextWithHint
import ImGui_InputTextMultiline
import ImGui_InputText
import ImGui_EndTable
import ImGui_TableNextColumn
import ImGui_TableNextRow
import ImGui_BeginTable
import ImGui_SetItemTooltip
import ImGui_SetItemTooltipUnformatted
import ImGui_MenuItem
import ImGui_EndMenu
import ImGui_BeginMenu
import ImGui_SetWindowFontScale
import ImGui_SetWindowSize
import ImGui_SetWindowPos
import ImGui_SetWindowCollapsed
import ImGui_IsWindowCollapsed
import ImGui_EndWindow
import ImGui_BeginWindow
import I_GetParm
import Com_TouchMemory
import Hunk_TempIsClear
import Hunk_Check
import Hunk_Print
import Hunk_SetMark
import Hunk_ClearToMark
import Hunk_CheckMark
import Hunk_SmallLog
import Hunk_Log
import Hunk_MemoryRemaining
import Hunk_ClearTempMemory
import Hunk_FreeTempMemory
import Hunk_AllocateTempMemory
import Hunk_Clear
import Hunk_Alloc
import Hunk_InitMemory
import Z_InitMemory
import Z_InitSmallZoneMemory
import CopyString
import Z_AvailableMemory
import Z_FreeTags
import Z_Free
import S_Malloc
import Z_Malloc
import Z_Realloc
import CPU_flags
import FS_ReadLine
import FS_ListFiles
import FS_FreeFileList
import FS_FreeFile
import FS_SetBFFIndex
import FS_GetCurrentChunkList
import FS_Initialized
import FS_FileIsInBFF
import FS_StripExt
import FS_AllowedExtension
import FS_GetFileList
import FS_LoadLibrary
import FS_CopyString
import FS_BuildOSPath
import FS_FilenameCompare
import FS_FileTell
import FS_FileLength
import FS_FileSeek
import FS_FileExists
import FS_LastBFFIndex
import FS_LoadStack
import FS_Rename
import FS_FOpenFileRead
import FS_FOpenAppend
import FS_FOpenRW
import FS_FOpenWrite
import FS_FOpenRead
import FS_FOpenFileWithMode
import FS_FOpenWithMode
import FS_FileToFileno
import FS_Printf
import FS_GetGamePath
import FS_GetHomePath
import FS_GetBasePath
import FS_GetBaseGameDir
import FS_GetCurrentGameDir
import FS_Flush
import FS_ForceFlush
import FS_FClose
import FS_LoadFile
import FS_WriteFile
import FS_Write
import FS_Read
import FS_Remove
import FS_Restart
import FS_Shutdown
import FS_InitFilesystem
import FS_Startup
import FS_VM_CloseFiles
import FS_VM_FileLength
import FS_VM_Read
import FS_VM_Write
import FS_VM_WriteFile
import FS_VM_FClose
import FS_VM_FOpenFileRead
import FS_VM_FOpenFileWrite
import FS_VM_FOpenFile
import FS_VM_FileTell
import FS_VM_FileSeek
import FS_VM_FOpenRW
import FS_VM_FOpenAppend
import FS_VM_FOpenWrite
import FS_VM_FOpenRead
import com_errorMessage
import com_fullyInitialized
import com_errorEntered
import com_cacheLine
import com_frameTime
import com_fps
import com_frameNumber
import com_maxfps
import sys_cpuString
import com_devmode
import com_version
import com_logfile
import com_journal
import com_demo
import Con_HistoryGetNext
import Con_HistoryGetPrev
import Con_SaveField
import Con_ResetHistory
import Field_CompleteCommand
import Field_CompleteFilename
import Field_CompleteKeyBind
import Field_CompleteKeyname
import Field_AutoComplete
import Field_Clear
import Cbuf_Init
import Cbuf_Clear
import Cbuf_AddText
import Cbuf_Execute
import Cbuf_InsertText
import Cbuf_ExecuteText
import va
import Cmd_CompleteArgument
import Cmd_CommandCompletion
import Cmd_Clear
import Cmd_Argv
import Cmd_ArgsFrom
import Cmd_SetCommandCompletionFunc
import Cmd_TokenizeStringIgnoreQuotes
import Cmd_TokenizeString
import Cmd_ArgvBuffer
import Cmd_Argc
import Cmd_ExecuteString
import Cmd_ExecuteText
import Cmd_ArgsBuffer
import Cmd_ExecuteCommand
import Cmd_RemoveCommand
import Cmd_AddCommand
import Cmd_Init
import keys
import Key_WriteBindings
import Key_SetOverstrikeMode
import Key_GetOverstrikeMode
import Key_GetKey
import Key_GetCatcher
import Key_SetCatcher
import Key_ClearStates
import Key_GetBinding
import Key_IsDown
import Key_KeynumToString
import Key_StringToKeynum
import Key_KeynameCompletion
import Com_EventLoop
import Com_KeyEvent
import Com_SendKeyEvents
import Com_QueueEvent
import Com_InitKeyCommands
import Parse3DMatrix
import Parse2DMatrix
import Parse1DMatrix
import ParseHex
import SkipRestOfLine
import SkipBracedSection
import com_tokentype
import COM_ParseComplex
import Com_BlockChecksum
import COM_ParseWarning
import COM_ParseError
import COM_Compress
import COM_ParseExt
import COM_Parse
import COM_GetCurrentParseLine
import COM_BeginParseSession
import COM_StripExtension
import COM_GetExtension
import Com_TruncateLongString
import Com_SortFileList
import Com_Base64Decode
import Com_HasPatterns
import Com_FilterPath
import Com_Filter
import Com_FilterExt
import Com_HexStrToInt
import COM_DefaultExtension
import Com_WriteConfig
import Con_RenderConsole
import Com_GenerateHashValue
import Com_Shutdown
import Com_Init
import Com_StartupVariable
import crc32_buffer
import Com_EarlyParseCmdLine
import Com_Milliseconds
import Com_Frame
import Sys_SnapVector
import Con_DPrintf
import Con_Printf
import Con_Shutdown
import Con_Init
import Con_DrawConsole
import Con_AddText
import ColorIndexFromChar
import g_color_table
import Info_RemoveKey
import Info_NextPair
import Info_ValidateKeyValue
import Info_Validate
import Info_SetValueForKey_s
import Info_ValueForKeyToken
import Info_Tokenize
import Info_ValueForKey
import Com_Clamp
import bytedirs
import N_isnan
import N_crandom
import N_random
import N_rand
import N_fabs
import N_acos
import N_log2
import ColorBytes4
import ColorBytes3
import VectorNormalize
import AddPointToBounds
import ClearBounds
import RadiusFromBounds
import NormalizeColor
import _VectorMA
import _VectorScale
import _VectorCopy
import _VectorAdd
import _VectorSubtract
import _DotProduct
import ByteToDir
import DirToByte
import CrossProduct
import VectorInverse
import VectorNormalizeFast
import DistanceSquared
import Distance
import VectorLengthSquared
import VectorLength
import VectorCompare
import BoundsIntersectPoint
import BoundsIntersectSphere
import BoundsIntersect
import disBetweenOBJ
import vec3_set
import vec3_get
import ClampShort
import ClampCharMove
import ClampChar
import N_exp2f
import N_log2f
import Q_rsqrt
import N_Error
import locase
import colorDkGrey
import colorMdGrey
import colorLtGrey
import colorWhite
import colorCyan
import colorMagenta
import colorYellow
import colorBlue
import colorGreen
import colorRed
import colorBlack
import vec2_origin
import vec3_origin
import COM_SkipPath
import Com_Split
import N_replace
import N_memcmp
import N_memchr
import N_memcpy
import N_memset
import N_strncpyz
import N_strncpy
import N_strcpy
import N_stradd
import N_strneq
import N_streq
import N_strlen
import N_atof
import N_atoi
import N_fmaxf
import N_stristr
import N_strcat
import N_strupr
import N_strlwr
import N_stricmpn
import N_stricmp
import N_strncmp
import N_strcmp
import N_isanumber
import N_isintegral
import N_isalpha
import N_isupper
import N_islower
import N_isprint
import Com_SkipCharset
import Com_SkipTokens
import Com_snprintf
import acos
import fabs
import abs
import tan
import atan2
import cos
import sin
import sqrt
import floor
import ceil
import sscanf
import vsprintf
import rand
import srand
import qsort
import toupper
import tolower
import strncmp
import strcmp
import strstr
import strchr
import strlen
import strcat
import strcpy
import memmove
import memset
import memchr
import memcpy
lit
align 1
LABELV $296
byte 1 112
byte 1 109
byte 1 46
byte 1 114
byte 1 105
byte 1 103
byte 1 104
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 102
byte 1 0
align 1
LABELV $294
byte 1 112
byte 1 109
byte 1 46
byte 1 108
byte 1 101
byte 1 102
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 102
byte 1 0
align 1
LABELV $292
byte 1 112
byte 1 109
byte 1 46
byte 1 98
byte 1 97
byte 1 99
byte 1 107
byte 1 119
byte 1 97
byte 1 114
byte 1 100
byte 1 58
byte 1 32
byte 1 37
byte 1 102
byte 1 0
align 1
LABELV $290
byte 1 112
byte 1 109
byte 1 46
byte 1 102
byte 1 111
byte 1 114
byte 1 119
byte 1 97
byte 1 114
byte 1 100
byte 1 58
byte 1 32
byte 1 37
byte 1 102
byte 1 0
align 1
LABELV $289
byte 1 115
byte 1 101
byte 1 108
byte 1 102
byte 1 45
byte 1 62
byte 1 111
byte 1 114
byte 1 105
byte 1 103
byte 1 105
byte 1 110
byte 1 91
byte 1 37
byte 1 105
byte 1 93
byte 1 58
byte 1 32
byte 1 37
byte 1 102
byte 1 0
align 1
LABELV $284
byte 1 80
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 32
byte 1 77
byte 1 111
byte 1 118
byte 1 101
byte 1 32
byte 1 77
byte 1 101
byte 1 116
byte 1 114
byte 1 105
byte 1 99
byte 1 115
byte 1 0
