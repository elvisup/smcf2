#include <stdio.h>
#include <module_manager.h>

extern smcf_context_t *context;

static module_block_t *search_mblock_tail(module_block_t *mlist)
{
	if (!mlist) {
		printf("ERROR\n");
		return NULL;
	}

	if (!mlist->next)
		return mlist;

	module_block_t *search_module = NULL;
	module_block_t *next_module = NULL;

	next_module = mlist->next;

	while(next_module) {
		search_module = next_module;
		next_module = search_module->next;
	}

	return search_module;
}

module_t *search_module_from_mblock_list(int module_id)
{
	if (module_id < 0) {
		printf("ERROR\n");
		return NULL;
	}

	if (context->module_list_ctx->module_block_list == NULL) {
		printf("ERROR\n");
		return NULL;
	}

	module_t *search_module = NULL;
	module_block_t *next_module_block = context->module_list_ctx->module_block_list;

	do {
		search_module = next_module_block->module;
		if (search_module->id == module_id)
			break;
		search_module = NULL;
		next_module_block = next_module_block->next;
	}while(next_module_block);

	return search_module;
}

int add_module2mblock_list(module_block_t *module)
{
	if (!module) {
		printf("ERROR\n");
		return -1;
	}

	//TODO: ensure this module next is NULL
	module->next = NULL;

	module_block_t *tmp_module = NULL;
	if (context->module_list_ctx->module_block_list == NULL) {
		context->module_list_ctx->module_block_list = module;
	} else {
		tmp_module = search_mblock_tail(context->module_list_ctx->module_block_list);
		tmp_module->next = module;
	}
	context->module_list_ctx->module_cnt++;

	return 0;
}

