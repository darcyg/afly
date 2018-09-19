#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 0
#include "common.h"
#include "log.h"
#include "hex.h"
#include "jansson.h"
#include "json_parser.h"

#include "alink_profile.h"

#define MAX_NUM_CMD_PROFILE		200
#define MAX_NUM_ATTR_PROFILE	200

//static int	cmd_profile_num = 0;
static cmd_profile_t	*cmd_profiles[MAX_NUM_CMD_PROFILE] = {0};
//static int	attr_profile_num = 0;
static attr_profile_t *attr_profiles[MAX_NUM_ATTR_PROFILE] = {0};


typedef json_t *(*B2J_FUNC)(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
typedef int     (*J2B_FUNC)(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
//static b2j////////////////////////////////////////////////////////////////////////////////////
static json_t *b2j_data8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_data64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_boolean(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_uint8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_uint64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_int8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_int64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_enum8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_enum16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);


static json_t *b2j_semi_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_single_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_double_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_octet_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_char_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_long_octet_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_long_char_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_struct(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_array(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_bitmap8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bitmap64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

static json_t *b2j_set(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bag(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_tod(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_date(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_utc(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_cluster_id(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_attr_id(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_bac_oid(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_ieee_addr(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);
static json_t *b2j_128_bit_sec_key(char *buf, int lenbit, int mask_code, int *used, int is_bitmap_type);

//static j2b////////////////////////////////////////////////////////////////////////////////////
static int j2b_data8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_data64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_boolean(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_uint8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_uint64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_int8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_int64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_enum8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_enum16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_semi_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_single_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_double_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_octet_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_char_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_long_octet_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_long_char_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_struct(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_array(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_bitmap8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bitmap64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static int j2b_set(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bag(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_tod(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_date(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_utc(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_cluster_id(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_attr_id(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_bac_oid(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_ieee_addr(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);
static int j2b_128_bit_sec_key(char *buf, json_t *jarg, int mask_code, int is_bitmap_type);

static stAlinkDataType_t alink_data_types[] = {
	{"data8",		ZCL_DATATYPE_DATA8,		8,	b2j_data8,  j2b_data8},
	{"data16",	ZCL_DATATYPE_DATA16,	16, b2j_data16, j2b_data16},
	{"data24",	ZCL_DATATYPE_DATA24,	24, b2j_data24, j2b_data24},
	{"data32",	ZCL_DATATYPE_DATA32,	32, b2j_data32, j2b_data32},
	{"data40",	ZCL_DATATYPE_DATA40,	40, b2j_data40, j2b_data40},
	{"data48",	ZCL_DATATYPE_DATA48,	48, b2j_data48, j2b_data48},
	{"data56",	ZCL_DATATYPE_DATA56,	56, b2j_data56, j2b_data56},
	{"data64",	ZCL_DATATYPE_DATA64,	64, b2j_data64, j2b_data64},

	{"boolean",	ZCL_DATATYPE_BOOLEAN,	8,	b2j_boolean, j2b_boolean},

	{"uint8",		ZCL_DATATYPE_UINT8,		8,	b2j_uint8,	j2b_uint8},
	{"uint16",	ZCL_DATATYPE_UINT16,	16, b2j_uint16, j2b_uint16},
	{"uint24",	ZCL_DATATYPE_UINT24,	24, b2j_uint24, j2b_uint24},
	{"uint32",	ZCL_DATATYPE_UINT32,	32, b2j_uint32, j2b_uint32},
	{"uint40",	ZCL_DATATYPE_UINT40,	40, b2j_uint40, j2b_uint40},
	{"uint48",	ZCL_DATATYPE_UINT48,	48, b2j_uint48, j2b_uint48},
	{"uint56",	ZCL_DATATYPE_UINT56,	56, b2j_uint56, j2b_uint56},
	{"uint64",	ZCL_DATATYPE_UINT64,	64, b2j_uint64, j2b_uint64},

	
	{"int8",		ZCL_DATATYPE_INT8,		8,	b2j_int8,		j2b_int8},
	{"int16",		ZCL_DATATYPE_INT16,		16, b2j_int16,	j2b_int16},
	{"int24",		ZCL_DATATYPE_INT24,		24, b2j_int24,	j2b_int24},
	{"int32",		ZCL_DATATYPE_INT32,		32, b2j_int32,	j2b_int32},
	{"int40",		ZCL_DATATYPE_INT40,		40, b2j_int40,	j2b_int40},
	{"int48",		ZCL_DATATYPE_INT48,		48, b2j_int48,	j2b_int48},
	{"int56",		ZCL_DATATYPE_INT56,		56, b2j_int56,	j2b_int56},
	{"int64",		ZCL_DATATYPE_INT64,		64, b2j_int64,	j2b_int64},

	{"enum8",		ZCL_DATATYPE_ENUM8,		8,	b2j_enum8,	j2b_enum8},
	{"enum16",	ZCL_DATATYPE_ENUM16,	8,	b2j_enum16, j2b_enum16},

	{"semi_float",			ZCL_DATATYPE_SEMI_PREC,			0,	b2j_semi_prec,		j2b_semi_prec},
	{"float ",					ZCL_DATATYPE_SINGLE_PREC,		0,	b2j_single_prec,	j2b_single_prec},
	{"double_float",		ZCL_DATATYPE_DOUBLE_PREC,		0,	b2j_double_prec,	j2b_double_prec},
	{"octet_str",				ZCL_DATATYPE_OCTET_STR,			0,	b2j_octet_prec,		j2b_octet_prec},
	{"string",					ZCL_DATATYPE_CHAR_STR,			0,	b2j_char_prec,		j2b_char_prec},
	{"long_octet_str",	ZCL_DATATYPE_LONG_OCTET_STR,0,	b2j_long_octet_prec,	j2b_long_octet_prec},
	{"long_string",			ZCL_DATATYPE_LONG_CHAR_STR, 0,	b2j_long_char_prec,		j2b_long_char_prec},

	{"struct",	ZCL_DATATYPE_STRUCT, 0, b2j_struct,			j2b_struct},
	{"arrry",		ZCL_DATATYPE_ARRAY, 0, b2j_array,				j2b_array},

	{"bitmap8",		ZCL_DATATYPE_BITMAP8,		8,	b2j_bitmap8,	j2b_bitmap8},
	{"bitmap16",	ZCL_DATATYPE_BITMAP16,	16, b2j_bitmap16, j2b_bitmap16},
	{"bitmap24",	ZCL_DATATYPE_BITMAP24,	24, b2j_bitmap24, j2b_bitmap24},
	{"bitmap32",	ZCL_DATATYPE_BITMAP32,	32, b2j_bitmap32, j2b_bitmap32},
	{"bitmap40",	ZCL_DATATYPE_BITMAP40,	40, b2j_bitmap40, j2b_bitmap40},
	{"bitmap48",	ZCL_DATATYPE_BITMAP48,	48, b2j_bitmap48, j2b_bitmap48},
	{"bitmap56",	ZCL_DATATYPE_BITMAP56,	56, b2j_bitmap56, j2b_bitmap56},
	{"bitmap64",	ZCL_DATATYPE_BITMAP64,	64, b2j_bitmap64, j2b_bitmap64},

	{"set",					ZCL_DATATYPE_SET, 0, b2j_set,								j2b_set},
	{"bag",					ZCL_DATATYPE_BAG, 0, b2j_bag,								j2b_bag},
	{"tdo",					ZCL_DATATYPE_TOD, 0, b2j_tod,								j2b_tod},
	{"date",				ZCL_DATATYPE_DATE, 0, b2j_date,							j2b_date},
	{"utc",					ZCL_DATATYPE_UTC, 0, b2j_utc,								j2b_utc},
	{"cluster_id",	ZCL_DATATYPE_CLUSTER_ID, 0, b2j_cluster_id, j2b_cluster_id},
	{"attr_id",			ZCL_DATATYPE_ATTR_ID, 0, b2j_attr_id,				j2b_attr_id},
	{"bac_oid",			ZCL_DATATYPE_BAC_OID, 0, b2j_bac_oid,				j2b_bac_oid},
	{"ieee_addr",		ZCL_DATATYPE_IEEE_ADDR, 0,b2j_ieee_addr,		j2b_ieee_addr},
	{"sec_key",			ZCL_DATATYPE_128_BIT_SEC_KEY, 0, b2j_128_bit_sec_key, j2b_128_bit_sec_key},
};

static int alink_is_bitmap_datatype(unsigned char type) {
	return (type >= ZCL_DATATYPE_BITMAP8 && type <= ZCL_DATATYPE_BITMAP64);
}
static stAlinkDataType_t *alink_datatype_search(unsigned char data_type) {
	int cnt = sizeof(alink_data_types)/sizeof(alink_data_types[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stAlinkDataType_t *adt = &alink_data_types[i];
		if (adt->id == data_type) {
			return adt;
		}
	}
	return NULL;
}
static json_t *alink_parse_buf2json(unsigned char data_type, unsigned int mask_code, char *buf, int lenbit, int *used, int is_bitmap_type) {
	stAlinkDataType_t *adt = alink_datatype_search(data_type);
	if (adt == NULL) {
		*used = 0;
		return NULL;
	}
	json_t * jret = ((B2J_FUNC)adt->buf2json)(buf, lenbit, mask_code, used, is_bitmap_type);
	return jret;
}
static int alink_parse_json2buf(json_t *jarg, unsigned char data_type, unsigned int mask_code, char *buf, int is_bitmap_type) {
	stAlinkDataType_t *adt = alink_datatype_search(data_type);
	if (adt == NULL) {
		return 0;
	}

	int used = ((J2B_FUNC)adt->json2buf)(buf, jarg, mask_code, is_bitmap_type);

	return used;
}

static int alink_argument2json(json_t *jparent, cmd_argument_t *arg, char *buf, int lenbit, int is_bitmap_type) {
	int used = 0;	//bit num parse used
	json_t *jarg = alink_parse_buf2json(arg->data_type, arg->mask_code, buf, lenbit, &used, is_bitmap_type);
	json_object_set_new(jparent, arg->arg_name, jarg);

	if (is_bitmap_type && arg->next_arg != NULL) {
		used = 0;
	}

	if (arg->child_argument != NULL) {
		used += alink_argument2json(jarg, arg->child_argument, buf+used/8, lenbit-used, alink_is_bitmap_datatype(arg->data_type));
	}

	if (arg->next_arg != NULL) {
		used += alink_argument2json(jparent, arg->next_arg, buf+used/8, lenbit-used, is_bitmap_type);
	}
	return used;
}

static int alink_attr2json(json_t *jparent, attr_element_t *arg, char *buf, int lenbit, int is_bitmap_type) {
	int used = 0;	//bit num parse used

	//log_debug("arg Name:%s, lenbit:%d", arg->element_name, lenbit);

	json_t *jarg = alink_parse_buf2json(arg->data_type, arg->mask_code, buf, lenbit, &used, is_bitmap_type);
	json_object_set_new(jparent, arg->element_name, jarg);

	if (is_bitmap_type && arg->next_element != NULL) {
		used = 0;
	}

	if (arg->child_element != NULL) {
		used += alink_attr2json(jarg, arg->child_element, buf+used/8, lenbit-used, alink_is_bitmap_datatype(arg->data_type));
	}

	if (arg->next_element != NULL) {
		used += alink_attr2json(jparent, arg->next_element, buf+used/8, lenbit-used, is_bitmap_type);
	}
	return used;
}

static int alink_json2argument(json_t *jparent, cmd_argument_t *arg,  char *buf, int is_bitmap_type) {
	int used = 0;

	json_t *jarg = json_object_get(jparent, arg->arg_name);

	used = alink_parse_json2buf(jarg, arg->data_type, arg->mask_code, buf, is_bitmap_type);

	if (is_bitmap_type && arg->next_arg != NULL) {
		used = 0;
	}

	if (arg->child_argument != NULL) {
		used += alink_json2argument(jarg, arg->child_argument, buf+used/8, alink_is_bitmap_datatype(arg->data_type));
	}

	if (arg->next_arg != NULL) {
		used += alink_json2argument(jparent, arg->next_arg, buf+used/8, is_bitmap_type);
	}
	return used;
}

static int alink_json2attr(json_t *jparent, attr_element_t *arg,  char *buf, int is_bitmap_type) {
	int used = 0;

	json_t *jarg = json_object_get(jparent, arg->element_name);

	used = alink_parse_json2buf(jarg, arg->data_type, arg->mask_code, buf, is_bitmap_type);

	if (is_bitmap_type && arg->next_element != NULL) {
		used = 0;
	}

	if (arg->child_element != NULL) {
		used += alink_json2attr(jarg, arg->child_element, buf+used/8, alink_is_bitmap_datatype(arg->data_type));
	}

	if (arg->next_element != NULL) {
		used += alink_json2attr(jparent, arg->next_element, buf+used/8, is_bitmap_type);
	}
	return used;
}




//interface//////////////////////////////////////////////////////////////////////
void alink_cmd_profile_add(cmd_profile_t *profile) {
	int i = 0;
	for (i = 0; i < sizeof(cmd_profiles)/sizeof(cmd_profiles[0]); i++) {
		if (cmd_profiles[i] == profile) {
			return;
		}

		if (cmd_profiles[i] == NULL) {
			cmd_profiles[i] = profile;
			return;
		}
	}
}
void alink_attr_profile_add(attr_profile_t *profile) {
	int i = 0;
	for (i = 0; i < sizeof(attr_profiles)/sizeof(attr_profiles[0]); i++) {
		if (attr_profiles[i] == profile) {
			return;
		}

		if (attr_profiles[i] == NULL) {
			attr_profiles[i] = profile;
			return;
		}
	}
}

cmd_profile_t * alink_cmd_profile_search_cmd(short clusterid, char cmdid) {
	int i = 0;
	for (i = 0; i < sizeof(cmd_profiles)/sizeof(cmd_profiles[0]); i++) {
		cmd_profile_t *cp = cmd_profiles[i];
		if (cp == NULL) {
			return NULL;
		}

		if (cp->cluster_id == (unsigned short)clusterid && 
				cp->cmd_id			== (unsigned char )cmdid) {
			return cp;
		}
	}

	return NULL;
}

attr_profile_t * alink_attr_profile_search_attr(short clusterid, short attrid) {
	int i = 0;
	for (i = 0; i < sizeof(attr_profiles)/sizeof(attr_profiles[0]); i++) {
		attr_profile_t *ap = attr_profiles[i];
		if (ap == NULL) {
			return NULL;
		}

		if (ap->cluster_id == (unsigned short)clusterid && 
				ap->attribute_id		== (unsigned short )attrid) {
			return ap;
		}
	}

	return NULL;
}


int alink_cmd_profile_search_by_cmdname(const char *cmd_name, cmd_profile_t *cps[], int cnt) {
	int i = 0;
	int j = 0;
	for (i = 0; i < sizeof(cmd_profiles)/sizeof(cmd_profiles[0]); i++) {
		cmd_profile_t *cp = cmd_profiles[i];
		if (cp == NULL) {
			break;
		}

		if (strcmp(cmd_name, cp->cmd_name) == 0) {
			cps[j++] = cp;
		}

		if (j >= cnt) {
			break;
		}
	}

	return j;
}

int alink_attr_profile_search_by_attrname(const char *attr_name, attr_profile_t *aps[], int cnt) {
	int i = 0;
	int j = 0;
	for (i = 0; i < sizeof(attr_profiles)/sizeof(attr_profiles[0]); i++) {
		attr_profile_t *ap = attr_profiles[i];
		if (ap == NULL) {
			break;
		}

		if (strcmp(ap->attr_name, attr_name) == 0) {
			aps[j++] = ap;
		}

		if (j >= cnt) {
			break;
		}
	}

	return j;
}

const char * alink_cmd_profile_buf2json(cmd_profile_t *cp, char *buf, int len) {
	json_t *jargs = json_object();

	if (cp->args != NULL) {
		alink_argument2json(jargs, cp->args, buf, len*8, 0);
	}

	const char *sret =  json_dumps(jargs, 0);

	json_decref(jargs);

	return sret;
}

const char * alink_attr_profile_buf2json(attr_profile_t *ap, char *buf, int len) {
	//json_t *jargs = json_object();

	int used = 0;
	json_t *jargs = alink_parse_buf2json(ap->data_type, 0, buf, len*8, &used, 0);

	if (ap->attr_element != NULL) {
		alink_attr2json(jargs, ap->attr_element, buf + used/8, len*8 - used, alink_is_bitmap_datatype(ap->data_type));
	}

	char *sret = NULL;
	if (json_is_string(jargs)) {
		const char *u = json_string_value(jargs);
		sret = (char *)malloc(strlen(u) + 1);
		strcpy(sret, u);
	} else {
		sret = json_dumps(jargs, 0);
	}

	json_decref(jargs);

	return sret;
}

int alink_cmd_profile_json2buf(cmd_profile_t *cp, const char *profile, char *buf) {
	json_error_t error;
	json_t *jargs = json_loads(profile, 0, &error);
	if (jargs == NULL) {
		return -1;
	}
	
	char tmpbuf[128];
	memset(tmpbuf, 0, sizeof(tmpbuf));
	int used = alink_json2argument(jargs, cp->args, tmpbuf, 0);

	json_decref(jargs);

	memcpy(buf, tmpbuf, used/8);

	return (used/8);
}

int alink_attr_profile_json2buf(attr_profile_t *ap, const char *profile, char *buf) {
	json_error_t error;
	json_t *jargs = json_loads(profile, 0, &error);
	if (jargs == NULL) {
		return -1;
	}
	
	char tmpbuf[128];
	memset(tmpbuf, 0, sizeof(tmpbuf));
	int used = alink_json2attr(jargs, ap->attr_element, tmpbuf, alink_is_bitmap_datatype(ap->data_type));

	json_decref(jargs);

	memcpy(buf, tmpbuf, used/8);

	return (used/8);
}

//b2j/////////////////////////////////////////////////////////////////////////////////////
static json_t * B2J_DATAN(int n, char *buf, int lenbit, int *used) {
	char tmp[64]; 
	json_t *jret = NULL; 
	*(used) = 0;

	int len = (n); 
	if ((lenbit) >= len) { 
		int i = 0; 
		int off = 0; 
		for (i = 0; i < len/8; i++) { 
			if (i == 0) { 
				off += sprintf((tmp) + off, "0x"); 
			} 
			off += sprintf((tmp) + off, "%02x", (buf)[i]); 
		} 
		jret = json_string(tmp); 
		*(used) = len; 
	} 
	return jret; 
} 

static json_t *B2J_BOOLEAN(char *buf, int lenbit, int *used) { 
	char tmp[64]; 
	json_t *jret = NULL; 
	*(used) = 0;
	
	int len = (8); 
	if ((lenbit) >= len) { 
		sprintf(tmp, "%d", (buf)[0]&0xff); 
		jret = json_string(tmp); 
		*(used) = len; 
	} 
	return jret; 
} 

static long long B2J_MASK(long long val, int mask) {
	int i = 0;
	int len = sizeof(mask);
	long long newval = 0;
	for (i = len-1; i >= 0; i--)  {
		int bit = (mask >> i)&0x1;
		if (bit == 1) {
			newval = (newval << 1) | ((val>>i)&0x1);
		}
	}
	return newval;
}

static json_t * B2J_UINTN(int n, char *buf, int lenbit, int *used, int mask)  { 
	char tmp[64]; 
	json_t *jret = NULL; 
	*(used) = 0;
	
	int len = (n); 
	if ((lenbit) >= len) { 
		int i = 0; 
		long long val = 0; 
		for (i = 0; i < len/8; i++) { 
			val = val | ((buf)[i]&0xff) << (i*8);
		} 
		if ((mask) != 0) { 
			val = B2J_MASK(val, mask);
		} 
		sprintf(tmp, "%llu", val); 
		jret = json_string(tmp); 
		*(used) = len; 
	} 
	return jret; 
} 

static json_t *B2J_INTN(int n, char *buf, int lenbit, int *used, int mask) {
	char tmp[64]; 
	json_t *jret = NULL; 
	*(used) = 0;
	
	int len = (n); 
	if ((lenbit) >= len) { 
		int i = 0; 
		long long val = 0; 
		for (i = 0; i < len/8; i++) { 
			val = val | ((buf)[i]&0xff) << (i*8);
		} 
		if ((mask) != 0) { 
			val = B2J_MASK(val, mask);
		} 
		sprintf(tmp, "%lld", val); 
		jret = json_string(tmp); 
		*(used) = len; 
	} 
	return jret; 
}

#define B2J_ENUMN(n, buf, lenbit, used, mask) B2J_UINTN(n, buf, lenbit, used, mask)

static json_t *b2j_data8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(8, buf, lenbit, used);
}
static json_t *b2j_data16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(16, buf, lenbit, used);
}
static json_t *b2j_data24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(24, buf, lenbit, used);
}
static json_t *b2j_data32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(32, buf, lenbit, used);
}
static json_t *b2j_data40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(40, buf, lenbit, used);
}
static json_t *b2j_data48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(48, buf, lenbit, used);
}
static json_t *b2j_data56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(56, buf, lenbit, used);
}
static json_t *b2j_data64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(64, buf, lenbit, used);
}

static json_t *b2j_boolean(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_BOOLEAN(buf, lenbit, used);
}

static json_t *b2j_uint8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(8, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(16, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(24, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(32, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(40, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(48, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(56, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_uint64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_UINTN(64, buf, lenbit, used, is_bitmap ? mask_code : 0);
}

static json_t *b2j_int8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(8, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(16, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(24, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(32, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(40, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(48, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(56, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_int64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_INTN(64, buf, lenbit, used, is_bitmap ? mask_code : 0);
}

static json_t *b2j_enum8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_ENUMN(8, buf, lenbit, used, is_bitmap ? mask_code : 0);
}
static json_t *b2j_enum16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_ENUMN(16, buf, lenbit, used, is_bitmap ? mask_code : 0);
}


static json_t *b2j_semi_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}
static json_t *b2j_single_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}
static json_t *b2j_double_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}
static json_t *b2j_octet_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}
static json_t *b2j_char_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}
static json_t *b2j_long_octet_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}
static json_t *b2j_long_char_prec(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return NULL;
}

static json_t *b2j_struct(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_array();
}
static json_t *b2j_array(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_array();
}

static json_t *b2j_bitmap8(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap16(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap24(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap32(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap40(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap48(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap56(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}
static json_t *b2j_bitmap64(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return json_object();
}

static json_t *b2j_set(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}
static json_t *b2j_bag(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}
static json_t *b2j_tod(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}
static json_t *b2j_date(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}
static json_t *b2j_utc(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}
static json_t *b2j_cluster_id(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(16, buf, lenbit, used);
}
static json_t *b2j_attr_id(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(16, buf, lenbit, used);
}
static json_t *b2j_bac_oid(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}
static json_t *b2j_ieee_addr(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	return B2J_DATAN(64, buf, lenbit, used);
}
static json_t *b2j_128_bit_sec_key(char *buf, int lenbit, int mask_code, int *used, int is_bitmap){
	*used = 0;
	return NULL;
}

//j2b////////////////////////////////////////////////////////////////////////////////////////
static int J2B_DATAN(int n, char *buf, json_t *jarg) {
	const char *sval = json_string_value(jarg);
	int ret = 0;
	if (sval != NULL) {
		int len = (n); 
		int i = 0;
		int limit = strlen(sval)/2;
		for (i = 0; i < len/8 && i < limit; i++) {
			hex_parse_byte(sval + i*2, (u8*)&buf[i]);
		}
		ret = i*8;
	}
	return ret;
} 

static int J2B_BOOLEAN(char *buf, json_t *jarg) { 
	const char *sval = json_string_value(jarg);
	int ret = 0;
	if (sval != NULL) {
		int bval = 0;
		sscanf(sval, "%d", &bval);
		buf[0] = !!bval;

		ret = 8;
	}
	return ret;
} 

static long long J2B_UNMASK(long long oldval,long long val, int mask) {
	int i = 0, j = 0;
	int len = sizeof(mask);
	long long newval = oldval;

	for (i = 0, j = 0; i <= len-1; i++)  {
		int bit = (mask >> i)&0x1;
		if (bit == 1) {
			newval |= ((val >> j)&0x1) << i;
			j++;
		}
	}

	return newval;
}

static int J2B_UINTN(int n, char *buf, int mask, json_t *jarg)  { 
	const char *sval = json_string_value(jarg);
	int ret = 0;
	if (sval != NULL) {
		long long lval = 0;
		sscanf(sval, "%llu", &lval);

		int i = 0;
		int len = (n); 
		if (mask != 0) {
			long long lval_old = 0;
			for (i = 0; i < len/8; i++) {
				lval_old = lval_old | ((buf)[i]&0xff) << (i*8);
			}
			lval = J2B_UNMASK(lval_old, lval, mask);
		}
		for (i = 0; i < len/8; i++) {
			buf[i] = (lval >> (i*8))&0xff;
		}

		ret = i*8;
	}
	return ret;
} 

static int J2B_INTN(int n, char *buf, int mask, json_t *jarg) {
	const char *sval = json_string_value(jarg);
	int ret = 0;
	if (sval != NULL) {
		long long lval = 0;
		sscanf(sval, "%lld", &lval);

		int i = 0;
		int len = (n); 
		if (mask != 0) {
			long long lval_old = 0;
			for (i = 0; i < len/8; i++) {
				lval_old = lval_old | ((buf)[i]&0xff) << (i*8);
			}
			lval = J2B_UNMASK(lval_old, lval, mask);
		}
		for (i = 0; i < len/8; i++) {
			buf[i] = (lval >> (i*8))&0xff;
		}

		ret = i*8;
	}
	return ret;
}

#define J2B_ENUMN J2B_UINTN

static int j2b_data8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(8, buf, jarg);
}
static int j2b_data16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(16, buf, jarg);
}
static int j2b_data24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(24, buf, jarg);
}
static int j2b_data32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(32, buf, jarg);
}
static int j2b_data40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(40, buf, jarg);
}
static int j2b_data48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(48, buf, jarg);
}
static int j2b_data56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(56, buf, jarg);
}
static int j2b_data64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(64, buf, jarg);
}

static int j2b_boolean(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_BOOLEAN(buf, jarg);
}

static int j2b_uint8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(8, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(16, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(24, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(32, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(40, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(48, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(56, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_uint64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_UINTN(64, buf, is_bitmap_type ? mask_code : 0, jarg);
}

static int j2b_int8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(8, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(16, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(24, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(32, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(40, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(48, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(56, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_int64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_INTN(64, buf, is_bitmap_type ? mask_code : 0, jarg);
}

static int j2b_enum8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_ENUMN(8, buf, is_bitmap_type ? mask_code : 0, jarg);
}
static int j2b_enum16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_ENUMN(16, buf, is_bitmap_type ? mask_code : 0, jarg);
}

static int j2b_semi_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_single_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_double_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_octet_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_char_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_long_octet_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_long_char_prec(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}

static int j2b_struct(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_array(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}

static int j2b_bitmap8(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap16(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap24(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap32(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap40(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap48(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap56(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bitmap64(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}

static int j2b_set(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_bag(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_tod(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_date(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_utc(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_cluster_id(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(16, buf, jarg);
}
static int j2b_attr_id(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(16, buf, jarg);
}
static int j2b_bac_oid(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
static int j2b_ieee_addr(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return J2B_DATAN(64, buf, jarg);
}
static int j2b_128_bit_sec_key(char *buf, json_t *jarg, int mask_code, int is_bitmap_type) {
	return 0;
}
#endif

