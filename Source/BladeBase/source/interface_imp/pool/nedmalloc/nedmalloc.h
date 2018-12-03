/* nedalloc, an alternative malloc implementation for multiple threads without
lock contention based on dlmalloc v2.8.3. (C) 2005 Niall Douglas

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef NEDMALLOC_H
#define NEDMALLOC_H


/* See malloc.c.h for what each function does.

REPLACE_SYSTEM_ALLOCATOR causes nedalloc's functions to be called malloc,
free etc. instead of nedmalloc, nedfree etc. You may or may not want this.

NO_NED_NAMESPACE prevents the functions from being defined in the nedalloc
namespace when in C++ (uses the global namespace instead).

EXTSPEC can be defined to be __declspec(dllexport) or
__attribute__ ((visibility("default"))) or whatever you like. It defaults
to extern.

USE_LOCKS can be 2 if you want to define your own MLOCK_T, INITIAL_LOCK,
ACQUIRE_LOCK, RELEASE_LOCK, TRY_LOCK, IS_LOCKED and NULL_LOCK_INITIALIZER.

*/

#include <stddef.h>   /* for size_t */

#ifndef EXTSPEC
 #define EXTSPEC extern
#endif

#if defined(_MSC_VER) && _MSC_VER>=1400
 #define MALLOCATTR __declspec(restrict)
#endif
#ifdef __GNUC__
 #define MALLOCATTR __attribute__ ((malloc))
#endif
#ifndef MALLOCATTR
 #define MALLOCATTR
#endif

#ifdef REPLACE_SYSTEM_ALLOCATOR
 #define nedmalloc               malloc
 #define nedcalloc               calloc
 #define nedrealloc              realloc
 #define nedfree                 free
 #define nedmemalign             memalign
 #define nedmallinfo             mallinfo
 #define nedmallopt              mallopt
 #define nedmalloc_trim          malloc_trim
 #define nedmalloc_stats         malloc_stats
 #define nedmalloc_footprint     malloc_footprint
 #define nedindependent_calloc   independent_calloc
 #define nedindependent_comalloc independent_comalloc
 #ifdef _MSC_VER
  #define nedblksize              _msize
 #endif
#endif

#define NO_MALLINFO (1)

#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif

#if !NO_MALLINFO
struct mallinfo;
#endif

#if defined(__cplusplus)
 #if !defined(NO_NED_NAMESPACE)
namespace nedalloc {
 #else
extern "C" {
 #endif
 #define THROWSPEC throw()
#else
 #define THROWSPEC
#endif

/* These are the global functions */

/* Gets the usable size of an allocated block. Note this will always be bigger than what was
asked for due to rounding etc.
*/
EXTSPEC size_t nedblksize(void *mem) THROWSPEC;

EXTSPEC void nedsetvalue(void *v) THROWSPEC;

EXTSPEC MALLOCATTR void * nedmalloc(size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedcalloc(size_t no, size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedrealloc(void *mem, size_t size) THROWSPEC;
EXTSPEC void   nedfree(void *mem) THROWSPEC;
EXTSPEC MALLOCATTR void * nedmemalign(size_t alignment, size_t bytes) THROWSPEC;
#if !NO_MALLINFO
EXTSPEC struct mallinfo nedmallinfo(void) THROWSPEC;
#endif
EXTSPEC int    nedmallopt(int parno, int value) THROWSPEC;
EXTSPEC int    nedmalloc_trim(size_t pad) THROWSPEC;
EXTSPEC void   nedmalloc_stats(void) THROWSPEC;
EXTSPEC size_t nedmalloc_footprint(void) THROWSPEC;
EXTSPEC MALLOCATTR void **nedindependent_calloc(size_t elemsno, size_t elemsize, void **chunks) THROWSPEC;
EXTSPEC MALLOCATTR void **nedindependent_comalloc(size_t elems, size_t *sizes, void **chunks) THROWSPEC;

/* These are the pool functions */
struct nedpool_t;
typedef struct nedpool_t nedpool;

/* Creates a memory pool for use with the nedp* functions below.
Capacity is how much to allocate immediately (if you know you'll be allocating a lot
of memory very soon) which you can leave at zero. Threads specifies how many threads
will *normally* be accessing the pool concurrently. Setting this to zero means it
extends on demand, but be careful of this as it can rapidly consume system resources
where bursts of concurrent threads use a pool at once.
*/
EXTSPEC MALLOCATTR nedpool *nedcreatepool(size_t capacity, int threads) THROWSPEC;

/* Destroys a memory pool previously created by nedcreatepool().
*/
EXTSPEC void neddestroypool(nedpool *p) THROWSPEC;

/* Sets a value to be associated with a pool. You can retrieve this value by passing
any memory block allocated from that pool.
*/
EXTSPEC void nedpsetvalue(nedpool *p, void *v) THROWSPEC;
/* Gets a previously set value using nedpsetvalue() or zero if memory is unknown.
Optionally can also retrieve pool.
*/
EXTSPEC void *nedgetvalue(nedpool **p, void *mem) THROWSPEC;

/* Disables the thread cache for the calling thread, returning any existing cache
data to the central pool.
*/
EXTSPEC void neddisablethreadcache(nedpool *p) THROWSPEC;

EXTSPEC MALLOCATTR void * nedpmalloc(nedpool *p, size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedpcalloc(nedpool *p, size_t no, size_t size) THROWSPEC;
EXTSPEC MALLOCATTR void * nedprealloc(nedpool *p, void *mem, size_t size) THROWSPEC;
EXTSPEC void   nedpfree(nedpool *p, void *mem) THROWSPEC;
EXTSPEC MALLOCATTR void * nedpmemalign(nedpool *p, size_t alignment, size_t bytes) THROWSPEC;
#if !NO_MALLINFO
EXTSPEC struct mallinfo nedpmallinfo(nedpool *p) THROWSPEC;
#endif
EXTSPEC int    nedpmallopt(nedpool *p, int parno, int value) THROWSPEC;
EXTSPEC int    nedpmalloc_trim(nedpool *p, size_t pad) THROWSPEC;
EXTSPEC void   nedpmalloc_stats(nedpool *p) THROWSPEC;
EXTSPEC size_t nedpmalloc_footprint(nedpool *p) THROWSPEC;
EXTSPEC MALLOCATTR void **nedpindependent_calloc(nedpool *p, size_t elemsno, size_t elemsize, void **chunks) THROWSPEC;
EXTSPEC MALLOCATTR void **nedpindependent_comalloc(nedpool *p, size_t elems, size_t *sizes, void **chunks) THROWSPEC;

#if defined(__cplusplus)
}
#endif

#undef MALLOCATTR
#undef EXTSPEC

#endif