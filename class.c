/**
 * Implementation of the class-related functions.
 */

#include "os.h"
#include "utils.h"
#include "selector.h"
#include "class_registry.h"
#include "sarray2.h"
#include "runtime.h"
#include "class.h"

#pragma mark -
#pragma mark Small Object Classes

Class objc_small_object_classes[OBJC_SMALL_OBJECT_CLASS_COUNT];

BOOL objc_register_small_object_class(Class cl, uintptr_t mask){
	if ((mask & OBJC_SMALL_OBJECT_MASK) != mask){
		// Wrong mask
		return NO;
	}
	
	if (sizeof(void*) ==  4){
		// 32-bit computer, only support 1 class
		if (objc_small_object_classes[0] == Nil) {
			objc_small_object_classes[0] = cl;
			return YES;
		}
		return NO;
	}
	
	if (objc_small_object_classes[mask] == Nil){
		objc_small_object_classes[mask] = cl;
	}
	return NO;
}


#pragma mark -
#pragma mark STATIC_VARIABLES_AND_MACROS

/**
 * A cached forwarding selectors.
 */
static SEL objc_forwarding_selector;
static SEL objc_drops_unrecognized_forwarding_selector;

/**
 * Class structures are versioned. If a class prototype of a different
 * version is encountered, it is either ignored, if the version is
 * 
 */
#define OBJC_MAX_CLASS_VERSION_SUPPORTED ((unsigned int)0)


#pragma mark -
#pragma mark Private Functions


/**
 * Returns the size required for instances of class 'cl'. This
 * includes the extra space required by extensions.
 */
OBJC_INLINE unsigned int _instance_size(Class cl){
	if (cl == Nil){
		return 0;
	}
	
	objc_assert(cl->flags.resolved, "Getting instance size of unresolved class (%s)!\n", class_getName(cl));
	
	if (cl->flags.meta){
		return sizeof(struct objc_class);
	}
	
	cl = objc_class_get_nonfake_inline(cl);
	
	return cl->instance_size;
}

/**
 * Searches for a method in a method list and returns it, or NULL 
 * if it hasn't been found.
 */
OBJC_INLINE Method _lookup_method_in_method_list(objc_method_list *method_list, SEL selector){
	if (method_list == NULL){
		return NULL;
	}
	
	while (method_list != NULL) {
		int i;
		for (i = 0; i < method_list->size; ++i){
			Method m = &method_list->list[i];
			if (m->selector == selector){
				return m;
			}
		}
		method_list = method_list->next;
	}
	
	// Nothing found.
	return NULL;
}

/**
 * Searches for an instance method for a selector.
 */
OBJC_INLINE Method _lookup_method(Class class, SEL selector){
	Method m;
	
	if (class == Nil || selector == null_selector){
		return NULL;
	}
	
	while (class != NULL){
		m = _lookup_method_in_method_list(class->methods, selector);
		if (m != NULL){
			return m;
		}
		class = class->super_class;
	}
	return NULL;
}

OBJC_INLINE Method _lookup_cached_method(Class cl, SEL selector){
	if (cl != Nil && selector != null_selector && cl->dtable != NULL){
		return (Method)SparseArrayLookup((SparseArray*)cl->dtable, selector);
	}
	return NULL;
}

OBJC_INLINE void _cache_method(Class cl, Method m){
	// TODO locking on creation
	// TODO not method, use a slot
	if (cl != Nil && m != NULL && cl->dtable != NULL){
		SparseArrayInsert((SparseArray*)cl->dtable, m->selector, m);
	}
}

/**
 * Looks up an instance method implementation within a class.
 * NULL if it hasn't been found, yet no-op function in case
 * the receiver is nil.
 */
