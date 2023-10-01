/********************************************************************************************************
 * @file    esmeEpCfg.c
 *
 * @brief   This is the source file for esmeEpCfg
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.

 *          Portions Copyright (c) 2023, Paul D.Smith (pau@pauldsmith.org.uk)
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#if (__PROJECT_ESME__)

/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zcl_include.h"
#include "esme.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */
#ifndef ZCL_BASIC_MFG_NAME
#define ZCL_BASIC_MFG_NAME     {10,'C','L','O','U','D','S','M','E','T','S'}
#endif
#ifndef ZCL_BASIC_MODEL_ID
#define ZCL_BASIC_MODEL_ID	   {9,'E','S','M','E','0','.','0','.','1'}
#endif


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * GLOBAL VARIABLES
 */
/**
 *  @brief Definition for Incoming cluster / Sever Cluster
 */
const u16 esme_inClusterList[] =
{
	ZCL_CLUSTER_GEN_BASIC,
	ZCL_CLUSTER_GEN_IDENTIFY,
#ifdef ZCL_OTA
    ZCL_CLUSTER_OTA,
#endif
};


/**
 *  @brief Definition for Outgoing cluster / Client Cluster
 */
const u16 esme_outClusterList[] =
{
	/******************************************************************
	 * ESME only supports time and smart energy clusters.
	 */
#ifdef ZCL_TIME
	ZCL_CLUSTER_GEN_TIME,
#endif
#ifdef ZCL_PRICE
	ZCL_CLUSTER_SE_PRICE
#endif
#ifdef ZCL_METERING
	ZCL_CLUSTER_SE_METERING
#endif
#ifdef ZCL_GROUP
	ZCL_CLUSTER_GEN_GROUPS,
#endif
#ifdef ZCL_SCENE
	ZCL_CLUSTER_GEN_SCENES,
#endif
#ifdef ZCL_ON_OFF
	ZCL_CLUSTER_GEN_ON_OFF,
#endif
#ifdef ZCL_LEVEL_CTRL
	ZCL_CLUSTER_GEN_LEVEL_CONTROL,
#endif
#ifdef ZCL_LIGHT_COLOR_CONTROL
	ZCL_CLUSTER_LIGHTING_COLOR_CONTROL,
#endif
#ifdef ZCL_DOOR_LOCK
	ZCL_CLUSTER_CLOSURES_DOOR_LOCK,
#endif
#ifdef ZCL_TEMPERATURE_MEASUREMENT
	ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT,
#endif
#ifdef ZCL_OCCUPANCY_SENSING
	ZCL_CLUSTER_MS_OCCUPANCY_SENSING,
#endif
#ifdef ZCL_IAS_ZONE
	ZCL_CLUSTER_SS_IAS_ZONE,
#endif
#ifdef ZCL_POLL_CTRL
	ZCL_CLUSTER_GEN_POLL_CONTROL,
#endif
};

/**
 *  @brief Definition for Server cluster number and Client cluster number
 */
#define SAMPLEGW_IN_CLUSTER_NUM		(sizeof(esme_inClusterList)/sizeof(esme_inClusterList[0]))
#define SAMPLEGW_OUT_CLUSTER_NUM	(sizeof(esme_outClusterList)/sizeof(esme_outClusterList[0]))

/**
 *  @brief Definition for simple description for HA profile
 */
const af_simple_descriptor_t esme_simpleDesc =
{
	HA_PROFILE_ID,                      /* Application profile identifier */
	HA_DEV_HOME_GATEWAY,                /* Application device identifier */
	ESME_ENDPOINT,                 /* Endpoint */
	0,                                  /* Application device version */
	0,									/* Reserved */
	SAMPLEGW_IN_CLUSTER_NUM,           	/* Application input cluster count */
	SAMPLEGW_OUT_CLUSTER_NUM,          	/* Application output cluster count */
	(u16 *)esme_inClusterList,    	/* Application input cluster list */
	(u16 *)esme_outClusterList,   	/* Application output cluster list */
};


