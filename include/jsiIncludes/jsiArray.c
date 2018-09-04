#ifndef JSI_LITE_ONLY
#ifndef JSI_AMALGAMATION
#include "jsiInt.h"
#endif

#if JSI__MUSL==1 || defined(__FreeBSD__)
#define NO_QSORT_R 1
#endif

static Jsi_RC ArrayPushCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Obj *obj;
    
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    obj = _this->d.obj;
    
    int argc = Jsi_ValueGetLength(interp, args);
    int curlen = Jsi_ObjGetLength(interp, obj);
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    
    int i;
    for (i = 0; i < argc; ++i) {
        Jsi_Value *ov = Jsi_ValueArrayIndex(interp, args, i);
        if (!ov) { Jsi_LogBug("Arguments Error"); ov = Jsi_ValueNew(interp); }
        jsi_ValueInsertArray(interp, _this, curlen + i, ov, 0);
    }
    
    Jsi_ValueMakeNumber(interp, ret, Jsi_ObjGetLength(interp, obj));
    return JSI_OK;
}

static Jsi_RC ArrayPopCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *v;
    Jsi_Obj *obj;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    obj = _this->d.obj;
    int i = Jsi_ObjGetLength(interp, obj) - 1;

    if (i < 0) {
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_OK;
    }
    
    if (obj->arr) {
        if ((v = obj->arr[i])) {
            obj->arr[i] = NULL;
            obj->arrCnt--;
        }
    } else {
        v = Jsi_ValueArrayIndex(interp, _this, i);
    }
    if (v) {
        Jsi_DecrRefCount(interp, *ret);
        *ret = v;
    }
    Jsi_ObjSetLength(interp, obj, i);
    return JSI_OK;
}


static Jsi_RC ArrayJoinCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *jstr = "";
    int argc, curlen;
    Jsi_DString dStr = {};
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        goto bail;
    }
    curlen = Jsi_ObjGetLength(interp, _this->d.obj);
    if (curlen == 0) {
        goto bail;
    }

    if (Jsi_ValueGetLength(interp, args) >= 1) {
        Jsi_Value *sc = Jsi_ValueArrayIndex(interp, args, 0);
        if (sc != NULL)
            jstr = Jsi_ValueToString(interp, sc, NULL);
    }
    
    if (0 == (argc=Jsi_ObjGetLength(interp, _this->d.obj))) {
        goto bail;
    }
    int i;
    for (i = 0; i < argc; ++i) {
        const char *cp;
        Jsi_Value *ov = Jsi_ValueArrayIndex(interp, _this, i);
        if (!ov) {
            /* TODO: are NULL args ok? */ 
            continue;
            cp = "";
        } else
            cp = Jsi_ValueToString(interp, ov, NULL);
        if (i && jstr[0])
            Jsi_DSAppend(&dStr, jstr, NULL);
        Jsi_DSAppend(&dStr, cp, NULL);
    }
    
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(Jsi_DSValue(&dStr)));
    Jsi_DSFree(&dStr);
    return JSI_OK;
bail:
    Jsi_ValueMakeString(interp, ret, Jsi_Strdup(""));
    return JSI_OK;        
}


Jsi_Value* Jsi_ValueArrayConcat(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2) {
    Jsi_Value *va;
    Jsi_Obj *obj;
    if (arg1->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, arg1->d.obj)) {
        return NULL;
    }
    if (arg2->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, arg2->d.obj)) {
        return NULL;
    }
    int len1 = arg1->d.obj->arrCnt;
    int len2 = arg2->d.obj->arrCnt;
    Jsi_Obj *nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    Jsi_ObjArraySizer(interp, nobj, len1+len2);

    int i, j = 0;
    obj = arg1->d.obj;
    for (i = 0; i<len1; i++, j++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[j] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+j, obj->arr[i]);
    }
    obj = arg2->d.obj;
    for (i = 0; i<len2; i++, j++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[j] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+j, obj->arr[i]);
    }
    Jsi_ObjSetLength(interp, nobj, len1+len2);
    va = Jsi_ValueMakeArrayObject(interp, NULL, nobj);
    return va;
}

Jsi_RC Jsi_ValueArrayPush(Jsi_Interp *interp, Jsi_Value *arg1, Jsi_Value *arg2) {
    Jsi_Obj *obj;
    if (arg1->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, arg1->d.obj))
        return JSI_ERROR;
    if (!arg2)
        return JSI_ERROR;
    int len1 = arg1->d.obj->arrCnt;
    obj = arg1->d.obj;
    Jsi_ObjArraySizer(interp, obj, len1);
    obj->arr[len1] = arg2;
    Jsi_IncrRefCount(interp, arg2);
    obj->arrCnt++;
    return JSI_OK;
}


