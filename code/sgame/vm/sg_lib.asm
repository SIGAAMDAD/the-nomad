export tolower
code
proc tolower 4 0
file "../sg_lib.c"
line 239
;1:#include "sg_lib.h"
;2:
;3:#ifdef Q3VM_LIB_NO_INTRINSICS
;4:size_t strlen(const char *str)
;5:{
;6:    const char *s = str;
;7:    while (*s) {
;8:        s++;
;9:    }
;10:    return (size_t)(s - str);
;11:}
;12:
;13:char *strcat( char *strDestination, const char *strSource )
;14:{
;15:	char	*s;
;16:
;17:	s = strDestination;
;18:	while ( *s ) {
;19:		s++;
;20:	}
;21:	while ( *strSource ) {
;22:		*s++ = *strSource++;
;23:	}
;24:	*s = 0;
;25:	return strDestination;
;26:}
;27:
;28:
;29:char* strchr(const char* string, int c)
;30:{
;31:    while ( *string ) {
;32:		if ( *string == c ) {
;33:			return ( char * )string;
;34:		}
;35:		string++;
;36:	}
;37:	return (char *)0;
;38:}
;39:
;40:char *strrchr(const char *string, int32_t c)
;41:{
;42:    const char *found, *p;
;43:
;44:    c = (unsigned char)c;
;45:
;46:    if (c == '\0')
;47:        return strchr(string, '\0');
;48:    
;49:    found = NULL;
;50:    while ((p = strchr(string, c)) != NULL) {
;51:        found = p;
;52:        string = p + 1;
;53:    }
;54:    return (char *)found;
;55:}
;56:
;57:char *strstr( const char *string, const char *strCharSet )
;58:{
;59:	while ( *string ) {
;60:		uint32_t i;
;61:
;62:		for ( i = 0 ; strCharSet[i] ; i++ ) {
;63:			if ( string[i] != strCharSet[i] ) {
;64:				break;
;65:			}
;66:		}
;67:		if ( !strCharSet[i] ) {
;68:			return (char *)string;
;69:		}
;70:		string++;
;71:	}
;72:	return (char *)0;
;73:}
;74:
;75:int strcmp(const char* string1, const char* string2)
;76:{
;77:    while ( *string1 == *string2 && *string1 && *string2 ) {
;78:		string1++;
;79:		string2++;
;80:	}
;81:	return *string1 - *string2;
;82:}
;83:
;84:char* strcpy(char *dst, const char *src)
;85:{
;86:    char *d;
;87:
;88:    d = dst;
;89:    while (*src) {
;90:        *d++ = *src++;
;91:    }
;92:    *d = 0;
;93:    return dst;
;94:}
;95:
;96:static char* med3(char*, char*, char*, cmp_t*);
;97:static void  swapfunc(char*, char*, int, int);
;98:
;99:/*
;100: * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
;101: */
;102:#define swapcode(TYPE, parmi, parmj, n)                                        \
;103:    {                                                                          \
;104:        long           i  = (n) / sizeof(TYPE);                                \
;105:        register TYPE* pi = (TYPE*)(parmi);                                    \
;106:        register TYPE* pj = (TYPE*)(parmj);                                    \
;107:        do                                                                     \
;108:        {                                                                      \
;109:            register TYPE t = *pi;                                             \
;110:            *pi++           = *pj;                                             \
;111:            *pj++           = t;                                               \
;112:        } while (--i > 0);                                                     \
;113:    }
;114:
;115:#define SWAPINIT(a, es)                                                        \
;116:    swaptype = ((char*)a - (char*)0) % sizeof(long) || es % sizeof(long)       \
;117:                   ? 2                                                         \
;118:                   : es == sizeof(long) ? 0 : 1;
;119:
;120:static void swapfunc(a, b, n, swaptype) char *a, *b;
;121:int32_t         n, swaptype;
;122:{
;123:    if (swaptype <= 1)
;124:        swapcode(long, a, b, n) else swapcode(char, a, b, n)
;125:}
;126:
;127:#define swap(a, b)                                                             \
;128:    if (swaptype == 0)                                                         \
;129:    {                                                                          \
;130:        long t      = *(long*)(a);                                             \
;131:        *(long*)(a) = *(long*)(b);                                             \
;132:        *(long*)(b) = t;                                                       \
;133:    }                                                                          \
;134:    else                                                                       \
;135:        swapfunc(a, b, es, swaptype)
;136:
;137:#define vecswap(a, b, n)                                                       \
;138:    if ((n) > 0)                                                               \
;139:    swapfunc(a, b, n, swaptype)
;140:
;141:static char *med3(a, b, c, cmp) char *a, *b, *c;
;142:cmp_t*       cmp;
;143:{
;144:    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
;145:                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
;146:}
;147:
;148:void   qsort(a, n, es, cmp) void* a;
;149:size_t n, es;
;150:cmp_t* cmp;
;151:{
;152:    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
;153:    int32_t   d, r, swaptype, swap_cnt;
;154:
;155:loop:
;156:    SWAPINIT(a, es);
;157:    swap_cnt = 0;
;158:    if (n < 7)
;159:    {
;160:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
;161:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
;162:                swap(pl, pl - es);
;163:        return;
;164:    }
;165:    pm = (char*)a + (n / 2) * es;
;166:    if (n > 7)
;167:    {
;168:        pl = a;
;169:        pn = (char*)a + (n - 1) * es;
;170:        if (n > 40)
;171:        {
;172:            d  = (n / 8) * es;
;173:            pl = med3(pl, pl + d, pl + 2 * d, cmp);
;174:            pm = med3(pm - d, pm, pm + d, cmp);
;175:            pn = med3(pn - 2 * d, pn - d, pn, cmp);
;176:        }
;177:        pm = med3(pl, pm, pn, cmp);
;178:    }
;179:    swap(a, pm);
;180:    pa = pb = (char*)a + es;
;181:
;182:    pc = pd = (char*)a + (n - 1) * es;
;183:    for (;;)
;184:    {
;185:        while (pb <= pc && (r = cmp(pb, a)) <= 0)
;186:        {
;187:            if (r == 0)
;188:            {
;189:                swap_cnt = 1;
;190:                swap(pa, pb);
;191:                pa += es;
;192:            }
;193:            pb += es;
;194:        }
;195:        while (pb <= pc && (r = cmp(pc, a)) >= 0)
;196:        {
;197:            if (r == 0)
;198:            {
;199:                swap_cnt = 1;
;200:                swap(pc, pd);
;201:                pd -= es;
;202:            }
;203:            pc -= es;
;204:        }
;205:        if (pb > pc)
;206:            break;
;207:        swap(pb, pc);
;208:        swap_cnt = 1;
;209:        pb += es;
;210:        pc -= es;
;211:    }
;212:    if (swap_cnt == 0)
;213:    { /* Switch to insertion sort */
;214:        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
;215:            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
;216:                swap(pl, pl - es);
;217:        return;
;218:    }
;219:
;220:    pn = (char*)a + n * es;
;221:    r  = min(pa - (char*)a, pb - pa);
;222:    vecswap(a, pb - r, r);
;223:    r = min(pd - pc, pn - pd - es);
;224:    vecswap(pb, pn - r, r);
;225:    if ((r = pb - pa) > es)
;226:        qsort(a, r / es, es, cmp);
;227:    if ((r = pd - pc) > es)
;228:    {
;229:        /* Iterate rather than recurse to save stack space */
;230:        a = pn - r;
;231:        n = r / es;
;232:        goto loop;
;233:    }
;234:    /*      qsort(pn - r, r / es, es, cmp);*/
;235:}
;236:#endif
;237:
;238:int32_t tolower(int32_t c)
;239:{
line 240
;240:    if (c >= 'A' && c <= 'Z') {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 65
LTI4 $2
ADDRLP4 0
INDIRI4
CNSTI4 90
GTI4 $2
line 241
;241:        c += 'a' - 'A';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 32
ADDI4
ASGNI4
line 242
;242:    }
LABELV $2
line 243
;243:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $1
endproc tolower 4 0
export toupper
proc toupper 4 0
line 247
;244:}
;245:
;246:int32_t toupper(int32_t c)
;247:{
line 248
;248:    if (c >= 'a' && c <= 'z') {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 97
LTI4 $5
ADDRLP4 0
INDIRI4
CNSTI4 122
GTI4 $5
line 249
;249:        c += 'A' - 'a';
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 -32
ADDI4
ASGNI4
line 250
;250:    }
LABELV $5
line 251
;251:    return c;
ADDRFP4 0
INDIRI4
RETI4
LABELV $4
endproc toupper 4 0
export atof
proc atof 32 0
line 256
;252:}
;253:
;254:
;255:double atof(const char* string)
;256:{
ADDRGP4 $9
JUMPV
LABELV $8
line 263
;257:    float sign;
;258:    float value;
;259:    int32_t   c;
;260:
;261:    // skip whitespace
;262:    while (*string <= ' ')
;263:    {
line 264
;264:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $11
line 265
;265:        {
line 266
;266:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $7
JUMPV
LABELV $11
line 268
;267:        }
;268:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 269
;269:    }
LABELV $9
line 262
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $8
line 272
;270:
;271:    // check sign
;272:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $16
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $17
ADDRGP4 $13
JUMPV
line 273
;273:    {
LABELV $16
line 275
;274:    case '+':
;275:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 276
;276:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 277
;277:        break;
ADDRGP4 $14
JUMPV
LABELV $17
line 279
;278:    case '-':
;279:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 280
;280:        sign = -1;
ADDRLP4 8
CNSTF4 3212836864
ASGNF4
line 281
;281:        break;
ADDRGP4 $14
JUMPV
LABELV $13
line 283
;282:    default:
;283:        sign = 1;
ADDRLP4 8
CNSTF4 1065353216
ASGNF4
line 284
;284:        break;
LABELV $14
line 288
;285:    }
;286:
;287:    // read digits
;288:    value = 0;
ADDRLP4 4
CNSTF4 0
ASGNF4
line 289
;289:    c     = string[0];
ADDRLP4 0
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
line 290
;290:    if (c != '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
EQI4 $18
line 291
;291:    {
LABELV $20
line 293
;292:        do
;293:        {
line 294
;294:            c = *string++;
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
line 295
;295:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $25
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $23
LABELV $25
line 296
;296:            {
line 297
;297:                break;
ADDRGP4 $19
JUMPV
LABELV $23
line 299
;298:            }
;299:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 300
;300:            value = value * 10 + c;
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
line 301
;301:        } while (1);
LABELV $21
ADDRGP4 $20
JUMPV
line 302
;302:    }
ADDRGP4 $19
JUMPV
LABELV $18
line 304
;303:    else
;304:    {
line 305
;305:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 306
;306:    }
LABELV $19
line 309
;307:
;308:    // check for decimal point
;309:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $26
line 310
;310:    {
line 313
;311:        double fraction;
;312:
;313:        fraction = 0.1;
ADDRLP4 20
CNSTF4 1036831949
ASGNF4
LABELV $28
line 315
;314:        do
;315:        {
line 316
;316:            c = *string++;
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
line 317
;317:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $33
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $31
LABELV $33
line 318
;318:            {
line 319
;319:                break;
ADDRGP4 $30
JUMPV
LABELV $31
line 321
;320:            }
;321:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 322
;322:            value += c * fraction;
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
line 323
;323:            fraction *= 0.1;
ADDRLP4 20
CNSTF4 1036831949
ADDRLP4 20
INDIRF4
MULF4
ASGNF4
line 324
;324:        } while (1);
LABELV $29
ADDRGP4 $28
JUMPV
LABELV $30
line 325
;325:    }
LABELV $26
line 329
;326:
;327:    // not handling 10e10 notation...
;328:
;329:    return value * sign;
ADDRLP4 4
INDIRF4
ADDRLP4 8
INDIRF4
MULF4
RETF4
LABELV $7
endproc atof 32 0
export _atof
proc _atof 36 0
line 333
;330:}
;331:
;332:double _atof(const char** stringPtr)
;333:{
line 337
;334:    const char* string;
;335:    float       sign;
;336:    float       value;
;337:    int32_t         c = '0'; // bk001211 - uninitialized use possible
ADDRLP4 0
CNSTI4 48
ASGNI4
line 339
;338:
;339:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $36
JUMPV
LABELV $35
line 343
;340:
;341:    // skip whitespace
;342:    while (*string <= ' ')
;343:    {
line 344
;344:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $38
line 345
;345:        {
line 346
;346:            *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 347
;347:            return 0;
CNSTF4 0
RETF4
ADDRGP4 $34
JUMPV
LABELV $38
line 349
;348:        }
;349:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 350
;350:    }
LABELV $36
line 342
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $35
line 353
;351:
;352:    // check sign
;353:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $43
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $44
ADDRGP4 $40
JUMPV
line 354
;354:    {
LABELV $43
line 356
;355:    case '+':
;356:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 357
;357:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 358
;358:        break;
ADDRGP4 $41
JUMPV
LABELV $44
line 360
;359:    case '-':
;360:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 361
;361:        sign = -1;
ADDRLP4 12
CNSTF4 3212836864
ASGNF4
line 362
;362:        break;
ADDRGP4 $41
JUMPV
LABELV $40
line 364
;363:    default:
;364:        sign = 1;
ADDRLP4 12
CNSTF4 1065353216
ASGNF4
line 365
;365:        break;
LABELV $41
line 369
;366:    }
;367:
;368:    // read digits
;369:    value = 0;
ADDRLP4 8
CNSTF4 0
ASGNF4
line 370
;370:    if (string[0] != '.')
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 46
EQI4 $45
line 371
;371:    {
LABELV $47
line 373
;372:        do
;373:        {
line 374
;374:            c = *string++;
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
line 375
;375:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $52
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $50
LABELV $52
line 376
;376:            {
line 377
;377:                break;
ADDRGP4 $49
JUMPV
LABELV $50
line 379
;378:            }
;379:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 380
;380:            value = value * 10 + c;
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
line 381
;381:        } while (1);
LABELV $48
ADDRGP4 $47
JUMPV
LABELV $49
line 382
;382:    }
LABELV $45
line 385
;383:
;384:    // check for decimal point
;385:    if (c == '.')
ADDRLP4 0
INDIRI4
CNSTI4 46
NEI4 $53
line 386
;386:    {
line 389
;387:        double fraction;
;388:
;389:        fraction = 0.1;
ADDRLP4 24
CNSTF4 1036831949
ASGNF4
LABELV $55
line 391
;390:        do
;391:        {
line 392
;392:            c = *string++;
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
line 393
;393:            if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $60
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $58
LABELV $60
line 394
;394:            {
line 395
;395:                break;
ADDRGP4 $57
JUMPV
LABELV $58
line 397
;396:            }
;397:            c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 398
;398:            value += c * fraction;
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
line 399
;399:            fraction *= 0.1;
ADDRLP4 24
CNSTF4 1036831949
ADDRLP4 24
INDIRF4
MULF4
ASGNF4
line 400
;400:        } while (1);
LABELV $56
ADDRGP4 $55
JUMPV
LABELV $57
line 401
;401:    }
LABELV $53
line 404
;402:
;403:    // not handling 10e10 notation...
;404:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 406
;405:
;406:    return value * sign;
ADDRLP4 8
INDIRF4
ADDRLP4 12
INDIRF4
MULF4
RETF4
LABELV $34
endproc _atof 36 0
export atoi
proc atoi 28 0
line 410
;407:}
;408:
;409:int32_t atoi(const char* string)
;410:{
ADDRGP4 $63
JUMPV
LABELV $62
line 417
;411:    int32_t sign;
;412:    int32_t value;
;413:    int32_t c;
;414:
;415:    // skip whitespace
;416:    while (*string <= ' ')
;417:    {
line 418
;418:        if (!*string)
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $65
line 419
;419:        {
line 420
;420:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $61
JUMPV
LABELV $65
line 422
;421:        }
;422:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 423
;423:    }
LABELV $63
line 416
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $62
line 426
;424:
;425:    // check sign
;426:    switch (*string)
ADDRLP4 12
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 43
EQI4 $70
ADDRLP4 12
INDIRI4
CNSTI4 45
EQI4 $71
ADDRGP4 $67
JUMPV
line 427
;427:    {
LABELV $70
line 429
;428:    case '+':
;429:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 430
;430:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 431
;431:        break;
ADDRGP4 $68
JUMPV
LABELV $71
line 433
;432:    case '-':
;433:        string++;
ADDRFP4 0
ADDRFP4 0
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 434
;434:        sign = -1;
ADDRLP4 8
CNSTI4 -1
ASGNI4
line 435
;435:        break;
ADDRGP4 $68
JUMPV
LABELV $67
line 437
;436:    default:
;437:        sign = 1;
ADDRLP4 8
CNSTI4 1
ASGNI4
line 438
;438:        break;
LABELV $68
line 442
;439:    }
;440:
;441:    // read digits
;442:    value = 0;
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $72
line 444
;443:    do
;444:    {
line 445
;445:        c = *string++;
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
line 446
;446:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $77
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $75
LABELV $77
line 447
;447:        {
line 448
;448:            break;
ADDRGP4 $74
JUMPV
LABELV $75
line 450
;449:        }
;450:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 451
;451:        value = value * 10 + c;
ADDRLP4 4
CNSTI4 10
ADDRLP4 4
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 452
;452:    } while (1);
LABELV $73
ADDRGP4 $72
JUMPV
LABELV $74
line 456
;453:
;454:    // not handling 10e10 notation...
;455:
;456:    return value * sign;
ADDRLP4 4
INDIRI4
ADDRLP4 8
INDIRI4
MULI4
RETI4
LABELV $61
endproc atoi 28 0
export _atoi
proc _atoi 32 0
line 460
;457:}
;458:
;459:int32_t _atoi(const char** stringPtr)
;460:{
line 466
;461:    int32_t         sign;
;462:    int32_t         value;
;463:    int32_t         c;
;464:    const char* string;
;465:
;466:    string = *stringPtr;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $80
JUMPV
LABELV $79
line 470
;467:
;468:    // skip whitespace
;469:    while (*string <= ' ')
;470:    {
line 471
;471:        if (!*string)
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $82
line 472
;472:        {
line 473
;473:            return 0;
CNSTI4 0
RETI4
ADDRGP4 $78
JUMPV
LABELV $82
line 475
;474:        }
;475:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 476
;476:    }
LABELV $80
line 469
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
LEI4 $79
line 479
;477:
;478:    // check sign
;479:    switch (*string)
ADDRLP4 16
ADDRLP4 4
INDIRP4
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 43
EQI4 $87
ADDRLP4 16
INDIRI4
CNSTI4 45
EQI4 $88
ADDRGP4 $84
JUMPV
line 480
;480:    {
LABELV $87
line 482
;481:    case '+':
;482:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 483
;483:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 484
;484:        break;
ADDRGP4 $85
JUMPV
LABELV $88
line 486
;485:    case '-':
;486:        string++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 487
;487:        sign = -1;
ADDRLP4 12
CNSTI4 -1
ASGNI4
line 488
;488:        break;
ADDRGP4 $85
JUMPV
LABELV $84
line 490
;489:    default:
;490:        sign = 1;
ADDRLP4 12
CNSTI4 1
ASGNI4
line 491
;491:        break;
LABELV $85
line 495
;492:    }
;493:
;494:    // read digits
;495:    value = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $89
line 497
;496:    do
;497:    {
line 498
;498:        c = *string++;
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
line 499
;499:        if (c < '0' || c > '9')
ADDRLP4 0
INDIRI4
CNSTI4 48
LTI4 $94
ADDRLP4 0
INDIRI4
CNSTI4 57
LEI4 $92
LABELV $94
line 500
;500:        {
line 501
;501:            break;
ADDRGP4 $91
JUMPV
LABELV $92
line 503
;502:        }
;503:        c -= '0';
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 48
SUBI4
ASGNI4
line 504
;504:        value = value * 10 + c;
ADDRLP4 8
CNSTI4 10
ADDRLP4 8
INDIRI4
MULI4
ADDRLP4 0
INDIRI4
ADDI4
ASGNI4
line 505
;505:    } while (1);
LABELV $90
ADDRGP4 $89
JUMPV
LABELV $91
line 509
;506:
;507:    // not handling 10e10 notation...
;508:
;509:    *stringPtr = string;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 511
;510:
;511:    return value * sign;
ADDRLP4 8
INDIRI4
ADDRLP4 12
INDIRI4
MULI4
RETI4
LABELV $78
endproc _atoi 32 0
export tan
proc tan 8 4
line 515
;512:}
;513:
;514:double tan(double x)
;515:{
line 516
;516:    return sin(x) / cos(x);
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
LABELV $95
endproc tan 8 4
data
align 4
LABELV randSeed
byte 4 0
export srand
code
proc srand 0 0
line 522
;517:}
;518:
;519:static int32_t randSeed = 0;
;520:
;521:void srand(unsigned seed)
;522:{
line 523
;523:    randSeed = seed;
ADDRGP4 randSeed
ADDRFP4 0
INDIRU4
CVUI4 4
ASGNI4
line 524
;524:}
LABELV $96
endproc srand 0 0
export rand
proc rand 4 0
line 527
;525:
;526:int32_t rand(void)
;527:{
line 528
;528:    randSeed = (69069 * randSeed + 1);
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
line 529
;529:    return randSeed & 0x7fff;
ADDRGP4 randSeed
INDIRI4
CNSTI4 32767
BANDI4
RETI4
LABELV $97
endproc rand 4 0
export abs
proc abs 4 0
line 534
;530:}
;531:
;532:
;533:int32_t abs(int32_t n)
;534:{
line 535
;535:    return n < 0 ? -n : n;
ADDRFP4 0
INDIRI4
CNSTI4 0
GEI4 $100
ADDRLP4 0
ADDRFP4 0
INDIRI4
NEGI4
ASGNI4
ADDRGP4 $101
JUMPV
LABELV $100
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
LABELV $101
ADDRLP4 0
INDIRI4
RETI4
LABELV $98
endproc abs 4 0
export fabs
proc fabs 4 0
line 539
;536:}
;537:
;538:double fabs(double x)
;539:{
line 540
;540:    return x < 0 ? -x : x;
ADDRFP4 0
INDIRF4
CNSTF4 0
GEF4 $104
ADDRLP4 0
ADDRFP4 0
INDIRF4
NEGF4
ASGNF4
ADDRGP4 $105
JUMPV
LABELV $104
ADDRLP4 0
ADDRFP4 0
INDIRF4
ASGNF4
LABELV $105
ADDRLP4 0
INDIRF4
RETF4
LABELV $102
endproc fabs 4 0
export AddInt
proc AddInt 56 0
line 558
;541:}
;542:
;543:#define ALT 0x00000001       /* alternate form */
;544:#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
;545:#define LADJUST 0x00000004   /* left adjustment */
;546:#define LONGDBL 0x00000008   /* long double */
;547:#define LONGINT 0x00000010   /* long integer */
;548:#define QUADINT 0x00000020   /* quad integer */
;549:#define SHORTINT 0x00000040  /* short integer */
;550:#define ZEROPAD 0x00000080   /* zero (as opposed to blank) pad */
;551:#define FPT 0x00000100       /* floating point32_t number */
;552:
;553:#define to_digit(c) ((c) - '0')
;554:#define is_digit(c) ((unsigned)to_digit(c) <= 9)
;555:#define to_char(n) ((n) + '0')
;556:
;557:void AddInt(char** buf_p, int32_t val, int32_t width, int32_t flags)
;558:{
line 564
;559:    char  text[32];
;560:    int32_t   digits;
;561:    int32_t   signedVal;
;562:    char* buf;
;563:
;564:    digits    = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 565
;565:    signedVal = val;
ADDRLP4 40
ADDRFP4 4
INDIRI4
ASGNI4
line 566
;566:    if (val < 0)
ADDRFP4 4
INDIRI4
CNSTI4 0
GEI4 $107
line 567
;567:    {
line 568
;568:        val = -val;
ADDRFP4 4
ADDRFP4 4
INDIRI4
NEGI4
ASGNI4
line 569
;569:    }
LABELV $107
LABELV $109
line 571
;570:    do
;571:    {
line 572
;572:        text[digits++] = '0' + val % 10;
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
line 573
;573:        val /= 10;
ADDRFP4 4
ADDRFP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 574
;574:    } while (val);
LABELV $110
ADDRFP4 4
INDIRI4
CNSTI4 0
NEI4 $109
line 576
;575:
;576:    if (signedVal < 0)
ADDRLP4 40
INDIRI4
CNSTI4 0
GEI4 $112
line 577
;577:    {
line 578
;578:        text[digits++] = '-';
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
line 579
;579:    }
LABELV $112
line 581
;580:
;581:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 583
;582:
;583:    if (!(flags & LADJUST))
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
NEI4 $123
line 584
;584:    {
ADDRGP4 $117
JUMPV
LABELV $116
line 586
;585:        while (digits < width)
;586:        {
line 587
;587:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $120
ADDRLP4 44
CNSTI4 48
ASGNI4
ADDRGP4 $121
JUMPV
LABELV $120
ADDRLP4 44
CNSTI4 32
ASGNI4
LABELV $121
ADDRLP4 48
INDIRP4
ADDRLP4 44
INDIRI4
CVII1 4
ASGNI1
line 588
;588:            width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 589
;589:        }
LABELV $117
line 585
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $116
line 590
;590:    }
ADDRGP4 $123
JUMPV
LABELV $122
line 593
;591:
;592:    while (digits--)
;593:    {
line 594
;594:        *buf++ = text[digits];
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
line 595
;595:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 596
;596:    }
LABELV $123
line 592
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
NEI4 $122
line 598
;597:
;598:    if (flags & LADJUST)
ADDRFP4 12
INDIRI4
CNSTI4 4
BANDI4
CNSTI4 0
EQI4 $125
line 599
;599:    {
ADDRGP4 $128
JUMPV
LABELV $127
line 601
;600:        while (width--)
;601:        {
line 602
;602:            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
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
EQI4 $131
ADDRLP4 48
CNSTI4 48
ASGNI4
ADDRGP4 $132
JUMPV
LABELV $131
ADDRLP4 48
CNSTI4 32
ASGNI4
LABELV $132
ADDRLP4 52
INDIRP4
ADDRLP4 48
INDIRI4
CVII1 4
ASGNI1
line 603
;603:        }
LABELV $128
line 600
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
NEI4 $127
line 604
;604:    }
LABELV $125
line 606
;605:
;606:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 607
;607:}
LABELV $106
endproc AddInt 56 0
export AddFloat
proc AddFloat 60 0
line 610
;608:
;609:void AddFloat(char** buf_p, float fval, int32_t width, int32_t prec)
;610:{
line 618
;611:    char  text[32];
;612:    int32_t   digits;
;613:    float signedVal;
;614:    char* buf;
;615:    int32_t   val;
;616:
;617:    // get the sign
;618:    signedVal = fval;
ADDRLP4 44
ADDRFP4 4
INDIRF4
ASGNF4
line 619
;619:    if (fval < 0)
ADDRFP4 4
INDIRF4
CNSTF4 0
GEF4 $134
line 620
;620:    {
line 621
;621:        fval = -fval;
ADDRFP4 4
ADDRFP4 4
INDIRF4
NEGF4
ASGNF4
line 622
;622:    }
LABELV $134
line 625
;623:
;624:    // write the float number
;625:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
line 626
;626:    val    = (int32_t)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
LABELV $136
line 628
;627:    do
;628:    {
line 629
;629:        text[digits++] = '0' + val % 10;
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
line 630
;630:        val /= 10;
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 10
DIVI4
ASGNI4
line 631
;631:    } while (val);
LABELV $137
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $136
line 633
;632:
;633:    if (signedVal < 0)
ADDRLP4 44
INDIRF4
CNSTF4 0
GEF4 $139
line 634
;634:    {
line 635
;635:        text[digits++] = '-';
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
line 636
;636:    }
LABELV $139
line 638
;637:
;638:    buf = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
ADDRGP4 $142
JUMPV
LABELV $141
line 641
;639:
;640:    while (digits < width)
;641:    {
line 642
;642:        *buf++ = ' ';
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
line 643
;643:        width--;
ADDRFP4 8
ADDRFP4 8
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 644
;644:    }
LABELV $142
line 640
ADDRLP4 0
INDIRI4
ADDRFP4 8
INDIRI4
LTI4 $141
ADDRGP4 $145
JUMPV
LABELV $144
line 647
;645:
;646:    while (digits--)
;647:    {
line 648
;648:        *buf++ = text[digits];
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
line 649
;649:    }
LABELV $145
line 646
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
NEI4 $144
line 651
;650:
;651:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 653
;652:
;653:    if (prec < 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
GEI4 $147
line 654
;654:        prec = 6;
ADDRFP4 12
CNSTI4 6
ASGNI4
LABELV $147
line 656
;655:    // write the fraction
;656:    digits = 0;
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $150
JUMPV
LABELV $149
line 658
;657:    while (digits < prec)
;658:    {
line 659
;659:        fval -= (int)fval;
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
line 660
;660:        fval *= 10.0;
ADDRFP4 4
CNSTF4 1092616192
ADDRFP4 4
INDIRF4
MULF4
ASGNF4
line 661
;661:        val            = (int)fval;
ADDRLP4 4
ADDRFP4 4
INDIRF4
CVFI4 4
ASGNI4
line 662
;662:        text[digits++] = '0' + val % 10;
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
line 663
;663:    }
LABELV $150
line 657
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $149
line 665
;664:
;665:    if (digits > 0)
ADDRLP4 0
INDIRI4
CNSTI4 0
LEI4 $152
line 666
;666:    {
line 667
;667:        buf    = *buf_p;
ADDRLP4 40
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 668
;668:        *buf++ = '.';
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
line 669
;669:        for (prec = 0; prec < digits; prec++)
ADDRFP4 12
CNSTI4 0
ASGNI4
ADDRGP4 $157
JUMPV
LABELV $154
line 670
;670:        {
line 671
;671:            *buf++ = text[prec];
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
line 672
;672:        }
LABELV $155
line 669
ADDRFP4 12
ADDRFP4 12
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $157
ADDRFP4 12
INDIRI4
ADDRLP4 0
INDIRI4
LTI4 $154
line 673
;673:        *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 40
INDIRP4
ASGNP4
line 674
;674:    }
LABELV $152
line 675
;675:}
LABELV $133
endproc AddFloat 60 0
export AddString
proc AddString 20 4
line 678
;676:
;677:void AddString(char** buf_p, char* string, int32_t width, int32_t prec)
;678:{
line 682
;679:    int32_t   size;
;680:    char* buf;
;681:
;682:    buf = *buf_p;
ADDRLP4 4
ADDRFP4 0
INDIRP4
INDIRP4
ASGNP4
line 684
;683:
;684:    if (string == NULL)
ADDRFP4 4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $159
line 685
;685:    {
line 686
;686:        string = "(null)";
ADDRFP4 4
ADDRGP4 $161
ASGNP4
line 687
;687:        prec   = -1;
ADDRFP4 12
CNSTI4 -1
ASGNI4
line 688
;688:    }
LABELV $159
line 690
;689:
;690:    if (prec >= 0)
ADDRFP4 12
INDIRI4
CNSTI4 0
LTI4 $162
line 691
;691:    {
line 692
;692:        for (size = 0; size < prec; size++)
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $167
JUMPV
LABELV $164
line 693
;693:        {
line 694
;694:            if (string[size] == '\0')
ADDRLP4 0
INDIRI4
ADDRFP4 4
INDIRP4
ADDP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $168
line 695
;695:            {
line 696
;696:                break;
ADDRGP4 $163
JUMPV
LABELV $168
line 698
;697:            }
;698:        }
LABELV $165
line 692
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $167
ADDRLP4 0
INDIRI4
ADDRFP4 12
INDIRI4
LTI4 $164
line 699
;699:    }
ADDRGP4 $163
JUMPV
LABELV $162
line 701
;700:    else
;701:    {
line 702
;702:        size = strlen(string);
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
line 703
;703:    }
LABELV $163
line 705
;704:
;705:    width -= size;
ADDRFP4 8
ADDRFP4 8
INDIRI4
ADDRLP4 0
INDIRI4
SUBI4
ASGNI4
ADDRGP4 $171
JUMPV
LABELV $170
line 708
;706:
;707:    while (size--)
;708:    {
line 709
;709:        *buf++ = *string++;
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
line 710
;710:    }
LABELV $171
line 707
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
NEI4 $170
ADDRGP4 $174
JUMPV
LABELV $173
line 713
;711:
;712:    while (width-- > 0)
;713:    {
line 714
;714:        *buf++ = ' ';
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
line 715
;715:    }
LABELV $174
line 712
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
GTI4 $173
line 717
;716:
;717:    *buf_p = buf;
ADDRFP4 0
INDIRP4
ADDRLP4 4
INDIRP4
ASGNP4
line 718
;718:}
LABELV $158
endproc AddString 20 4
export vsprintf
proc vsprintf 72 16
line 721
;719:
;720:int32_t vsprintf(char* buffer, const char* fmt, va_list argptr)
;721:{
line 731
;722:    int32_t*    arg;
;723:    char*       buf_p;
;724:    char        ch;
;725:    int32_t     flags;
;726:    int32_t     width;
;727:    int32_t     prec;
;728:    int32_t     n;
;729:    char        sign;
;730:
;731:    buf_p = buffer;
ADDRLP4 4
ADDRFP4 0
INDIRP4
ASGNP4
line 732
;732:    arg   = (int32_t *)argptr;
ADDRLP4 24
ADDRFP4 8
INDIRP4
ASGNP4
ADDRGP4 $178
JUMPV
LABELV $177
line 735
;733:
;734:    while (1)
;735:    {
line 737
;736:        // run through the format string until we hit a '%' or '\0'
;737:        for (ch = *fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
ADDRLP4 0
ADDRFP4 4
INDIRP4
INDIRI1
ASGNI1
ADDRGP4 $183
JUMPV
LABELV $180
line 738
;738:        {
line 739
;739:            *buf_p++ = ch;
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
line 740
;740:        }
LABELV $181
line 737
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
LABELV $183
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
EQI4 $184
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 37
NEI4 $180
LABELV $184
line 741
;741:        if (ch == '\0')
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $185
line 742
;742:        {
line 743
;743:            goto done;
ADDRGP4 $187
JUMPV
LABELV $185
line 747
;744:        }
;745:
;746:        // skip over the '%'
;747:        fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 750
;748:
;749:        // reset formatting state
;750:        flags = 0;
ADDRLP4 16
CNSTI4 0
ASGNI4
line 751
;751:        width = 0;
ADDRLP4 12
CNSTI4 0
ASGNI4
line 752
;752:        prec  = -1;
ADDRLP4 20
CNSTI4 -1
ASGNI4
line 753
;753:        sign  = '\0';
ADDRLP4 28
CNSTI1 0
ASGNI1
LABELV $188
line 756
;754:
;755:    rflag:
;756:        ch = *fmt++;
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
LABELV $189
line 758
;757:    reswitch:
;758:        switch (ch)
ADDRLP4 36
ADDRLP4 0
INDIRI1
CVII4 1
ASGNI4
ADDRLP4 36
INDIRI4
CNSTI4 99
LTI4 $211
ADDRLP4 36
INDIRI4
CNSTI4 105
GTI4 $212
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $213-396
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $213
address $206
address $207
address $190
address $208
address $190
address $190
address $207
code
LABELV $211
ADDRLP4 36
INDIRI4
CNSTI4 37
LTI4 $190
ADDRLP4 36
INDIRI4
CNSTI4 57
GTI4 $190
ADDRLP4 36
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $215-148
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $215
address $210
address $190
address $190
address $190
address $190
address $190
address $190
address $190
address $193
address $194
address $190
address $201
address $202
address $202
address $202
address $202
address $202
address $202
address $202
address $202
address $202
code
LABELV $212
ADDRLP4 36
INDIRI4
CNSTI4 115
EQI4 $209
ADDRGP4 $190
JUMPV
line 759
;759:        {
LABELV $193
line 761
;760:        case '-':
;761:            flags |= LADJUST;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 4
BORI4
ASGNI4
line 762
;762:            goto rflag;
ADDRGP4 $188
JUMPV
LABELV $194
line 764
;763:        case '.':
;764:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $196
JUMPV
LABELV $195
line 766
;765:            while (is_digit((ch = *fmt++)))
;766:            {
line 767
;767:                n = 10 * n + (ch - '0');
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
line 768
;768:            }
LABELV $196
line 765
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
LEU4 $195
line 769
;769:            prec = n < 0 ? -1 : n;
ADDRLP4 8
INDIRI4
CNSTI4 0
GEI4 $199
ADDRLP4 56
CNSTI4 -1
ASGNI4
ADDRGP4 $200
JUMPV
LABELV $199
ADDRLP4 56
ADDRLP4 8
INDIRI4
ASGNI4
LABELV $200
ADDRLP4 20
ADDRLP4 56
INDIRI4
ASGNI4
line 770
;770:            goto reswitch;
ADDRGP4 $189
JUMPV
LABELV $201
line 772
;771:        case '0':
;772:            flags |= ZEROPAD;
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 128
BORI4
ASGNI4
line 773
;773:            goto rflag;
ADDRGP4 $188
JUMPV
LABELV $202
line 783
;774:        case '1':
;775:        case '2':
;776:        case '3':
;777:        case '4':
;778:        case '5':
;779:        case '6':
;780:        case '7':
;781:        case '8':
;782:        case '9':
;783:            n = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $203
line 785
;784:            do
;785:            {
line 786
;786:                n  = 10 * n + (ch - '0');
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
line 787
;787:                ch = *fmt++;
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
line 788
;788:            } while (is_digit(ch));
LABELV $204
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
SUBI4
CVIU4 4
CNSTU4 9
LEU4 $203
line 789
;789:            width = n;
ADDRLP4 12
ADDRLP4 8
INDIRI4
ASGNI4
line 790
;790:            goto reswitch;
ADDRGP4 $189
JUMPV
LABELV $206
line 792
;791:        case 'c':
;792:            *buf_p++ = (char)*arg;
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
line 793
;793:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 794
;794:            break;
ADDRGP4 $191
JUMPV
LABELV $207
line 797
;795:        case 'd':
;796:        case 'i':
;797:            AddInt(&buf_p, *arg, width, flags);
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
line 798
;798:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 799
;799:            break;
ADDRGP4 $191
JUMPV
LABELV $208
line 801
;800:        case 'f':
;801:            AddFloat(&buf_p, *(double*)arg, width, prec);
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
line 803
;802:#ifdef __LCC__
;803:            arg += 1; // everything is 32 bit in my compiler
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 807
;804:#else
;805:            arg += 2;
;806:#endif
;807:            break;
ADDRGP4 $191
JUMPV
LABELV $209
line 809
;808:        case 's':
;809:            AddString(&buf_p, (char*)*arg, width, prec);
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
line 810
;810:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 811
;811:            break;
ADDRGP4 $191
JUMPV
LABELV $210
line 813
;812:        case '%':
;813:            *buf_p++ = ch;
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
line 814
;814:            break;
ADDRGP4 $191
JUMPV
LABELV $190
line 816
;815:        default:
;816:            *buf_p++ = (char)*arg;
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
line 817
;817:            arg++;
ADDRLP4 24
ADDRLP4 24
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 818
;818:            break;
LABELV $191
line 820
;819:        }
;820:    }
LABELV $178
line 734
ADDRGP4 $177
JUMPV
LABELV $187
line 823
;821:
;822:done:
;823:    *buf_p = 0;
ADDRLP4 4
INDIRP4
CNSTI1 0
ASGNI1
line 824
;824:    return buf_p - buffer;
ADDRLP4 4
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
RETI4
LABELV $176
endproc vsprintf 72 16
export sscanf
proc sscanf 28 4
line 829
;825:}
;826:
;827:/* this is really crappy */
;828:int32_t sscanf( const char *buffer, const char *fmt, ... )
;829:{
line 834
;830:	int32_t	cmd;
;831:	int32_t	**arg;
;832:	int32_t	count;
;833:
;834:	arg = (int32_t **)&fmt + 1;
ADDRLP4 4
ADDRFP4 4+4
ASGNP4
line 835
;835:	count = 0;
ADDRLP4 8
CNSTI4 0
ASGNI4
ADDRGP4 $220
JUMPV
LABELV $219
line 837
;836:
;837:	while ( *fmt ) {
line 838
;838:		if ( fmt[0] != '%' ) {
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 37
EQI4 $222
line 839
;839:			fmt++;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
ASGNP4
line 840
;840:			continue;
ADDRGP4 $220
JUMPV
LABELV $222
line 843
;841:		}
;842:
;843:		cmd = fmt[1];
ADDRLP4 0
ADDRFP4 4
INDIRP4
CNSTI4 1
ADDP4
INDIRI1
CVII4 1
ASGNI4
line 844
;844:		fmt += 2;
ADDRFP4 4
ADDRFP4 4
INDIRP4
CNSTI4 2
ADDP4
ASGNP4
line 846
;845:
;846:		switch ( cmd ) {
ADDRLP4 16
CNSTI4 100
ASGNI4
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
EQI4 $226
ADDRLP4 0
INDIRI4
CNSTI4 102
EQI4 $227
ADDRLP4 0
INDIRI4
CNSTI4 105
EQI4 $226
ADDRLP4 0
INDIRI4
ADDRLP4 16
INDIRI4
LTI4 $224
LABELV $228
ADDRLP4 0
INDIRI4
CNSTI4 117
EQI4 $226
ADDRGP4 $224
JUMPV
LABELV $226
line 850
;847:		case 'i':
;848:		case 'd':
;849:		case 'u':
;850:			**arg = _atoi( &buffer );
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
line 851
;851:			break;
ADDRGP4 $225
JUMPV
LABELV $227
line 853
;852:		case 'f':
;853:			*(float *)*arg = _atof( &buffer );
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
line 854
;854:			break;
LABELV $224
LABELV $225
line 856
;855:		}
;856:		arg++;
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 4
ADDP4
ASGNP4
line 857
;857:	}
LABELV $220
line 837
ADDRFP4 4
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $219
line 859
;858:
;859:	return count;
ADDRLP4 8
INDIRI4
RETI4
LABELV $217
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
import strcat
import strcpy
import memmove
import memset
import memchr
import memcpy
lit
align 1
LABELV $161
byte 1 40
byte 1 110
byte 1 117
byte 1 108
byte 1 108
byte 1 41
byte 1 0
