
#ifndef OBJC_FOUNDATION_H
#define OBJC_FOUNDATION_H

#import "NSArray.h"
#import "NSBundle.h"
#import "NSDictionary.h"
#import "NSEnumerator.h"
#import "NSException.h"
#import "NSIndexSet.h"
#import "NSMethodSignature.h"
#import "NSNull.h"
#import "NSObject.h"
#import "NSSet.h"
#import "NSString.h"
#import "NSThread.h"
#import "NSTypes.h"
#import "NSValue.h"

extern Class NSClassFromString(NSString *class);

extern NSString *NSStringFromSelector(SEL select);
extern SEL NSSelectorFromString(NSString *selName);

extern const char *NSGetSizeAndAlignment(const char *typePtr, NSUInteger *sizep,
										 NSUInteger *alignp);


/* Macros from the GNUstep foundation. */

#ifndef SUPERINIT
#define SUPERINIT							\
	if ((self = [super init]) == nil){		\
		return nil;							\
	}
#endif

#ifndef SELFINIT
#define SELFINIT							\
	if ((self = [self init]) == nil){		\
		return nil;							\
	}
#endif

#ifndef	ASSIGN
#define	ASSIGN(object,value)	({\
  id __object = object; \
  object = [(value) retain]; \
  [__object release]; \
})
#endif

#ifndef DESTROY
#define	DESTROY(object) 	({ \
  id __o = object; \
  object = nil; \
  [__o release]; \
})
#endif

#ifndef	AUTORELEASE
#define	AUTORELEASE(object)	[(object) autorelease]
#endif

#ifndef RELEASE
#define RELEASE(obj) [obj release]
#endif

#ifndef FOREACH
	#define FOREACH(arr, var, type) for (type var in arr)
#endif

#ifndef D
	#define D(objs...) [NSDictionary dictionaryWithObjectsAndKeys:objs]
#endif

#ifndef A
	#define A(objs...) [NSArray arrayWithObjects:objs]
#endif

#ifndef NSAssert
#define NSAssert(condition, format...) objc_assert((condition), "%s",		\
													[[NSString stringWithFormat:format] UTF8String]);
#endif

#define NSCParameterAssert(condition)			\
	objc_assert((condition), "Invalid parameter not satisfying: %s\n", #condition)
#endif