Jsi_Value *Jsi_ValueArrayPop(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Jsi_ValueArrayPop, target is not object");
        return NULL;
    }
    Jsi_Obj *o = v->d.obj;
    if (!o->isarrlist) {
        Jsi_LogBug("Jsi_ValueArrayPop, target is not array");
        return NULL;
    }
    if (o->arrCnt<=0)
        return NULL;
    int idx = o->arrCnt-1;
    if (!o->arr[idx])
        return NULL;
    Jsi_DecrRefCount(interp, o->arr[idx]);
    Jsi_Value *ret = o->arr[idx];
    o->arr[idx] = NULL;
    o->arrCnt--;
    return ret;
}


Jsi_Value *Jsi_ValueArrayUnshift(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Jsi_ValueArrayUnshift, target is not object");
        return NULL;
    }
    Jsi_Obj *o = v->d.obj;
    if (!o->isarrlist) {
        Jsi_LogBug("Jsi_ValueArrayUnshift, target is not array");
        return NULL;
    }
    if (o->arrCnt<=0)
        return NULL;
    if (!o->arr[0])
        return NULL;
    Jsi_DecrRefCount(interp, o->arr[0]);
    Jsi_Value *ret = o->arr[0];
    o->arr[0] = NULL;
    o->arrCnt--;
    return ret;
}

/* delete array[0], array[1]->array[0] */
void Jsi_ValueArrayShift(Jsi_Interp *interp, Jsi_Value *v)
{
    if (v->vt != JSI_VT_OBJECT) {
        Jsi_LogBug("Jsi_ValueArrayShift, target is not object");
        return;
    }
    Jsi_Obj *o = v->d.obj;
    if (o->isarrlist) {
        uint i;
        if (!o->arrCnt)
            return;
        if (o->arr[0])
            Jsi_DecrRefCount(interp, o->arr[0]);
        for (i=1; i<o->arrCnt; i++) {
            o->arr[i-1] = o->arr[i];
        }
        o->arr[o->arrCnt--] = NULL;
        return;
    }
    
    int len = Jsi_ObjGetLength(interp, v->d.obj);
    if (len <= 0) return;
    
    Jsi_Value *v0 = Jsi_ValueArrayIndex(interp, v, 0);
    if (!v0) return;
    
    Jsi_ValueReset(interp, &v0);
    
    int i;
    Jsi_Value *last = v0;
    for (i = 1; i < len; ++i) {
        Jsi_Value *t = Jsi_ValueArrayIndex(interp, v, i);
        if (!t) return;
        Jsi_ValueCopy(interp, last, t);
        Jsi_ValueReset(interp, &t);
        last = t;
    }
    Jsi_ObjSetLength(interp, v->d.obj, len - 1);
}

static Jsi_RC ArrayConcatCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Value *va;
    Jsi_Obj *obj;
    int curlen, argc, nsiz;
    Jsi_RC rc = JSI_OK;
    Jsi_Obj *nobj;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        goto bail;
    }
    obj = _this->d.obj;
    
    argc = Jsi_ValueGetLength(interp, args);
    curlen = Jsi_ObjGetLength(interp, obj);
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    Jsi_ObjListifyArray(interp, obj);
   
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    nsiz = obj->arrMaxSize;
    if (nsiz<=0) nsiz = 100;
    if (Jsi_ObjArraySizer(interp, nobj, nsiz+1) <= 0) {
        rc = JSI_ERROR;
        Jsi_LogError("index too large: %d", nsiz+1);
        goto bail;
    }

    int i, j, m;
    for (i = 0; i<curlen; i++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[i] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+i, obj->arr[i]);
    }
    m = i;
    for (i = 0; i < argc; i++) {
         va = Jsi_ValueArrayIndex(interp, args, i);
         if (va->vt == JSI_VT_OBJECT && Jsi_ObjIsArray(interp, va->d.obj)) {
            int margc = Jsi_ValueGetLength(interp, va);
            Jsi_Obj *mobj = va->d.obj;
            Jsi_ObjListifyArray(interp, mobj);
            if (Jsi_ObjArraySizer(interp, nobj, curlen += margc) <= 0) {
                rc = JSI_ERROR;
                Jsi_LogError("index too large: %d", curlen);
                goto bail;
            }
            for (j = 0; j<margc; j++, m++)
            {
                if (!mobj->arr[j]) continue;
                nobj->arr[m] = NULL;
                Jsi_ValueDup2(interp, nobj->arr+m, mobj->arr[j]);
            }
        } else {
            if (Jsi_ObjArraySizer(interp, nobj, ++curlen) <= 0) {
                rc = JSI_ERROR;
                Jsi_LogError("index too large: %d", curlen);
                goto bail;
            }
            nobj->arr[m] = NULL;
            Jsi_ValueDup2(interp, nobj->arr+m++, va);
       }
    }
    Jsi_ObjSetLength(interp, nobj, curlen);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    return JSI_OK;
        
bail:
    Jsi_ValueMakeNull(interp, ret);
    return rc;
}