OBJC_INLINE IMP _lookup_method_impl(Class cl, SEL selector){
	Method m = _lookup_cached_method(cl, selector);
	if (m != NULL){
		return m->implementation;
	}
	
	while (cl != Nil && cl->flags.fake) {
		cl = cl->super_class;
	}
	
	m = _lookup_method(cl, selector);
	if (m == NULL){
		return NULL;
	}
	
	_cache_method(cl, m);
	return m->implementation;
}

/**
 * Returns whether cl is a subclass of superclass_candidate.
 */
OBJC_INLINE BOOL _class_is_subclass_of_class(Class cl, Class superclass_candidate){
	while (cl != Nil) {
		if (cl->super_class == superclass_candidate){
			return YES;
		}
		cl = cl->super_class;
	}
	return NO;
}


/**
 * Crashes the program because forwarding isn't supported by the class of the object.
 */
OBJC_INLINE void _forwarding_not_supported_abort(id obj, SEL selector){
	/* i.e. the object doesn't respond to the
	 forwarding selector either. */
	objc_log("%s doesn't support forwarding and doesn't respond to selector %s!\n", objc_object_get_nonfake_class_inline(obj)->name, sel_getName(selector));
	objc_abort("Class doesn't support forwarding.");
}

/**
 * The first part of the forwarding mechanism. obj is called a method
 * forwardedMethodForSelector: which should return a Method pointer
 * to the actual method to be called. If it doesn't wish to forward this 
 * particular selector, return NULL. Note that the Method doesn't need
 * to be registered anywhere, it may be a faked pointer.
 *
 * If NULL is returned, the run-time moves on to the second step:
 * asking the object whether to drop the message (a nil-receiver
 * method is returned), or whether to raise an exception.
 */
OBJC_INLINE Method _forward_method_invocation(id obj, SEL selector){
	if (objc_selectors_equal(selector, objc_forwarding_selector) || objc_selectors_equal(selector, objc_drops_unrecognized_forwarding_selector)){
		/* Make sure the app really crashes and doesn't create an infinite cycle. */
		return NULL;
	}else{
		/* Forwarding. */
		IMP forwarding_imp;
		
		if (objc_forwarding_selector == null_selector){
			// TODO
		}
		
		forwarding_imp = _lookup_method_impl(obj->isa, objc_forwarding_selector);
		if (forwarding_imp == NULL){
			objc_log("Class %s doesn't respond to selector %s.\n", objc_object_get_nonfake_class_inline(obj)->name, sel_getName(selector));
			return NULL;
		}
		
		return ((Method(*)(id,SEL,SEL))forwarding_imp)(obj, objc_forwarding_selector, selector);
	}
}

/**
 * Looks for an ivar named name in ivar_list.
 */
OBJC_INLINE Ivar _ivar_named_in_ivar_list(objc_ivar_list *ivar_list, const char *name){
	if (ivar_list == NULL){
		return NULL;
	}
	
	for (int i = 0; i < ivar_list->size; ++i){
		Ivar var = &ivar_list->list[i];
		if (objc_strings_equal(var->name, name)){
			return var;
		}
	}
	
	return NULL;
}

/**
 * Finds an Ivar in class with name.
 */
OBJC_INLINE Ivar _ivar_named(Class cl, const char *name){
	if (name == NULL){
		return NULL;
	}
	
	while (cl != Nil){
		Ivar var = _ivar_named_in_ivar_list(cl->ivars, name);
		if (var != NULL){
			return var;
		}
		cl = cl->super_class;
	}
	return NULL;
}

/**
 * Returns number of ivars on class cl. Used only
 * by the functions copying ivar lists.
 */
OBJC_INLINE unsigned int _ivar_count(Class cl){
	objc_ivar_list *ivar_list = cl->ivars;
	if (ivar_list == NULL){
		return 0;
	}else{
		return ivar_list->size;
	}
}

/**
 * Copies over ivars declared on cl into list.
 */