#if AF_TEST_ENABLE
/**
 *  @brief Definition for Incoming cluster / Sever Cluster
 */
const u16 sampleTest_inClusterList[] =
{
	ZCL_CLUSTER_TELINK_SDK_TEST_REQ,
	ZCL_CLUSTER_TELINK_SDK_TEST_RSP,
	ZCL_CLUSTER_TELINK_SDK_TEST_CLEAR_REQ,
	ZCL_CLUSTER_TELINK_SDK_TEST_CLEAR_RSP,
};


/**
 *  @brief Definition for Outgoing cluster / Client Cluster
 */
const u16 sampleTest_outClusterList[] =
{
	ZCL_CLUSTER_TELINK_SDK_TEST_REQ,
	ZCL_CLUSTER_TELINK_SDK_TEST_RSP,
	ZCL_CLUSTER_TELINK_SDK_TEST_CLEAR_REQ,
	ZCL_CLUSTER_TELINK_SDK_TEST_CLEAR_RSP,
};

/**
 *  @brief Definition for Server cluster number and Client cluster number
 */
#define SAMPLE_TEST_IN_CLUSTER_NUM		(sizeof(sampleTest_inClusterList)/sizeof(sampleTest_inClusterList[0]))
#define SAMPLE_TEST_OUT_CLUSTER_NUM		(sizeof(sampleTest_outClusterList)/sizeof(sampleTest_outClusterList[0]))

/**
 *  @brief Definition for simple description for HA profile
 */
const af_simple_descriptor_t sampleTestDesc =
{
	HA_PROFILE_ID,                      /* Application profile identifier */
	HA_DEV_HOME_GATEWAY,                /* Application device identifier */
	SAMPLE_TEST_ENDPOINT,               /* Endpoint */
	0,                                  /* Application device version */
	0,									/* Reserved */
	SAMPLE_TEST_IN_CLUSTER_NUM,         /* Application input cluster count */
	SAMPLE_TEST_OUT_CLUSTER_NUM,        /* Application output cluster count */
	(u16 *)sampleTest_inClusterList,    /* Application input cluster list */
	(u16 *)sampleTest_outClusterList,   /* Application output cluster list */
};
#endif	/* AF_TEST_ENABLE */


/* Basic */
zcl_basicAttr_t g_zcl_basicAttrs =
{
	.zclVersion 	= 0x03,
	.appVersion 	= 0x00,
	.stackVersion 	= 0x02,
	.hwVersion		= 0x00,
	.manuName		= ZCL_BASIC_MFG_NAME,
	.modelId		= ZCL_BASIC_MODEL_ID,
	.powerSource	= POWER_SOURCE_MAINS_1_PHASE,
	.deviceEnable	= TRUE,
};