static Jsi_RC ArrayMapCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Obj *obj, *nobj;
    int curlen, nsiz;
    int i;
    Jsi_RC rc = JSI_OK;
    Jsi_Value *func, *vpargs, *nthis = NULL, *sthis;

    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        goto bail;
    }
    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    sthis = Jsi_ValueArrayIndex(interp, args, 1);
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);
    obj = _this->d.obj;
    curlen = Jsi_ObjGetLength(interp, obj);    
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    Jsi_ObjListifyArray(interp, obj);
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    nsiz = obj->arrCnt;
    if (nsiz<=0) nsiz = 1;
    if (Jsi_ObjArraySizer(interp, nobj, nsiz) <= 0) {
        Jsi_LogError("index too large: %d", nsiz);
        rc = JSI_ERROR;
        goto bail;
    }
    Jsi_ValueMakeArrayObject(interp, ret, nobj);

    for (i = 0; i < curlen; i++) {
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, obj->arr+i, 1, 0));
        Jsi_IncrRefCount(interp, vpargs);
        nobj->arr[i] = Jsi_ValueNew1(interp);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, nobj->arr+i, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        if( JSI_OK!=rc ) {
            goto bail;
        }
    }
    Jsi_ObjSetLength(interp, nobj, curlen);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return JSI_OK;
        
bail:
    Jsi_ValueMakeNull(interp, ret);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return rc;
}

static Jsi_RC ArrayFilterCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Obj *obj, *nobj;
    int curlen, nsiz;
    int i;
    Jsi_RC rc = JSI_OK;
    Jsi_Value *func, *vpargs, *nthis = NULL;
    int fval, n = 0;
    Jsi_Value *sthis, *nrPtr = NULL;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        goto bail;
    }
    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    sthis = Jsi_ValueArrayIndex(interp, args, 1);
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);
    obj = _this->d.obj;
    curlen = Jsi_ObjGetLength(interp, obj);    
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    Jsi_ObjListifyArray(interp, obj);
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    nsiz = obj->arrCnt;
    if (nsiz<=0) nsiz = 1;
    if (Jsi_ObjArraySizer(interp, nobj, nsiz) <= 0) {
        Jsi_LogError("index too large: %d", nsiz);
        rc = JSI_ERROR;
        goto bail;
    }
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    nrPtr = Jsi_ValueNew1(interp);
    for (i = 0; i < curlen; i++) {
        if (!obj->arr[i]) continue;
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, obj->arr+i, 1, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, &nrPtr, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        fval = Jsi_ValueIsTrue(interp, nrPtr);
        Jsi_ValueMakeUndef(interp, &nrPtr);
        if( JSI_OK!=rc ) {
            goto bail;
        }
        if (fval) {
            nobj->arr[n++] = obj->arr[i];
            Jsi_IncrRefCount(interp, obj->arr[i]);
        }
    }
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    Jsi_DecrRefCount(interp, nrPtr);
    Jsi_ObjSetLength(interp, nobj, n);
    return JSI_OK;
        
bail:
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    if (nrPtr)
        Jsi_DecrRefCount(interp, nrPtr);
    Jsi_ValueMakeNull(interp, ret);
    return rc;
}


static Jsi_RC ArrayReverseCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this, Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Obj *obj;
    int i, n, m;
    Jsi_Value *tval;

    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    Jsi_Value *sthis = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *nthis = NULL;
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);

    obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    m = obj->arrCnt/2;
    for (i = 0, n=obj->arrCnt-1; i < m; i++, n--) {
        tval = obj->arr[i];
        obj->arr[i] = obj->arr[n];
        obj->arr[n] = tval;
    }
    Jsi_ValueDup2(interp, ret, _this);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return JSI_OK;
}

static Jsi_RC ArrayForeachCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) 
{
    Jsi_Obj *obj;
    int curlen;
    uint i;
    Jsi_Value *func, *vpargs;

    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    Jsi_Value *sthis = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *nthis = NULL;
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);

    obj = _this->d.obj;
    curlen = Jsi_ObjGetLength(interp, obj);    
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    Jsi_ObjListifyArray(interp, obj);

    for (i = 0; i < obj->arrCnt; i++) {
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, obj->arr+i, 1, 0));
        Jsi_IncrRefCount(interp, vpargs);
        Jsi_RC rc = Jsi_FunctionInvoke(interp, func, vpargs, ret, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        if( JSI_OK!=rc ) {
            return rc;
        }
    }
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    return JSI_OK;
}

