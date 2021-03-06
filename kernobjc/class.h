
#ifndef LIBKERNOBJC_CLASS_H
#define LIBKERNOBJC_CLASS_H

#define OBJC_SMALL_OBJECT_SHIFT ((sizeof(void*) == 4) ? 1 : 3)
#define OBJC_SMALL_OBJECT_MASK ((sizeof(void*) == 4) ? 1 : 7)
#define OBJC_SMALL_OBJECT_CLASS_COUNT ((sizeof(void*) == 4) ? 1 : 4)

/*
 * Returns the name of the class.
 */
const char *class_getName(Class cls);

/*
 * Returns true if it is a meta class.
 */
BOOL class_isMetaClass(Class cls);

/*
 * Returns the classes superclass, 
 */
Class class_getSuperclass(Class cls);

/*
 * Returns size of an instance of a class.
 */
size_t class_getInstanceSize(Class cls);

/*
 * Returns an ivar for name.
 */
Ivar class_getInstanceVariable(Class cls, const char *name);

/*
 * Returns a list of ivars. The caller is responsible for
 * freeing it using the free function.
 *
 * Note that this function only returns ivars that are
 * declared directly on the class. Ivars declared
 * on the class' superclasses are not included.
 */
Ivar *class_copyIvarList(Class cls, unsigned int *outCount);

Method class_getMethod(Class cls, SEL name);
Method class_getInstanceMethod(Class cls, SEL name);
Method class_getInstanceMethodNonRecursive(Class cls, SEL name);
Method class_getClassMethod(Class cls, SEL name);
IMP class_getMethodImplementation(Class cls, SEL name);
IMP class_getMethodImplementation_stret(Class cls, SEL name);

/*
 * The following function returns a list of methods
 * implemented on a class. The caller is responsible 
 * for freeing it using the free function.
 *
 * Note that this function only returns methods that are
 * implemented directly on the class. Methods implemented
 * on the class' superclasses are not included.
 */
Method *class_copyMethodList(Class cls, unsigned int *outCount);

BOOL class_respondsToSelector(Class cls, SEL sel);

BOOL class_conformsToProtocol(Class cls, Protocol *protocol);
Protocol * __unsafe_unretained *class_copyProtocolList(Class cls,
						       unsigned int *outCount);

objc_property_t class_getProperty(Class cls, const char *name);
objc_property_t *class_copyPropertyList(Class cls, unsigned int *outCount);


/*
 * Sets or gets version of a class.
 */
int class_getVersion(Class theClass);
void class_setVersion(Class theClass, int version);


/*
 * The following functions add methods or a single method to the class.
 *
 * If a method with the same selector is already attached to the class,
 * this doesn't override it. This is due to the method lists being a linked
 * list and the new methods being attached to the end of the list
 * as well as maintaining behavior of other run-times.
 *
 * Note that the types are here merely for type checking. If they don't 
 * correspond to the types of the selector, the runtime aborts.
 */
BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);

/*
 * This function takes all methods from source and isntalls them on target.
 * This is used for making _KKConstString class act like NSString when Foundation
 * is loaded.
 */
void class_addMethodsFromClass(Class target, Class source);

/*
 * Replaces a method implementation for another one.
 *
 * Returns the old implementation.
 */
IMP class_replaceMethod(Class cls, SEL name, IMP imp);

/*
 * Adds an ivar to a class. If the class is resolved,
 * calling this function aborts the program.
 */
BOOL class_addIvar(Class cls, const char *name, size_t size,
		   uint8_t alignment, const char *types);


/*
 * Creates an instance of class. You shouldn't be using this directly, used
 * by +alloc.
 */
id class_createInstance(Class cl, size_t extraBytes);


BOOL class_addProtocol(Class cls, Protocol *protocol);


BOOL class_addProperty(Class cls, const char *name,
		       const objc_property_attribute_t *attributes,
		       unsigned int attributeCount);

void class_replaceProperty(Class cls, const char *name,
			   const objc_property_attribute_t *attributes,
			   unsigned int attributeCount);


#endif
