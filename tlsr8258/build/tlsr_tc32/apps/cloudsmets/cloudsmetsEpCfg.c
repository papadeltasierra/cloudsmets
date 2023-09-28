/********************************************************************************************************
 * @file    cloudsmetsEpCfg.c
 *
 * @brief   This is the source file for cloudsmetsEpCfg
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
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

#if (__PROJECT_CLOUDSMETS__)

/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zcl_include.h"
#include "cloudsmets.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */
#if (__PROJECT_CLOUDSMETS__)
#ifndef ZCL_BASIC_MFG_NAME
#define ZCL_BASIC_MFG_NAME     {10,'C','L','O','U','D','S','M','E','T','S'}
#endif
#ifndef ZCL_BASIC_MODEL_ID
#define ZCL_BASIC_MODEL_ID	   {3,'1','0','0'}
#endif
#endif /* __PROJECT_CLOUDSMETS__ */


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * GLOBAL VARIABLES
 */
/**
 *  @brief Definition for Incoming cluster / Sever Cluster
 */
const u16 cloudsmets_inClusterList[] =
{
	ZCL_CLUSTER_GEN_BASIC,
	ZCL_CLUSTER_GEN_IDENTIFY,
};

/**
 *  @brief Definition for Outgoing cluster / Client Cluster
 */
const u16 cloudsmets_outClusterList[] =
{
	ZCL_CLUSTER_GEN_BASIC,
	ZCL_CLUSTER_GEN_TIME,
	ZCL_CLUSTER_SE_PRICE,
	ZCL_CLUSTER_SE_METERING
};

/**
 *  @brief Definition for Server cluster number and Client cluster number
 */
#define cloudsmets_IN_CLUSTER_NUM		(sizeof(cloudsmets_inClusterList)/sizeof(cloudsmets_inClusterList[0]))
#define cloudsmets_OUT_CLUSTER_NUM	(sizeof(cloudsmets_outClusterList)/sizeof(cloudsmets_outClusterList[0]))

/**
 *  @brief Definition for simple description for HA profile
 */
const af_simple_descriptor_t cloudsmets_simpleDesc =
{
	HA_PROFILE_ID,                      	/* Application profile identifier */
	HA_DEV_ONOFF_SWITCH,                	/* Application device identifier */
	CLOUDSMETS_ENDPOINT,                 /* Endpoint */
	2,                                  	/* Application device version */
	0,										/* Reserved */
	cloudsmets_IN_CLUSTER_NUM,           	/* Application input cluster count */
	cloudsmets_OUT_CLUSTER_NUM,          	/* Application output cluster count */
	(u16 *)cloudsmets_inClusterList,    	/* Application input cluster list */
	(u16 *)cloudsmets_outClusterList,   	/* Application output cluster list */
};


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

#define	ZCL_BASIC_ATTR_NUM		 sizeof(basic_attrTbl) / sizeof(zclAttrInfo_t)


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

#define	ZCL_IDENTIFY_ATTR_NUM			sizeof(identify_attrTbl) / sizeof(zclAttrInfo_t)

// !!PDS: Don't believe we require this.
#if 0
#ifdef ZCL_POLL_CTRL
/* Poll Control */
zcl_pollCtrlAttr_t g_zcl_pollCtrlAttrs =
{
	.chkInInterval			= 0x3840,
	.longPollInterval		= 0x14,
	.shortPollInterval		= 0x02,
	.fastPollTimeout		= 0x28,
	.chkInIntervalMin		= 0x00,
	.longPollIntervalMin	= 0x00,
	.fastPollTimeoutMax		= 0x00,
};

