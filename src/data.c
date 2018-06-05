#include <stdio.h>
#include <stdlib.h>
#include <smcf.h>
#include <data.h>
#include <module_manager.h>
#include <utils/node_list.h>

static int insert_dac_list(data_chn_attr_t *dca)
{
	data_chn_attr_t *tmp_dca = NULL;
	data_chn_attr_t *next_dca = gdc_table.dca;
	do {
		tmp_dca = next_dca;
		if (!tmp_dca->next)
			break;
		tmp_dca = NULL;
		next_dca = next_dca->next;
	} while(next_dca);
	if (tmp_dca)
		tmp_dca->next = dca;
	else
		return -1;

	return 0;
}

int check_producer_consumer_correspondence(void)
{
	if (gdc_table.schannel_cnt != gdc_table.rchannel_cnt) {
		printf("ERROR: pchannel_cnt[%d] != cchannel_cnt[%d]\n", \
			gdc_table.schannel_cnt, gdc_table.rchannel_cnt);
		return -1;
	}

	int channel_cnt = gdc_table.schannel_cnt + \
			  gdc_table.rchannel_cnt;

	int i = 0;
	data_chn_attr_t *search_dca = gdc_table.dca;
	for (i = 0; i < channel_cnt; i++) {
		if (search_dca->ca.chnrole == DATA_CHANNEL_RECEIVER) {
			int s_mid  = search_dca->ca.smid;
			int s_mchnid = search_dca->ca.schnid;
			int s_mchn_attr = search_dca->ca.chnattr;

			data_chn_attr_t *tmp_dca = NULL;
			data_chn_attr_t *next_dca = gdc_table.dca;
			do {
				tmp_dca = next_dca;
				if ((tmp_dca->ca.rmid == s_mid) ||
				    (tmp_dca->ca.rchnid == s_mchnid) ||
				    (tmp_dca->ca.chnattr == s_mchn_attr) ||
				    (tmp_dca->ca.chnrole == DATA_CHANNEL_SENDER)) {
					break;
				}
				tmp_dca = NULL;
				next_dca = next_dca->next;
			} while(next_dca);

			if (!tmp_dca) {
				printf("ERROR: sender & receiver not match!!\n");
				goto error;
			}
		}
		search_dca = search_dca->next;
	}

	//TODO: check N consummer to 1 producer
	return 0;
error:
	return -1;
}

int create_module_channel(module_t *module, int chn_num, int attr)
{
	/* TODO: module para check */

	int cur_module_info_cnt = module_chn_info.total_module_chn_cnt;

	/* check channel type*/
	int node_size = -1;
	int node_type = -1;

	int node_num;
	if (attr == NORMAL_CHN) {
		node_type = NODE_DATA_TYPE_BUFFER;
		node_num  = module->data_channel[chn_num].num;
		node_size = module->data_channel[chn_num].size;
	} else if (attr == HOOK_CHN) {
		node_type = NODE_DATA_TYPE_POINTER;
		node_num  = module->data_hchannel[chn_num].num;
	} else {
		printf("%s:%d -> attr error: %d\n", __func__, __LINE__, attr );
		goto error;
	}

	int node_list = Node_List_Init(node_size, node_num, NODE_KEEP_UPDATE, NODE_BLOCK, node_type);
	if (!node_list) {
		printf("ERROR\n");
		goto error;
	}

	module_chn_info.chn_info[cur_module_info_cnt].chn_node_list = node_list;
	module_chn_info.chn_info[cur_module_info_cnt].producer_id = module->id;
	if (attr == NORMAL_CHN) {
		module_chn_info.chn_info[cur_module_info_cnt].producer_chn_id = chn_num;
		module_chn_info.chn_info[cur_module_info_cnt].producer_hchn_id = -1;
	} else {
		module_chn_info.chn_info[cur_module_info_cnt].producer_chn_id = -1;
		module_chn_info.chn_info[cur_module_info_cnt].producer_hchn_id = chn_num;
	}
	module_chn_info.chn_info[cur_module_info_cnt].chn_use_node_cnt = 0;
	module_chn_info.chn_info[cur_module_info_cnt].chn_free_node_cnt = node_num;
	module_chn_info.total_module_chn_cnt++;

	return 0;
error:
	//TODO: unregister node_list before
	return -1;
}

