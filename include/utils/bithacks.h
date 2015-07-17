/*
** Src: http://www.catonmat.net/blog/bit-hacks-header-file/
** bithacks.h - bit hacks macros. v1.0
**
** Released under the MIT license.
*/

#ifndef BITHACKS_H
#define BITHACKS_H


static inline int align(int x, int n) {
  return (((x >> n) + 1) << n );
}

#define FOUR_KB_ALIGN(x) align(x, 12)
#define MB_ALIGN(x) align(x, 20)

#define HEXIFY(X) 0x ## X ## LU

#define B_JOIN(a, b, shift) (a | (b << shift))

#define B_EXTRACT(x, max, min) (((x) >> (min)) & (((unsigned int)1 << ((max) - (min) + 1)) - 1))

/* test if x is even */
#define B_EVEN(x)        (((x)&1)==0)

/* test if x is odd */
#define B_ODD(x)         (!B_EVEN((x)))

/* test if n-th bit in x is set */
#define B_IS_SET(x, n)   (((x) & (1<<(n))) ? 1 : 0)

/* set n-th bit in x */
#define B_SET(x, n)      ((x) | (1<<(n)))

/* unset n-th bit in x */
#define B_UNSET(x, n)    ((x) & ~(1<<(n)))

/* toggle n-th bit in x */
#define B_TOGGLE(x, n)   ((x) ^ (1<<(n)))

/* turn off right-most 1-bit in x */
#define B_TURNOFF_1(x)   ((x) & ((x)-1))

/* isolate right-most 1-bit in x */
#define B_ISOLATE_1(x)   ((x) & (-(x)))

/* right-propagate right-most 1-bit in x */
#define B_PROPAGATE_1(x) ((x) | ((x)-1))

/* isolate right-most 0-bit in x */
#define B_ISOLATE_0(x)   (~(x) & ((x)+1))

/* turn on right-most 0-bit in x */
#define B_TURNON_0(x)    ((x) | ((x)+1))

#endif
