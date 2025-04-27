#if !defined(LC_TEMPLATING_H)
#define LC_TEMPLATING_H

#define _lcore_cat(a, b) a##b
#define _lcore_concat(a, b) _lcore_cat(a, b)
#define _lcore_join(a, b) _lcore_concat(a, _lcore_concat(_, b))
#define _lcore_mfunc(fname) _lcore_join(Self, fname)

#endif // LC_TEMPLATING_H