static Jsi_RC ArraySomeCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Obj *obj;
    int curlen;
    uint i;
    Jsi_RC rc = JSI_OK;
    Jsi_Value *func, *vpargs;
    Jsi_Value *sthis = Jsi_ValueArrayIndex(interp, args, 1);

    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    func = Jsi_ValueArrayIndex(interp, args, 0);
    if (!Jsi_ValueIsFunction(interp, func)) 
        return Jsi_LogError("expected function");
    Jsi_Value *nthis = NULL;
    if (!sthis)
        sthis = nthis = Jsi_ValueNew1(interp);

    obj = _this->d.obj;
    curlen = Jsi_ObjGetLength(interp, obj);    
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    Jsi_ObjListifyArray(interp, obj);
    int fval = 0;
    Jsi_Value *nrPtr = Jsi_ValueNew1(interp);
    for (i = 0; i < obj->arrCnt && rc == JSI_OK; i++) {
        vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, obj->arr+i, 1, 0));
        Jsi_IncrRefCount(interp, vpargs);
        rc = Jsi_FunctionInvoke(interp, func, vpargs, &nrPtr, sthis);
        Jsi_DecrRefCount(interp, vpargs);
        fval = Jsi_ValueIsTrue(interp, nrPtr);
        Jsi_ValueMakeUndef(interp, &nrPtr);
        if (fval)
            break;
    }
    if (rc == JSI_OK)
        Jsi_ValueMakeBool(interp, ret, fval);
    if (nthis)
        Jsi_DecrRefCount(interp, nthis);
    Jsi_DecrRefCount(interp, nrPtr);
    return rc;

}
/* // Not required as typeof returns "array"
static int ArrayIsarrayCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    int isa = (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj));
    Jsi_ValueMakeBool(interp, ret, isa);
    return JSI_OK;
}*/

static Jsi_RC ArrayIndexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, 1);
    int istart, n;
    Jsi_Obj *obj = _this->d.obj;
    if (!seq) {
        goto bail;
    }
    
    n = Jsi_ObjGetLength(interp, obj);    
    if (n == 0) {
        goto bail;
    }
    Jsi_Number nstart;
    istart = 0;
    if (start && Jsi_GetNumberFromValue(interp,start, &nstart)==JSI_OK) {
        istart = (int) nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            istart = 0;
    }
    Jsi_ObjListifyArray(interp, obj);
    int i;
    for (i = istart; i < n; i++)
    {
        if (obj->arr[i] && Jsi_ValueCmp(interp, obj->arr[i], seq, JSI_CMP_EXACT)==0) {
            Jsi_ValueMakeNumber(interp, ret, i);
            return JSI_OK;
        }
    }
bail:
    Jsi_ValueMakeNumber(interp, ret, -1);
    return JSI_OK;
}

static Jsi_RC ArrayLastindexOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_Value *seq = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, 1);
    int istart, n;
    Jsi_Obj *obj = _this->d.obj;
    if (!seq) {
        goto bail;
    }
    
    n = Jsi_ObjGetLength(interp, obj);    
    if (n == 0) {
        goto bail;
    }
    Jsi_Number nstart;
    istart = n-1;
    if (start && Jsi_GetNumberFromValue(interp,start, &nstart)==JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            goto bail;
    }
    Jsi_ObjListifyArray(interp, obj);
    int i;
    for (i = istart; i >= 0; i--)
    {
        if (obj->arr[i] && Jsi_ValueCmp(interp, obj->arr[i], seq, JSI_CMP_EXACT)==0) {
            Jsi_ValueMakeNumber(interp, ret, i);
            return JSI_OK;
        }
    }
bail:
    Jsi_ValueMakeNumber(interp, ret, -1);
    return JSI_OK;
}


static Jsi_RC ArraySizeOfCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    int i;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj))
        i = 0;
    else
        i = Jsi_ObjGetLength(interp, _this->d.obj);
    Jsi_ValueMakeNumber(interp, ret, i);
    return JSI_OK;
}

static Jsi_RC ArrayShiftCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Obj *obj;
    Jsi_Value *v;
    uint n;
    
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    obj = _this->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    n = Jsi_ObjGetLength(interp, obj);
    assert(n <= obj->arrCnt);
    if (n<=0) {
        Jsi_ValueMakeUndef(interp, ret);
    } else {
        n--;
        v = obj->arr[0];
        memmove(obj->arr, obj->arr+1, n*sizeof(Jsi_Value*));
        obj->arr[n] = NULL;
        Jsi_ValueDup2(interp, ret, v);
        Jsi_DecrRefCount(interp, v);
        Jsi_ObjSetLength(interp, obj, n);
    }
    return JSI_OK;
}

static Jsi_RC ArrayUnshiftCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
    Jsi_Obj *obj;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    obj = _this->d.obj;
    
    int argc = Jsi_ValueGetLength(interp, args);
    int curlen = Jsi_ObjGetLength(interp, obj);
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    if (argc <= 0) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_ObjListifyArray(interp, obj);
    if (Jsi_ObjArraySizer(interp, obj, curlen+argc)<=0) 
        return Jsi_LogError("too long");
    memmove(obj->arr+argc, obj->arr, (curlen)*sizeof(Jsi_Value*));
    obj->arrCnt += argc;
    int i;
    for (i = 0; i < argc; ++i) {
        Jsi_Value *ov = Jsi_ValueArrayIndex(interp, args, i);
        obj->arr[i] = NULL;
        if (!ov) { Jsi_LogBug("Arguments Error"); continue; }
        obj->arr[i] = ov;
        Jsi_IncrRefCount(interp, ov);
    }
    Jsi_ObjSetLength(interp, obj, curlen+argc);
    
    Jsi_ValueMakeNumber(interp, ret, Jsi_ObjGetLength(interp, obj));
    return JSI_OK;
}

