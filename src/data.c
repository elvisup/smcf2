#include <stdio.h>
#include <smcf.h>
#include <data.h>
#include <module_manager.h>
#include <utils/node_list.h>

int check_producer_consumer_correspondence(void)
{
	if (g_data_chn_table.pchannel_cnt != g_data_chn_table.cchannel_cnt) {
		printf("ERROR: pchannel_cnt[%d] != cchannel_cnt[%d]\n", \
			g_data_chn_table.pchannel_cnt, g_data_chn_table.cchannel_cnt);
		return -1;
	}

	int channel_cnt = g_data_chn_table.pchannel_cnt + \
			  g_data_chn_table.cchannel_cnt;

	int i = 0;
	for (i = 0; i < channel_cnt; i++) {
		if (g_data_chn_table.data_chn_table[i][DTABLE_CH_ATTR] == DATA_CHANNEL_RECEIVER) {
			int pro_mid  = g_data_chn_table.data_chn_table[i][DTABLE_PRO_MODULE_ID];
			int pro_mcnt = g_data_chn_table.data_chn_table[i][DTABLE_PRO_MODULE_CH_ID];

			int k = 0;
			for (k = 0; k < channel_cnt; k++) {
				if ((g_data_chn_table.data_chn_table[k][DTABLE_CH_ATTR] == DATA_CHANNEL_SENDER) && \
				    (g_data_chn_table.data_chn_table[k][DTABLE_OWN_MODULE_ID] == pro_mid) && \
				    (g_data_chn_table.data_chn_table[k][DTABLE_OWN_MODULE_CH_ID] == pro_mcnt)) {
					break;
				}
			}

			if (channel_cnt == k) {
				printf("ERROR: producer & consummer not match!!\n");
				goto error;
			}
		}
	}

	//TODO: check N consummer to 1 producer
	return 0;
error:
	return -1;
}

int create_module_channel(module_t *module, int chn_num)
{
	/* TODO: module para check */

	int cur_module_info_cnt = module_chn_info.total_module_chn_cnt;

	/* check channel type*/
	int node_size = -1;
	int node_type = -1;

	node_size = module->data_channel[chn_num].size;
	node_type = NODE_DATA_TYPE_BUFFER;

	int node_num  = module->data_channel[chn_num].num;
	int node_list = Node_List_Init(node_size, node_num, NODE_KEEP_UPDATE, NODE_BLOCK, node_type);
	if (!node_list) {
		printf("ERROR\n");
		goto error;
	}

	module_chn_info.chn_info[cur_module_info_cnt].chn_node_list = node_list;
	module_chn_info.chn_info[cur_module_info_cnt].producer_id = module->id;
	module_chn_info.chn_info[cur_module_info_cnt].producer_chn_id = chn_num;
	module_chn_info.chn_info[cur_module_info_cnt].chn_use_node_cnt = 0;
	module_chn_info.chn_info[cur_module_info_cnt].chn_free_node_cnt = node_num;
	module_chn_info.total_module_chn_cnt++;

	return 0;
error:
	//TODO: unregister node_list before
	return -1;
}

int attach_module_channel()
{
	int ret = -1;

	// check correspondence
	ret = check_producer_consumer_correspondence();
	if (ret) {
		printf("ERROR\n");
		return ret;
	}

	// create channel node_list
	int channel_cnt = g_data_chn_table.pchannel_cnt + \
			  g_data_chn_table.cchannel_cnt;

	int i = 0;
	for (i = 0; i < channel_cnt; i++) {
		if (g_data_chn_table.data_chn_table[i][DTABLE_CH_ATTR] == DATA_CHANNEL_SENDER) {
			module_t *cur_module = NULL;
			int module_id = g_data_chn_table.data_chn_table[i][DTABLE_OWN_MODULE_ID];
			cur_module = search_module_from_mblock_list(module_id);
			if (!cur_module) {
				printf("ERROR:\n");
				goto error;
			}

			int mchn_id = g_data_chn_table.data_chn_table[i][DTABLE_OWN_MODULE_CH_ID];
			ret = create_module_channel(cur_module, mchn_id);
			if (ret) {
				printf("ERROR:\n");
				goto error;
			}
		}
	}

	//TODO: create correspondence table list

	return 0;
error:
	return -1;
}

int module_data_channel_init(module_t *module)
{
	if (!module) {
		printf("ERROR!\n");
		return -1;
	}

	int module_id = module->id;

	int i = 0, table_cnt = 0;
	/* normal channel */
	for (i = 0; i < MAX_MODULE_CHANNEL_CNT; i++) {
		if (module->data_channel[i].enable == MODULE_CHANNEL_ENABLE) {
			if ((module->data_channel[i].role == DATA_CHANNEL_SENDER) || \
			    (module->data_channel[i].role == DATA_CHANNEL_RECEIVER)) {
				data_chn_attr_t *dca = (data_chn_attr_t *)malloc(sizeof(data_chn_attr_t));
				if (dca == NULL) {
					printf("%s:%d -> malloc dca error!\n", __func__, __LINE__);
					goto error;
				}
				dca->next = NULL;
				dca->chnattr = NORMAL_CHN;

				if (module->data_channel[i].role == DATA_CHANNEL_SENDER) {
					dca->simd   = module_id;
					dca->schnid = i;
					gdc_table.schannel_cnt++;
				} else {
					dca->rimd   = module_id;
					dca->rchnid = i;
					gdc_table.rchannel_cnt++;
				}
				//TODO: need to check
				if (!gdc_table->dca)
					gdc_table->dca = dca;
				else {
					data_chn_attr_t *tmp_dca = NULL;
					data_chn_attr_t *next_dca = gdc_table->dca;
					do {
						tmp_dca = next_dca;
						if (!tmp_dca->next)
							break;
						tmp_dca = NULL;
						next_dca = next_dca->next;
					} while(next_dca);
					tmp_dca->next = dca;
				}
			} else {
				printf("ERROR, channel role invaild!\n");
				goto error;
			}
		}
	}

	/* hook channel */
	for (i = 0; i < MAX_MODULE_CHANNEL_CNT; i++) {
		if (module->data_hchannel[i].enable == MODULE_CHANNEL_ENABLE) {
			if ((module->data_hchannel[i].role == DATA_CHANNEL_SENDER) || \
			    (module->data_hchannel[i].role == DATA_CHANNEL_RECEIVER)) {
				data_chn_attr_t *dca = (data_chn_attr_t *)malloc(sizeof(data_chn_attr_t));
				if (dca == NULL) {
					printf("%s:%d -> malloc dca error!\n", __func__, __LINE__);
					goto error;
				}
				dca->next = NULL;
				dca->chnattr = HOOK_CHN;

				if (module->data_hchannel[i].role == DATA_CHANNEL_SENDER) {
					dca->simd   = module_id;
					dca->schnid = i;
					gdc_table.schannel_cnt++;
				} else {
					dca->rimd   = module_id;
					dca->rchnid = i;
					gdc_table.rchannel_cnt++;
				}
				//TODO: need to check
				if (!gdc_table->dca)
					gdc_table->dca = dca;
				else {
					data_chn_attr_t *tmp_dca = NULL;
					data_chn_attr_t *next_dca = gdc_table->dca;
					do {
						tmp_dca = next_dca;
						if (!tmp_dca->next)
							break;
						tmp_dca = NULL;
						next_dca = next_dca->next;
					} while(next_dca);
					tmp_dca->next = dca;
				}
			} else {
				printf("ERROR, channel role invaild!\n");
				goto error;
			}
		}
	}

	return 0;
error:
	return -1;
}
