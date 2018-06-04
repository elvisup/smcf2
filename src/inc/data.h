/**
 * data.h
 **/
#ifndef __DATA_H__
#define __DATA_H__

#define NORMAL_CHN 0
#define HOOK_CHN   1

#define MAX_CHN_THRESHOLD    128

typedef struct __chn_attr {
	int chnrole;  /* chn attr */
	int chnattr;  /* chn attr */

	int smid;      /* send module id */
	int schnid;    /* send chn id */

	int rmid;      /* recv module id */
	int rchnid;    /* recv chn id */
} chn_attr_t;

typedef struct __data_chn_attr data_chn_attr_t;
typedef struct __data_chn_attr {
	chn_attr_t ca;        /* chn attr */
	data_chn_attr_t *next;
} data_chn_attr_t;

typedef struct __data_chn_table {
	int schannel_cnt; /* send channel cnt */
	int rchannel_cnt; /* recv channel cnt */
	data_chn_attr_t *dca;
} data_chn_table_t;
data_chn_table_t gdc_table = {
	.schannel_cnt = 0,
	.rchannel_cnt = 0,
	.dca = NULL,
};

//{node_list, pro_id, pro_chn_id, con_id, con_chn_id, run_cnt, run_time, use_cnt, free_cnt}
typedef struct __channel_info {
	int chn_node_list;
	int producer_id;
	int producer_chn_id;
	int producer_hchn_id;
	int consumer_id;
	int consumer_chn_id;
	int consumer_hchn_id;
	int chn_use_node_cnt;
	int chn_free_node_cnt;
} channel_info_t;

typedef struct __module_chn_info {
	int total_module_chn_cnt;
	channel_info_t chn_info[MAX_CHN_THRESHOLD];
} module_chn_info_t;
module_chn_info_t module_chn_info;

#endif