static Jsi_RC ArrayFillCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
   if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) 
        return Jsi_LogError("expected array");
    Jsi_Value *value = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, 1);
    Jsi_Value *end = Jsi_ValueArrayIndex(interp, args, 2);
    int istart = 0, iend, n, nsiz;
    Jsi_RC rc = JSI_OK;
    Jsi_Obj *obj = _this->d.obj;
    n = Jsi_ObjGetLength(interp, obj);
    Jsi_Number nstart = 0;  // TODO: merge with code in ArraySliceCmd.
    Jsi_Number nend = 0;
    if (start && Jsi_GetNumberFromValue(interp, start, &nstart) == JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            goto bail;
    }
      
    if (n == 0) {
        goto bail;
    }
    iend = n-1;
    if (end && Jsi_GetNumberFromValue(interp,end, &nend) == JSI_OK) {
        iend = (int) nend;
        if (iend >= n)
            iend = n;
        if (iend < 0)
            iend = (n+iend);
        if (iend<0)
            goto bail;
    }
    nsiz = iend-istart+1;
    if (nsiz<=0)
        goto bail;

    int i, m;
    for (m = 0, i = istart; i <= iend; i++, m++)
    {
        if (obj->arr[i] == value) continue;
        if (obj->arr[i]) Jsi_DecrRefCount(interp, obj->arr[i]);
        obj->arr[m] = value;
        Jsi_IncrRefCount(interp, value);
    }
bail:
    if (_this != *ret) {
        if (*ret)
            Jsi_DecrRefCount(interp, *ret);
        *ret = _this;
        Jsi_IncrRefCount(interp, *ret);
    }
    return rc;
}


static Jsi_RC ArraySliceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr) {
   if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_Value *start = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *end = Jsi_ValueArrayIndex(interp, args, 1);
    int istart = 0, iend, n, nsiz;
    Jsi_RC rc = JSI_OK;
    Jsi_Obj *nobj, *obj;
    if (!start) {
        goto bail;
    }
    obj = _this->d.obj;
    n = Jsi_ObjGetLength(interp, obj);
    Jsi_Number nstart;
    if (Jsi_GetNumberFromValue(interp,start, &nstart) == JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            goto bail;
    }
      
    if (n == 0) {
        Jsi_ValueMakeArrayObject(interp, ret, Jsi_ObjNewType(interp, JSI_OT_ARRAY));
        return JSI_OK;
    }
    Jsi_Number nend;
    iend = n-1;
    if (end && Jsi_GetNumberFromValue(interp,end, &nend) == JSI_OK) {
        iend = (int) nend;
        if (iend >= n)
            iend = n;
        if (iend < 0)
            iend = (n+iend);
        if (iend<0)
            goto bail;
    }
    nsiz = iend-istart+1;
    if (nsiz<=0)
        goto bail;
    Jsi_ObjListifyArray(interp, obj);
    
    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);

    if (Jsi_ObjArraySizer(interp, nobj, nsiz) <= 0) {
        Jsi_LogError("index too large: %d", nsiz);
        rc = JSI_ERROR;
        goto bail;
    }

    int i, m;
    for (m = 0, i = istart; i <= iend; i++, m++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[m] = NULL;
        Jsi_ValueDup2(interp, nobj->arr+m, obj->arr[i]);
    }
    Jsi_ObjSetLength(interp, nobj, nsiz);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    return JSI_OK;
    
bail:
    Jsi_ValueMakeNull(interp, ret);
    return rc;
}

typedef struct {
    Jsi_Interp *interp;
    int flags;
    int mode;
    bool unique;
    Jsi_Value *compare;
    int errCnt;
} SortInfo;

static const char *sortArrayStrs[] = {"default", "desc", "dict", "nocase", 0};

static Jsi_OptionSpec ArraySortOptions[] = {
    JSI_OPT(CUSTOM, SortInfo, mode,     .help="Mode to sort by", .flags=0, .custom=Jsi_Opt_SwitchEnum,  .data=sortArrayStrs),
    JSI_OPT(FUNC,   SortInfo, compare,  .help="Function to do comparison", .flags=0, .custom=0, .data=(void*)"val1,val2"),
    JSI_OPT(BOOL,   SortInfo, unique,   .help="Eliminate duplicate items"),
    JSI_OPT_END(SortInfo)
};