OBJC_INLINE void _ivars_copy_to_list(Class cl, Ivar *list, unsigned int max_count){
	unsigned int counter = 0;
	objc_ivar_list *ivar_list = cl->ivars;
	
	if (ivar_list == NULL){
		/* NULL-termination */
		list[0] = NULL;
		return;
	}
	
	for (int i = 0; i < ivar_list->size; ++i){
		if (counter >= max_count) {
			break;
		}
		
		list[counter] = &ivar_list->list[i];
		++counter;
	}
	
	/* NULL termination */
	list[max_count] = NULL;
}

/**
 * Looks up method. If obj is nil, returns the nil receiver method.
 *
 * If the method is not found, forwarding takes place.
 */
OBJC_INLINE Method _lookup_object_method(id obj, SEL selector){
	Method method = NULL;
	
	if (obj == nil){
		// TODO
		return NULL;
	}
	
	method = _lookup_cached_method(obj->isa, selector);
	if (method != NULL){
		return method;
	}
	
	method = _lookup_method(objc_object_get_class_inline(obj), selector);
	if (method != NULL){
		_cache_method(obj->isa, method);
	}
	
	if (method == NULL){
		/* Not found! Prepare for forwarding. */
		Method forwarded_method = _forward_method_invocation(obj, selector);
		if (forwarded_method != NULL){
			/** The object returned a method 
			 * that should be called instead.
			 */
			
			/** ++ to invalidate inline cache. */
			++forwarded_method->version;
			return forwarded_method;
		}
		
		if (forwarded_method == NULL){
			// TODO
			return NULL;
		}
		
		_forwarding_not_supported_abort(obj, selector);
		return NULL;
	}
	
	return method;
}

/**
 * The same scenario as above, but in this case a call to the superclass.
 */
OBJC_INLINE Method _lookup_method_super(struct objc_super *sup, SEL selector){
	Method method;
	
	if (sup == NULL){
		return NULL;
	}
	
	if (sup->receiver == nil){
		// TODO
		return NULL;
	}
	
	method = _lookup_method(sup->class, selector);
	
	if (method == NULL){
		/* Not found! Prepare for forwarding. */
		objc_log("Called super to class %s, which doesn't implement selector %s.\n", sup->class->name, sel_getName(selector));
		if (_forward_method_invocation(sup->receiver, selector)){
			// TODO
			return NULL;
		}else{
			_forwarding_not_supported_abort(sup->receiver, selector);
			return NULL;
		}
	}
	
	return method;
}





#pragma mark -
#pragma mark Regular lookup functions

Method class_getMethod(Class cls, SEL selector){
	cls = objc_class_get_nonfake_inline(cls);
	if (cls == Nil || selector == null_selector){
		return NULL;
	}
	return _lookup_method(cls, selector);
}

Method class_getInstanceMethod(Class cls, SEL selector){
	if (cls != Nil && cls->flags.meta){
		cls = (Class)objc_getClass(cls->name);
	}
	
	cls = objc_class_get_nonfake_inline(cls);
	if (cls == Nil || selector == null_selector){
		return NULL;
	}
	
	return _lookup_method(cls, selector);
}

Method class_getInstanceMethodNonRecursive(Class cls, SEL selector){
	if (cls != Nil && cls->flags.meta){
		cls = (Class)objc_getClass(cls->name);
	}
	
	cls = objc_class_get_nonfake_inline(cls);
	if (cls == Nil || selector == null_selector){
		return NULL;
	}
	
	return _lookup_method_in_method_list(cls->methods, selector);
}

Method class_getClassMethod(Class cls, SEL selector){
	if (cls == Nil || selector == null_selector){
		return NULL;
	}
	
	if (!cls->flags.meta){
		cls = cls->isa;
	}
	
	return _lookup_method(cls, selector);
}

#pragma mark -
#pragma mark Setting and getting class of an object

Class object_getClass(id obj){
	return obj == nil ? Nil : objc_object_get_nonfake_class_inline(obj);
}

const char *object_getClassName(id obj){
	if (obj == nil){
		return NULL;
	}
	return class_getName(object_getClass(obj));
}

