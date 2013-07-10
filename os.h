/**
 * This header file contains various declarations of functions.
 */

#ifndef OBJC_OS_H_
#define OBJC_OS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define OBJC_INLINE static inline

/**
 * Some compilers might not support this attribute
 * that forces the compiler to always inline the functions.
 */
// Not to be used in this runtime, to be removed later
#define OBJC_ALWAYS_INLINE 

#define PAGE_SIZE 4096

// TODO
#define objc_lock void
#define objc_array void
typedef struct {
	// TODO
} objc_rw_lock;

// TODO
static inline void panic(const char *reason){
	printf("%s", reason);
	exit(1);
}

static inline void *objc_zero_alloc(size_t size){
	return calloc(1, size);
}

static inline void *objc_alloc(size_t size){
	return malloc(size);
}

static inline void *objc_alloc_page(void){
	// TODO use page allocator directly?
	return malloc(PAGE_SIZE);
}

static inline void objc_rw_lock_init(objc_rw_lock *lock){
	// TODO
}
static inline int objc_rw_lock_rlock(objc_rw_lock *lock){
	return 0;
}
static inline int objc_rw_lock_wlock(objc_rw_lock *lock){
	return 0;
}
static inline int objc_rw_lock_unlock(objc_rw_lock *lock){
	return 0;
}

#define objc_assert(condition, description) \
		if (!(condition)){\
			panic(description);\
		}



/**
 * Some small macros that are used in a few places,
 * mostly for stitching.
 */
#define REALLY_PREFIX_SUFFIX(x, y) x ## y
#define PREFIX_SUFFIX(x, y) REALLY_PREFIX_SUFFIX(x, y)

#endif /* OBJC_OS_H_ */
