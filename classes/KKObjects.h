
#include "../types.h"

@interface KKObject {
  id isa;
  int retain_count;
}

+(id)alloc;
+(id)new;

+(Class)class;

-(id)init;

-(id)retain;
-(void)release;

@end


@interface _KKConstString : KKObject {
  const char *_cString;
  unsigned int _length;
}

-(const char*)cString;
-(unsigned int)length;

@end


