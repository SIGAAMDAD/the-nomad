export strcat
code
proc strcat 16 0
file "../sg_lib.c"
line 4
;1:#include "sg_lib.h"
;2:
;3:char *strcat( char *strDestination, const char *strSource )
;4:{
line 7
;5:	char	*s;
;6:
;7:	s = strDestination;
ADDRLP4 0
ADDRFP4 0
INDIRP4
ASGNP4
ADDRGP4 $3
JUMPV
LABELV $2
line 8
;8:	while ( *s ) {
line 9
;9:		s++;
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 10
;10:	}
LABELV $3
line 8
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $2
ADDRGP4 $6
JUMPV
LABELV $5
line 11
;11:	while ( *strSource ) {
line 12
;12:		*s++ = *strSource++;
ADDRLP4 4
ADDRLP4 0
INDIRP4
ASGNP4
ADDRLP4 12
CNSTI4 1
ASGNI4
ADDRLP4 0
ADDRLP4 4
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
ASGNP4
ADDRLP4 8
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 8
INDIRP4
ADDRLP4 12
INDIRI4
ADDP4
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 8
INDIRP4
INDIRI1
ASGNI1
line 13
;13:	}
LABELV $6
line 11
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $5
line 14
;14:	*s = 0;
ADDRLP4 0
INDIRP4
CNSTI1 0
ASGNI1
line 15
;15:	return strDestination;
ADDRFP4 0
INDIRP4
RETP4
LABELV $1
endproc strcat 16 0
export tolower
proc tolower 4 0
line 238
;16:}
;17:
;18:#ifdef Q3VM_USE_DEBUG_FUNCS
;19:size_t strlen(const char *str)
;20:{
;21:    const char *s = str;
;22:    while (*s) {
;23:        s++;
;24:    }
;25:    return (size_t)(s - str);
;26:}
;27:
;28:char* strchr(const char* string, int c)
;29:{
;30:    while ( *string ) {
;31:		if ( *string == c ) {
;32:			return ( char * )string;
;33:		}
;34:		string++;
;35:	}
;36:	return (char *)0;
;37:}
;38:
;39:char *strrchr(const char *string, int32_t c)
;40:{
;41:    const char *found, *p;
;42:
;43:    c = (unsigned char)c;
;44:
;45:    if (c == '\0')
;46:        return strchr(string, '\0');
;47:    
;48:    found = NULL;
;49:    while ((p = strchr(string, c)) != NULL) {
;50:        found = p;
;51:        string = p + 1;
;52:    }
;53:    return (char *)found;
;54:}
;55:
;56:char *strstr( const char *string, const char *strCharSet )
;57:{
;58:	while ( *string ) {
;59:		uint32_t i;
;60:
;61:		for ( i = 0 ; strCharSet[i] ; i++ ) {
;62:			if ( string[i] != strCharSet[i] ) {
;63:				break;
;64:			}
;65:		}
;66:		if ( !strCharSet[i] ) {
;67:			return (char *)string;
;68:		}
;69:		string++;
;70:	}
;71:	return (char *)0;
;72:}
;73:
;74:int strcmp(const char* string1, const char* string2)
;75:{
;76:    while ( *string1 == *string2 && *string1 && *string2 ) {
;77:		string1++;
;78:		string2++;
;79:	}
;80:	return *string1 - *string2;
;81:}
;82:
;83:char* strcpy(char *dst, const char *src)
;84:{
;85:    char *d;
;86:
;87:    d = dst;
;88:    while (*src) {
;89:        *d++ = *src++;
;90:    }
;91:    *d = 0;
;92:    return dst;
;93:}
;94:
;95:static char* med3(char*, char*, char*, cmp_t*);
;96:static void  swapfunc(char*, char*, int, int);
;97:
;98:/*
;99: * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
;100: */
;101:#define swapcode(TYPE, parmi, parmj, n)                                        \
;102:    {                                                                          \
;103:        long           i  = (n) / sizeof(TYPE);                                \
;104:        register TYPE* pi = (TYPE*)(parmi);                                    \
;105:        register TYPE* pj = (TYPE*)(parmj);                                    \
;106:        do                                                                     \
;107:        {                                                                      \
;108:            register TYPE t = *pi;                                             \
;109:            *pi++           = *pj;                                             \
;110:            *pj++           = t;                                               \
;111:        } while (--i > 0);                                                     \
;112:    }
;113:
;114:#define SWAPINIT(a, es)                                                        \
;115:    swaptype = ((char*)a - (char*)0) % sizeof(long) || es % sizeof(long)       \
;116:                   ? 2                                                         \
;117:                   : es == sizeof(long) ? 0 : 1;
;118:
;119:static void swapfunc(a, b, n, swaptype) char *a, *b;
;120:int32_t         n, swaptype;
;121:{
;122:    if (swaptype <= 1)
;123:        swapcode(long, a, b, n) else swapcode(char, a, b, n)
;124:}
;125:
;126:#define swap(a, b)                                                             \
;127:    if (swaptype == 0)                                                         \
;128:    {                                                                          \
;129:        long t      = *(long*)(a);                                             \
;130:        *(long*)(a) = *(long*)(b);                                             \
;131:        *(long*)(b) = t;                                                       \
;132:    }                                                                          \
;133:    else                                                                       \
;134:        swapfunc(a, b, es, swaptype)
;135:
;136:#define vecswap(a, b, n)                                                       \
;137:    if ((n) > 0)                                                               \
;138:    swapfunc(a, b, n, swaptype)
;139:
;140:static char *med3(a, b, c, cmp) char *a, *b, *c;
;141:cmp_t*       cmp;
;142:{
;143:    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
;144:                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
;145:}
;146:
;147:void   qsort(a, n, es, cmp) void* a;
;148:size_t n, es;
;149:cmp_t* cmp;
;150:{
;151:    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
;152:    int32_t   d, r, swaptype, swap_cnt;
;153:
;154:loop:
;155:    SWAPINIT(a, es);
;156:    swap_cnt = 0;
;157:    if (n < 7)
;158:    {
;159:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
;160:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
;161:                swap(pl, pl - es);
;162:        return;
;163:    }
;164:    pm = (char*)a + (n / 2) * es;
;165:    if (n > 7)
;166:    {
;167:        pl = a;
;168:        pn = (char*)a + (n - 1) * es;
;169:        if (n > 40)
;170:        {
;171:            d  = (n / 8) * es;
;172:            pl = med3(pl, pl + d, pl + 2 * d, cmp);
;173:            pm = med3(pm - d, pm, pm + d, cmp);
;174:            pn = med3(pn - 2 * d, pn - d, pn, cmp);
;175:        }
;176:        pm = med3(pl, pm, pn, cmp);
;177:    }
;178:    swap(a, pm);
;179:    pa = pb = (char*)a + es;
;180:
;181:    pc = pd = (char*)a + (n - 1) * es;
;182:    for (;;)
;183:    {
;184:        while (pb <= pc && (r = cmp(pb, a)) <= 0)
;185:        {
;186:            if (r == 0)
;187:            {
;188:                swap_cnt = 1;
;189:                swap(pa, pb);
;190:                pa += es;
;191:            }
;192:            pb += es;
;193:        }
;194:        while (pb <= pc && (r = cmp(pc, a)) >= 0)
;195:        {
;196:            if (r == 0)
;197:            {
;198:                swap_cnt = 1;
;199:                swap(pc, pd);
;200:                pd -= es;
;201:            }
;202:            pc -= es;
;203:        }
;204:        if (pb > pc)
;205:            break;
;206:        swap(pb, pc);
;207:        swap_cnt = 1;
;208:        pb += es;
;209:        pc -= es;
;210:    }
;211:    if (swap_cnt == 0)
;212:    { /* Switch to insertion sort */
;213:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
;214:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
;215:                swap(pl, pl - es);
;216:        return;
;217:    }
;218:
;219:    pn = (char*)a + n * es;
;220:    r  = min(pa - (char*)a, pb - pa);
;221:    vecswap(a, pb - r, r);
;222:    r = min(pd - pc, pn - pd - es);
;223:    vecswap(pb, pn - r, r);
;224:    if ((r = pb - pa) > es)
;225:        qsort(a, r / es, es, cmp);
;226:    if ((r = pd - pc) > es)
;227:    {
;228:        /* Iterate rather than recurse to save stack space */
;229:        a = pn - r;
;230:        n = r / es;
;231:        goto loop;
;232:    }
;233:    /*      qsort(pn - r, r / es, es, cmp);*/
;234:}
;235:#endif
;236:
;237:int32_t tolower(int32_t c)
;238:{
line 239
;239:    if (c >= 'A' && c <= 'Z') {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 65
LTI4 $9
ADDRLP4 0
INDIRI4
CNSTI4 90
GTI4 $9
line 240
;240:        c += 'a' - 'A';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 32
ADDI4
ASGNI4
line 241
;241:    }
LABELV $9
line 242
;242:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $8
endproc tolower 4 0
export toupper
proc toupper 4 0
line 246
;243:}
;244:
;245:int32_t toupper(int32_t c)
;246:{
line 247
;247:    if (c >= 'a' && c <= 'z') {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 97
LTI4 $12
ADDRLP4 0
INDIRI4
CNSTI4 122
GTI4 $12
line 248
;248:        c += 'A' - 'a';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 -32
ADDI4
ASGNI4
line 249
;249:    }
LABELV $12
line 250
;250:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $11
endproc toupper 4 0
export atof
proc atof 32 0
line 255
;251:}
;252:
;253:
;254:double atof(const char* string)
;255:{
ADDRGP4 $16
JUMPV
LABELV $15
line 262
;256:    float sign;
;257:    float value;
;258:    int32_t   c;
;259:
;260:    // skip whitespace
;261:    while (*string <= ' ')
;262:    {
line 263
;263:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $18
line 264
;264:        {
line 265
;265:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $14
JUMPV
LABELV $18
line 267
;266:        }
;267:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 268
;268:    }
LABELV $16
line 261
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $15
line 271
;269:
;270:    // check sign
;271:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $23
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $24
ADDRGP4 $20
JUMPV
line 272
;272:    {
LABELV $23
line 274
;273:    case '+':
;274:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 275
;275:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 276
;276:        break;
ADDRGP4 $21
JUMPV
LABELV $24
line 278
;277:    case '-':
;278:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 279
;279:        sign = -1;
ADDRLP4 8
CNSTF4 3212836864
ASGNF4
line 280
;280:        break;
ADDRGP4 $21
JUMPV
LABELV $20
line 282
;281:    default:
;282:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 283
;283:        break;
LABELV $21
line 287
;284:    }
;285:
;286:    // read digits
;287:    value = 0;
ADDRLP4 4
CNSTF4 0
ASGNF4
line 288
;288:    c     = string[0];
ADDRLP4 0
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 289
;289:    if (c != '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
EQI4 $25
line 290
;290:    {
LABELV $27
line 292
;291:        do
;292:        {
line 293
;293:            c = *string++;
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
line 294
;294:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $32
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $30
LABELV $32
line 295
;295:            {
line 296
;296:                break;
ADDRGP4 $26
JUMPV
LABELV $30
line 298
;297:            }
;298:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 299
;299:            value = value * 10 + c;
ADDRLP4 4
CNSTF4 1092616192
ADDRLP4 4
INDIRF4
MULF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDF4
ASGNF4
line 300
;300:        } while (1);
LABELV $28
ADDRGP4 $27
JUMPV
line 301
;301:    }
ADDRGP4 $26
JUMPV
LABELV $25
line 303
;302:    else
;303:    {
line 304
;304:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 305
;305:    }
LABELV $26
line 308
;306:
;307:    // check for decimal point
;308:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $33
line 309
;309:    {
line 312
;310:        double fraction;
;311:
;312:        fraction = 0.1;
ADDRLP4 20
CNSTF4 1036831949
ASGNF4
LABELV $35
line 314
;313:        do
;314:        {
line 315
;315:            c = *string++;
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
line 316
;316:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $40
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $38
LABELV $40
line 317
;317:            {
line 318
;318:                break;
ADDRGP4 $37
JUMPV
LABELV $38
line 320
;319:            }
;320:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 321
;321:            value += c * fraction;
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
line 322
;322:            fraction *= 0.1;
ADDRLP4 20
CNSTF4 1036831949
ADDRLP4 20
INDIRF4
MULF4
ASGNF4
line 323
;323:        } while (1);
LABELV $36
ADDRGP4 $35
JUMPV
LABELV $37
line 324
;324:    }
LABELV $33
line 328
;325:
;326:    // not handling 10e10 notation...
;327:
;328:    return value * sign;
ADDRLP4 4
INDIRF4
ADDRLP4 8
INDIRF4
MULF4
RETF4
LABELV $14
endproc atof 32 0
export _atof
proc _atof 36 0
line 332
;329:}
;330:
;331:double _atof(const char** stringPtr)
;332:{
line 336
;333:    const char* string;
;334:    float       sign;
;335:    float       value;
;336:    int32_t         c = '0'; // bk001211 - uninitialized use possible
ADDRLP4 0
CNSTI4 48
ASGNI4
line 338
;337:
;338:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $43
JUMPV
LABELV $42
line 342
;339:
;340:    // skip whitespace
;341:    while (*string <= ' ')
;342:    {
line 343
;343:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $45
line 344
;344:        {
line 345
;345:            *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 346
;346:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $41
JUMPV
LABELV $45
line 348
;347:        }
;348:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 349
;349:    }
LABELV $43
line 341
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $42
line 352
;350:
;351:    // check sign
;352:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $50
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $51
ADDRGP4 $47
JUMPV
line 353
;353:    {
LABELV $50
line 355
;354:    case '+':
;355:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 356
;356:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 357
;357:        break;
ADDRGP4 $48
JUMPV
LABELV $51
line 359
;358:    case '-':
;359:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 360
;360:        sign = -1;
ADDRLP4 12
CNSTF4 3212836864
ASGNF4
line 361
;361:        break;
ADDRGP4 $48
JUMPV
LABELV $47
line 363
;362:    default:
;363:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 364
;364:        break;
LABELV $48
line 368
;365:    }
;366:
;367:    // read digits
;368:    value = 0;
ADDRLP4 8
CNSTF4 0
ASGNF4
line 369
;369:    if (string[0] != '.')
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 46
EQI4 $52
line 370
;370:    {
LABELV $54
line 372
;371:        do
;372:        {
line 373
;373:            c = *string++;
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
line 374
;374:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $59
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $57
LABELV $59
line 375
;375:            {
line 376
;376:                break;
ADDRGP4 $56
JUMPV
LABELV $57
line 378
;377:            }
;378:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 379
;379:            value = value * 10 + c;
ADDRLP4 8
CNSTF4 1092616192
ADDRLP4 8
INDIRF4
MULF4
ADDRLP4 0
INDIRI4
CVIF4 4
ADDF4
ASGNF4
line 380
;380:        } while (1);
LABELV $55
ADDRGP4 $54
JUMPV
LABELV $56
line 381
;381:    }
LABELV $52
line 384
;382:
;383:    // check for decimal point
;384:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $60
line 385
;385:    {
line 388
;386:        double fraction;
;387:
;388:        fraction = 0.1;
ADDRLP4 24
CNSTF4 1036831949
ASGNF4
LABELV $62
line 390
;389:        do
;390:        {
line 391
;391:            c = *string++;
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
line 392
;392:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $67
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $65
LABELV $67
line 393
;393:            {
line 394
;394:                break;
ADDRGP4 $64
JUMPV
LABELV $65
line 396
;395:            }
;396:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 397
;397:            value += c * fraction;
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
line 398
;398:            fraction *= 0.1;
ADDRLP4 24
CNSTF4 1036831949
ADDRLP4 24
INDIRF4
MULF4
ASGNF4
line 399
;399:        } while (1);
LABELV $63
ADDRGP4 $62
JUMPV
LABELV $64
line 400
;400:    }
LABELV $60
line 403
;401:
;402:    // not handling 10e10 notation...
;403:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 405
;404:
;405:    return value * sign;
ADDRLP4 8
INDIRF4
ADDRLP4 12
INDIRF4
MULF4
RETF4
LABELV $41
endproc _atof 36 0
export atoi
proc atoi 28 0
line 409
;406:}
;407:
;408:int32_t atoi(const char* string)
;409:{
ADDRGP4 $70
JUMPV
LABELV $69
line 416
;410:    int32_t sign;
;411:    int32_t value;
;412:    int32_t c;
;413:
;414:    // skip whitespace
;415:    while (*string <= ' ')
;416:    {
line 417
;417:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $72
line 418
;418:        {
line 419
;419:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $68
JUMPV
LABELV $72
line 421
;420:        }
;421:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 422
;422:    }
LABELV $70
line 415
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $69
line 425
;423:
;424:    // check sign
;425:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $77
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $78
ADDRGP4 $74
JUMPV
line 426
;426:    {
LABELV $77
line 428
;427:    case '+':
;428:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 429
;429:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 430
;430:        break;
ADDRGP4 $75
JUMPV
LABELV $78
line 432
;431:    case '-':
;432:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 433
;433:        sign = -1;
ADDRLP4 8
CNSTI4 -1
ASGNI4
line 434
;434:        break;
ADDRGP4 $75
JUMPV
LABELV $74
line 436
;435:    default:
;436:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 437
;437:        break;
LABELV $75
line 441
;438:    }
;439:
;440:    // read digits
;441:    value = 0;
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $79
line 443
;442:    do
;443:    {
line 444
;444:        c = *string++;
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
line 445
;445:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $84
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $82
LABELV $84
line 446
;446:        {
line 447
;447:            break;
ADDRGP4 $81
JUMPV
LABELV $82
line 449
;448:        }
;449:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 450
;450:        value = value * 10 + c;
ADDRLP4 4
CNSTI4 10
ADDRLP4 4
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 451
;451:    } while (1);
LABELV $80
ADDRGP4 $79
JUMPV
LABELV $81
line 455
;452:
;453:    // not handling 10e10 notation...
;454:
;455:    return value * sign;
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
MULI4
RETI4
LABELV $68
endproc atoi 28 0
export _atoi
proc _atoi 32 0
line 459
;456:}
;457:
;458:int32_t _atoi(const char** stringPtr)
;459:{
line 465
;460:    int32_t         sign;
;461:    int32_t         value;
;462:    int32_t         c;
;463:    const char* string;
;464:
;465:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $87
JUMPV
LABELV $86
line 469
;466:
;467:    // skip whitespace
;468:    while (*string <= ' ')
;469:    {
line 470
;470:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $89
line 471
;471:        {
line 472
;472:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $85
JUMPV
LABELV $89
line 474
;473:        }
;474:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 475
;475:    }
LABELV $87
line 468
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $86
line 478
;476:
;477:    // check sign
;478:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $94
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $95
ADDRGP4 $91
JUMPV
line 479
;479:    {
LABELV $94
line 481
;480:    case '+':
;481:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 482
;482:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 483
;483:        break;
ADDRGP4 $92
JUMPV
LABELV $95
line 485
;484:    case '-':
;485:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 486
;486:        sign = -1;
ADDRLP4 12
CNSTI4 -1
ASGNI4
line 487
;487:        break;
ADDRGP4 $92
JUMPV
LABELV $91
line 489
;488:    default:
;489:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 490
;490:        break;
LABELV $92
line 494
;491:    }
;492:
;493:    // read digits
;494:    value = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $96
line 496
;495:    do
;496:    {
line 497
;497:        c = *string++;
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
line 498
;498:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $101
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $99
LABELV $101
line 499
;499:        {
line 500
;500:            break;
ADDRGP4 $98
JUMPV
LABELV $99
line 502
;501:        }
;502:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 503
;503:        value = value * 10 + c;
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 504
;504:    } while (1);
LABELV $97
ADDRGP4 $96
JUMPV
LABELV $98
line 508
;505:
;506:    // not handling 10e10 notation...
;507:
;508:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 510
;509:
;510:    return value * sign;
ADDRLP4 8
INDIRI4
ADDRLP4 12
INDIRI4
MULI4
RETI4
LABELV $85
endproc _atoi 32 0
export tan
proc tan 8 4
line 514
;511:}
;512:
;513:double tan(double x)
;514:{
line 515
;515:    return sin(x) / cos(x);
ADDRFP4 0
INDIRF4
ARGF4
ADDRLP4 0
ADDRGP4 sin
CALLF4
ASGNF4
ADDRFP4 0
INDIRF4
ARGF4
ADDRLP4 4
ADDRGP4 cos
CALLF4
ASGNF4
ADDRLP4 0
INDIRF4
ADDRLP4 4
INDIRF4
DIVF4
RETF4
LABELV $102
endproc tan 8 4
data
align 4
LABELV randSeed
byte 4 0
export srand
code
proc srand 0 0
line 521
;516:}
;517:
;518:static int32_t randSeed = 0;
;519:
;520:void srand(unsigned seed)
;521:{
line 522
;522:    randSeed = seed;
ADDRGP4 randSeed
ADDRFP4 0
INDIRU4
CVUI4 4
ASGNI4
line 523
;523:}
LABELV $103
endproc srand 0 0
export rand
proc rand 4 0
line 526
;524:
;525:int32_t rand(void)
;526:{
line 527
;527:    randSeed = (69069 * randSeed + 1);
ADDRLP4 0
ADDRGP4 randSeed
ASGNP4
ADDRLP4 0
INDIRP4
CNSTI4 69069
ADDRLP4 0
INDIRP4
INDIRI4
MULI4
CNSTI4 1
ADDI4
ASGNI4
line 528
;528:    return randSeed & 0x7fff;
ADDRGP4 randSeed
INDIRI4
CNSTI4 32767
BANDI4
RETI4
LABELV $104
endproc rand 4 0
export abs
proc abs 4 0
line 533
;529:}
;530:
;531:
;532:int32_t abs(int32_t n)
;533:{
line 534
;534:    return n < 0 ? -n : n;
ADDRFP4 0
INDIRI4
CNSTI4 0
GEI4 $107
ADDRLP4 0
ADDRFP4 0
INDIRI4
NEGI4
ASGNI4
ADDRGP4 $108
JUMPV
LABELV $107
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
LABELV $108
ADDRLP4 0
INDIRI4
RETI4
LABELV $105
endproc abs 4 0
export fabs
proc fabs 4 0
line 538
;535:}
;536:
;537:double fabs(double x)
;538:{
line 539
;539:    return x < 0 ? -x : x;
ADDRFP4 0
INDIRF4
CNSTF4 0
GEF4 $111
ADDRLP4 0
ADDRFP4 0
INDIRF4
NEGF4
ASGNF4
ADDRGP4 $112
JUMPV
LABELV $111
ADDRLP4 0
ADDRFP4 0
INDIRF4
ASGNF4
LABELV $112
ADDRLP4 0
INDIRF4
RETF4
LABELV $109
endproc fabs 4 0
export AddInt
proc AddInt 56 0
line 557
;540:}
;541:
;542:#define ALT 0x00000001       /* alternate form */
;543:#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
;544:#define LADJUST 0x00000004   /* left adjustment */
;545:#define LONGDBL 0x00000008   /* long double */
;546:#define LONGINT 0x00000010   /* long integer */
;547:#define QUADINT 0x00000020   /* quad integer */
;548:#define SHORTINT 0x00000040  /* short integer */
;549:#define ZEROPAD 0x00000080   /* zero (as opposed to blank) pad */
;550:#define FPT 0x00000100       /* floating point32_t number */
;551:
;552:#define to_digit(c) ((c) - '0')
;553:#define is_digit(c) ((unsigned)to_digit(c) <= 9)
;554:#define to_char(n) ((n) + '0')
;555:
;556:void AddInt(char** buf_p, int32_t val, int32_t width, int32_t flags)
;557:{
line 563
;558:    char  text[32];
;559:    int32_t   digits;
;560:    int32_t   signedVal;
;561:    char* buf;
;562:
;563:    digits    = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 564
;564:    signedVal = val;
ADDRLP4 40
ADDRFP4 4
INDIRI4
ASGNI4
line 565
;565:    if (val < 0)
ADDRFP4 4
INDIRI4
CNSTI4 0
GEI4 $114
line 566
;566:    {
line 567
;567:        val = -val;
ADDRFP4 4
ADDRFP4 4
INDIRI4
NEGI4
ASGNI4
line 568
;568:    }
LABELV $114
LABELV $116
line 570
;569:    do
;570:    {
line 571
;571:        text[digits++] = '0' + val % 10;
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
line 572
;572:        val /= 10;
ADDRFP4 4
ADDRFP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 573
;573:    } while (val);
LABELV $117
ADDRFP4 4
INDIRI4
CNSTI4 0
NEI4 $116
line 575
;574:
;575:    if (signedVal < 0)
ADDRLP4 40
INDIRI4
CNSTI4 0
GEI4 $119
line 576
;576:    {
line 577
;577:        text[digits++] = '-';
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
line 578
;578:    }
LABELV $119
line 580
;579:
;580:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 582
;581:
;582:    if (!(flags & LADJUST))
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
NEI4 $130
line 583
;583:    {
ADDRGP4 $124
JUMPV
LABELV $123
line 585
;584:        while (digits < width)
;585:        {
line 586
;586:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $127
ADDRLP4 44
CNSTI4 48
ASGNI4
ADDRGP4 $128
JUMPV
LABELV $127
ADDRLP4 44
CNSTI4 32
ASGNI4
LABELV $128
ADDRLP4 48
INDIRP4
ADDRLP4 44
INDIRI4
CVII1 4
ASGNI1
line 587
;587:            width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 588
;588:        }
LABELV $124
line 584
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $123
line 589
;589:    }
ADDRGP4 $130
JUMPV
LABELV $129
line 592
;590:
;591:    while (digits--)
;592:    {
line 593
;593:        *buf++ = text[digits];
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
line 594
;594:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 595
;595:    }
LABELV $130
line 591
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
NEI4 $129
line 597
;596:
;597:    if (flags & LADJUST)
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
EQI4 $132
line 598
;598:    {
ADDRGP4 $135
JUMPV
LABELV $134
line 600
;599:        while (width--)
;600:        {
line 601
;601:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $138
ADDRLP4 48
CNSTI4 48
ASGNI4
ADDRGP4 $139
JUMPV
LABELV $138
ADDRLP4 48
CNSTI4 32
ASGNI4
LABELV $139
ADDRLP4 52
INDIRP4
ADDRLP4 48
INDIRI4
CVII1 4
ASGNI1
line 602
;602:        }
LABELV $135
line 599
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
NEI4 $134
line 603
;603:    }
LABELV $132
line 605
;604:
;605:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 606
;606:}
LABELV $113
endproc AddInt 56 0
export AddUInt
proc AddUInt 52 0
line 609
;607:
;608:void AddUInt(char** buf_p, uint32_t val, int32_t width, int32_t flags)
;609:{
line 614
;610:    char  text[32];
;611:    uint32_t   digits;
;612:    char* buf;
;613:
;614:    digits    = 0;
ADDRLP4 0
CNSTU4 0
ASGNU4
LABELV $141
line 616
;615:    do
;616:    {
line 617
;617:        text[digits++] = '0' + val % 10;
ADDRLP4 40
ADDRLP4 0
INDIRU4
ASGNU4
ADDRLP4 0
ADDRLP4 40
INDIRU4
CNSTU4 1
ADDU4
ASGNU4
ADDRLP4 40
INDIRU4
ADDRLP4 8
ADDP4
ADDRFP4 4
INDIRU4
CNSTU4 10
MODU4
CNSTU4 48
ADDU4
CVUI4 4
CVII1 4
ASGNI1
line 618
;618:        val /= 10;
ADDRFP4 4
ADDRFP4 4
INDIRU4
CNSTU4 10
DIVU4
ASGNU4
line 619
;619:    } while (val);
LABELV $142
ADDRFP4 4
INDIRU4
CNSTU4 0
NEU4 $141
line 621
;620:
;621:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 623
;622:
;623:    if (!(flags & LADJUST))
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
NEI4 $153
line 624
;624:    {
ADDRGP4 $147
JUMPV
LABELV $146
line 626
;625:        while (digits < width)
;626:        {
line 627
;627:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
ADDRFP4 12
INDIRI4
CNSTI4 128
BANDI4
CNSTI4 0
EQI4 $150
ADDRLP4 40
CNSTI4 48
ASGNI4
ADDRGP4 $151
JUMPV
LABELV $150
ADDRLP4 40
CNSTI4 32
ASGNI4
LABELV $151
ADDRLP4 44
INDIRP4
ADDRLP4 40
INDIRI4
CVII1 4
ASGNI1
line 628
;628:            width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 629
;629:        }
LABELV $147
line 625
ADDRLP4 0
INDIRU4
ADDRFP4 8
INDIRI4
CVIU4 4
LTU4 $146
line 630
;630:    }
ADDRGP4 $153
JUMPV
LABELV $152
line 633
;631:
;632:    while (digits--)
;633:    {
line 634
;634:        *buf++ = text[digits];
ADDRLP4 40
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 4
ADDRLP4 40
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
ADDRLP4 40
INDIRP4
ADDRLP4 0
INDIRU4
ADDRLP4 8
ADDP4
INDIRI1
ASGNI1
line 635
;635:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 636
;636:    }
LABELV $153
line 632
ADDRLP4 40
ADDRLP4 0
INDIRU4
ASGNU4
ADDRLP4 0
ADDRLP4 40
INDIRU4
CNSTU4 1
SUBU4
ASGNU4
ADDRLP4 40
INDIRU4
CNSTU4 0
NEU4 $152
line 638
;637:
;638:    if (flags & LADJUST)
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
EQI4 $155
line 639
;639:    {
ADDRGP4 $158
JUMPV
LABELV $157
line 641
;640:        while (width--)
;641:        {
line 642
;642:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $161
ADDRLP4 44
CNSTI4 48
ASGNI4
ADDRGP4 $162
JUMPV
LABELV $161
ADDRLP4 44
CNSTI4 32
ASGNI4
LABELV $162
ADDRLP4 48
INDIRP4
ADDRLP4 44
INDIRI4
CVII1 4
ASGNI1
line 643
;643:        }
LABELV $158
line 640
ADDRLP4 44
ADDRFP4 8
INDIRI4
ASGNI4
ADDRFP4 8
ADDRLP4 44
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
ADDRLP4 44
INDIRI4
CNSTI4 0
NEI4 $157
line 644
;644:    }
LABELV $155
line 646
;645:
;646:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 647
;647:}
LABELV $140
endproc AddUInt 52 0
export AddFloat
proc AddFloat 60 0
line 650
;648:
;649:void AddFloat(char** buf_p, float fval, int32_t width, int32_t prec)
;650:{
line 658
;651:    char  text[32];
;652:    int32_t   digits;
;653:    float signedVal;
;654:    char* buf;
;655:    int32_t   val;
;656:
;657:    // get the sign
;658:    signedVal = fval;
ADDRLP4 44
ADDRFP4 4
INDIRF4
ASGNF4
line 659
;659:    if (fval < 0)
ADDRFP4 4
INDIRF4
CNSTF4 0
GEF4 $164
line 660
;660:    {
line 661
;661:        fval = -fval;
ADDRFP4 4
ADDRFP4 4
INDIRF4
NEGF4
ASGNF4
line 662
;662:    }
LABELV $164
line 665
;663:
;664:    // write the float number
;665:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 666
;666:    val    = (int32_t)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
LABELV $166
line 668
;667:    do
;668:    {
line 669
;669:        text[digits++] = '0' + val % 10;
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
line 670
;670:        val /= 10;
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 671
;671:    } while (val);
LABELV $167
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $166
line 673
;672:
;673:    if (signedVal < 0)
ADDRLP4 44
INDIRF4
CNSTF4 0
GEF4 $169
line 674
;674:    {
line 675
;675:        text[digits++] = '-';
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
line 676
;676:    }
LABELV $169
line 678
;677:
;678:    buf = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $172
JUMPV
LABELV $171
line 681
;679:
;680:    while (digits < width)
;681:    {
line 682
;682:        *buf++ = ' ';
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
line 683
;683:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 684
;684:    }
LABELV $172
line 680
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $171
ADDRGP4 $175
JUMPV
LABELV $174
line 687
;685:
;686:    while (digits--)
;687:    {
line 688
;688:        *buf++ = text[digits];
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
line 689
;689:    }
LABELV $175
line 686
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
NEI4 $174
line 691
;690:
;691:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 693
;692:
;693:    if (prec < 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
GEI4 $177
line 694
;694:        prec = 6;
ADDRFP4 12
CNSTI4 6
ASGNI4
LABELV $177
line 696
;695:    // write the fraction
;696:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $180
JUMPV
LABELV $179
line 698
;697:    while (digits < prec)
;698:    {
line 699
;699:        fval -= (int)fval;
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
line 700
;700:        fval *= 10.0;
ADDRFP4 4
CNSTF4 1092616192
ADDRFP4 4
INDIRF4
MULF4
ASGNF4
line 701
;701:        val            = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
line 702
;702:        text[digits++] = '0' + val % 10;
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
line 703
;703:    }
LABELV $180
line 697
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $179
line 705
;704:
;705:    if (digits > 0)
ADDRLP4 0
INDIRI4
CNSTI4 0
LEI4 $182
line 706
;706:    {
line 707
;707:        buf    = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 708
;708:        *buf++ = '.';
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
line 709
;709:        for (prec = 0; prec < digits; prec++)
ADDRFP4 12
CNSTI4 0
ASGNI4
ADDRGP4 $187
JUMPV
LABELV $184
line 710
;710:        {
line 711
;711:            *buf++ = text[prec];
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
line 712
;712:        }
LABELV $185
line 709
ADDRFP4 12
ADDRFP4 12
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $187
ADDRFP4 12
INDIRI4
ADDRLP4 0
INDIRI4
LTI4 $184
line 713
;713:        *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 714
;714:    }
LABELV $182
line 715
;715:}
LABELV $163
endproc AddFloat 60 0
export AddString
proc AddString 20 4
line 718
;716:
;717:void AddString(char** buf_p, char* string, int32_t width, int32_t prec)
;718:{
line 722
;719:    int32_t   size;
;720:    char* buf;
;721:
;722:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 724
;723:
;724:    if (string == NULL)
ADDRFP4 4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $189
line 725
;725:    {
line 726
;726:        string = "(null)";
ADDRFP4 4
ADDRGP4 $191
ASGNP4
line 727
;727:        prec   = -1;
ADDRFP4 12
CNSTI4 -1
ASGNI4
line 728
;728:    }
LABELV $189
line 730
;729:
;730:    if (prec >= 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
LTI4 $192
line 731
;731:    {
line 732
;732:        for (size = 0; size < prec; size++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $197
JUMPV
LABELV $194
line 733
;733:        {
line 734
;734:            if (string[size] == '\0')
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $198
line 735
;735:            {
line 736
;736:                break;
ADDRGP4 $193
JUMPV
LABELV $198
line 738
;737:            }
;738:        }
LABELV $195
line 732
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $197
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $194
line 739
;739:    }
ADDRGP4 $193
JUMPV
LABELV $192
line 741
;740:    else
;741:    {
line 742
;742:        size = strlen(string);
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 8
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRLP4 0
ADDRLP4 8
INDIRU4
CVUI4 4
ASGNI4
line 743
;743:    }
LABELV $193
line 745
;744:
;745:    width -= size;
ADDRFP4 8
ADDRFP4 8
INDIRI4
ADDRLP4 0
INDIRI4
SUBI4
ASGNI4
ADDRGP4 $201
JUMPV
LABELV $200
line 748
;746:
;747:    while (size--)
;748:    {
line 749
;749:        *buf++ = *string++;
ADDRLP4 8
ADDRLP4 4
INDIRP4
ASGNP4
ADDRLP4 16
CNSTI4 1
ASGNI4
ADDRLP4 4
ADDRLP4 8
INDIRP4
ADDRLP4 16
INDIRI4
ADDP4
ASGNP4
ADDRLP4 12
ADDRFP4 4
INDIRP4
ASGNP4
ADDRFP4 4
ADDRLP4 12
INDIRP4
ADDRLP4 16
INDIRI4
ADDP4
ASGNP4
ADDRLP4 8
INDIRP4
ADDRLP4 12
INDIRP4
INDIRI1
ASGNI1
line 750
;750:    }
LABELV $201
line 747
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
NEI4 $200
ADDRGP4 $204
JUMPV
LABELV $203
line 753
;751:
;752:    while (width-- > 0)
;753:    {
line 754
;754:        *buf++ = ' ';
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
line 755
;755:    }
LABELV $204
line 752
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
GTI4 $203
line 757
;756:
;757:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 758
;758:}
LABELV $188
endproc AddString 20 4
export vsprintf
proc vsprintf 72 16
line 761
;759:
;760:int32_t vsprintf(char* buffer, const char* fmt, va_list argptr)
;761:{
line 771
;762:    int32_t*    arg;
;763:    char*       buf_p;
;764:    char        ch;
;765:    int32_t     flags;
;766:    int32_t     width;
;767:    int32_t     prec;
;768:    int32_t     n;
;769:    char        sign;
;770:
;771:    buf_p = buffer;
ADDRLP4 4
ADDRFP4 0
INDIRP4
ASGNP4
line 772
;772:    arg   = (int32_t *)argptr;
ADDRLP4 24
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $208
JUMPV
LABELV $207
line 775
;773:
;774:    while (1)
;775:    {
line 777
;776:        // run through the format string until we hit a '%' or '\0'
;777:        for (ch = *fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
ADDRLP4 0
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRGP4 $213
JUMPV
LABELV $210
line 778
;778:        {
line 779
;779:            *buf_p++ = ch;
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
line 780
;780:        }
LABELV $211
line 777
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
LABELV $213
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
EQI4 $214
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 37
NEI4 $210
LABELV $214
line 781
;781:        if (ch == '\0')
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $215
line 782
;782:        {
line 783
;783:            goto done;
ADDRGP4 $217
JUMPV
LABELV $215
line 787
;784:        }
;785:
;786:        // skip over the '%'
;787:        fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 790
;788:
;789:        // reset formatting state
;790:        flags = 0;
ADDRLP4 16
CNSTI4 0
ASGNI4
line 791
;791:        width = 0;
ADDRLP4 12
CNSTI4 0
ASGNI4
line 792
;792:        prec  = -1;
ADDRLP4 20
CNSTI4 -1
ASGNI4
line 793
;793:        sign  = '\0';
ADDRLP4 28
CNSTI1 0
ASGNI1
LABELV $218
line 796
;794:
;795:    rflag:
;796:        ch = *fmt++;
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
LABELV $219
line 798
;797:    reswitch:
;798:        switch (ch)
ADDRLP4 36
ADDRLP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 99
LTI4 $241
ADDRLP4 36
INDIRI4
CNSTI4 105
GTI4 $242
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $243-396
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $243
address $236
address $237
address $220
address $238
address $220
address $220
address $237
code
LABELV $241
ADDRLP4 36
INDIRI4
CNSTI4 37
LTI4 $220
ADDRLP4 36
INDIRI4
CNSTI4 57
GTI4 $220
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $245-148
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $245
address $240
address $220
address $220
address $220
address $220
address $220
address $220
address $220
address $223
address $224
address $220
address $231
address $232
address $232
address $232
address $232
address $232
address $232
address $232
address $232
address $232
code
LABELV $242
ADDRLP4 36
INDIRI4
CNSTI4 115
EQI4 $239
ADDRGP4 $220
JUMPV
line 799
;799:        {
LABELV $223
line 801
;800:        case '-':
;801:            flags |= LADJUST;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 4
BORI4
ASGNI4
line 802
;802:            goto rflag;
ADDRGP4 $218
JUMPV
LABELV $224
line 804
;803:        case '.':
;804:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $226
JUMPV
LABELV $225
line 806
;805:            while (is_digit((ch = *fmt++)))
;806:            {
line 807
;807:                n = 10 * n + (ch - '0');
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
ADDI4
ASGNI4
line 808
;808:            }
LABELV $226
line 805
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
LEU4 $225
line 809
;809:            prec = n < 0 ? -1 : n;
ADDRLP4 8
INDIRI4
CNSTI4 0
GEI4 $229
ADDRLP4 56
CNSTI4 -1
ASGNI4
ADDRGP4 $230
JUMPV
LABELV $229
ADDRLP4 56
ADDRLP4 8
INDIRI4
ASGNI4
LABELV $230
ADDRLP4 20
ADDRLP4 56
INDIRI4
ASGNI4
line 810
;810:            goto reswitch;
ADDRGP4 $219
JUMPV
LABELV $231
line 812
;811:        case '0':
;812:            flags |= ZEROPAD;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 128
BORI4
ASGNI4
line 813
;813:            goto rflag;
ADDRGP4 $218
JUMPV
LABELV $232
line 823
;814:        case '1':
;815:        case '2':
;816:        case '3':
;817:        case '4':
;818:        case '5':
;819:        case '6':
;820:        case '7':
;821:        case '8':
;822:        case '9':
;823:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $233
line 825
;824:            do
;825:            {
line 826
;826:                n  = 10 * n + (ch - '0');
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
ADDI4
ASGNI4
line 827
;827:                ch = *fmt++;
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
line 828
;828:            } while (is_digit(ch));
LABELV $234
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $233
line 829
;829:            width = n;
ADDRLP4 12
ADDRLP4 8
INDIRI4
ASGNI4
line 830
;830:            goto reswitch;
ADDRGP4 $219
JUMPV
LABELV $236
line 832
;831:        case 'c':
;832:            *buf_p++ = (char)*arg;
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
line 833
;833:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 834
;834:            break;
ADDRGP4 $221
JUMPV
LABELV $237
line 837
;835:        case 'd':
;836:        case 'i':
;837:            AddInt(&buf_p, *arg, width, flags);
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
line 838
;838:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 839
;839:            break;
ADDRGP4 $221
JUMPV
LABELV $238
line 841
;840:        case 'f':
;841:            AddFloat(&buf_p, *(double*)arg, width, prec);
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
line 843
;842:#ifdef __LCC__
;843:            arg += 1; // everything is 32 bit in my compiler
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 847
;844:#else
;845:            arg += 2;
;846:#endif
;847:            break;
ADDRGP4 $221
JUMPV
LABELV $239
line 849
;848:        case 's':
;849:            AddString(&buf_p, (char*)*arg, width, prec);
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
line 850
;850:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 851
;851:            break;
ADDRGP4 $221
JUMPV
LABELV $240
line 853
;852:        case '%':
;853:            *buf_p++ = ch;
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
line 854
;854:            break;
ADDRGP4 $221
JUMPV
LABELV $220
line 856
;855:        default:
;856:            *buf_p++ = (char)*arg;
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
line 857
;857:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 858
;858:            break;
LABELV $221
line 860
;859:        }
;860:    }
LABELV $208
line 774
ADDRGP4 $207
JUMPV
LABELV $217
line 863
;861:
;862:done:
;863:    *buf_p = 0;
ADDRLP4 4
INDIRP4
CNSTI1 0
ASGNI1
line 864
;864:    return buf_p - buffer;
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $206
endproc vsprintf 72 16
export sscanf
proc sscanf 28 4
line 869
;865:}
;866:
;867:/* this is really crappy */
;868:int32_t sscanf( const char *buffer, const char *fmt, ... )
;869:{
line 874
;870:	int32_t	cmd;
;871:	int32_t	**arg;
;872:	int32_t	count;
;873:
;874:	arg = (int32_t **)&fmt + 1;
ADDRLP4 4
ADDRFP4 4+4
ASGNP4
line 875
;875:	count = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $250
JUMPV
LABELV $249
line 877
;876:
;877:	while ( *fmt ) {
line 878
;878:		if ( fmt[0] != '%' ) {
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 37
EQI4 $252
line 879
;879:			fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 880
;880:			continue;
ADDRGP4 $250
JUMPV
LABELV $252
line 883
;881:		}
;882:
;883:		cmd = fmt[1];
ADDRLP4 0
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
INDIRI1
CVII4 1
ASGNI4
line 884
;884:		fmt += 2;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 2
ADDP4
ASGNP4
line 886
;885:
;886:		switch ( cmd ) {
ADDRLP4 16
CNSTI4 100
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
EQI4 $256
ADDRLP4 0
INDIRI4
CNSTI4 102
EQI4 $257
ADDRLP4 0
INDIRI4
CNSTI4 105
EQI4 $256
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
LTI4 $254
LABELV $258
ADDRLP4 0
INDIRI4
CNSTI4 117
EQI4 $256
ADDRGP4 $254
JUMPV
LABELV $256
line 890
;887:		case 'i':
;888:		case 'd':
;889:		case 'u':
;890:			**arg = _atoi( &buffer );
ADDRFP4 0
ARGP4
ADDRLP4 20
ADDRGP4 _atoi
CALLI4
ASGNI4
ADDRLP4 4
INDIRP4
INDIRP4
ADDRLP4 20
INDIRI4
ASGNI4
line 891
;891:			break;
ADDRGP4 $255
JUMPV
LABELV $257
line 893
;892:		case 'f':
;893:			*(float *)*arg = _atof( &buffer );
ADDRFP4 0
ARGP4
ADDRLP4 24
ADDRGP4 _atof
CALLF4
ASGNF4
ADDRLP4 4
INDIRP4
INDIRP4
ADDRLP4 24
INDIRF4
ASGNF4
line 894
;894:			break;
LABELV $254
LABELV $255
line 896
;895:		}
;896:		arg++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 897
;897:	}
LABELV $250
line 877
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $249
line 899
;898:
;899:	return count;
ADDRLP4 8
INDIRI4
RETI4
LABELV $247
endproc sscanf 28 4
import acos
import atan2
import cos
import sin
import sqrt
import floor
import ceil
import qsort
import strncmp
import strcmp
import strstr
import strchr
import strlen
import strcpy
import memmove
import memset
import memchr
import memcpy
lit
align 1
LABELV $191
byte 1 40
byte 1 110
byte 1 117
byte 1 108
byte 1 108
byte 1 41
byte 1 0
