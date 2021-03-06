
#ifndef OBJC_HOOKS_H
#define OBJC_HOOKS_H

/*
 * The proxy lookup. When the method isn't cached the slow msg lookup goes on
 * to ask the proxy hook to supply a different receiver. If none is supplied,
 * the dispatch goes to the the __objc_msg_forward3 hook.
 */
extern id (*objc_proxy_lookup)(id receiver, SEL op);

/*
 * When slow method lookup fails and the proxy lookup hook returns no receiver,
 * the dispatch tries this hook for forwarding.
 */
extern struct objc_slot *(*__objc_msg_forward3)(id receiver, SEL op);

/*
 * An export of the other otherwise
 */
extern struct objc_slot *(*objc_plane_lookup)(id *receiver, SEL op, id sender);

/*
 * Allows supplying classes on the fly.
 */
extern Class(*objc_class_lookup_hook)(const char *name);

/* Compatibility reasons. */
#define _objc_lookup_class objc_class_lookup_hook

/*
 * Allows own handling of methods that are in modules that were unloaded.
 */
extern IMP objc_unloaded_module_method;

#endif /* !OBJC_HOOKS_H */