const zclAttrInfo_t pollCtrl_attrTbl[] =
{
	{ ZCL_ATTRID_CHK_IN_INTERVAL,  		ZCL_DATA_TYPE_UINT32, ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE, (u8*)&g_zcl_pollCtrlAttrs.chkInInterval },
	{ ZCL_ATTRID_LONG_POLL_INTERVAL, 	ZCL_DATA_TYPE_UINT32, ACCESS_CONTROL_READ, 						  (u8*)&g_zcl_pollCtrlAttrs.longPollInterval },
	{ ZCL_ATTRID_SHORT_POLL_INTERVAL, 	ZCL_DATA_TYPE_UINT16, ACCESS_CONTROL_READ, 						  (u8*)&g_zcl_pollCtrlAttrs.shortPollInterval },
	{ ZCL_ATTRID_FAST_POLL_TIMEOUT, 	ZCL_DATA_TYPE_UINT16, ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE, (u8*)&g_zcl_pollCtrlAttrs.fastPollTimeout },
	{ ZCL_ATTRID_CHK_IN_INTERVAL_MIN, 	ZCL_DATA_TYPE_UINT32, ACCESS_CONTROL_READ, 						  (u8*)&g_zcl_pollCtrlAttrs.chkInIntervalMin},
	{ ZCL_ATTRID_LONG_POLL_INTERVAL_MIN,ZCL_DATA_TYPE_UINT32, ACCESS_CONTROL_READ, 						  (u8*)&g_zcl_pollCtrlAttrs.longPollIntervalMin },
	{ ZCL_ATTRID_FAST_POLL_TIMEOUT_MAX, ZCL_DATA_TYPE_UINT16, ACCESS_CONTROL_READ, 						  (u8*)&g_zcl_pollCtrlAttrs.fastPollTimeoutMax},

	{ ZCL_ATTRID_GLOBAL_CLUSTER_REVISION, ZCL_DATA_TYPE_UINT16,  ACCESS_CONTROL_READ,  					  (u8*)&zcl_attr_global_clusterRevision},
};

#define	ZCL_POLLCTRL_ATTR_NUM			sizeof(pollCtrl_attrTbl) / sizeof(zclAttrInfo_t)
#endif
#endif

/* Basic */
zcl_timeAttr_t g_zcl_timeAttrs =
{
	.time		= 0,
	.localTime 	= 0,
};

const zclAttrInfo_t time_attrTbl[] =
{
	{ ZCL_ATTRID_TIME,      		ZCL_DATA_TYPE_UTC,    ACCESS_CONTROL_READ,	(u32*)&g_zcl_timeAttrs.time},
	{ ZCL_ATTRID_LOCAL_TIME,      	ZCL_DATA_TYPE_UTC,    ACCESS_CONTROL_READ,  (u32*)&g_zcl_timeAttrs.localTime}
};

#define ZCL_TIME_ATTR_NUM	  sizeof(time_attrTbl) / sizeof(zclAttrInfo_t)

/**
 *  @brief Definition for CloudSMETS specific cluster
 */
const zcl_specClusterInfo_t g_cloudsmetsClusterList[] =
{
	// !!PDS:
#if 1
	{ZCL_CLUSTER_GEN_TIME,			MANUFACTURER_CODE_NONE,	ZCL_TIME_ATTR_NUM, 		time_attrTbl,  		zcl_time_register,		cloudsmets_timeCb},
	{ZCL_CLUSTER_SE_METERING,		MANUFACTURER_CODE_NONE,	ZCL_BASIC_ATTR_NUM, 	basic_attrTbl,  	zcl_metering_register,	NULL},
#else
//!!PDS: Remove this and referenced code.
	{ZCL_CLUSTER_GEN_BASIC,			MANUFACTURER_CODE_NONE,	ZCL_BASIC_ATTR_NUM, 	basic_attrTbl,  	zcl_basic_register,		cloudsmets_basicCb},
	{ZCL_CLUSTER_GEN_IDENTIFY,		MANUFACTURER_CODE_NONE,	ZCL_IDENTIFY_ATTR_NUM,	identify_attrTbl,	zcl_identify_register,	cloudsmets_identifyCb},
#ifdef ZCL_GROUP
	{ZCL_CLUSTER_GEN_GROUPS,		MANUFACTURER_CODE_NONE,	0, 						NULL,  				zcl_group_register,		cloudsmets_groupCb},
#endif
#ifdef ZCL_SCENE
	{ZCL_CLUSTER_GEN_SCENES,		MANUFACTURER_CODE_NONE,	0,						NULL,				zcl_scene_register,		cloudsmets_sceneCb},
#endif
#ifdef ZCL_POLL_CTRL
	{ZCL_CLUSTER_GEN_POLL_CONTROL,	MANUFACTURER_CODE_NONE,	ZCL_POLLCTRL_ATTR_NUM,	pollCtrl_attrTbl, 	zcl_pollCtrl_register, 	cloudsmets_pollCtrlCb},
#endif
#endif
};

u8 CLOUDSMETS_CB_CLUSTER_NUM = (sizeof(g_cloudsmetsClusterList)/sizeof(g_cloudsmetsClusterList[0]));

/**********************************************************************
 * FUNCTIONS
 */

#endif	/* __PROJECT_CLOUDSMETS__ */
