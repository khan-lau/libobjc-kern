
#ifndef OBJC_FOUNDATION_TYPES_H
#define OBJC_FOUNDATION_TYPES_H


/* Currently, we only support 64-bit computers. */
typedef unsigned long long	NSUInteger;
typedef long long			NSInteger;


typedef struct _NSRange {
	NSUInteger location;
	NSUInteger length;
} NSRange;

static inline NSRange NSMakeRange(NSUInteger location, NSUInteger length){
	NSRange r;
	r.location = location;
	r.length = length;
	return r;
}

#define NSNotFound ((NSUInteger)-1)

#endif