Class object_setClass(id obj, Class new_class){
	Class old_class;
	if (obj == nil){
		return Nil;
	}
	
	if ((uintptr_t)obj & OBJC_SMALL_OBJECT_MASK){
		return objc_object_get_class_inline(obj);
	}
	
	old_class = obj->isa;
	obj->isa = new_class;
	return old_class;
}

#pragma mark -
#pragma mark Object creation, copying and destruction

id class_createInstance(Class cl, size_t extraBytes){
	if (!cl->flags.resolved){
		objc_log("Trying to create an instance of unfinished class (%s).", cl->name);
		return nil;
	}
	
	if (cl->flags.meta){
		cl = (Class)objc_getClass(cl->name);
	}
	
	// TODO add a flag on the class?
	// Check if cl is a small object class
	if (sizeof(void *) == 4 && objc_small_object_classes[0] == cl){
		return (id)1;
	}else{
		for (uintptr_t i = 0; i < OBJC_SMALL_OBJECT_MASK; ++i){
			if (objc_small_object_classes[i] == cl){
				return (id)((i << 1) + 1);
			}
		}
	}
	
	size_t size = _instance_size(cl);
	
	id obj = (id)objc_zero_alloc(size);
	obj->isa = cl;
	
	objc_debug_log("Created instance %p of class %s\n", obj, class_getName(cl));
	
	// TODO - cxx_construct?
	
	return obj;
}
void object_dispose(id obj){
	objc_debug_log("Deallocating instance %p\n", obj);
	
	if (obj == nil){
		return;
	}
	
	objc_dealloc(obj);
}

id object_copy(id obj, size_t size){
	if (obj == nil){
		return nil;
	}
	
	id copy = objc_zero_alloc(size);
	objc_copy_memory(copy, obj, size);
	
	return copy;
}

#pragma mark -
#pragma mark Object lookup

Method objc_object_lookup_method(id obj, SEL selector){
	return _lookup_object_method(obj, selector);
}
Method objc_object_lookup_method_super(struct objc_super *sup, SEL selector){
	return _lookup_method_super(sup, selector);
}
IMP objc_object_lookup_impl(id obj, SEL selector){
	return _lookup_object_method(obj, selector)->implementation;
}
IMP objc_object_lookup_impl_super(struct objc_super *sup, SEL selector){
	return _lookup_method_super(sup, selector)->implementation;
}

/***** INFORMATION GETTERS *****/
#pragma mark -
#pragma mark Information getters

BOOL objc_class_is_resolved(Class cl){
	if (cl == Nil){
		return NO;
	}
	return cl->flags.resolved;
}
const char *class_getName(Class cl){
	cl = objc_class_get_nonfake_inline(cl);
	if (cl == Nil){
		return "nil";
	}
	
	return cl->name;
}

Class class_getSuperclass(Class cls){
	if (cls == Nil){
		return Nil;
	}
	
	if (!cls->flags.resolved){
		objc_class_resolve(cls);
	}
	return cls->super_class;
}

BOOL class_isMetaClass(Class cls){
	return cls == Nil ? NO : cls->flags.meta;
}



id objc_getMetaClass(const char *name){
	Class cl = (Class)objc_getClass(name);
	return cl == Nil ? nil : (id)cl->isa;
}

id objc_getRequiredClass(const char *name){
	id cl = objc_getClass(name);
	if (cl == nil){
		objc_abort("Couldn't find required class %s\n", name = NULL ? "[NULL]" : name);
	}
	return cl;
}

size_t class_getInstanceSize(Class cls){
	return _instance_size(cls);
}
Method *class_copyMethodList(Class cls, unsigned int *outCount){
	cls = objc_class_get_nonfake_inline(cls);
	if (cls == Nil){
		return NULL;
	}
	return objc_method_list_copy_list(cls->methods, outCount);
}


/***** IVAR-RELATED *****/
#pragma mark -
#pragma mark Ivar-related

