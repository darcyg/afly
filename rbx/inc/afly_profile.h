#ifndef _ALINK_PROFILE_H_
#define _ALINK_PROFILE_H_

#if  0

#include "alink_export_zigbee.h"
#include "alink_export_gateway.h"


void alink_cmd_profile_add(cmd_profile_t *profile);
void alink_attr_profile_add(attr_profile_t *profile);

cmd_profile_t * alink_cmd_profile_search_cmd(short clusterid, char cmdid);
attr_profile_t * alink_attr_profile_search_attr(short clusterid, short attrid);

const char * alink_cmd_profile_buf2json(cmd_profile_t *cp, char *buf, int len);
const char * alink_attr_profile_buf2json(attr_profile_t *ap, char *buf, int len);

int alink_cmd_profile_json2buf(cmd_profile_t *cp, const char *profile, char *buf);
int alink_attr_profile_json2buf(attr_profile_t *ap, const char *profile, char *buf);

int alink_cmd_profile_search_by_cmdname(const char *cmd_name, cmd_profile_t *cps[], int cnt);
int alink_attr_profile_search_by_attrname(const char *attr_name, attr_profile_t *aps[], int cnt);

typedef struct stAlinkDataType {
	const char *name;
	int id;
	int len;				/* bit */
	void *buf2json;
	void *json2buf;
}stAlinkDataType_t;


enum {
	ZCL_DATATYPE_NODATA	= 0x00,

	ZCL_DATATYPE_DATA8	= 0x08,
	ZCL_DATATYPE_DATA16	= 0x09,
	ZCL_DATATYPE_DATA24	= 0x0a,
	ZCL_DATATYPE_DATA32	= 0x0b,
	ZCL_DATATYPE_DATA40	= 0x0c,
	ZCL_DATATYPE_DATA48	= 0x0d,
	ZCL_DATATYPE_DATA56	= 0x0e,
	ZCL_DATATYPE_DATA64	= 0x0f,

	ZCL_DATATYPE_BOOLEAN		= 0x10,

	ZCL_DATATYPE_UINT8		= 0x20,
	ZCL_DATATYPE_UINT16	= 0x21,
	ZCL_DATATYPE_UINT24	= 0x22,
	ZCL_DATATYPE_UINT32	= 0x23,
	ZCL_DATATYPE_UINT40	= 0x24,
	ZCL_DATATYPE_UINT48	= 0x25,
	ZCL_DATATYPE_UINT56	= 0x26,
	ZCL_DATATYPE_UINT64	= 0x27,

	ZCL_DATATYPE_INT8		= 0x28,
	ZCL_DATATYPE_INT16		= 0x29,
	ZCL_DATATYPE_INT24		= 0x2a,
	ZCL_DATATYPE_INT32		= 0x2b,
	ZCL_DATATYPE_INT40		= 0x2c,
	ZCL_DATATYPE_INT48		= 0x2d,
	ZCL_DATATYPE_INT56		= 0x2e,
	ZCL_DATATYPE_INT64		= 0x2f,

	ZCL_DATATYPE_ENUM8		= 0x30,
	ZCL_DATATYPE_ENUM16	= 0X31,



	ZCL_DATATYPE_SEMI_PREC		= 0x38,
	ZCL_DATATYPE_SINGLE_PREC	= 0x39,
	ZCL_DATATYPE_DOUBLE_PREC	= 0x3a,

	ZCL_DATATYPE_OCTET_STR		= 0x41,
	ZCL_DATATYPE_CHAR_STR		= 0x42,
	ZCL_DATATYPE_LONG_OCTET_STR	= 0x43,
	ZCL_DATATYPE_LONG_CHAR_STR	= 0x44,

	ZCL_DATATYPE_STRUCT		= 0x4c,
	ZCL_DATATYPE_ARRAY			= 0x48,

	ZCL_DATATYPE_BITMAP8		= 0X18,
	ZCL_DATATYPE_BITMAP16		= 0x19,
	ZCL_DATATYPE_BITMAP24		= 0x1a,
	ZCL_DATATYPE_BITMAP32		= 0x1b,
	ZCL_DATATYPE_BITMAP40		= 0x1c,
	ZCL_DATATYPE_BITMAP48		= 0x1d,
	ZCL_DATATYPE_BITMAP56		= 0x1e,
	ZCL_DATATYPE_BITMAP64		= 0x1f,

	ZCL_DATATYPE_SET			= 0x50,
	ZCL_DATATYPE_BAG			= 0x51,
	ZCL_DATATYPE_TOD				= 0xe0,
	ZCL_DATATYPE_DATE			= 0xe1,
	ZCL_DATATYPE_UTC				= 0xe2,
	ZCL_DATATYPE_CLUSTER_ID	= 0xe8,
	ZCL_DATATYPE_ATTR_ID	= 0xe9,
	ZCL_DATATYPE_BAC_OID		= 0xea,
	ZCL_DATATYPE_IEEE_ADDR			= 0xf0,
	ZCL_DATATYPE_128_BIT_SEC_KEY		= 0xf1,

	/*ZCL_DATATYPE_OPAUE			= 0x, */
	ZCL_DATATYPE_UNK				= 0xff,

};
#endif


#endif
