#include "../os.h"
#include "../kernobjc/types.h"
#include "../types.h"
#import "../kernobjc/KKObjects.h"


typedef struct {
	long x, y, width, height;
} KKRect;

static inline KKRect KKRectMake(long x, long y, long width, long height) {
	KKRect rect;
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;
	return rect;
}

static inline BOOL KKRectsEqual(KKRect rect1, KKRect rect2){
	return (rect1.x == rect2.x) &&
	(rect1.y == rect2.y) &&
	(rect1.width == rect2.width) &&
	(rect1.height == rect2.height);
}


@interface KKTest : KKObject



@property (readwrite, assign) int integer;
// @property (readwrite, assign) double noninteger;
@property (readwrite, assign) KKRect structure;

@end


@implementation KKTest

@end


@interface KKTest2 : KKObject

@end
@implementation KKTest2

@end


void compiler_test(void);
void compiler_test(void) {
	KKTest *test = [[[KKTest alloc] init] autorelease];
	
	[test setInteger:5];
	objc_assert([test integer] == 5, "Failed integer assignment\n");
	
	//  [test setNoninteger:4.32];
	//  objc_assert([test noninteger] == 4.32, "Failed non-integer assignment\n");
	
	KKRect rect = KKRectMake(1, 2, 3, 4);
	[test setStructure:rect];
	objc_assert(KKRectsEqual(rect, [test structure]), "Failed structure assignment\n");
	
	
	objc_log("===================\n");
	objc_log("Passed compiler tests.\n\n");
}

