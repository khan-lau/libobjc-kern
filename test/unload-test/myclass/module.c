#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/module.h>
#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/linker.h>
#include <sys/limits.h>

#include "../../../os.h"
#include "../../../kernobjc/types.h"
#include "../../../loader.h"

static int event_handler(struct module *module, int event, void *arg) {
	int e = 0;
	switch (event) {
		case MOD_LOAD:
			_objc_load_kernel_module(module);
			break;
		case MOD_UNLOAD:
			if (!_objc_unload_kernel_module(module)){
				e = EOPNOTSUPP;
			}
			break;
		default:
			e = EOPNOTSUPP;
			break;
	}
	return (e);
}

static moduledata_t myclass_conf = {
	"MyClass", 	/* Module name. */
	event_handler,  /* Event handler. */
	NULL 		/* Extra data */
};

DECLARE_MODULE(my_class, myclass_conf, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
MODULE_VERSION(my_class, 0);

/* Depend on libobjc */
MODULE_DEPEND(my_class, libobjc, 0, 0, 999);