const zclAttrInfo_t basic_attrTbl[] =
{
	{ ZCL_ATTRID_BASIC_ZCL_VER,      		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.zclVersion},
	{ ZCL_ATTRID_BASIC_APP_VER,      		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.appVersion},
	{ ZCL_ATTRID_BASIC_STACK_VER,    		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.stackVersion},
	{ ZCL_ATTRID_BASIC_HW_VER,       		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.hwVersion},
	{ ZCL_ATTRID_BASIC_MFR_NAME,     		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_basicAttrs.manuName},
	{ ZCL_ATTRID_BASIC_MODEL_ID,     		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_basicAttrs.modelId},
	{ ZCL_ATTRID_BASIC_POWER_SOURCE, 		ZCL_DATA_TYPE_ENUM8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.powerSource},
	{ ZCL_ATTRID_BASIC_DEV_ENABLED,  		ZCL_DATA_TYPE_BOOLEAN,  ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE, (u8*)&g_zcl_basicAttrs.deviceEnable},

	{ ZCL_ATTRID_GLOBAL_CLUSTER_REVISION, 	ZCL_DATA_TYPE_UINT16,  	ACCESS_CONTROL_READ,  						(u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_BASIC_ATTR_NUM	  sizeof(basic_attrTbl) / sizeof(zclAttrInfo_t)


/* Identify */
zcl_identifyAttr_t g_zcl_identifyAttrs =
{
	.identifyTime	= 0x0000,
};

const zclAttrInfo_t identify_attrTbl[] =
{
	{ ZCL_ATTRID_IDENTIFY_TIME,  			ZCL_DATA_TYPE_UINT16,   ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE, (u8*)&g_zcl_identifyAttrs.identifyTime },

	{ ZCL_ATTRID_GLOBAL_CLUSTER_REVISION, 	ZCL_DATA_TYPE_UINT16,  	ACCESS_CONTROL_READ,  						(u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_IDENTIFY_ATTR_NUM	 sizeof(identify_attrTbl) / sizeof(zclAttrInfo_t)

/**********************************************************************
 * For some reason the Telink library does not define this structure.
 */

typedef u32 utc;
typedef u8 map8;

#define TIMESTATUS_MASTER			1
#define TIMESTATUS_SYNCHRONIZED		1
#define TIMESTATUS_MASTERZONEDST	1
#define TIMESTATUS_SUPERSEDING		1

typedef struct{
	utc	 time;
	map8 timeStatus;
	i32	 timeZone;
	u32	 dstStart;
	u32	 dstEnd;
	i32  dstShift;
	u32	 standardTime;
	u32	 localTime;
	utc	 lastSetTime;
	utc	 validUntilTime;
}zcl_timeAttr_t;

/* Time */
zcl_timeAttr_t g_zcl_timeAttrs =
{
	.time			= 1696172476,		// 2023/10/01 15:05:16
	.timeStatus		= TIMESTATUS_SYNCHRONIZED,
	.timeZone		= 0,				// UK is GMT.
	.dstStart		= 1679792400,		// 2023/03/26 01:00:00
	.dstEnd			= 1698541200,		// 2023/10/29 01:00:00
	.dstShift		= 3600,				// UK DST is +1 hour, 3600s
	.standardTime	= 1696172476,		// 2023/10/01 15:05:16
	.localTime		= 1696176316,		// 2023/10/01 16:05:16
	.lastSetTime	= 1672531200,		// 2023/01/01 00:00:00
	.validUntilTime = 1704067200		// 2024/01/01 00:00:00
};

const zclAttrInfo_t time_attrTbl[] =
{
	{ ZCL_ATTRID_TIME,      		ZCL_DATA_TYPE_UTC,    	ACCESS_CONTROL_READ,	(utc*)&g_zcl_timeAttrs.time},
	{ ZCL_ATTRID_TIME_STATUS,      	ZCL_DATA_TYPE_BITMAP,   ACCESS_CONTROL_READ,    (map8*)&g_zcl_timeAttrs.timeStatus},
	{ ZCL_ATTRID_TIMEZONE,    		ZCL_DATA_TYPE_INT32,    ACCESS_CONTROL_READ,  	(i32*)&g_zcl_timeAttrs.timeZone},
	{ ZCL_ATTRID_DST_START,       	ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u32*)&g_zcl_timeAttrs.dstStart},
	{ ZCL_ATTRID_DST_END,       	ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u32*)&g_zcl_timeAttrs.dstEnd},
	{ ZCL_ATTRID_DST_SHIFT,    		ZCL_DATA_TYPE_INT32,    ACCESS_CONTROL_READ,  	(i32*)&g_zcl_timeAttrs.dstShift},
	{ ZCL_ATTRID_STANDARD_TIME,     ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u32*)&g_zcl_timeAttrs.standardTime},
	{ ZCL_ATTRID_LOCAL_TIME,       	ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u32*)&g_zcl_timeAttrs.localTime},
	{ ZCL_ATTRID_LAST_SET_TIME,     ZCL_DATA_TYPE_UTC,      ACCESS_CONTROL_READ,  	(utc*)&g_zcl_timeAttrs.lastSetTime},
	{ ZCL_ATTRID_VALID_UNTIL_TIME,  ZCL_DATA_TYPE_UTC,      ACCESS_CONTROL_READ,  	(utc*)&g_zcl_timeAttrs.validUntilTime},
};

#define ZCL_TIME_ATTR_NUM	  sizeof(basic_attrTbl) / sizeof(zclAttrInfo_t)

#if 0 // !!PD: No support for price yet.
/* Price */
zcl_priceAttr_t g_zcl_priceAttrs =
{
	.zclVersion 	= 0x03,
	.appVersion 	= 0x00,
	.stackVersion 	= 0x02,
	.hwVersion		= 0x00,
	.manuName		= ZCL_BASIC_MFG_NAME,
	.modelId		= ZCL_BASIC_MODEL_ID,
	.powerSource	= POWER_SOURCE_MAINS_1_PHASE,
	.deviceEnable	= TRUE,
};

const zclAttrInfo_t price_attrTbl[] =
{
	{ ZCL_ATTRID_BASIC_ZCL_VER,      		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.zclVersion},
	{ ZCL_ATTRID_BASIC_APP_VER,      		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.appVersion},
	{ ZCL_ATTRID_BASIC_STACK_VER,    		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.stackVersion},
	{ ZCL_ATTRID_BASIC_HW_VER,       		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.hwVersion},
	{ ZCL_ATTRID_BASIC_MFR_NAME,     		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_basicAttrs.manuName},
	{ ZCL_ATTRID_BASIC_MODEL_ID,     		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_basicAttrs.modelId},
	{ ZCL_ATTRID_BASIC_POWER_SOURCE, 		ZCL_DATA_TYPE_ENUM8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.powerSource},
	{ ZCL_ATTRID_BASIC_DEV_ENABLED,  		ZCL_DATA_TYPE_BOOLEAN,  ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE, (u8*)&g_zcl_basicAttrs.deviceEnable},

	{ ZCL_ATTRID_GLOBAL_CLUSTER_REVISION, 	ZCL_DATA_TYPE_UINT16,  	ACCESS_CONTROL_READ,  						(u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_PRICE_ATTR_NUM	  sizeof(basic_attrTbl) / sizeof(zclAttrInfo_t)
#endif

/**********************************************************************
 * Short term we are only supporting a single attribute.
 */
/* Metering */
zcl_meteringAttr_t g_zcl_meteringAttrs =
{
	.zclVersion 	= 0x03,
	.appVersion 	= 0x00,
	.stackVersion 	= 0x02,
	.hwVersion		= 0x00,
	.manuName		= ZCL_BASIC_MFG_NAME,
	.modelId		= ZCL_BASIC_MODEL_ID,
	.powerSource	= POWER_SOURCE_MAINS_1_PHASE,
	.deviceEnable	= TRUE,
};

const zclAttrInfo_t metering_attrTbl[] =
{
	{ ZCL_ATTRID_BASIC_ZCL_VER,      		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.zclVersion},
	{ ZCL_ATTRID_BASIC_APP_VER,      		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.appVersion},
	{ ZCL_ATTRID_BASIC_STACK_VER,    		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.stackVersion},
	{ ZCL_ATTRID_BASIC_HW_VER,       		ZCL_DATA_TYPE_UINT8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.hwVersion},
	{ ZCL_ATTRID_BASIC_MFR_NAME,     		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_basicAttrs.manuName},
	{ ZCL_ATTRID_BASIC_MODEL_ID,     		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_basicAttrs.modelId},
	{ ZCL_ATTRID_BASIC_POWER_SOURCE, 		ZCL_DATA_TYPE_ENUM8,    ACCESS_CONTROL_READ,  						(u8*)&g_zcl_basicAttrs.powerSource},
	{ ZCL_ATTRID_BASIC_DEV_ENABLED,  		ZCL_DATA_TYPE_BOOLEAN,  ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE, (u8*)&g_zcl_basicAttrs.deviceEnable},

	{ ZCL_ATTRID_GLOBAL_CLUSTER_REVISION, 	ZCL_DATA_TYPE_UINT16,  	ACCESS_CONTROL_READ,  						(u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_METERING_ATTR_NUM	  sizeof(metering_attrTbl) / sizeof(zclAttrInfo_t)


/**
 *  @brief Definition for simple GW ZCL specific cluster
 */
const zcl_specClusterInfo_t g_esmeClusterList[] =
{
	{ZCL_CLUSTER_GEN_BASIC,						MANUFACTURER_CODE_NONE, ZCL_BASIC_ATTR_NUM, 	basic_attrTbl,  	zcl_basic_register,		esme_basicCb},
	{ZCL_CLUSTER_GEN_IDENTIFY,					MANUFACTURER_CODE_NONE, ZCL_IDENTIFY_ATTR_NUM,	identify_attrTbl,	zcl_identify_register,	esme_identifyCb},
#ifdef ZCL_TIME
	{ZCL_CLUSTER_SE_TIME,						MANUFACTURER_CODE_NONE, 0, 						time_attrTbl,		zcl_time_register,		NULL},
#endif
#ifdef ZCL_SE_PRICE
	{ZCL_CLUSTER_SE_PRICE,						MANUFACTURER_CODE_NONE, 0, 						price_attrTbl,  	zcl_price_register,		NULL},
#endif
#ifdef ZCL_SE_METERING
	{ZCL_CLUSTER_SE_METERING,					MANUFACTURER_CODE_NONE, 0, 						metering_attrTbl,	zcl_metering_register,	NULL},
#endif
#ifdef ZCL_GROUP
	{ZCL_CLUSTER_GEN_GROUPS,					MANUFACTURER_CODE_NONE, 0, 						NULL,  				zcl_group_register,		esme_groupCb},
#endif
#ifdef ZCL_SCENE
	{ZCL_CLUSTER_GEN_SCENES,					MANUFACTURER_CODE_NONE, 0,						NULL,				zcl_scene_register,		esme_sceneCb},
#endif
#ifdef ZCL_DOOR_LOCK
	{ZCL_CLUSTER_CLOSURES_DOOR_LOCK,			MANUFACTURER_CODE_NONE, 0, 						NULL, 				zcl_doorLock_register, 	&esme_doorLockCb},
#endif
#ifdef ZCL_TEMPERATURE_MEASUREMENT
	{ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT,	MANUFACTURER_CODE_NONE, 0, 						NULL, 				zcl_temperature_measurement_register, 	NULL},
#endif
#ifdef ZCL_OCCUPANCY_SENSING
	{ZCL_CLUSTER_MS_OCCUPANCY_SENSING,			MANUFACTURER_CODE_NONE, 0, 						NULL, 				zcl_occupancySensing_register, 	NULL},
#endif
#ifdef ZCL_IAS_ZONE
	{ZCL_CLUSTER_SS_IAS_ZONE,					MANUFACTURER_CODE_NONE, 0, 						NULL, 				zcl_iasZone_register, 	&esme_iasZoneCb},
#endif
#ifdef ZCL_POLL_CTRL
	{ZCL_CLUSTER_GEN_POLL_CONTROL,				MANUFACTURER_CODE_NONE, 0, 						NULL, 				zcl_pollCtrl_register, 	&esme_pollCtrlCb},
#endif
};

u8 ESME_CB_CLUSTER_NUM = (sizeof(g_esmeClusterList)/sizeof(g_esmeClusterList[0]));


/**********************************************************************
 * FUNCTIONS
 */




#endif	/* __PROJECT_ESME__ */