#ifdef NO_QSORT_R

SortInfo *curSortInfo = NULL;

static int SortSubCmd(const void *p1, const void *p2) {
    SortInfo *si = curSortInfo;
#else
#ifdef __WIN32
static int SortSubCmd(void *thunk, const void *p1, const void *p2)
#else
static int SortSubCmd(const void *p1, const void *p2, void *thunk)
#endif
{
    SortInfo *si = (SortInfo *)thunk;
#endif
    Jsi_Interp *interp = si->interp;
    int sortFlags = si->flags;

    if (interp == NULL || interp->deleting)
        return 0;
    Jsi_Value *v1 = *(Jsi_Value**)p1, *v2 = *(Jsi_Value**)p2;
    VALCHK(v1);
    VALCHK(v2);
    int rc = 0;
    if (v1 != NULL && v2 != NULL) {
        if (!si->compare)
            rc = Jsi_ValueCmp(interp, v1, v2, sortFlags);
        else {
            Jsi_Value *vv[2] = {v1, v2};
            Jsi_Value *retP = Jsi_ValueNew1(interp);
            Jsi_Value *vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vv, 2, 0));
            Jsi_IncrRefCount(interp, vpargs);
            rc = Jsi_FunctionInvoke(interp, si->compare, vpargs, &retP, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_Number d = 0;
            if (Jsi_ValueGetNumber(interp, retP, &d) == JSI_OK)
                rc = -(int)d;
            else {
                if (!si->errCnt)
                    Jsi_LogWarn("invalid function return");
                si->errCnt++;
            }
            Jsi_DecrRefCount(interp, retP);
        }
    } else {
        if (v1 == v2) 
            rc = 0;
        else if (v1 == NULL)
            rc = 1;
        else
            rc = -1;
    }
    if ((sortFlags&JSI_SORT_DESCEND))
        return rc;
    return -rc;
}

Jsi_RC Jsi_ValueArraySort(Jsi_Interp *interp, Jsi_Value *val, int flags)
{
    if (val->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, val->d.obj)) {
        return JSI_ERROR;
    }
    Jsi_Obj *obj = val->d.obj;
    Jsi_ObjListifyArray(interp, obj);
    if (obj->arrCnt <= 0) {
        return JSI_OK;
    }
#ifdef __WIN32
#define qsort_r qsort_s
#endif

    SortInfo si = {};
    si.interp = interp;
    si.flags = flags;
#ifdef NO_QSORT_R
    curSortInfo = &si;
    qsort(obj->arr, obj->arrCnt, sizeof(Jsi_Value*), SortSubCmd);
    curSortInfo = NULL;
#else
    qsort_r(obj->arr, obj->arrCnt, sizeof(Jsi_Value*), SortSubCmd, &si);
#endif
    return JSI_OK;
}

static Jsi_RC ArraySortCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int flags = 0, i, curlen;
    Jsi_Obj *obj;
    Jsi_Value *v;
    SortInfo si = {};
    Jsi_Value *arg = NULL;
    int hasopt = 0;
    si.interp = interp;

    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        goto bail;
    }
    obj = _this->d.obj;
    curlen = obj->arrCnt;

    if (curlen <= 1) {
        goto done;
    }
    
    arg = Jsi_ValueArrayIndex(interp, args, 0);
    if (arg) {
        if (Jsi_ValueIsObjType(interp, arg, JSI_OT_OBJECT)) {
            if (Jsi_OptionsProcess(interp, ArraySortOptions, &si, arg, 0) < 0)
                return JSI_ERROR;
            hasopt = 1;
            switch (si.mode) {
                case 1: flags |= JSI_SORT_DESCEND; break;
                case 2: flags |= JSI_SORT_DICT; break;
                case 3: flags |= JSI_SORT_NOCASE; break;
            }
        } else if (Jsi_ValueIsObjType(interp, arg, JSI_OT_FUNCTION))
            si.compare = arg;
        else 
            return Jsi_LogError("expected object or function");
    }
    si.flags = flags;
    Jsi_ObjListifyArray(interp, obj);
#ifdef NO_QSORT_R
    /* TODO: mutex. */
    curSortInfo = &si;
    qsort(obj->arr, curlen, sizeof(Jsi_Value*), SortSubCmd);
#else
    qsort_r(obj->arr, curlen, sizeof(Jsi_Value*), SortSubCmd, &si);