void attach_sender_receiver_chn_info(void)
{
	if (gdc_table.schannel_cnt != gdc_table.rchannel_cnt) {
		printf("ERROR: pchannel_cnt[%d] != cchannel_cnt[%d]\n", \
			gdc_table.schannel_cnt, gdc_table.rchannel_cnt);
		return;
	}

	int channel_cnt = gdc_table.schannel_cnt + \
			  gdc_table.rchannel_cnt;
	//TODO: check total_module_schn_cnt = channel_cnt / 2;
	int cur_module_info_cnt = module_chn_info.total_module_chn_cnt;

	int i = 0;
	data_chn_attr_t *search_dca = gdc_table.dca;
	for (i = 0; i < channel_cnt; i++) {
		if (search_dca->ca.chnrole == DATA_CHANNEL_RECEIVER) {
			int r_mid  = search_dca->ca.rmid;
			int r_mchnid = search_dca->ca.rchnid;
			int rs_mid  = search_dca->ca.rsmid;
			int rs_mchnid = search_dca->ca.rschnid;
			int r_mchn_attr = search_dca->ca.chnattr;

			int k = 0;
			for (k = 0; k < cur_module_info_cnt; k++) {
				if (module_chn_info.chn_info[k].producer_id == rs_mid) {
					if (r_mchn_attr == NORMAL_CHN) {
						if (module_chn_info.chn_info[k].producer_chn_id == rs_mchnid) {
							module_chn_info.chn_info[k].consumer_id = r_mid;
							module_chn_info.chn_info[k].consumer_chn_id = r_mchnid;
							break;
						} else {
							printf("ERROR\n");
							goto error;
						}
					} else if (r_mchn_attr == HOOK_CHN) {
						if (module_chn_info.chn_info[k].producer_hchn_id == rs_mchnid) {
							module_chn_info.chn_info[k].consumer_id = r_mid;
							module_chn_info.chn_info[k].consumer_hchn_id = r_mchnid;
							break;
						} else {
							printf("ERROR\n");
							goto error;
						}
					}
				}
				if (k == cur_module_info_cnt) {
					printf("ERROR\n");
					goto error;
				}
			}

		}

		search_dca = search_dca->next;
	}

error:
	return;
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
	/*
	 *int channel_cnt = gdc_table.schannel_cnt + \
	 *                  gdc_table.rchannel_cnt;
	 */

	data_chn_attr_t *tmp_dca = NULL;
	data_chn_attr_t *dca = gdc_table.dca;
	do {
		tmp_dca = dca;
		if (tmp_dca->ca.chnrole == DATA_CHANNEL_SENDER) {
			module_t *cur_module = NULL;
			int module_id = tmp_dca->ca.smid;
			int mchn_id = tmp_dca->ca.schnid;

			cur_module = search_module_from_mblock_list(module_id);
			if (!cur_module) {
				printf("ERROR:\n");
				goto error;
			}

			ret = create_module_channel(cur_module, mchn_id, tmp_dca->ca.chnattr);
			if (ret) {
				printf("ERROR:\n");
				goto error;
			}
		}
		tmp_dca = NULL;
		dca = dca->next;
	} while(dca);

	//TODO: insert correspondence [receiver <-> sender]
	attach_sender_receiver_chn_info();

	return 0;
error:
	return -1;
}

