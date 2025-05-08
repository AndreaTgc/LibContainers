#if !defined(LC_TEMPLATING_H)
#define LC_TEMPLATING_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define lc_malloc(T, Size) ((T *)malloc(Size))
#define lc_calloc(T, TSize, Count) ((T *)calloc(Count, TSize))

#define _lc_cat(a, b) a##b
#define _lc_concat(a, b) _lc_cat(a, b)
#define _lc_join(a, b) _lc_concat(a, _lc_concat(_, b))
#define _lc_mfunc(fname) _lc_join(Self, fname)
#define _lc_mfunc_priv(fname) _lc_join(__, _lc_mfunc(fname))

#endif // LC_TEMPLATING_H
