#ifndef BOOL_H
#define BOOL_H

#ifdef FALSE
# undef FALSE
#endif

#ifdef TRUE
# undef TRUE
#endif

#ifdef false
# undef false
#endif

#ifdef true
# undef true
#endif

#ifdef bool
# undef bool
#endif

#define FALSE false
#define TRUE  true
#define true  (!false)
#define false 0
#define bool  __bool__


typedef unsigned char __bool__;

#endif