int module_data_channel_init(module_t *module)
{
	int ret = -1;
	if (!module) {
		printf("ERROR!\n");
		return -1;
	}

	int module_id = module->id;

	int i = 0;
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
				dca->ca.chnattr = NORMAL_CHN;
				dca->ca.chnrole = module->data_channel[i].role;

				if (module->data_channel[i].role == DATA_CHANNEL_SENDER) {
					dca->ca.smid   = module_id;
					dca->ca.schnid = i;
					gdc_table.schannel_cnt++;
				} else {
					dca->ca.rmid   = module_id;
					dca->ca.rchnid = i;
					dca->ca.rsmid   = module->data_channel[i].module_id;
					dca->ca.rschnid = module->data_channel[i].channel;
					gdc_table.rchannel_cnt++;
				}
				if (!gdc_table.dca)
					gdc_table.dca = dca;
				else {
					ret = insert_dac_list(dca);
					if (ret) {
						printf("ERROR\n");
						goto error;
					}
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
				dca->ca.chnattr = HOOK_CHN;
				dca->ca.chnrole   = module->data_hchannel[i].role;

				if (module->data_hchannel[i].role == DATA_CHANNEL_SENDER) {
					dca->ca.smid   = module_id;
					dca->ca.schnid = i;
					gdc_table.schannel_cnt++;
				} else {
					dca->ca.rmid   = module_id;
					dca->ca.rchnid = i;
					dca->ca.rsmid   = module->data_hchannel[i].module_id;
					dca->ca.rschnid = module->data_hchannel[i].channel;
					gdc_table.rchannel_cnt++;
				}
				if (!gdc_table.dca)
					gdc_table.dca = dca;
				else {
					ret = insert_dac_list(dca);
					if (ret) {
						printf("ERROR\n");
						goto error;
					}
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

int search_data_node_list(int module_id, int chn_id)
{
	return 0;
}

void *get_data(int current_module_id, int channel_id, int chn_type)
{
	module_t *cur_module = NULL;
	cur_module = search_module_from_mblock_list(current_module_id);
	if (!cur_module) {
		printf("ERROR:\n");
		goto error;
	}

	int mid;
	int cid;
	int module_role;
	if (chn_type == NORMAL_CHN) {
		module_role = cur_module->data_channel[channel_id].role;
	} else if (chn_type == HOOK_CHN) {
		module_role = cur_module->data_hchannel[channel_id].role;
	} else {
		printf("ERROR\n");
		goto error;
	}

	if (module_role == DATA_CHANNEL_SENDER) {
		mid = current_module_id;
		cid = channel_id;
	} else if (module_role == DATA_CHANNEL_RECEIVER) {
		if (chn_type == NORMAL_CHN) {
			mid = cur_module->data_channel[channel_id].module_id;
			cid = cur_module->data_channel[channel_id].channel;
		} else {
			mid = cur_module->data_hchannel[channel_id].module_id;
			cid = cur_module->data_hchannel[channel_id].channel;
		}
	} else {
		printf("ERROR\n");
		goto error;
	}

	int cur_module_info_cnt = module_chn_info.total_module_chn_cnt;
	int i = 0;
	for (i = 0; i < cur_module_info_cnt; i++) {
		if (((module_chn_info.chn_info[cur_module_info_cnt].producer_id == mid) && \
		     (module_chn_info.chn_info[cur_module_info_cnt].producer_chn_id == cid)) || \
		    ((module_chn_info.chn_info[cur_module_info_cnt].producer_id == mid) && \
		     (module_chn_info.chn_info[cur_module_info_cnt].producer_hchn_id == cid))) {
			break;
		}
	}

	if (i == cur_module_info_cnt) {
		printf("ERROR\n");
		goto error;
	}

	int nodelist = module_chn_info.chn_info[cur_module_info_cnt].chn_node_list;

	node_t *node = NULL;
	if (module_role == DATA_CHANNEL_SENDER) {
		node = Get_Free_Node(nodelist);
		module_chn_info.chn_info[cur_module_info_cnt].chn_free_node_cnt--;
	} else {
		node = Get_Use_Node(nodelist);
		module_chn_info.chn_info[cur_module_info_cnt].chn_use_node_cnt--;
	}

	return node;
error:
	return NULL;
}

int put_data(int current_module_id, int channel_id, void *data, int chn_type)
{
	module_t *cur_module = NULL;
	cur_module = search_module_from_mblock_list(current_module_id);
	if (!cur_module) {
		printf("ERROR:\n");
		goto error;
	}

	int mid;
	int cid;
	int module_role;
	if (chn_type == NORMAL_CHN) {
		module_role = cur_module->data_channel[channel_id].role;
	} else if (chn_type == HOOK_CHN) {
		module_role = cur_module->data_hchannel[channel_id].role;
	} else {
		printf("ERROR\n");
		goto error;
	}

	if (module_role == DATA_CHANNEL_SENDER) {
		mid = current_module_id;
		cid = channel_id;
	} else if (module_role == DATA_CHANNEL_RECEIVER) {
		if (chn_type == NORMAL_CHN) {
			mid = cur_module->data_channel[channel_id].module_id;
			cid = cur_module->data_channel[channel_id].channel;
		} else {
			mid = cur_module->data_hchannel[channel_id].module_id;
			cid = cur_module->data_hchannel[channel_id].channel;
		}
	} else {
		printf("ERROR\n");
		goto error;
	}

	int cur_module_info_cnt = module_chn_info.total_module_chn_cnt;
	int i = 0;
	for (i = 0; i < cur_module_info_cnt; i++) {
		if (((module_chn_info.chn_info[cur_module_info_cnt].producer_id == mid) && \
		     (module_chn_info.chn_info[cur_module_info_cnt].producer_chn_id == cid)) || \
		    ((module_chn_info.chn_info[cur_module_info_cnt].producer_id == mid) && \
		     (module_chn_info.chn_info[cur_module_info_cnt].producer_hchn_id == cid))) {
			break;
		}
	}

	if (i == cur_module_info_cnt) {
		printf("ERROR\n");
		goto error;
	}

	int nodelist = module_chn_info.chn_info[cur_module_info_cnt].chn_node_list;

	node_t *node = (node_t *)data;
	if (module_role == DATA_CHANNEL_SENDER) {
		Put_Use_Node(nodelist, node);
		module_chn_info.chn_info[cur_module_info_cnt].chn_use_node_cnt++;
	} else {
		node = Get_Use_Node(nodelist);
		Put_Free_Node(nodelist, node);
		module_chn_info.chn_info[cur_module_info_cnt].chn_free_node_cnt++;
	}

	return 0;
error:
	return -1;
}

void *get_normal_chn_data(int current_module_id, int channel_id)
{
	return get_data(current_module_id, channel_id, NORMAL_CHN);
}

int put_normal_chn_data(int current_module_id, int channel_id, void *data)
{
	return put_data(current_module_id, channel_id, data, NORMAL_CHN);
}

void *get_hook_chn_data(int current_module_id, int channel_id)
{
	return get_data(current_module_id, channel_id, HOOK_CHN);
}

int put_hook_chn_data(int current_module_id, int channel_id, void *data)
{
	return put_data(current_module_id, channel_id, data, HOOK_CHN);
}
