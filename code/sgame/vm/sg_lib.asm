export strcat
code
proc strcat 12 0
file "../sg_lib.c"
line 159
;1:#include "../engine/n_shared.h"
;2:
;3:#ifndef Q3_VM
;4:    #error Never include this in engine builds
;5:#endif
;6:
;7:#if 0
;8:
;9:void *memset(void *dst, int fill, size_t n)
;10:{
;11:    size_t i;
;12:
;13:    if ((((long)dst | n) & 3) == 0) {
;14:        n >>= 2;
;15:        fill = fill | (fill << 8) | (fill << 16) | (fill << 24);
;16:
;17:        for (i = 0; i < n; i++) {
;18:            ((int *)dst)[i] = fill;
;19:        }
;20:    }
;21:    else {
;22:        for (i = 0; i < n; i++) {
;23:            ((char *)dst)[i] = fill;
;24:        }
;25:    }
;26:    return dst;
;27:}
;28:
;29:void *memcpy( void *dst, const void *src, size_t n )
;30:{
;31:    size_t i;
;32:
;33:    if ( (((long)dst | (long)src | n) & 3) == 0 ) {
;34:        n >>= 2;
;35:
;36:        for ( i = 0; i < n; i++) {
;37:            ((int *)dst)[i] = ((const int *)src)[i];
;38:        }
;39:    }
;40:    else {
;41:        for ( i = 0; i < n; i++ ) {
;42:            ((char *)dst)[i] = ((const char *)src)[i];
;43:        }
;44:    }
;45:    return dst;
;46:}
;47:
;48:void *memchr( void *ptr, int delegate, size_t n )
;49:{
;50:    char *p = ptr;
;51:
;52:    while ( n-- ) {
;53:        if ( *p++ == delegate ) {
;54:            return (void *)p;
;55:        }
;56:    }
;57:    return ptr;
;58:}
;59:
;60:void *memmove( void *dst, const void *src, size_t n )
;61:{
;62:    char *d = dst;
;63:    const char *s = src;
;64:    
;65:    if ( d > s ) {
;66:        while ( n-- ) {
;67:            *d-- = *s--;
;68:        }
;69:    }
;70:    else {
;71:        while ( n-- ) {
;72:            *d++ = *s++;
;73:        }
;74:    }
;75:    return dst;
;76:}
;77:#endif
;78:
;79:// bk001211 - gcc errors on compiling strcpy:  parse error before `__extension__'
;80:#ifdef Q3_VM
;81:
;82:#if 0
;83:size_t strlen(const char *str)
;84:{
;85:    const char *s = str;
;86:    while (*s) {
;87:        s++;
;88:    }
;89:    return (size_t)(s - str);
;90:}
;91:
;92:char* strchr(const char* string, int c)
;93:{
;94:    while ( *string ) {
;95:		if ( *string == c ) {
;96:			return ( char * )string;
;97:		}
;98:		string++;
;99:	}
;100:	return (char *)0;
;101:}
;102:
;103:char *strrchr(const char *string, int c)
;104:{
;105:    const char *found, *p;
;106:
;107:    c = (unsigned char)c;
;108:
;109:    if (c == '\0')
;110:        return strchr(string, '\0');
;111:    
;112:    found = NULL;
;113:    while ((p = strchr(string, c)) != NULL) {
;114:        found = p;
;115:        string = p + 1;
;116:    }
;117:    return (char *)found;
;118:}
;119:
;120:char *strstr( const char *string, const char *strCharSet ) {
;121:	while ( *string ) {
;122:		int		i;
;123:
;124:		for ( i = 0 ; strCharSet[i] ; i++ ) {
;125:			if ( string[i] != strCharSet[i] ) {
;126:				break;
;127:			}
;128:		}
;129:		if ( !strCharSet[i] ) {
;130:			return (char *)string;
;131:		}
;132:		string++;
;133:	}
;134:	return (char *)0;
;135:}
;136:
;137:int strcmp(const char* string1, const char* string2)
;138:{
;139:    while ( *string1 == *string2 && *string1 && *string2 ) {
;140:		string1++;
;141:		string2++;
;142:	}
;143:	return *string1 - *string2;
;144:}
;145:
;146:char* strcpy(char *dst, const char *src)
;147:{
;148:    char *d;
;149:
;150:    d = dst;
;151:    while (*src) {
;152:        *d++ = *src++;
;153:    }
;154:    *d = 0;
;155:    return dst;
;156:}
;157:#endif
;158:
;159:char *strcat( char *strDestination, const char *strSource ) {
line 162
;160:	char	*s;
;161:
;162:	s = strDestination;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $24
JUMPV
LABELV $23
line 163
;163:	while ( *s ) {
line 164
;164:		s++;
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 165
;165:	}
LABELV $24
line 163
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $23
ADDRGP4 $27
JUMPV
LABELV $26
line 166
;166:	while ( *strSource ) {
line 167
;167:		*s++ = *strSource++;
ADDRLP4 4
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 8
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 8
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 8
INDIRP4
INDIRI1
ASGNI1
line 168
;168:	}
LABELV $27
line 166
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $26
line 169
;169:	*s = 0;
ADDRLP4 0
INDIRP4
CNSTI1 0
ASGNI1
line 170
;170:	return (char *)strDestination;
ADDRFP4 0
INDIRP4
RETP4
LABELV $22
endproc strcat 12 0
export tolower
proc tolower 4 0
line 174
;171:}
;172:
;173:int tolower(int c)
;174:{
line 175
;175:    if (c >= 'A' && c <= 'Z') {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 65
LTI4 $30
ADDRLP4 0
INDIRI4
CNSTI4 90
GTI4 $30
line 176
;176:        c += 'a' - 'A';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 32
ADDI4
ASGNI4
line 177
;177:    }
LABELV $30
line 178
;178:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $29
endproc tolower 4 0
export toupper
proc toupper 4 0
line 182
;179:}
;180:
;181:int toupper(int c)
;182:{
line 183
;183:    if (c >= 'a' && c <= 'z') {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 97
LTI4 $33
ADDRLP4 0
INDIRI4
CNSTI4 122
GTI4 $33
line 184
;184:        c += 'A' - 'a';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 -32
ADDI4
ASGNI4
line 185
;185:    }
LABELV $33
line 186
;186:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $32
endproc toupper 4 0
proc swapfunc 24 0
line 217
;187:}
;188:#endif
;189:
;190:
;191:static char* med3(char*, char*, char*, cmp_t*);
;192:static void  swapfunc(char*, char*, int, int);
;193:
;194:/*
;195: * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
;196: */
;197:#define swapcode(TYPE, parmi, parmj, n)                                        \
;198:    {                                                                          \
;199:        long           i  = (n) / sizeof(TYPE);                                \
;200:        register TYPE* pi = (TYPE*)(parmi);                                    \
;201:        register TYPE* pj = (TYPE*)(parmj);                                    \
;202:        do                                                                     \
;203:        {                                                                      \
;204:            register TYPE t = *pi;                                             \
;205:            *pi++           = *pj;                                             \
;206:            *pj++           = t;                                               \
;207:        } while (--i > 0);                                                     \
;208:    }
;209:
;210:#define SWAPINIT(a, es)                                                        \
;211:    swaptype = ((char*)a - (char*)0) % sizeof(long) || es % sizeof(long)       \
;212:                   ? 2                                                         \
;213:                   : es == sizeof(long) ? 0 : 1;
;214:
;215:static void swapfunc(a, b, n, swaptype) char *a, *b;
;216:int         n, swaptype;
;217:{
line 218
;218:    if (swaptype <= 1)
ADDRFP4 12
INDIRI4
CNSTI4 1
GTI4 $36
line 219
;219:        swapcode(long, a, b, n) else swapcode(char, a, b, n)
ADDRLP4 8
ADDRFP4 8
INDIRI4
CVIU4 4
CNSTI4 2
RSHU4
CVUI4 4
ASGNI4
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
LABELV $38
ADDRLP4 12
ADDRLP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 16
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 16
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
ADDRLP4 16
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 20
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 20
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 12
INDIRI4
ASGNI4
LABELV $39
ADDRLP4 12
ADDRLP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 8
ADDRLP4 12
INDIRI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
GTI4 $38
ADDRGP4 $37
JUMPV
LABELV $36
ADDRLP4 8
ADDRFP4 8
INDIRI4
CVIU4 4
CVUI4 4
ASGNI4
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
LABELV $41
ADDRLP4 12
ADDRLP4 0
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 16
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 0
ADDRLP4 16
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 16
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 20
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 20
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 12
INDIRI1
ASGNI1
LABELV $42
ADDRLP4 12
ADDRLP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 8
ADDRLP4 12
INDIRI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
GTI4 $41
LABELV $37
line 220
;220:}
LABELV $35
endproc swapfunc 24 0
proc med3 40 8
line 238
;221:
;222:#define swap(a, b)                                                             \
;223:    if (swaptype == 0)                                                         \
;224:    {                                                                          \
;225:        long t      = *(long*)(a);                                             \
;226:        *(long*)(a) = *(long*)(b);                                             \
;227:        *(long*)(b) = t;                                                       \
;228:    }                                                                          \
;229:    else                                                                       \
;230:        swapfunc(a, b, es, swaptype)
;231:
;232:#define vecswap(a, b, n)                                                       \
;233:    if ((n) > 0)                                                               \
;234:    swapfunc(a, b, n, swaptype)
;235:
;236:static char *med3(a, b, c, cmp) char *a, *b, *c;
;237:cmp_t*       cmp;
;238:{
line 239
;239:    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
ADDRFP4 0
INDIRP4
ARGP4
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 20
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 20
INDIRI4
CNSTI4 0
GEI4 $50
ADDRFP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 24
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 24
INDIRI4
CNSTI4 0
GEI4 $52
ADDRLP4 4
ADDRFP4 4
INDIRP4
ASGNP4
ADDRGP4 $53
JUMPV
LABELV $52
ADDRFP4 0
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 28
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 28
INDIRI4
CNSTI4 0
GEI4 $54
ADDRLP4 8
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $55
JUMPV
LABELV $54
ADDRLP4 8
ADDRFP4 0
INDIRP4
ASGNP4
LABELV $55
ADDRLP4 4
ADDRLP4 8
INDIRP4
ASGNP4
LABELV $53
ADDRLP4 0
ADDRLP4 4
INDIRP4
ASGNP4
ADDRGP4 $51
JUMPV
LABELV $50
ADDRFP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 32
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 32
INDIRI4
CNSTI4 0
LEI4 $56
ADDRLP4 12
ADDRFP4 4
INDIRP4
ASGNP4
ADDRGP4 $57
JUMPV
LABELV $56
ADDRFP4 0
INDIRP4
ARGP4
ADDRFP4 8
INDIRP4
ARGP4
ADDRLP4 36
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 0
GEI4 $58
ADDRLP4 16
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $59
JUMPV
LABELV $58
ADDRLP4 16
ADDRFP4 8
INDIRP4
ASGNP4
LABELV $59
ADDRLP4 12
ADDRLP4 16
INDIRP4
ASGNP4
LABELV $57
ADDRLP4 0
ADDRLP4 12
INDIRP4
ASGNP4
LABELV $51
ADDRLP4 0
INDIRP4
RETP4
LABELV $44
endproc med3 40 8
export qsort
proc qsort 96 16
line 246
;240:                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
;241:}
;242:
;243:void   qsort(a, n, es, cmp) void* a;
;244:size_t n, es;
;245:cmp_t* cmp;
;246:{
LABELV $61
line 251
;247:    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
;248:    int   d, r, swaptype, swap_cnt;
;249:
;250:loop:
;251:    SWAPINIT(a, es);
ADDRFP4 0
INDIRP4
CVPU4 4
CVUI4 4
CVIU4 4
CNSTU4 3
BANDU4
CNSTU4 0
NEU4 $66
ADDRFP4 8
INDIRU4
CNSTU4 3
BANDU4
CNSTU4 0
EQU4 $64
LABELV $66
ADDRLP4 44
CNSTI4 2
ASGNI4
ADDRGP4 $65
JUMPV
LABELV $64
ADDRFP4 8
INDIRU4
CNSTU4 4
NEU4 $67
ADDRLP4 48
CNSTI4 0
ASGNI4
ADDRGP4 $68
JUMPV
LABELV $67
ADDRLP4 48
CNSTI4 1
ASGNI4
LABELV $68
ADDRLP4 44
ADDRLP4 48
INDIRI4
ASGNI4
LABELV $65
ADDRLP4 16
ADDRLP4 44
INDIRI4
ASGNI4
line 252
;252:    swap_cnt = 0;
ADDRLP4 28
CNSTI4 0
ASGNI4
line 253
;253:    if (n < 7)
ADDRFP4 4
INDIRU4
CNSTU4 7
GEU4 $69
line 254
;254:    {
line 255
;255:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
ADDRLP4 32
ADDRFP4 8
INDIRU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRGP4 $74
JUMPV
LABELV $71
line 256
;256:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
ADDRLP4 0
ADDRLP4 32
INDIRP4
ASGNP4
ADDRGP4 $78
JUMPV
LABELV $75
line 257
;257:                swap(pl, pl - es);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $79
ADDRLP4 52
ADDRLP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ADDRLP4 52
INDIRI4
ASGNI4
ADDRGP4 $80
JUMPV
LABELV $79
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 56
ADDRFP4 8
INDIRU4
ASGNU4
ADDRLP4 0
INDIRP4
ADDRLP4 56
INDIRU4
SUBP4
ARGP4
ADDRLP4 56
INDIRU4
CVUI4 4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $80
LABELV $76
line 256
ADDRLP4 0
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ASGNP4
LABELV $78
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
LEU4 $81
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 64
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 64
INDIRI4
CNSTI4 0
GTI4 $75
LABELV $81
LABELV $72
line 255
ADDRLP4 32
ADDRFP4 8
INDIRU4
ADDRLP4 32
INDIRP4
ADDP4
ASGNP4
LABELV $74
ADDRLP4 32
INDIRP4
CVPU4 4
ADDRFP4 4
INDIRU4
ADDRFP4 8
INDIRU4
MULU4
ADDRFP4 0
INDIRP4
ADDP4
CVPU4 4
LTU4 $71
line 258
;258:        return;
ADDRGP4 $60
JUMPV
LABELV $69
line 260
;259:    }
;260:    pm = (char*)a + (n / 2) * es;
ADDRLP4 32
ADDRFP4 4
INDIRU4
CNSTI4 1
RSHU4
ADDRFP4 8
INDIRU4
MULU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
line 261
;261:    if (n > 7)
ADDRFP4 4
INDIRU4
CNSTU4 7
LEU4 $82
line 262
;262:    {
line 263
;263:        pl = a;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
line 264
;264:        pn = (char*)a + (n - 1) * es;
ADDRLP4 36
ADDRFP4 4
INDIRU4
CNSTU4 1
SUBU4
ADDRFP4 8
INDIRU4
MULU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
line 265
;265:        if (n > 40)
ADDRFP4 4
INDIRU4
CNSTU4 40
LEU4 $84
line 266
;266:        {
line 267
;267:            d  = (n / 8) * es;
ADDRLP4 40
ADDRFP4 4
INDIRU4
CNSTI4 3
RSHU4
ADDRFP4 8
INDIRU4
MULU4
CVUI4 4
ASGNI4
line 268
;268:            pl = med3(pl, pl + d, pl + 2 * d, cmp);
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 56
ADDRLP4 40
INDIRI4
ASGNI4
ADDRLP4 56
INDIRI4
ADDRLP4 0
INDIRP4
ADDP4
ARGP4
ADDRLP4 56
INDIRI4
CNSTI4 1
LSHI4
ADDRLP4 0
INDIRP4
ADDP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 60
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 60
INDIRP4
ASGNP4
line 269
;269:            pm = med3(pm - d, pm, pm + d, cmp);
ADDRLP4 68
ADDRLP4 40
INDIRI4
ASGNI4
ADDRLP4 32
INDIRP4
ADDRLP4 68
INDIRI4
SUBP4
ARGP4
ADDRLP4 32
INDIRP4
ARGP4
ADDRLP4 68
INDIRI4
ADDRLP4 32
INDIRP4
ADDP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 72
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 32
ADDRLP4 72
INDIRP4
ASGNP4
line 270
;270:            pn = med3(pn - 2 * d, pn - d, pn, cmp);
ADDRLP4 80
ADDRLP4 40
INDIRI4
ASGNI4
ADDRLP4 36
INDIRP4
ADDRLP4 80
INDIRI4
CNSTI4 1
LSHI4
SUBP4
ARGP4
ADDRLP4 36
INDIRP4
ADDRLP4 80
INDIRI4
SUBP4
ARGP4
ADDRLP4 36
INDIRP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 84
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 36
ADDRLP4 84
INDIRP4
ASGNP4
line 271
;271:        }
LABELV $84
line 272
;272:        pm = med3(pl, pm, pn, cmp);
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 32
INDIRP4
ARGP4
ADDRLP4 36
INDIRP4
ARGP4
ADDRFP4 12
INDIRP4
ARGP4
ADDRLP4 52
ADDRGP4 med3
CALLP4
ASGNP4
ADDRLP4 32
ADDRLP4 52
INDIRP4
ASGNP4
line 273
;273:    }
LABELV $82
line 274
;274:    swap(a, pm);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $86
ADDRLP4 52
ADDRFP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRFP4 0
INDIRP4
ADDRLP4 32
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 32
INDIRP4
ADDRLP4 52
INDIRI4
ASGNI4
ADDRGP4 $87
JUMPV
LABELV $86
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 32
INDIRP4
ARGP4
ADDRFP4 8
INDIRU4
CVUI4 4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $87
line 275
;275:    pa = pb = (char*)a + es;
ADDRLP4 52
ADDRFP4 8
INDIRU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRLP4 4
ADDRLP4 52
INDIRP4
ASGNP4
ADDRLP4 20
ADDRLP4 52
INDIRP4
ASGNP4
line 277
;276:
;277:    pc = pd = (char*)a + (n - 1) * es;
ADDRLP4 56
ADDRFP4 4
INDIRU4
CNSTU4 1
SUBU4
ADDRFP4 8
INDIRU4
MULU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRLP4 24
ADDRLP4 56
INDIRP4
ASGNP4
ADDRLP4 8
ADDRLP4 56
INDIRP4
ASGNP4
line 278
;278:    for (;;)
line 279
;279:    {
ADDRGP4 $93
JUMPV
LABELV $92
line 281
;280:        while (pb <= pc && (r = cmp(pb, a)) <= 0)
;281:        {
line 282
;282:            if (r == 0)
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $95
line 283
;283:            {
line 284
;284:                swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 285
;285:                swap(pa, pb);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $97
ADDRLP4 60
ADDRLP4 20
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 20
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 4
INDIRP4
ADDRLP4 60
INDIRI4
ASGNI4
ADDRGP4 $98
JUMPV
LABELV $97
ADDRLP4 20
INDIRP4
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRU4
CVUI4 4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $98
line 286
;286:                pa += es;
ADDRLP4 20
ADDRFP4 8
INDIRU4
ADDRLP4 20
INDIRP4
ADDP4
ASGNP4
line 287
;287:            }
LABELV $95
line 288
;288:            pb += es;
ADDRLP4 4
ADDRFP4 8
INDIRU4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
line 289
;289:        }
LABELV $93
line 280
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
GTU4 $99
ADDRLP4 4
INDIRP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 64
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 64
INDIRI4
ASGNI4
ADDRLP4 64
INDIRI4
CNSTI4 0
LEI4 $92
LABELV $99
ADDRGP4 $101
JUMPV
LABELV $100
line 291
;290:        while (pb <= pc && (r = cmp(pc, a)) >= 0)
;291:        {
line 292
;292:            if (r == 0)
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $103
line 293
;293:            {
line 294
;294:                swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 295
;295:                swap(pc, pd);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $105
ADDRLP4 68
ADDRLP4 8
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 8
INDIRP4
ADDRLP4 24
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 24
INDIRP4
ADDRLP4 68
INDIRI4
ASGNI4
ADDRGP4 $106
JUMPV
LABELV $105
ADDRLP4 8
INDIRP4
ARGP4
ADDRLP4 24
INDIRP4
ARGP4
ADDRFP4 8
INDIRU4
CVUI4 4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $106
line 296
;296:                pd -= es;
ADDRLP4 24
ADDRLP4 24
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ASGNP4
line 297
;297:            }
LABELV $103
line 298
;298:            pc -= es;
ADDRLP4 8
ADDRLP4 8
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ASGNP4
line 299
;299:        }
LABELV $101
line 290
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
GTU4 $107
ADDRLP4 8
INDIRP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 72
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 72
INDIRI4
ASGNI4
ADDRLP4 72
INDIRI4
CNSTI4 0
GEI4 $100
LABELV $107
line 300
;300:        if (pb > pc)
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
LEU4 $108
line 301
;301:            break;
ADDRGP4 $90
JUMPV
LABELV $108
line 302
;302:        swap(pb, pc);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $110
ADDRLP4 76
ADDRLP4 4
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 4
INDIRP4
ADDRLP4 8
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 8
INDIRP4
ADDRLP4 76
INDIRI4
ASGNI4
ADDRGP4 $111
JUMPV
LABELV $110
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 8
INDIRP4
ARGP4
ADDRFP4 8
INDIRU4
CVUI4 4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $111
line 303
;303:        swap_cnt = 1;
ADDRLP4 28
CNSTI4 1
ASGNI4
line 304
;304:        pb += es;
ADDRLP4 4
ADDRFP4 8
INDIRU4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
line 305
;305:        pc -= es;
ADDRLP4 8
ADDRLP4 8
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ASGNP4
line 306
;306:    }
line 278
ADDRGP4 $93
JUMPV
LABELV $90
line 307
;307:    if (swap_cnt == 0)
ADDRLP4 28
INDIRI4
CNSTI4 0
NEI4 $112
line 308
;308:    { /* Switch to insertion sort */
line 309
;309:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
ADDRLP4 32
ADDRFP4 8
INDIRU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
ADDRGP4 $117
JUMPV
LABELV $114
line 310
;310:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
ADDRLP4 0
ADDRLP4 32
INDIRP4
ASGNP4
ADDRGP4 $121
JUMPV
LABELV $118
line 311
;311:                swap(pl, pl - es);
ADDRLP4 16
INDIRI4
CNSTI4 0
NEI4 $122
ADDRLP4 60
ADDRLP4 0
INDIRP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
INDIRI4
ASGNI4
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ADDRLP4 60
INDIRI4
ASGNI4
ADDRGP4 $123
JUMPV
LABELV $122
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 64
ADDRFP4 8
INDIRU4
ASGNU4
ADDRLP4 0
INDIRP4
ADDRLP4 64
INDIRU4
SUBP4
ARGP4
ADDRLP4 64
INDIRU4
CVUI4 4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $123
LABELV $119
line 310
ADDRLP4 0
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ASGNP4
LABELV $121
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
LEU4 $124
ADDRLP4 0
INDIRP4
ADDRFP4 8
INDIRU4
SUBP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 72
ADDRFP4 12
INDIRP4
CALLI4
ASGNI4
ADDRLP4 72
INDIRI4
CNSTI4 0
GTI4 $118
LABELV $124
LABELV $115
line 309
ADDRLP4 32
ADDRFP4 8
INDIRU4
ADDRLP4 32
INDIRP4
ADDP4
ASGNP4
LABELV $117
ADDRLP4 32
INDIRP4
CVPU4 4
ADDRFP4 4
INDIRU4
ADDRFP4 8
INDIRU4
MULU4
ADDRFP4 0
INDIRP4
ADDP4
CVPU4 4
LTU4 $114
line 312
;312:        return;
ADDRGP4 $60
JUMPV
LABELV $112
line 315
;313:    }
;314:
;315:    pn = (char*)a + n * es;
ADDRLP4 36
ADDRFP4 4
INDIRU4
ADDRFP4 8
INDIRU4
MULU4
ADDRFP4 0
INDIRP4
ADDP4
ASGNP4
line 316
;316:    r  = min(pa - (char*)a, pb - pa);
ADDRLP4 64
ADDRLP4 20
INDIRP4
CVPU4 4
ASGNU4
ADDRLP4 64
INDIRU4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 64
INDIRU4
SUBU4
CVUI4 4
GEI4 $126
ADDRLP4 60
ADDRLP4 20
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRGP4 $127
JUMPV
LABELV $126
ADDRLP4 60
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 20
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
LABELV $127
ADDRLP4 12
ADDRLP4 60
INDIRI4
ASGNI4
line 317
;317:    vecswap(a, pb - r, r);
ADDRLP4 12
INDIRI4
CNSTI4 0
LEI4 $128
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $128
line 318
;318:    r = min(pd - pc, pn - pd - es);
ADDRLP4 76
ADDRLP4 24
INDIRP4
CVPU4 4
ASGNU4
ADDRLP4 76
INDIRU4
ADDRLP4 8
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
CVIU4 4
ADDRLP4 36
INDIRP4
CVPU4 4
ADDRLP4 76
INDIRU4
SUBU4
CVUI4 4
CVIU4 4
ADDRFP4 8
INDIRU4
SUBU4
GEU4 $131
ADDRLP4 72
ADDRLP4 24
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
CVIU4 4
ASGNU4
ADDRGP4 $132
JUMPV
LABELV $131
ADDRLP4 72
ADDRLP4 36
INDIRP4
CVPU4 4
ADDRLP4 24
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
CVIU4 4
ADDRFP4 8
INDIRU4
SUBU4
ASGNU4
LABELV $132
ADDRLP4 12
ADDRLP4 72
INDIRU4
CVUI4 4
ASGNI4
line 319
;319:    vecswap(pb, pn - r, r);
ADDRLP4 12
INDIRI4
CNSTI4 0
LEI4 $133
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 36
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 swapfunc
CALLV
pop
LABELV $133
line 320
;320:    if ((r = pb - pa) > es)
ADDRLP4 84
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRLP4 20
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRLP4 12
ADDRLP4 84
INDIRI4
ASGNI4
ADDRLP4 84
INDIRI4
CVIU4 4
ADDRFP4 8
INDIRU4
LEU4 $135
line 321
;321:        qsort(a, r / es, es, cmp);
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 88
ADDRFP4 8
INDIRU4
ASGNU4
ADDRLP4 12
INDIRI4
CVIU4 4
ADDRLP4 88
INDIRU4
DIVU4
ARGU4
ADDRLP4 88
INDIRU4
ARGU4
ADDRFP4 12
INDIRP4
ARGP4
ADDRGP4 qsort
CALLV
pop
LABELV $135
line 322
;322:    if ((r = pd - pc) > es)
ADDRLP4 92
ADDRLP4 24
INDIRP4
CVPU4 4
ADDRLP4 8
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
ASGNI4
ADDRLP4 12
ADDRLP4 92
INDIRI4
ASGNI4
ADDRLP4 92
INDIRI4
CVIU4 4
ADDRFP4 8
INDIRU4
LEU4 $137
line 323
;323:    {
line 325
;324:        /* Iterate rather than recurse to save stack space */
;325:        a = pn - r;
ADDRFP4 0
ADDRLP4 36
INDIRP4
ADDRLP4 12
INDIRI4
SUBP4
ASGNP4
line 326
;326:        n = r / es;
ADDRFP4 4
ADDRLP4 12
INDIRI4
CVIU4 4
ADDRFP4 8
INDIRU4
DIVU4
ASGNU4
line 327
;327:        goto loop;
ADDRGP4 $61
JUMPV
LABELV $137
line 330
;328:    }
;329:    /*      qsort(pn - r, r / es, es, cmp);*/
;330:}
LABELV $60
endproc qsort 96 16
export atof
proc atof 32 0
line 334
;331:
;332:
;333:double atof(const char* string)
;334:{
ADDRGP4 $141
JUMPV
LABELV $140
line 341
;335:    float sign;
;336:    float value;
;337:    int   c;
;338:
;339:    // skip whitespace
;340:    while (*string <= ' ')
;341:    {
line 342
;342:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $143
line 343
;343:        {
line 344
;344:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $139
JUMPV
LABELV $143
line 346
;345:        }
;346:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 347
;347:    }
LABELV $141
line 340
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $140
line 350
;348:
;349:    // check sign
;350:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $148
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $149
ADDRGP4 $145
JUMPV
line 351
;351:    {
LABELV $148
line 353
;352:    case '+':
;353:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 354
;354:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 355
;355:        break;
ADDRGP4 $146
JUMPV
LABELV $149
line 357
;356:    case '-':
;357:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 358
;358:        sign = -1;
ADDRLP4 8
CNSTF4 3212836864
ASGNF4
line 359
;359:        break;
ADDRGP4 $146
JUMPV
LABELV $145
line 361
;360:    default:
;361:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 362
;362:        break;
LABELV $146
line 366
;363:    }
;364:
;365:    // read digits
;366:    value = 0;
ADDRLP4 4
CNSTF4 0
ASGNF4
line 367
;367:    c     = string[0];
ADDRLP4 0
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 368
;368:    if (c != '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
EQI4 $150
line 369
;369:    {
LABELV $152
line 371
;370:        do
;371:        {
line 372
;372:            c = *string++;
ADDRLP4 20
ADDRFP4 0
INDIRP4
ASGNP4
ADDRFP4 0
ADDRLP4 20
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 20
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 373
;373:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $157
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $155
LABELV $157
line 374
;374:            {
line 375
;375:                break;
ADDRGP4 $151
JUMPV
LABELV $155
line 377
;376:            }
;377:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 378
;378:            value = value * 10 + c;
ADDRLP4 4
ADDRLP4 4
INDIRF4
CNSTF4 1092616192
MULF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDF4
ASGNF4
line 379
;379:        } while (1);
LABELV $153
ADDRGP4 $152
JUMPV
line 380
;380:    }
ADDRGP4 $151
JUMPV
LABELV $150
line 382
;381:    else
;382:    {
line 383
;383:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 384
;384:    }
LABELV $151
line 387
;385:
;386:    // check for decimal point
;387:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $158
line 388
;388:    {
line 391
;389:        double fraction;
;390:
;391:        fraction = 0.1;
ADDRLP4 20
CNSTF4 1036831949
ASGNF4
LABELV $160
line 393
;392:        do
;393:        {
line 394
;394:            c = *string++;
ADDRLP4 24
ADDRFP4 0
INDIRP4
ASGNP4
ADDRFP4 0
ADDRLP4 24
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 24
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 395
;395:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $165
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $163
LABELV $165
line 396
;396:            {
line 397
;397:                break;
ADDRGP4 $162
JUMPV
LABELV $163
line 399
;398:            }
;399:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 400
;400:            value += c * fraction;
ADDRLP4 4
ADDRLP4 4
INDIRF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDRLP4 20
INDIRF4
MULF4
ADDF4
ASGNF4
line 401
;401:            fraction *= 0.1;
ADDRLP4 20
ADDRLP4 20
INDIRF4
CNSTF4 1036831949
MULF4
ASGNF4
line 402
;402:        } while (1);
LABELV $161
ADDRGP4 $160
JUMPV
LABELV $162
line 403
;403:    }
LABELV $158
line 407
;404:
;405:    // not handling 10e10 notation...
;406:
;407:    return value * sign;
ADDRLP4 4
INDIRF4
ADDRLP4 8
INDIRF4
MULF4
RETF4
LABELV $139
endproc atof 32 0
export _atof
proc _atof 36 0
line 411
;408:}
;409:
;410:double _atof(const char** stringPtr)
;411:{
line 415
;412:    const char* string;
;413:    float       sign;
;414:    float       value;
;415:    int         c = '0'; // bk001211 - uninitialized use possible
ADDRLP4 0
CNSTI4 48
ASGNI4
line 417
;416:
;417:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $168
JUMPV
LABELV $167
line 421
;418:
;419:    // skip whitespace
;420:    while (*string <= ' ')
;421:    {
line 422
;422:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $170
line 423
;423:        {
line 424
;424:            *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 425
;425:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $166
JUMPV
LABELV $170
line 427
;426:        }
;427:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 428
;428:    }
LABELV $168
line 420
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $167
line 431
;429:
;430:    // check sign
;431:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $175
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $176
ADDRGP4 $172
JUMPV
line 432
;432:    {
LABELV $175
line 434
;433:    case '+':
;434:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 435
;435:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 436
;436:        break;
ADDRGP4 $173
JUMPV
LABELV $176
line 438
;437:    case '-':
;438:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 439
;439:        sign = -1;
ADDRLP4 12
CNSTF4 3212836864
ASGNF4
line 440
;440:        break;
ADDRGP4 $173
JUMPV
LABELV $172
line 442
;441:    default:
;442:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 443
;443:        break;
LABELV $173
line 447
;444:    }
;445:
;446:    // read digits
;447:    value = 0;
ADDRLP4 8
CNSTF4 0
ASGNF4
line 448
;448:    if (string[0] != '.')
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 46
EQI4 $177
line 449
;449:    {
LABELV $179
line 451
;450:        do
;451:        {
line 452
;452:            c = *string++;
ADDRLP4 24
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 24
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 24
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 453
;453:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $184
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $182
LABELV $184
line 454
;454:            {
line 455
;455:                break;
ADDRGP4 $181
JUMPV
LABELV $182
line 457
;456:            }
;457:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 458
;458:            value = value * 10 + c;
ADDRLP4 8
ADDRLP4 8
INDIRF4
CNSTF4 1092616192
MULF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDF4
ASGNF4
line 459
;459:        } while (1);
LABELV $180
ADDRGP4 $179
JUMPV
LABELV $181
line 460
;460:    }
LABELV $177
line 463
;461:
;462:    // check for decimal point
;463:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $185
line 464
;464:    {
line 467
;465:        double fraction;
;466:
;467:        fraction = 0.1;
ADDRLP4 24
CNSTF4 1036831949
ASGNF4
LABELV $187
line 469
;468:        do
;469:        {
line 470
;470:            c = *string++;
ADDRLP4 28
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 28
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 28
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 471
;471:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $192
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $190
LABELV $192
line 472
;472:            {
line 473
;473:                break;
ADDRGP4 $189
JUMPV
LABELV $190
line 475
;474:            }
;475:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 476
;476:            value += c * fraction;
ADDRLP4 8
ADDRLP4 8
INDIRF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDRLP4 24
INDIRF4
MULF4
ADDF4
ASGNF4
line 477
;477:            fraction *= 0.1;
ADDRLP4 24
ADDRLP4 24
INDIRF4
CNSTF4 1036831949
MULF4
ASGNF4
line 478
;478:        } while (1);
LABELV $188
ADDRGP4 $187
JUMPV
LABELV $189
line 479
;479:    }
LABELV $185
line 482
;480:
;481:    // not handling 10e10 notation...
;482:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 484
;483:
;484:    return value * sign;
ADDRLP4 8
INDIRF4
ADDRLP4 12
INDIRF4
MULF4
RETF4
LABELV $166
endproc _atof 36 0
export atoi
proc atoi 28 0
line 490
;485:}
;486:
;487:#ifdef Q3_VM
;488:
;489:int atoi(const char* string)
;490:{
ADDRGP4 $195
JUMPV
LABELV $194
line 497
;491:    int sign;
;492:    int value;
;493:    int c;
;494:
;495:    // skip whitespace
;496:    while (*string <= ' ')
;497:    {
line 498
;498:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $197
line 499
;499:        {
line 500
;500:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $193
JUMPV
LABELV $197
line 502
;501:        }
;502:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 503
;503:    }
LABELV $195
line 496
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $194
line 506
;504:
;505:    // check sign
;506:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $202
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $203
ADDRGP4 $199
JUMPV
line 507
;507:    {
LABELV $202
line 509
;508:    case '+':
;509:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 510
;510:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 511
;511:        break;
ADDRGP4 $200
JUMPV
LABELV $203
line 513
;512:    case '-':
;513:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 514
;514:        sign = -1;
ADDRLP4 8
CNSTI4 -1
ASGNI4
line 515
;515:        break;
ADDRGP4 $200
JUMPV
LABELV $199
line 517
;516:    default:
;517:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 518
;518:        break;
LABELV $200
line 522
;519:    }
;520:
;521:    // read digits
;522:    value = 0;
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $204
line 524
;523:    do
;524:    {
line 525
;525:        c = *string++;
ADDRLP4 20
ADDRFP4 0
INDIRP4
ASGNP4
ADDRFP4 0
ADDRLP4 20
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 20
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 526
;526:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $209
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $207
LABELV $209
line 527
;527:        {
line 528
;528:            break;
ADDRGP4 $206
JUMPV
LABELV $207
line 530
;529:        }
;530:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 531
;531:        value = value * 10 + c;
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 10
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 532
;532:    } while (1);
LABELV $205
ADDRGP4 $204
JUMPV
LABELV $206
line 536
;533:
;534:    // not handling 10e10 notation...
;535:
;536:    return value * sign;
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
MULI4
RETI4
LABELV $193
endproc atoi 28 0
export _atoi
proc _atoi 32 0
line 540
;537:}
;538:
;539:int _atoi(const char** stringPtr)
;540:{
line 546
;541:    int         sign;
;542:    int         value;
;543:    int         c;
;544:    const char* string;
;545:
;546:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $212
JUMPV
LABELV $211
line 550
;547:
;548:    // skip whitespace
;549:    while (*string <= ' ')
;550:    {
line 551
;551:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $214
line 552
;552:        {
line 553
;553:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $210
JUMPV
LABELV $214
line 555
;554:        }
;555:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 556
;556:    }
LABELV $212
line 549
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $211
line 559
;557:
;558:    // check sign
;559:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $219
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $220
ADDRGP4 $216
JUMPV
line 560
;560:    {
LABELV $219
line 562
;561:    case '+':
;562:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 563
;563:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 564
;564:        break;
ADDRGP4 $217
JUMPV
LABELV $220
line 566
;565:    case '-':
;566:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 567
;567:        sign = -1;
ADDRLP4 12
CNSTI4 -1
ASGNI4
line 568
;568:        break;
ADDRGP4 $217
JUMPV
LABELV $216
line 570
;569:    default:
;570:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 571
;571:        break;
LABELV $217
line 575
;572:    }
;573:
;574:    // read digits
;575:    value = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $221
line 577
;576:    do
;577:    {
line 578
;578:        c = *string++;
ADDRLP4 24
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 24
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 24
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 579
;579:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $226
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $224
LABELV $226
line 580
;580:        {
line 581
;581:            break;
ADDRGP4 $223
JUMPV
LABELV $224
line 583
;582:        }
;583:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 584
;584:        value = value * 10 + c;
ADDRLP4 8
ADDRLP4 8
INDIRI4
CNSTI4 10
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 585
;585:    } while (1);
LABELV $222
ADDRGP4 $221
JUMPV
LABELV $223
line 589
;586:
;587:    // not handling 10e10 notation...
;588:
;589:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 591
;590:
;591:    return value * sign;
ADDRLP4 8
INDIRI4
ADDRLP4 12
INDIRI4
MULI4
RETI4
LABELV $210
endproc _atoi 32 0
export tan
proc tan 8 4
line 596
;592:}
;593:
;594:#ifdef Q3_VM
;595:double tan(double x)
;596:{
line 597
;597:    return sin(x) / cos(x);
ADDRFP4 0
INDIRF4
ARGF4
ADDRLP4 0
ADDRGP4 sin
CALLI4
ASGNI4
ADDRFP4 0
INDIRF4
ARGF4
ADDRLP4 4
ADDRGP4 cos
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 4
INDIRI4
DIVI4
CVIF4 4
RETF4
LABELV $227
endproc tan 8 4
data
align 4
LABELV randSeed
byte 4 0
export srand
code
proc srand 0 0
line 604
;598:}
;599:#endif
;600:
;601:static int randSeed = 0;
;602:
;603:void srand(unsigned seed)
;604:{
line 605
;605:    randSeed = seed;
ADDRGP4 randSeed
ADDRFP4 0
INDIRU4
CVUI4 4
ASGNI4
line 606
;606:}
LABELV $228
endproc srand 0 0
export rand
proc rand 4 0
line 609
;607:
;608:int rand(void)
;609:{
line 610
;610:    randSeed = (69069 * randSeed + 1);
ADDRLP4 0
ADDRGP4 randSeed
ASGNP4
ADDRLP4 0
INDIRP4
ADDRLP4 0
INDIRP4
INDIRI4
CNSTI4 69069
MULI4
CNSTI4 1
ADDI4
ASGNI4
line 611
;611:    return randSeed & 0x7fff;
ADDRGP4 randSeed
INDIRI4
CNSTI4 32767
BANDI4
RETI4
LABELV $229
endproc rand 4 0
export abs
proc abs 4 0
line 616
;612:}
;613:
;614:
;615:int abs(int n)
;616:{
line 617
;617:    return n < 0 ? -n : n;
ADDRFP4 0
INDIRI4
CNSTI4 0
GEI4 $232
ADDRLP4 0
ADDRFP4 0
INDIRI4
NEGI4
ASGNI4
ADDRGP4 $233
JUMPV
LABELV $232
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
LABELV $233
ADDRLP4 0
INDIRI4
RETI4
LABELV $230
endproc abs 4 0
export fabs
proc fabs 4 0
line 621
;618:}
;619:
;620:double fabs(double x)
;621:{
line 622
;622:    return x < 0 ? -x : x;
ADDRFP4 0
INDIRF4
CNSTF4 0
GEF4 $236
ADDRLP4 0
ADDRFP4 0
INDIRF4
NEGF4
ASGNF4
ADDRGP4 $237
JUMPV
LABELV $236
ADDRLP4 0
ADDRFP4 0
INDIRF4
ASGNF4
LABELV $237
ADDRLP4 0
INDIRF4
RETF4
LABELV $234
endproc fabs 4 0
export AddInt
proc AddInt 56 0
line 640
;623:}
;624:
;625:#define ALT 0x00000001       /* alternate form */
;626:#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
;627:#define LADJUST 0x00000004   /* left adjustment */
;628:#define LONGDBL 0x00000008   /* long double */
;629:#define LONGINT 0x00000010   /* long integer */
;630:#define QUADINT 0x00000020   /* quad integer */
;631:#define SHORTINT 0x00000040  /* short integer */
;632:#define ZEROPAD 0x00000080   /* zero (as opposed to blank) pad */
;633:#define FPT 0x00000100       /* floating point number */
;634:
;635:#define to_digit(c) ((c) - '0')
;636:#define is_digit(c) ((unsigned)to_digit(c) <= 9)
;637:#define to_char(n) ((n) + '0')
;638:
;639:void AddInt(char** buf_p, int val, int width, int flags)
;640:{
line 646
;641:    char  text[32];
;642:    int   digits;
;643:    int   signedVal;
;644:    char* buf;
;645:
;646:    digits    = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 647
;647:    signedVal = val;
ADDRLP4 40
ADDRFP4 4
INDIRI4
ASGNI4
line 648
;648:    if (val < 0)
ADDRFP4 4
INDIRI4
CNSTI4 0
GEI4 $239
line 649
;649:    {
line 650
;650:        val = -val;
ADDRFP4 4
ADDRFP4 4
INDIRI4
NEGI4
ASGNI4
line 651
;651:    }
LABELV $239
LABELV $241
line 653
;652:    do
;653:    {
line 654
;654:        text[digits++] = '0' + val % 10;
ADDRLP4 44
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 44
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 44
INDIRI4
ADDRLP4 8
ADDP4
ADDRFP4 4
INDIRI4
CNSTI4 10
MODI4
CNSTI4 48
ADDI4
CVII1 4
ASGNI1
line 655
;655:        val /= 10;
ADDRFP4 4
ADDRFP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 656
;656:    } while (val);
LABELV $242
ADDRFP4 4
INDIRI4
CNSTI4 0
NEI4 $241
line 658
;657:
;658:    if (signedVal < 0)
ADDRLP4 40
INDIRI4
CNSTI4 0
GEI4 $244
line 659
;659:    {
line 660
;660:        text[digits++] = '-';
ADDRLP4 44
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 44
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 44
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 45
ASGNI1
line 661
;661:    }
LABELV $244
line 663
;662:
;663:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 665
;664:
;665:    if (!(flags & LADJUST))
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
NEI4 $255
line 666
;666:    {
ADDRGP4 $249
JUMPV
LABELV $248
line 668
;667:        while (digits < width)
;668:        {
line 669
;669:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
ADDRLP4 48
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRFP4 12
INDIRI4
CNSTI4 128
BANDI4
CNSTI4 0
EQI4 $252
ADDRLP4 44
CNSTI4 48
ASGNI4
ADDRGP4 $253
JUMPV
LABELV $252
ADDRLP4 44
CNSTI4 32
ASGNI4
LABELV $253
ADDRLP4 48
INDIRP4
ADDRLP4 44
INDIRI4
CVII1 4
ASGNI1
line 670
;670:            width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 671
;671:        }
LABELV $249
line 667
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $248
line 672
;672:    }
ADDRGP4 $255
JUMPV
LABELV $254
line 675
;673:
;674:    while (digits--)
;675:    {
line 676
;676:        *buf++ = text[digits];
ADDRLP4 44
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 44
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 44
INDIRP4
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 677
;677:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 678
;678:    }
LABELV $255
line 674
ADDRLP4 44
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 44
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 44
INDIRI4
CNSTI4 0
NEI4 $254
line 680
;679:
;680:    if (flags & LADJUST)
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
EQI4 $257
line 681
;681:    {
ADDRGP4 $260
JUMPV
LABELV $259
line 683
;682:        while (width--)
;683:        {
line 684
;684:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
ADDRLP4 52
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 52
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRFP4 12
INDIRI4
CNSTI4 128
BANDI4
CNSTI4 0
EQI4 $263
ADDRLP4 48
CNSTI4 48
ASGNI4
ADDRGP4 $264
JUMPV
LABELV $263
ADDRLP4 48
CNSTI4 32
ASGNI4
LABELV $264
ADDRLP4 52
INDIRP4
ADDRLP4 48
INDIRI4
CVII1 4
ASGNI1
line 685
;685:        }
LABELV $260
line 682
ADDRLP4 48
ADDRFP4 8
INDIRI4
ASGNI4
ADDRFP4 8
ADDRLP4 48
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 48
INDIRI4
CNSTI4 0
NEI4 $259
line 686
;686:    }
LABELV $257
line 688
;687:
;688:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 689
;689:}
LABELV $238
endproc AddInt 56 0
export AddFloat
proc AddFloat 60 0
line 692
;690:
;691:void AddFloat(char** buf_p, float fval, int width, int prec)
;692:{
line 700
;693:    char  text[32];
;694:    int   digits;
;695:    float signedVal;
;696:    char* buf;
;697:    int   val;
;698:
;699:    // get the sign
;700:    signedVal = fval;
ADDRLP4 44
ADDRFP4 4
INDIRF4
ASGNF4
line 701
;701:    if (fval < 0)
ADDRFP4 4
INDIRF4
CNSTF4 0
GEF4 $266
line 702
;702:    {
line 703
;703:        fval = -fval;
ADDRFP4 4
ADDRFP4 4
INDIRF4
NEGF4
ASGNF4
line 704
;704:    }
LABELV $266
line 707
;705:
;706:    // write the float number
;707:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 708
;708:    val    = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
LABELV $268
line 710
;709:    do
;710:    {
line 711
;711:        text[digits++] = '0' + val % 10;
ADDRLP4 48
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 48
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 48
INDIRI4
ADDRLP4 8
ADDP4
ADDRLP4 4
INDIRI4
CNSTI4 10
MODI4
CNSTI4 48
ADDI4
CVII1 4
ASGNI1
line 712
;712:        val /= 10;
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 713
;713:    } while (val);
LABELV $269
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $268
line 715
;714:
;715:    if (signedVal < 0)
ADDRLP4 44
INDIRF4
CNSTF4 0
GEF4 $271
line 716
;716:    {
line 717
;717:        text[digits++] = '-';
ADDRLP4 48
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 48
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 48
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 45
ASGNI1
line 718
;718:    }
LABELV $271
line 720
;719:
;720:    buf = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $274
JUMPV
LABELV $273
line 723
;721:
;722:    while (digits < width)
;723:    {
line 724
;724:        *buf++ = ' ';
ADDRLP4 48
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 48
INDIRP4
CNSTI1 32
ASGNI1
line 725
;725:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 726
;726:    }
LABELV $274
line 722
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $273
ADDRGP4 $277
JUMPV
LABELV $276
line 729
;727:
;728:    while (digits--)
;729:    {
line 730
;730:        *buf++ = text[digits];
ADDRLP4 48
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 48
INDIRP4
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 731
;731:    }
LABELV $277
line 728
ADDRLP4 48
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 48
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 48
INDIRI4
CNSTI4 0
NEI4 $276
line 733
;732:
;733:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 735
;734:
;735:    if (prec < 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
GEI4 $279
line 736
;736:        prec = 6;
ADDRFP4 12
CNSTI4 6
ASGNI4
LABELV $279
line 738
;737:    // write the fraction
;738:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $282
JUMPV
LABELV $281
line 740
;739:    while (digits < prec)
;740:    {
line 741
;741:        fval -= (int)fval;
ADDRLP4 52
ADDRFP4 4
INDIRF4
ASGNF4
ADDRFP4 4
ADDRLP4 52
INDIRF4
ADDRLP4 52
INDIRF4
CVFI4 4
CVIF4 4
SUBF4
ASGNF4
line 742
;742:        fval *= 10.0;
ADDRFP4 4
ADDRFP4 4
INDIRF4
CNSTF4 1092616192
MULF4
ASGNF4
line 743
;743:        val            = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
line 744
;744:        text[digits++] = '0' + val % 10;
ADDRLP4 56
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 56
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 56
INDIRI4
ADDRLP4 8
ADDP4
ADDRLP4 4
INDIRI4
CNSTI4 10
MODI4
CNSTI4 48
ADDI4
CVII1 4
ASGNI1
line 745
;745:    }
LABELV $282
line 739
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $281
line 747
;746:
;747:    if (digits > 0)
ADDRLP4 0
INDIRI4
CNSTI4 0
LEI4 $284
line 748
;748:    {
line 749
;749:        buf    = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 750
;750:        *buf++ = '.';
ADDRLP4 52
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 52
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 52
INDIRP4
CNSTI1 46
ASGNI1
line 751
;751:        for (prec = 0; prec < digits; prec++)
ADDRFP4 12
CNSTI4 0
ASGNI4
ADDRGP4 $289
JUMPV
LABELV $286
line 752
;752:        {
line 753
;753:            *buf++ = text[prec];
ADDRLP4 56
ADDRLP4 40
INDIRP4
ASGNP4
ADDRLP4 40
ADDRLP4 56
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 56
INDIRP4
ADDRFP4 12
INDIRI4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 754
;754:        }
LABELV $287
line 751
ADDRFP4 12
ADDRFP4 12
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $289
ADDRFP4 12
INDIRI4
ADDRLP4 0
INDIRI4
LTI4 $286
line 755
;755:        *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 756
;756:    }
LABELV $284
line 757
;757:}
LABELV $265
endproc AddFloat 60 0
export AddString
proc AddString 16 4
line 760
;758:
;759:void AddString(char** buf_p, char* string, int width, int prec)
;760:{
line 764
;761:    int   size;
;762:    char* buf;
;763:
;764:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 766
;765:
;766:    if (string == NULL)
ADDRFP4 4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $291
line 767
;767:    {
line 768
;768:        string = "(null)";
ADDRFP4 4
ADDRGP4 $293
ASGNP4
line 769
;769:        prec   = -1;
ADDRFP4 12
CNSTI4 -1
ASGNI4
line 770
;770:    }
LABELV $291
line 772
;771:
;772:    if (prec >= 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
LTI4 $294
line 773
;773:    {
line 774
;774:        for (size = 0; size < prec; size++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $299
JUMPV
LABELV $296
line 775
;775:        {
line 776
;776:            if (string[size] == '\0')
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $300
line 777
;777:            {
line 778
;778:                break;
ADDRGP4 $295
JUMPV
LABELV $300
line 780
;779:            }
;780:        }
LABELV $297
line 774
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $299
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $296
line 781
;781:    }
ADDRGP4 $295
JUMPV
LABELV $294
line 783
;782:    else
;783:    {
line 784
;784:        size = strlen(string);
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 8
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 8
INDIRI4
ASGNI4
line 785
;785:    }
LABELV $295
line 787
;786:
;787:    width -= size;
ADDRFP4 8
ADDRFP4 8
INDIRI4
ADDRLP4 0
INDIRI4
SUBI4
ASGNI4
ADDRGP4 $303
JUMPV
LABELV $302
line 790
;788:
;789:    while (size--)
;790:    {
line 791
;791:        *buf++ = *string++;
ADDRLP4 8
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 8
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 12
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 12
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 8
INDIRP4
ADDRLP4 12
INDIRP4
INDIRI1
ASGNI1
line 792
;792:    }
LABELV $303
line 789
ADDRLP4 8
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
ADDRLP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 8
INDIRI4
CNSTI4 0
NEI4 $302
ADDRGP4 $306
JUMPV
LABELV $305
line 795
;793:
;794:    while (width-- > 0)
;795:    {
line 796
;796:        *buf++ = ' ';
ADDRLP4 12
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 12
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 12
INDIRP4
CNSTI1 32
ASGNI1
line 797
;797:    }
LABELV $306
line 794
ADDRLP4 12
ADDRFP4 8
INDIRI4
ASGNI4
ADDRFP4 8
ADDRLP4 12
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
GTI4 $305
line 799
;798:
;799:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 800
;800:}
LABELV $290
endproc AddString 16 4
export vsprintf
proc vsprintf 72 16
line 803
;801:
;802:int vsprintf(char* buffer, const char* fmt, va_list argptr)
;803:{
line 813
;804:    int*  arg;
;805:    char* buf_p;
;806:    char  ch;
;807:    int   flags;
;808:    int   width;
;809:    int   prec;
;810:    int   n;
;811:    char  sign;
;812:
;813:    buf_p = buffer;
ADDRLP4 4
ADDRFP4 0
INDIRP4
ASGNP4
line 814
;814:    arg   = (int*)argptr;
ADDRLP4 24
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $310
JUMPV
LABELV $309
line 817
;815:
;816:    while (1)
;817:    {
line 819
;818:        // run through the format string until we hit a '%' or '\0'
;819:        for (ch = *fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
ADDRLP4 0
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRGP4 $315
JUMPV
LABELV $312
line 820
;820:        {
line 821
;821:            *buf_p++ = ch;
ADDRLP4 32
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 32
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 32
INDIRP4
ADDRLP4 0
INDIRI1
ASGNI1
line 822
;822:        }
LABELV $313
line 819
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
LABELV $315
ADDRLP4 29
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 0
ADDRLP4 29
INDIRI1
ASGNI1
ADDRLP4 29
INDIRI1
CVII4 1
CNSTI4 0
EQI4 $316
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 37
NEI4 $312
LABELV $316
line 823
;823:        if (ch == '\0')
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $317
line 824
;824:        {
line 825
;825:            goto done;
ADDRGP4 $319
JUMPV
LABELV $317
line 829
;826:        }
;827:
;828:        // skip over the '%'
;829:        fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 832
;830:
;831:        // reset formatting state
;832:        flags = 0;
ADDRLP4 16
CNSTI4 0
ASGNI4
line 833
;833:        width = 0;
ADDRLP4 12
CNSTI4 0
ASGNI4
line 834
;834:        prec  = -1;
ADDRLP4 20
CNSTI4 -1
ASGNI4
line 835
;835:        sign  = '\0';
ADDRLP4 28
CNSTI1 0
ASGNI1
LABELV $320
line 838
;836:
;837:    rflag:
;838:        ch = *fmt++;
ADDRLP4 32
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 32
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 32
INDIRP4
INDIRI1
ASGNI1
LABELV $321
line 840
;839:    reswitch:
;840:        switch (ch)
ADDRLP4 36
ADDRLP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 99
LTI4 $343
ADDRLP4 36
INDIRI4
CNSTI4 105
GTI4 $344
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $345-396
ADDP4
INDIRP4
JUMPV
data
align 4
LABELV $345
address $338
address $339
address $322
address $340
address $322
address $322
address $339
code
LABELV $343
ADDRLP4 36
INDIRI4
CNSTI4 37
LTI4 $322
ADDRLP4 36
INDIRI4
CNSTI4 57
GTI4 $322
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $347-148
ADDP4
INDIRP4
JUMPV
data
align 4
LABELV $347
address $342
address $322
address $322
address $322
address $322
address $322
address $322
address $322
address $325
address $326
address $322
address $333
address $334
address $334
address $334
address $334
address $334
address $334
address $334
address $334
address $334
code
LABELV $344
ADDRLP4 36
INDIRI4
CNSTI4 115
EQI4 $341
ADDRGP4 $322
JUMPV
line 841
;841:        {
LABELV $325
line 843
;842:        case '-':
;843:            flags |= LADJUST;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 4
BORI4
ASGNI4
line 844
;844:            goto rflag;
ADDRGP4 $320
JUMPV
LABELV $326
line 846
;845:        case '.':
;846:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $328
JUMPV
LABELV $327
line 848
;847:            while (is_digit((ch = *fmt++)))
;848:            {
line 849
;849:                n = 10 * n + (ch - '0');
ADDRLP4 8
ADDRLP4 8
INDIRI4
CNSTI4 10
MULI4
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
ADDI4
ASGNI4
line 850
;850:            }
LABELV $328
line 847
ADDRLP4 48
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 48
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 52
ADDRLP4 48
INDIRP4
INDIRI1
ASGNI1
ADDRLP4 0
ADDRLP4 52
INDIRI1
ASGNI1
ADDRLP4 52
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $327
line 851
;851:            prec = n < 0 ? -1 : n;
ADDRLP4 8
INDIRI4
CNSTI4 0
GEI4 $331
ADDRLP4 56
CNSTI4 -1
ASGNI4
ADDRGP4 $332
JUMPV
LABELV $331
ADDRLP4 56
ADDRLP4 8
INDIRI4
ASGNI4
LABELV $332
ADDRLP4 20
ADDRLP4 56
INDIRI4
ASGNI4
line 852
;852:            goto reswitch;
ADDRGP4 $321
JUMPV
LABELV $333
line 854
;853:        case '0':
;854:            flags |= ZEROPAD;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 128
BORI4
ASGNI4
line 855
;855:            goto rflag;
ADDRGP4 $320
JUMPV
LABELV $334
line 865
;856:        case '1':
;857:        case '2':
;858:        case '3':
;859:        case '4':
;860:        case '5':
;861:        case '6':
;862:        case '7':
;863:        case '8':
;864:        case '9':
;865:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $335
line 867
;866:            do
;867:            {
line 868
;868:                n  = 10 * n + (ch - '0');
ADDRLP4 8
ADDRLP4 8
INDIRI4
CNSTI4 10
MULI4
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
ADDI4
ASGNI4
line 869
;869:                ch = *fmt++;
ADDRLP4 60
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 60
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 0
ADDRLP4 60
INDIRP4
INDIRI1
ASGNI1
line 870
;870:            } while (is_digit(ch));
LABELV $336
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $335
line 871
;871:            width = n;
ADDRLP4 12
ADDRLP4 8
INDIRI4
ASGNI4
line 872
;872:            goto reswitch;
ADDRGP4 $321
JUMPV
LABELV $338
line 874
;873:        case 'c':
;874:            *buf_p++ = (char)*arg;
ADDRLP4 60
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 60
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 60
INDIRP4
ADDRLP4 24
INDIRP4
INDIRI4
CVII1 4
ASGNI1
line 875
;875:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 876
;876:            break;
ADDRGP4 $323
JUMPV
LABELV $339
line 879
;877:        case 'd':
;878:        case 'i':
;879:            AddInt(&buf_p, *arg, width, flags);
ADDRLP4 4
ARGP4
ADDRLP4 24
INDIRP4
INDIRI4
ARGI4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRGP4 AddInt
CALLV
pop
line 880
;880:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 881
;881:            break;
ADDRGP4 $323
JUMPV
LABELV $340
line 883
;882:        case 'f':
;883:            AddFloat(&buf_p, *(double*)arg, width, prec);
ADDRLP4 4
ARGP4
ADDRLP4 24
INDIRP4
INDIRF4
ARGF4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 20
INDIRI4
ARGI4
ADDRGP4 AddFloat
CALLV
pop
line 885
;884:#ifdef __LCC__
;885:            arg += 1; // everything is 32 bit in my compiler
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 889
;886:#else
;887:            arg += 2;
;888:#endif
;889:            break;
ADDRGP4 $323
JUMPV
LABELV $341
line 891
;890:        case 's':
;891:            AddString(&buf_p, (char*)*arg, width, prec);
ADDRLP4 4
ARGP4
ADDRLP4 24
INDIRP4
INDIRI4
CVIU4 4
CVUP4 4
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 20
INDIRI4
ARGI4
ADDRGP4 AddString
CALLV
pop
line 892
;892:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 893
;893:            break;
ADDRGP4 $323
JUMPV
LABELV $342
line 895
;894:        case '%':
;895:            *buf_p++ = ch;
ADDRLP4 64
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 64
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 64
INDIRP4
ADDRLP4 0
INDIRI1
ASGNI1
line 896
;896:            break;
ADDRGP4 $323
JUMPV
LABELV $322
line 898
;897:        default:
;898:            *buf_p++ = (char)*arg;
ADDRLP4 68
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 68
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 68
INDIRP4
ADDRLP4 24
INDIRP4
INDIRI4
CVII1 4
ASGNI1
line 899
;899:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 900
;900:            break;
LABELV $323
line 902
;901:        }
;902:    }
LABELV $310
line 816
ADDRGP4 $309
JUMPV
LABELV $319
line 905
;903:
;904:done:
;905:    *buf_p = 0;
ADDRLP4 4
INDIRP4
CNSTI1 0
ASGNI1
line 906
;906:    return buf_p - buffer;
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $308
endproc vsprintf 72 16
export sscanf
proc sscanf 24 4
line 910
;907:}
;908:
;909:/* this is really crappy */
;910:int sscanf( const char *buffer, const char *fmt, ... ) {
line 915
;911:	int		cmd;
;912:	int		**arg;
;913:	int		count;
;914:
;915:	arg = (int **)&fmt + 1;
ADDRLP4 4
ADDRFP4 4+4
ASGNP4
line 916
;916:	count = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $352
JUMPV
LABELV $351
line 918
;917:
;918:	while ( *fmt ) {
line 919
;919:		if ( fmt[0] != '%' ) {
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 37
EQI4 $354
line 920
;920:			fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 921
;921:			continue;
ADDRGP4 $352
JUMPV
LABELV $354
line 924
;922:		}
;923:
;924:		cmd = fmt[1];
ADDRLP4 0
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
INDIRI1
CVII4 1
ASGNI4
line 925
;925:		fmt += 2;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 2
ADDP4
ASGNP4
line 927
;926:
;927:		switch ( cmd ) {
ADDRLP4 0
INDIRI4
CNSTI4 100
EQI4 $358
ADDRLP4 0
INDIRI4
CNSTI4 102
EQI4 $359
ADDRLP4 0
INDIRI4
CNSTI4 105
EQI4 $358
ADDRLP4 0
INDIRI4
CNSTI4 100
LTI4 $356
LABELV $360
ADDRLP4 0
INDIRI4
CNSTI4 117
EQI4 $358
ADDRGP4 $356
JUMPV
LABELV $358
line 931
;928:		case 'i':
;929:		case 'd':
;930:		case 'u':
;931:			**arg = _atoi( &buffer );
ADDRFP4 0
ARGP4
ADDRLP4 16
ADDRGP4 _atoi
CALLI4
ASGNI4
ADDRLP4 4
INDIRP4
INDIRP4
ADDRLP4 16
INDIRI4
ASGNI4
line 932
;932:			break;
ADDRGP4 $357
JUMPV
LABELV $359
line 934
;933:		case 'f':
;934:			*(float *)*arg = _atof( &buffer );
ADDRFP4 0
ARGP4
ADDRLP4 20
ADDRGP4 _atof
CALLF4
ASGNF4
ADDRLP4 4
INDIRP4
INDIRP4
ADDRLP4 20
INDIRF4
ASGNF4
line 935
;935:			break;
LABELV $356
LABELV $357
line 937
;936:		}
;937:		arg++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 938
;938:	}
LABELV $352
line 918
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $351
line 940
;939:
;940:	return count;
ADDRLP4 8
INDIRI4
RETI4
LABELV $349
endproc sscanf 24 4
import strlen
import cos
import sin
import I_GetParm
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
import ClearBounds
import RadiusFromBounds
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
lit
align 1
LABELV $293
byte 1 40
byte 1 110
byte 1 117
byte 1 108
byte 1 108
byte 1 41
byte 1 0
