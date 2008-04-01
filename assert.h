
#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>




/** check that cond is true, str is displayed if it fails */
#define c_assert2(cond, str) __assert((cond), __LINE__, __FILE__, ___FUNCTION, (str), #cond)

/** check that cond is true, "assertion failed" is displayed if it failed */
#define c_assert(cond) c_assert2((cond), "assertion failed")


/** warm by showing str if cond is not true */
#define c_warning2(cond, str) __warning((cond), __LINE__, __FILE__, ___FUNCTION, (str), #cond)

/** warm by showing "warning" if cond is not true */
#define c_warning(cond) c_warning2((cond), "warning")



#ifndef DISABLE_DEBUG_PRINT

#define dbg_printf(args ...) \
          do {fprintf(stderr, "debug> line %d: in %s::%s(): ", __LINE__, __FILE__, ___FUNCTION); fprintf(stderr, args);} while(0)

#else

#define dbg_printf(format, args ...)

#endif


#ifndef DISABLE_ASSERT


#define __assert(cond, line, file, fn, str, cond_str) \
          do { if(!cond) { fprintf(stderr, "assertion error on `" cond_str "' in "  file "::%s() at line %d: %s\n", fn, line, str); abort(); } } while(0)
#define __warning(cond, line, file, fn, str, cond_str) \
          do { if(!cond) { fprintf(stderr, "warning on `" cond_str "' in " file "::%s() at line %d: %s\n", fn, line, str) ; } } while(0)

#else

#define __assert(cond, line, file, fn, str, cond_str)
#define __warning(cond, line, file, fn, str, cond_str)

#endif

#define ___FUNCTION __FUNCTION__




#endif
