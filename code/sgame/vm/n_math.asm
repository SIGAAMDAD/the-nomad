data
export vec2_origin
align 4
LABELV vec2_origin
byte 4 0
byte 4 0
export vec3_origin
align 4
LABELV vec3_origin
byte 4 0
byte 4 0
skip 4
export colorBlack
align 4
LABELV colorBlack
byte 4 0
byte 4 0
byte 4 0
byte 4 1065353216
export colorRed
align 4
LABELV colorRed
byte 4 1065353216
byte 4 0
byte 4 0
byte 4 1065353216
export colorGreen
align 4
LABELV colorGreen
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 1065353216
export colorBlue
align 4
LABELV colorBlue
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 1065353216
export colorYellow
align 4
LABELV colorYellow
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1065353216
export colorMagenta
align 4
LABELV colorMagenta
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1065353216
export colorCyan
align 4
LABELV colorCyan
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
export colorWhite
align 4
LABELV colorWhite
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
export colorLtGrey
align 4
LABELV colorLtGrey
byte 4 1061158912
byte 4 1061158912
byte 4 1061158912
byte 4 1065353216
export colorMdGrey
align 4
LABELV colorMdGrey
byte 4 1056964608
byte 4 1056964608
byte 4 1056964608
byte 4 1065353216
export colorDkGrey
align 4
LABELV colorDkGrey
byte 4 1048576000
byte 4 1048576000
byte 4 1048576000
byte 4 1065353216
export g_color_table
align 4
LABELV g_color_table
byte 4 0
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1045220557
byte 4 1045220557
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1056964608
byte 4 0
byte 4 1065353216
byte 4 1058642330
byte 4 1058642330
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1049178302
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1056964608
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1060857761
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1060857761
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1056964608
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1049178302
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1049178302
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1056964608
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1060857761
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1060857761
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1056964608
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1049178302
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1049178302
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1056964608
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1060857761
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1060857761
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1056964608
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 1049178302
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1065353216
byte 4 1056964608
byte 4 1056964608
byte 4 1056964608
byte 4 1065353216
skip 448
export ColorIndexFromChar
code
proc ColorIndexFromChar 12 0
ADDRFP4 0
ADDRFP4 0
INDIRI4
CVII1 4
ASGNI1
file "../../engine/n_math.c"
line 81
;1:#ifndef GDR_DLLCOMPILE
;2:#include "n_shared.h"
;3:#else
;4:#include "../engine/n_shared.h"
;5:#endif
;6:
;7:#if defined(__SSE2__) || defined(_MSC_SSE2_)
;8:#define USING_SSE2
;9:#ifdef _MSC
;10:#include <intrin.h>
;11:#else
;12:#include <immintrin.h>
;13:#include <xmmintrin.h>
;14:#endif
;15:#endif
;16:
;17:int CPU_flags;
;18:
;19:const vec2_t vec2_origin = {0, 0};
;20:const vec3_t vec3_origin = {0, 0};
;21:
;22:const vec4_t		colorBlack	= {0, 0, 0, 1};
;23:const vec4_t		colorRed	= {1, 0, 0, 1};
;24:const vec4_t		colorGreen	= {0, 1, 0, 1};
;25:const vec4_t		colorBlue	= {0, 0, 1, 1};
;26:const vec4_t		colorYellow	= {1, 1, 0, 1};
;27:const vec4_t		colorMagenta= {1, 0, 1, 1};
;28:const vec4_t		colorCyan	= {0, 1, 1, 1};
;29:const vec4_t		colorWhite	= {1, 1, 1, 1};
;30:const vec4_t		colorLtGrey	= {0.75, 0.75, 0.75, 1};
;31:const vec4_t		colorMdGrey	= {0.5, 0.5, 0.5, 1};
;32:const vec4_t		colorDkGrey	= {0.25, 0.25, 0.25, 1};
;33:
;34:// actually there are 35 colors but we want to use bitmask safely
;35:const vec4_t g_color_table[ 64 ] = {
;36:
;37:	{0.0f, 0.0f, 0.0f, 1.0f},
;38:	{1.0f, 0.0f, 0.0f, 1.0f},
;39:	{0.0f, 1.0f, 0.0f, 1.0f},
;40:	{1.0f, 1.0f, 0.0f, 1.0f},
;41:	{0.2f, 0.2f, 1.0f, 1.0f}, //{0.0, 0.0, 1.0, 1.0},
;42:	{0.0f, 1.0f, 1.0f, 1.0f},
;43:	{1.0f, 0.0f, 1.0f, 1.0f},
;44:	{1.0f, 1.0f, 1.0f, 1.0f},
;45:
;46:	// extended color codes from CPMA/CNQ3:
;47:	{ 1.00000f, 0.50000f, 0.00000f, 1.00000f },	// 8
;48:	{ 0.60000f, 0.60000f, 1.00000f, 1.00000f },	// 9
;49:
;50:	// CPMA's alphabet rainbow
;51:	{ 1.00000f, 0.00000f, 0.00000f, 1.00000f },	// a
;52:	{ 1.00000f, 0.26795f, 0.00000f, 1.00000f },	// b
;53:	{ 1.00000f, 0.50000f, 0.00000f, 1.00000f },	// c
;54:	{ 1.00000f, 0.73205f, 0.00000f, 1.00000f },	// d
;55:	{ 1.00000f, 1.00000f, 0.00000f, 1.00000f },	// e
;56:	{ 0.73205f, 1.00000f, 0.00000f, 1.00000f },	// f
;57:	{ 0.50000f, 1.00000f, 0.00000f, 1.00000f },	// g
;58:	{ 0.26795f, 1.00000f, 0.00000f, 1.00000f },	// h
;59:	{ 0.00000f, 1.00000f, 0.00000f, 1.00000f },	// i
;60:	{ 0.00000f, 1.00000f, 0.26795f, 1.00000f },	// j
;61:	{ 0.00000f, 1.00000f, 0.50000f, 1.00000f },	// k
;62:	{ 0.00000f, 1.00000f, 0.73205f, 1.00000f },	// l
;63:	{ 0.00000f, 1.00000f, 1.00000f, 1.00000f },	// m
;64:	{ 0.00000f, 0.73205f, 1.00000f, 1.00000f },	// n
;65:	{ 0.00000f, 0.50000f, 1.00000f, 1.00000f },	// o
;66:	{ 0.00000f, 0.26795f, 1.00000f, 1.00000f },	// p
;67:	{ 0.00000f, 0.00000f, 1.00000f, 1.00000f },	// q
;68:	{ 0.26795f, 0.00000f, 1.00000f, 1.00000f },	// r
;69:	{ 0.50000f, 0.00000f, 1.00000f, 1.00000f },	// s
;70:	{ 0.73205f, 0.00000f, 1.00000f, 1.00000f },	// t
;71:	{ 1.00000f, 0.00000f, 1.00000f, 1.00000f },	// u
;72:	{ 1.00000f, 0.00000f, 0.73205f, 1.00000f },	// v
;73:	{ 1.00000f, 0.00000f, 0.50000f, 1.00000f },	// w
;74:	{ 1.00000f, 0.00000f, 0.26795f, 1.00000f },	// x
;75:	{ 1.0, 1.0, 1.0, 1.0 }, // y, white, duped so all colors can be expressed with this palette
;76:	{ 0.5, 0.5, 0.5, 1.0 }, // z, grey
;77:};
;78:
;79:
;80:int ColorIndexFromChar( char ccode )
;81:{
line 82
;82:	if ( ccode >= '0' && ccode <= '9' ) {
ADDRLP4 0
ADDRFP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $31
ADDRLP4 0
INDIRI4
CNSTI4 57
GTI4 $31
line 83
;83:		return ( ccode - '0' );
ADDRFP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
RETI4
ADDRGP4 $30
JUMPV
LABELV $31
line 85
;84:	}
;85:	else if ( ccode >= 'a' && ccode <= 'z' ) {
ADDRLP4 4
ADDRFP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 97
LTI4 $33
ADDRLP4 4
INDIRI4
CNSTI4 122
GTI4 $33
line 86
;86:		return ( ccode - 'a' + 10 );
ADDRFP4 0
INDIRI1
CVII4 1
CNSTI4 97
SUBI4
CNSTI4 10
ADDI4
RETI4
ADDRGP4 $30
JUMPV
LABELV $33
line 88
;87:	}
;88:	else if ( ccode >= 'A' && ccode <= 'Z' ) {
ADDRLP4 8
ADDRFP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 8
INDIRI4
CNSTI4 65
LTI4 $35
ADDRLP4 8
INDIRI4
CNSTI4 90
GTI4 $35
line 89
;89:		return ( ccode - 'A' + 10 );
ADDRFP4 0
INDIRI1
CVII4 1
CNSTI4 65
SUBI4
CNSTI4 10
ADDI4
RETI4
ADDRGP4 $30
JUMPV
LABELV $35
line 91
;90:	}
;91:	else {
line 92
;92:		return  ColorIndex( S_COLOR_WHITE );
CNSTI4 7
RETI4
LABELV $30
endproc ColorIndexFromChar 12 0
export Q_root
proc Q_root 24 0
line 97
;93:	}
;94:}
;95:
;96:float Q_root(float x)
;97:{
line 99
;98:	long        i;								// The integer interpretation of x
;99:	float       x_half = x * 0.5f;
ADDRLP4 8
CNSTF4 1056964608
ADDRFP4 0
INDIRF4
MULF4
ASGNF4
line 100
;100:	float       r_sqrt = x;
ADDRLP4 0
ADDRFP4 0
INDIRF4
ASGNF4
line 101
;101:	const float threehalfs = 1.5F;
ADDRLP4 12
CNSTF4 1069547520
ASGNF4
line 104
;102:
;103:	// trick c/c++, bit hack
;104:	i = *(long *)&r_sqrt;					    // oh yes, undefined behaviour, who gives a fuck?
ADDRLP4 4
ADDRLP4 0
INDIRI4
ASGNI4
line 105
;105:	i = 0x5f375a86 - (i >> 1);				    // weird magic base-16 nums
ADDRLP4 4
CNSTI4 1597463174
ADDRLP4 4
INDIRI4
CNSTI4 1
RSHI4
SUBI4
ASGNI4
line 106
;106:	r_sqrt = *(float *) &i;
ADDRLP4 0
ADDRLP4 4
INDIRF4
ASGNF4
line 108
;107:
;108:	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 1st Newton iteration
ADDRLP4 16
ADDRLP4 0
INDIRF4
ASGNF4
ADDRLP4 0
ADDRLP4 16
INDIRF4
ADDRLP4 12
INDIRF4
ADDRLP4 8
INDIRF4
ADDRLP4 16
INDIRF4
MULF4
ADDRLP4 16
INDIRF4
MULF4
SUBF4
MULF4
ASGNF4
line 109
;109:	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 2nd Newton iteration
ADDRLP4 20
ADDRLP4 0
INDIRF4
ASGNF4
ADDRLP4 0
ADDRLP4 20
INDIRF4
ADDRLP4 12
INDIRF4
ADDRLP4 8
INDIRF4
ADDRLP4 20
INDIRF4
MULF4
ADDRLP4 20
INDIRF4
MULF4
SUBF4
MULF4
ASGNF4
line 111
;110:
;111:	return x * r_sqrt; // x * (1/sqrt(x)) := sqrt(x)
ADDRFP4 0
INDIRF4
ADDRLP4 0
INDIRF4
MULF4
RETF4
LABELV $37
endproc Q_root 24 0
export Q_rsqrt
proc Q_rsqrt 20 0
line 115
;112:}
;113:
;114:float Q_rsqrt(float number)
;115:{
line 139
;116:#ifdef USING_SSE2
;117:	// does this cpu actually support sse2?
;118:	if (!(CPU_flags & CPU_SSE2)) {
;119:		long x;
;120:    	float x2, y;
;121:		const float threehalfs = 1.5F;
;122:
;123:    	x2 = number * 0.5F;
;124:    	x = *(long *)&number;                    // evil floating point bit level hacking
;125:    	x = 0x5f3759df - (x >> 1);               // what the fuck?
;126:    	y = *(float *)&x;
;127:    	y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
;128:  	//	y = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed
;129:
;130:    	return y;
;131:	}
;132:
;133:	float ret;
;134:	_mm_store_ss( &ret, _mm_rsqrt_ss( _mm_load_ss( &number ) ) );
;135:	return ret;
;136:#else
;137:    long x;
;138:    float x2, y;
;139:	const float threehalfs = 1.5F;
ADDRLP4 12
CNSTF4 1069547520
ASGNF4
line 141
;140:
;141:    x2 = number * 0.5F;
ADDRLP4 8
CNSTF4 1056964608
ADDRFP4 0
INDIRF4
MULF4
ASGNF4
line 142
;142:    x = *(long *)&number;                    // evil floating point bit level hacking
ADDRLP4 4
ADDRFP4 0
INDIRI4
ASGNI4
line 143
;143:    x = 0x5f3759df - (x >> 1);               // what the fuck?
ADDRLP4 4
CNSTI4 1597463007
ADDRLP4 4
INDIRI4
CNSTI4 1
RSHI4
SUBI4
ASGNI4
line 144
;144:    y = *(float *)&x;
ADDRLP4 0
ADDRLP4 4
INDIRF4
ASGNF4
line 145
;145:    y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
ADDRLP4 0
ADDRLP4 0
INDIRF4
ADDRLP4 12
INDIRF4
ADDRLP4 8
INDIRF4
ADDRLP4 0
INDIRF4
MULF4
ADDRLP4 0
INDIRF4
MULF4
SUBF4
MULF4
ASGNF4
line 148
;146://  y = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed
;147:
;148:    return y;
ADDRLP4 0
INDIRF4
RETF4
LABELV $38
endproc Q_rsqrt 20 0
export disBetweenOBJ
proc disBetweenOBJ 40 8
line 152
;149:#endif
;150:}
;151:float disBetweenOBJ(const vec3_t src, const vec3_t tar)
;152:{
line 153
;153:	if (src[1] == tar[1]) // horizontal
ADDRLP4 0
CNSTI4 4
ASGNI4
ADDRFP4 0
INDIRP4
ADDRLP4 0
INDIRI4
ADDP4
INDIRF4
ADDRFP4 4
INDIRP4
ADDRLP4 0
INDIRI4
ADDP4
INDIRF4
NEF4 $40
line 154
;154:		return src[0] > tar[0] ? (src[0] - tar[0]) : (tar[0] - src[0]);
ADDRFP4 0
INDIRP4
INDIRF4
ADDRFP4 4
INDIRP4
INDIRF4
LEF4 $43
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRF4
ADDRFP4 4
INDIRP4
INDIRF4
SUBF4
ASGNF4
ADDRGP4 $44
JUMPV
LABELV $43
ADDRLP4 4
ADDRFP4 4
INDIRP4
INDIRF4
ADDRFP4 0
INDIRP4
INDIRF4
SUBF4
ASGNF4
LABELV $44
ADDRLP4 4
INDIRF4
RETF4
ADDRGP4 $39
JUMPV
LABELV $40
line 155
;155:	else if (src[0] == tar[0]) // vertical
ADDRFP4 0
INDIRP4
INDIRF4
ADDRFP4 4
INDIRP4
INDIRF4
NEF4 $45
line 156
;156:		return src[1] > tar[1] ? (src[1] - tar[1]) : (tar[1] - src[1]);
ADDRLP4 12
CNSTI4 4
ASGNI4
ADDRFP4 0
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
INDIRF4
ADDRFP4 4
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
INDIRF4
LEF4 $48
ADDRLP4 16
CNSTI4 4
ASGNI4
ADDRLP4 8
ADDRFP4 0
INDIRP4
ADDRLP4 16
INDIRI4
ADDP4
INDIRF4
ADDRFP4 4
INDIRP4
ADDRLP4 16
INDIRI4
ADDP4
INDIRF4
SUBF4
ASGNF4
ADDRGP4 $49
JUMPV
LABELV $48
ADDRLP4 20
CNSTI4 4
ASGNI4
ADDRLP4 8
ADDRFP4 4
INDIRP4
ADDRLP4 20
INDIRI4
ADDP4
INDIRF4
ADDRFP4 0
INDIRP4
ADDRLP4 20
INDIRI4
ADDP4
INDIRF4
SUBF4
ASGNF4
LABELV $49
ADDRLP4 8
INDIRF4
RETF4
ADDRGP4 $39
JUMPV
LABELV $45
line 158
;157:	else // diagonal
;158:		return Q_root((pow((src[0] - tar[0]), 2) + pow((src[1] - tar[1]), 2)));
ADDRFP4 0
INDIRP4
INDIRF4
ADDRFP4 4
INDIRP4
INDIRF4
SUBF4
ARGF4
CNSTI4 2
ARGI4
ADDRLP4 24
ADDRGP4 pow
CALLI4
ASGNI4
ADDRLP4 28
CNSTI4 4
ASGNI4
ADDRFP4 0
INDIRP4
ADDRLP4 28
INDIRI4
ADDP4
INDIRF4
ADDRFP4 4
INDIRP4
ADDRLP4 28
INDIRI4
ADDP4
INDIRF4
SUBF4
ARGF4
CNSTI4 2
ARGI4
ADDRLP4 32
ADDRGP4 pow
CALLI4
ASGNI4
ADDRLP4 24
INDIRI4
ADDRLP4 32
INDIRI4
ADDI4
CVIF4 4
ARGF4
ADDRLP4 36
ADDRGP4 Q_root
CALLF4
ASGNF4
ADDRLP4 36
INDIRF4
RETF4
LABELV $39
endproc disBetweenOBJ 40 8
export VectorNormalize
proc VectorNormalize 36 4
line 162
;159:}
;160:
;161:vec_t VectorNormalize(vec3_t v)
;162:{
line 165
;163:	float ilength, length;
;164:
;165:	length = DotProduct(v, v);
ADDRLP4 8
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 12
ADDRLP4 8
INDIRP4
INDIRF4
ASGNF4
ADDRLP4 16
ADDRLP4 8
INDIRP4
CNSTI4 4
ADDP4
INDIRF4
ASGNF4
ADDRLP4 20
ADDRLP4 8
INDIRP4
CNSTI4 8
ADDP4
INDIRF4
ASGNF4
ADDRLP4 0
ADDRLP4 12
INDIRF4
ADDRLP4 12
INDIRF4
MULF4
ADDRLP4 16
INDIRF4
ADDRLP4 16
INDIRF4
MULF4
ADDF4
ADDRLP4 20
INDIRF4
ADDRLP4 20
INDIRF4
MULF4
ADDF4
ASGNF4
line 167
;166:
;167:	if (length) {
ADDRLP4 0
INDIRF4
CNSTF4 0
EQF4 $51
line 170
;168:#if 1
;169:		// writing it this way allows g++ to recognize that rsqrt can be used
;170:		ilength = 1/(float)sqrt(length);
ADDRLP4 0
INDIRF4
ARGF4
ADDRLP4 24
ADDRGP4 sqrt
CALLI4
ASGNI4
ADDRLP4 4
CNSTF4 1065353216
ADDRLP4 24
INDIRI4
CVIF4 4
DIVF4
ASGNF4
line 174
;171:#else
;172:		ilength = Q_rsqrt(length);
;173:#endif
;174:		length *= ilength;
ADDRLP4 0
ADDRLP4 0
INDIRF4
ADDRLP4 4
INDIRF4
MULF4
ASGNF4
line 176
;175:
;176:		v[0] *= ilength;
ADDRLP4 28
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 28
INDIRP4
ADDRLP4 28
INDIRP4
INDIRF4
ADDRLP4 4
INDIRF4
MULF4
ASGNF4
line 177
;177:		v[1] *= ilength;
ADDRLP4 32
ADDRFP4 0
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
ADDRLP4 32
INDIRP4
ADDRLP4 32
INDIRP4
INDIRF4
ADDRLP4 4
INDIRF4
MULF4
ASGNF4
line 178
;178:	}
LABELV $51
line 179
;179:	return length;
ADDRLP4 0
INDIRF4
RETF4
LABELV $50
endproc VectorNormalize 36 4
import sqrt
import pow
import G_LoadBFF
import BFF_FetchTexture
import BFF_FetchLevel
import BFF_FetchScript
import BFF_FreeInfo
import BFF_FetchInfo
import BFF_OpenArchive
import BFF_CloseArchive
import B_GetChunk
import I_GetParm
bss
export CPU_flags
align 4
LABELV CPU_flags
skip 4
import FS_ReadLine
import FS_ListFiles
import FS_FreeFile
import FS_SetBFFIndex
import FS_GetCurrentChunkList
import FS_Initialized
import FS_FileIsInBFF
import FS_StripExt
import FS_AllowedExtension
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
import FS_Init
import FS_VM_CloseFiles
import FS_VM_FOpenFileWrite
import FS_VM_FileSeek
import FS_VM_FOpenFileRead
import FS_VM_CreateTmp
import FS_VM_WriteFile
import FS_VM_Write
import FS_VM_Read
import FS_VM_FClose
import FS_VM_FOpenRead
import FS_VM_FOpenWrite
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
import Cmd_CompleteArgument
import Cmd_CommandCompletion
import va
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
import crc32_buffer
import I_NomadInit
import Cvar_SetBooleanValue
import Cvar_SetStringValue
import Cvar_SetFloatValue
import Cvar_SetIntegerValue
import Cvar_SetModified
import Cvar_SetValueSafe
import Cvar_Set
import Cvar_SetSafe
import Cvar_SetDescription
import Cvar_SetGroup
import Cvar_Reset
import Cvar_Command
import Cvar_Get
import Cvar_Update
import Cvar_Flags
import Cvar_CheckRange
import Cvar_VariableString
import Cvar_VariableBoolean
import Cvar_VariableFloat
import Cvar_VariableInteger
import Cvar_VariableStringBufferSafe
import Cvar_VariableStringBuffer
import Cvar_Set2
import Cvar_CommandCompletion
import Cvar_CompleteCvarName
import Cvar_Register
import Cvar_Restart
import N_booltostr
import N_strtobool
import Com_Clamp
import N_isnan
import PerpendicularVector
import AngleVectors
import MatrixMultiply
import MakeNormalVectors
import RotateAroundDirection
import RotatePointAroundVector
import ProjectPointOnPlane
import PlaneFromPoints
import AngleDelta
import AngleNormalize180
import AngleNormalize360
import AnglesSubtract
import AngleSubtract
import LerpAngle
import AngleMod
import BoundsIntersectPoint
import BoundsIntersectSphere
import BoundsIntersect
import AxisCopy
import AxisClear
import AnglesToAxis
import vectoangles
import N_crandom
import N_random
import N_rand
import N_acos
import N_log2
import VectorRotate
import Vector4Scale
import VectorNormalize2
import CrossProduct
import VectorInverse
import VectorNormalizeFast
import DistanceSquared
import Distance
import VectorLengthSquared
import VectorLength
import VectorCompare
import AddPointToBounds
import ClearBounds
import RadiusFromBounds
import NormalizeColor
import ColorBytes4
import ColorBytes3
import _VectorMA
import _VectorScale
import _VectorCopy
import _VectorAdd
import _VectorSubtract
import _DotProduct
import ByteToDir
import DirToByte
import ClampShort
import ClampCharMove
import ClampChar
import Q_exp2f
import Q_log2f
import Q_fabs
import locase
import mat4_identity
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
import Com_Error
import Com_Printf
import G_Printf
import vsprintf
import fabs
import abs
import _atoi
import atoi
import _atof
import atof
import rand
import srand
import qsort
import toupper
import tolower
import strncmp
import strcmp
import strstr
import strchr
import strrchr
import strcat
import strncpy
import strcpy
import memmove
import memset
import memccpy
import strlen
import memchr
import memcpy
