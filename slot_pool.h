#define POOL_NAME slot
#define POOL_TYPE struct objc_slot
#include "pool.h"
#include "utils.h"

/**
 * Allocates a new slot and initialises it for this method.
 */
static inline struct objc_slot *new_slot_for_method_in_class(Method method, 
                                                             Class class)
{
	
	struct objc_slot *slot = slot_pool_alloc();
	slot->owner = class;
	slot->types = method->selector_name + objc_strlen(method->selector_name) + 1;
	slot->selector = method->selector;
	slot->method = method->implementation;
	slot->version = 1;
	
	printf("Allocing slot for class %s, superclass ptr %p, selector %s \n", class->name, class->super_class, objc_selector_get_name(method->selector));
	
	return slot;
}