BOOL class_addIvar(Class cls, const char *name, size_t size, uint8_t alignment, const char *types){
	Ivar variable;
	
	if (cls == Nil || name == NULL || size == 0 || types == NULL){
		return NO;
	}
	
	if (cls->flags.initialized){
		objc_log("Class %s is already initialized!\n", cls->name);
		objc_abort("Trying to add ivar to a class that is already initialized.");
	}
	
	if (_ivar_named(cls, name) != NULL){
		objc_log("Class %s, or one of its superclasses already have an ivar named %s!\n", cls->name, name);
		return NO;
	}
	
	objc_rw_lock_wlock(&objc_runtime_lock);
	if (cls->ivars == NULL){
		cls->ivars = objc_ivar_list_create(1);
	}else{
		cls->ivars = objc_ivar_list_expand_by(cls->ivars, 1);
	}
	
	variable = &cls->ivars->list[cls->ivars->size - 1];
	variable->name = objc_strcpy(name);
	variable->type = objc_strcpy(types);
	variable->size = size;
	variable->align = alignment;
	
	/**
	 * Offsets and stuff gets computer on class resolving.
	 */
		
	objc_rw_lock_unlock(&objc_runtime_lock);
	
	return YES;
}
Ivar class_getInstanceVariable(Class cls, const char *name){
	cls = objc_class_get_nonfake_inline(cls);
	return _ivar_named(cls, name);
}
Ivar *class_copyIvarList(Class cl, unsigned int *outCount){
	cl = objc_class_get_nonfake_inline(cl);
	unsigned int number_of_ivars = _ivar_count(cl);
	if (number_of_ivars == 0){
		if (outCount != NULL){
			*outCount = 0;
		}
		return NULL;
	}
	
	Ivar *ivars = objc_alloc(sizeof(Ivar) * number_of_ivars);
	_ivars_copy_to_list(cl, ivars, number_of_ivars);
	if (outCount != NULL){
		*outCount = number_of_ivars;
	}
	
	return ivars;
}
void *object_getIndexedIvars(id obj){
	if (obj == nil){
		return NULL;
	}
	
	Class cl = object_getClass(obj);
	if (cl->instance_size == 0 || cl->flags.meta){
		return (char*)obj + sizeof(struct objc_class);
	}
	
	return (char*)obj + cl->instance_size;
}


Ivar objc_object_get_variable_named(id obj, const char *name, void **out_value){
	Ivar ivar;
	
	if (obj == nil || name == NULL || out_value == NULL){
		return NULL;
	}
	
	ivar = _ivar_named(objc_object_get_nonfake_class_inline(obj), name);
	if (ivar == NULL){
		return NULL;
	}
	
	objc_copy_memory(*out_value, (char*)obj + ivar->offset, ivar->size);
	
	return ivar;
}
Ivar objc_object_set_variable_named(id obj, const char *name, void *value){
	Ivar ivar;
	
	if (obj == nil || name == NULL){
		return NULL;
	}
	
	ivar = _ivar_named(objc_object_get_nonfake_class_inline(obj), name);
	if (ivar == NULL){
		return NULL;
	}
	
	objc_copy_memory((char*)obj + ivar->offset, value, ivar->size);
	
	return ivar;
}

#pragma mark -
#pragma mark Ivars

id object_getIvar(id obj, Ivar ivar){
	char *var_ptr;
	
	if (obj == nil || ivar == NULL){
		return NULL;
	}
	
	// TODO check for weak refs and use loadWeak
	
	var_ptr = (char*)obj;
	var_ptr += ivar->offset;
	return (id)var_ptr;
}

void object_setIvar(id obj, Ivar ivar, id value){
	if (obj == nil || ivar == NULL){
		return;
	}
	
	// TODO - check for weak refs and use storeWeak
	objc_copy_memory((char*)obj + ivar->offset, value, ivar->size);
}

