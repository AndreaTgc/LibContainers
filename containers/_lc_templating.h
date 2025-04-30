#if !defined(LC_TEMPLATING_H)
#define LC_TEMPLATING_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define lc_malloc(T, Size) ((T *)malloc(Size))
#define lc_calloc(T, TSize, Count) ((T *)calloc(Count, TSize))

#define _lcore_cat(a, b) a##b
#define _lcore_concat(a, b) _lcore_cat(a, b)
#define _lcore_join(a, b) _lcore_concat(a, _lcore_concat(_, b))
#define _lcore_mfunc(fname) _lcore_join(Self, fname)
#define _lcore_mfunc_priv(fname) _lcore_join(__, _lcore_mfunc(fname))

#endif // LC_TEMPLATING_H
