#import "../Foundation/Foundation.h"
#import "LKObject.h"

typedef unsigned long long mpz_t;

@interface BigInt : NSNumber {
@public
  /**
   * Value for this object.  Public so it can be accessed from others to
   * slightly lower the cost of operations on BigInts.
   */
}
+ (BigInt*) bigIntWithCString:(const char*) aString;
+ (BigInt*) bigIntWithLongLong:(long long)aVal;
+ (BigInt*) bigIntWithLong:(long)aVal;
+ (BigInt*) bigIntWithUnsignedLong:(unsigned long)aVal;
+ (BigInt*) bigIntWithMP:(mpz_t)aVal;
@end
#ifndef OBJC_SMALL_OBJECT_SHIFT
#define OBJC_SMALL_OBJECT_SHIFT ((sizeof(id) == 4) ? 1 : 3)
#endif

static inline LKObject LKObjectFromNSInteger(NSInteger integer)
{
	if((integer << OBJC_SMALL_OBJECT_SHIFT >> OBJC_SMALL_OBJECT_SHIFT) != integer)
	{
		return LKObjectFromObject([BigInt bigIntWithLongLong: (long long)integer]);
	}
	else
	{
		return LKObjectFromObject((id)(void*)((integer << OBJC_SMALL_OBJECT_SHIFT) | 1));
	}
}