#endif

    if (interp->deleting) {
#ifdef NO_QSORT_R
        curSortInfo = NULL;
#endif
        return JSI_ERROR;
    }
    if (si.unique) {
        int n, diff = 1, dupCnt=0;
        for (n=0, i=1; i<(int)obj->arrCnt; i++) {
            if (obj->arr[n] == obj->arr[i])
                diff = 1;
            else
#ifdef NO_QSORT_R
                diff = SortSubCmd(&obj->arr[n], &obj->arr[i]);
#else
#ifdef __WIN32
                diff = SortSubCmd(&si, &obj->arr[n], &obj->arr[i]);
#else
                diff = SortSubCmd(&obj->arr[n], &obj->arr[i], &si);
#endif
#endif
            if (diff) {
                n++;
                if (n!=i)
                    obj->arr[n] = obj->arr[i];
            } else {
                dupCnt++;
                if (obj->arr[i])
                    Jsi_DecrRefCount(interp, obj->arr[i]);
                obj->arr[i] = 0;
            }
        }
        obj->arrCnt -= dupCnt;
    }
#ifdef NO_QSORT_R
    curSortInfo = NULL;
#endif
    if (hasopt)
        Jsi_OptionsFree(interp, ArraySortOptions, &si, 0);
done:
    v = Jsi_ValueMakeObject(interp, NULL, obj);
    Jsi_ValueReplace(interp, ret, v);
    return JSI_OK;
bail:
    Jsi_ValueMakeNull(interp, ret);
    return JSI_OK;
}

static Jsi_RC ArraySpliceCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *va;
    Jsi_Obj *obj, *nobj;
    int newlen, argc, istart, n, rhowmany, ilen, curlen;
    Jsi_Value *start, *howmany;
    if (_this->vt != JSI_VT_OBJECT || !Jsi_ObjIsArray(interp, _this->d.obj)) {
        goto bail2;
    }
    obj = _this->d.obj;
    
    start = Jsi_ValueArrayIndex(interp, args, 0);
    howmany = Jsi_ValueArrayIndex(interp, args, 1);
    argc = Jsi_ValueGetLength(interp, args);
    istart = 0;
    ilen = (argc>=2 ? argc - 2 : 0);
    n = Jsi_ObjGetLength(interp, obj);
    curlen = n;
    
    if (!start) {
        goto bail2;
    }

    nobj = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
    Jsi_ValueMakeArrayObject(interp, ret, nobj);
    Jsi_ObjSetLength(interp, nobj, 0);
    
    /* Determine start index. */
    Jsi_Number nstart;
    if (Jsi_GetNumberFromValue(interp, start, &nstart) == JSI_OK) {
        istart = (int)nstart;
        if (istart > n)
            goto bail;
        if (istart < 0)
            istart = (n+istart);
        if (istart<0)
            istart=0;
    }
      
    Jsi_Number nhow;
    rhowmany = n-istart;
    if (howmany && Jsi_GetNumberFromValue(interp, howmany, &nhow) == JSI_OK) {
        rhowmany = (int)nhow;
        if (rhowmany >= (n-istart))
            rhowmany = n-istart;
        if (rhowmany < 0)
            rhowmany = (n-istart);
        if (rhowmany<0)
            goto bail;
    }
    
    if (curlen < 0) {
        Jsi_ObjSetLength(interp, obj, 0);
    }
    Jsi_ObjListifyArray(interp, obj);
   
    Jsi_ObjArraySizer(interp, nobj, rhowmany);

    /* Move elements to return object. */
    int i, j, m;
    for (m=0, j = 0, i = istart; m<rhowmany && m<curlen; m++,i++, j++)
    {
        if (!obj->arr[i]) continue;
        nobj->arr[m] = NULL;
        nobj->arr[m] = obj->arr[i];
    }
    Jsi_ObjSetLength(interp, nobj, m);
    
    /* Shift remaining down. */
    for (; rhowmany && i<curlen; i++)
    {
        obj->arr[i-rhowmany] = obj->arr[i];
        obj->arr[i] = NULL;
    }
    curlen -= j;
    /* Add elements. */
    newlen = curlen + argc - (argc>=2?2:1);
    if (Jsi_ObjArraySizer(interp, obj, newlen+3) <= 0) {
        Jsi_LogError("too long");
        Jsi_ValueMakeUndef(interp, ret);
        return JSI_ERROR;
    }
    if (ilen>0) {
        for (i = curlen-1; i>=istart; i--) {
            obj->arr[i+ilen] = obj->arr[i];
            obj->arr[i] = NULL;
        }
        for (m=istart, i = 2; i<argc; m++,i++) {
            va = Jsi_ValueArrayIndex(interp, args, i);
            if (!va) continue;
            obj->arr[m] = NULL;
            Jsi_ValueDup2(interp, obj->arr+m, va);
        }
    }
    Jsi_ObjSetLength(interp, obj, newlen);
bail:    
    return JSI_OK;
     
            
bail2:
    Jsi_ValueMakeNull(interp, ret);
    return JSI_OK;
}

