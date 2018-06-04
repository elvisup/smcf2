#ifndef __MODULE_MANAGER_H__
#define __MODULE_MANAGER_H__

#include <core.h>
#include <smcf.h>

module_t *search_module_from_mblock_list(int module_id);
int add_module2mblock_list(module_block_t *module);

#endif /*__MODULE_MANAGER_H__*/