static Jsi_RC ArrayConstructor(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    int argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *target;
    
    if (Jsi_FunctionIsConstructor(funcPtr)) target = _this;
    else {
        Jsi_Obj *o = Jsi_ObjNewType(interp, JSI_OT_ARRAY);
        o->__proto__ = interp->Array_prototype;
        Jsi_ValueMakeObject(interp, ret, o);
        target = *ret;
    }

    if (argc == 1) {
        Jsi_Number nv;
        Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
        if (v && Jsi_GetNumberFromValue(interp,v, &nv) == JSI_OK) {
            int len = (int)nv;
            if (!Jsi_NumberIsInteger(v->d.num) || len < 0) 
                return Jsi_LogError("Invalid array length");
            target->d.obj->isarrlist = 1;
            if (Jsi_ObjArraySizer(interp, target->d.obj, len) <= 0)
                return JSI_ERROR;
            return JSI_OK;
        }
    }

    int i;
    target->d.obj->isarrlist = 1;
    if (Jsi_ObjArraySizer(interp, target->d.obj, 0) <= 0)
            return JSI_ERROR;
    
    for (i = 0; i < argc; ++i) {
        Jsi_Value *argv = Jsi_ValueArrayIndex(interp, args, i);   ;
        jsi_ValueInsertArray(interp, _this, i, argv, 0);
    }
    return JSI_OK;
}
            
static Jsi_CmdSpec arrayCmds[] = {
    { "Array",      ArrayConstructor,   0,-1, "...", .help="Array constructor", .retType=(uint)JSI_TT_ARRAY, .flags=JSI_CMD_IS_CONSTRUCTOR },
    { "concat",     ArrayConcatCmd,     0,-1, "...", .help="Return array with args appended", .retType=(uint)JSI_TT_ARRAY },
    { "fill",       ArrayFillCmd,       1, 3, "value:any, start:number=0, end:number=-1", .help="Fill an array with values", .retType=(uint)JSI_TT_ARRAY },
    { "filter",     ArrayFilterCmd,     1, 2, "callback:function, this:object=void", .help="Return a filtered array", .retType=(uint)JSI_TT_ARRAY },
    { "forEach",    ArrayForeachCmd,    1, 2, "callback:function, this:object=void", .help="Invoke function with each item in object", .retType=(uint)JSI_TT_UNDEFINED },
    { "indexOf",    ArrayIndexOfCmd,    1, 2, "str:any, startIdx:number=0", .help="Return index of first occurrance in array", .retType=(uint)JSI_TT_NUMBER },
    { "join",       ArrayJoinCmd,       0, 1, "sep:string=''", .help="Return elements joined by char", .retType=(uint)JSI_TT_STRING },
    { "lastIndexOf",ArrayLastindexOfCmd,1, 2, "val:any, start:number=0", .help="Return index of last occurence in array", .retType=(uint)JSI_TT_NUMBER },
    { "map",        ArrayMapCmd,        1, 2, "callback:function, this:object=void", .help="Creates a new array with the results of calling a provided function on every element in this array", .retType=(uint)JSI_TT_ARRAY },
    { "pop",        ArrayPopCmd,        0, 0, "", .help="Remove and return last element of array", .retType=(uint)JSI_TT_ANY },
    { "push",       ArrayPushCmd,       1,-1, "val:any, ...", .help="Push one or more elements onto array and return size", .retType=(uint)JSI_TT_NUMBER },
    { "shift",      ArrayShiftCmd,      0, 0, "", .help="Remove first element and shift downwards", .retType=(uint)JSI_TT_ANY },
    { "sizeOf",     ArraySizeOfCmd,     0, 0, "", .help="Return size of array", .retType=(uint)JSI_TT_NUMBER },
    { "slice",      ArraySliceCmd,      1, 2, "start:number, end:number=void", .help="Return sub-array", .retType=(uint)JSI_TT_ARRAY },
    { "some",       ArraySomeCmd,       1, 2, "callback:function, this:object=void", .help="Return true if function returns true some element", .retType=(uint)JSI_TT_BOOLEAN },
    { "sort",       ArraySortCmd,       0, 1, "options:function|object=void", .help="Sort an array", .retType=(uint)JSI_TT_ARRAY, .flags=0, .info=0, .opts=ArraySortOptions },
    { "splice",     ArraySpliceCmd,     1,-1, "start:number, howmany:number=void, ...", .help="Change the content of an array, adding new elements while removing old elements", .retType=(uint)JSI_TT_ARRAY },
    { "reverse",    ArrayReverseCmd,    0, 0, "", .help="Reverse order of all elements in an array", .retType=(uint)JSI_TT_ARRAY },
    { "unshift",    ArrayUnshiftCmd,    0,-1, "...", .help="Add new elements to start of array and return size", .retType=(uint)JSI_TT_NUMBER },
    { NULL, 0,0,0,0, .help="Provide access to array objects" }
};

Jsi_RC jsi_InitArray(Jsi_Interp *interp, int release)
{
    if (release) return JSI_OK;
    interp->Array_prototype = Jsi_CommandCreateSpecs(interp, "Array", arrayCmds, NULL, JSI_CMDSPEC_ISOBJ);
    return JSI_OK;
}

#endif