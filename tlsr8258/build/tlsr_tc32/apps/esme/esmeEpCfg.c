/********************************************************************************************************
 * @file    esmeEpCfg.c
 *
 * @brief   This is the source file for esmeEpCfg.  This is a ZigBee coordinator that
 *          emulates ESME, Electricity Smart Metering Equipment, as defined in the
 *          Smart Metering Implementation Programme Great Britain Companion
 *          Specification (GBCS), v0.8.1.
 *
 *          The ESME only provides sufficient support (minimal number of attributes
 *          of fixed values) to allow testing of a CAD (Customer Access Device) under
 *          development.  Support is defined in the SMETS documentation above.
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
#define ZCL_BASIC_MFG_NAME     {10,'C','l','o','u','d','S','M','E','T','S'}
#endif
#ifndef ZCL_BASIC_MODEL_ID
#define ZCL_BASIC_MODEL_ID	   {11,'E','S','M','E',' ','v','0','.','0','.','1'}
#endif


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * GLOBAL VARIABLES
 */
/**
 *  @brief Definition for Incoming cluster / Server Cluster
 *         This device is never intended to be a client.
 */
const u16 esme_inClusterList[] =
{
	ZCL_CLUSTER_GEN_BASIC,
	ZCL_CLUSTER_GEN_IDENTIFY
};

/**
 *  @brief Definition for Outgoing cluster / Client Cluster
 */
const u16 esme_outClusterList[] =
{
	ZCL_CLUSTER_GEN_BASIC,
	ZCL_CLUSTER_GEN_IDENTIFY,
#ifdef ZCL_TIME
	ZCL_CLUSTER_GEN_TIME,
#endif
#ifdef ZCL_PRICE
	ZCL_CLUSTER_SE_PRICE,
#endif
#ifdef ZCL_METERING
	ZCL_CLUSTER_SE_METERING,
#endif
#ifdef ZCL_PREPAYMENT
	ZCL_CLUSTER_SE_PREPAYMENT,
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
	SE_PROFILE_ID,                      	/* Smart Energy profile identifier */
	SE_DEV_METERING_DEVICE,    				/* Smart Energy device identifier */
	ESME_ENDPOINT,                 			/* Endpoint number */
	0,                                  	/* Application device version */
	0,										/* Reserved */
	SAMPLEGW_IN_CLUSTER_NUM,           		/* Application input cluster count */
	SAMPLEGW_OUT_CLUSTER_NUM,          		/* Application output cluster count */
	(u16 *)esme_inClusterList,    			/* Application input cluster list */
	(u16 *)esme_outClusterList,   			/* Application output cluster list */
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

#ifdef ZCL_TIME

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
	UTCTime	 time;
	map8 timeStatus;
	s32	 timeZone;
	u32	 dstStart;
	u32	 dstEnd;
	s32  dstShift;
	u32	 standardTime;
	u32	 localTime;
	utc	 lastSetTime;
	utc	 validUntilTime;
} zcl_timeAttr_t;

/* Time */
zcl_timeAttr_t g_zcl_timeAttrs =
{
	.time			= 749487676,		// 2023/10/01 15:05:16
	.timeStatus		= TIMESTATUS_SYNCHRONIZED,
	.timeZone		= 0,				// UK is GMT.
	.dstStart		= 733107600,		// 2023/03/26 01:00:00
	.dstEnd			= 1698541200,		// 2023/10/29 01:00:00
	.dstShift		= 3600,				// UK DST is +1 hour, 3600s
	.standardTime	= 751856400,		// 2023/10/01 15:05:16
	.localTime		= 749491516,		// 2023/10/01 16:05:16
	.lastSetTime	= 725846400,		// 2023/01/01 00:00:00
	.validUntilTime = 757382400			// 2024/01/01 00:00:00
};

const zclAttrInfo_t time_attrTbl[] =
{
	{ ZCL_ATTRID_TIME,      		ZCL_DATA_TYPE_UTC,    	ACCESS_CONTROL_READ,	(u8*)&g_zcl_timeAttrs.time},
	{ ZCL_ATTRID_TIME_STATUS,      	ZCL_DATA_TYPE_BITMAP8,  ACCESS_CONTROL_READ,    (u8*)&g_zcl_timeAttrs.timeStatus},
	{ ZCL_ATTRID_TIMEZONE,    		ZCL_DATA_TYPE_INT32,    ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.timeZone},
	{ ZCL_ATTRID_DST_START,       	ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.dstStart},
	{ ZCL_ATTRID_DST_END,       	ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.dstEnd},
	{ ZCL_ATTRID_DST_SHIFT,    		ZCL_DATA_TYPE_INT32,    ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.dstShift},
	{ ZCL_ATTRID_STANDARD_TIME,     ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.standardTime},
	{ ZCL_ATTRID_LOCAL_TIME,       	ZCL_DATA_TYPE_UINT32,   ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.localTime},
	{ ZCL_ATTRID_LAST_SET_TIME,     ZCL_DATA_TYPE_UTC,      ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.lastSetTime},
	{ ZCL_ATTRID_VALID_UNTIL_TIME,  ZCL_DATA_TYPE_UTC,      ACCESS_CONTROL_READ,  	(u8*)&g_zcl_timeAttrs.validUntilTime},
};

#define ZCL_TIME_ATTR_NUM	  sizeof(basic_attrTbl) / sizeof(zclAttrInfo_t)

#endif /* ZCL_TIME */

/*
 * For the following SE (Smart Energy) clusters, we just support a minial (typically
 * single) attribute just so that we can prove we can read them properly.  More support
 * might be added for future CloudSMETS testing needs.
 */

#ifdef ZCL_PRICE
/* SE Price (D.4) */

#define ISO_4217_CURRENCY_GDB 826
#define ESME_PRICE_RATE_LABEL 	{0x08,'P','r','e','m','i','u','m','!'}

// Note that ZigBee time is relative to 2000-01-01 00:00:00 and is not Linux time.
zcl_price_publishPriceCmd_t g_zcl_pricePublishPriceCmd =
{
	.providerId = 0xAABBCCDD,
    .rateLabel = ESME_PRICE_RATE_LABEL,
	.issuerEventId = 0xFFEEDDCC,
    .currentTime = 751724488,				// 2023-10-27 12:21:18
    .unitsOfMeasure = UOM_KWH_OR_KW,
    .currency = ISO_4217_CURRENCY_GDB,
    .priceTrailingDigitAndPriceTier = 0x21,	// Two decimals, Tier1PriceLabel
    .numPriceTiersAndRegisterTier = 0x21,	// Two tiers, CurerntTier1SummationDelivered attribute in use
    .startTime = 725846400,					// 2023-01-01 00:00:00
    .durationInMins = 0xFFFF,				// Until changed
    .price = 456,							// £4.56 per kWh!
    .priceRatio = 10,						// Price ratio 1.0 (units of 0.1)
    .generationPrice = 0xFFFFFFFF,			// Not used
    .generationpriceRatio = 0xFF,			// Not used
    .alternateCostDelivered = 0xFFFFFFFF,	// Not used
    .alternateCostUnit = 0xFF,				// Not used
    .alternateCostTrailingDigit = 0xFF,		// Not used
    .numBlockThresholds = 0xFF,				// Not used
    .priceControl = 0x00,					// Not used
    .numGenerationTiers = 0,				// Not used
    .generationTier = 0xFF,					// Not used
    .extendedNumPriceTiers = 0,				// Number of Price Tiers sub-field only
    .extendedPriceTiers = 0,				// Price-tier sub-field only
    .extendedRegisterTier = 0,				// Register Tier sub-field only
};

#define ZCL_SE_PRICE_TIER_LABEL_MAX_LENGTH		13

typedef struct {
	u8			tier1PriceLabel[ZCL_SE_PRICE_TIER_LABEL_MAX_LENGTH];
} zcl_priceTierLabelAttr_t;

#define ZCL_SE_PRICE_TIER_1_PRICE_LABEL		{ 11,'E','M','S','E',' ','T','i','e','r',' ','1'}

zcl_priceTierLabelAttr_t g_zcl_priceTierLabelAttrs =
{
	.tier1PriceLabel	= ZCL_SE_PRICE_TIER_1_PRICE_LABEL
};

const zclAttrInfo_t price_attrTbl[] =
{
	{ ZCL_ATTRID_TIER1_PRICE_LABEL,    		ZCL_DATA_TYPE_CHAR_STR, ACCESS_CONTROL_READ,  						(u8*)g_zcl_priceTierLabelAttrs.tier1PriceLabel}
};

#define ZCL_PRICE_ATTR_NUM	  sizeof(price_attrTbl) / sizeof(zclAttrInfo_t)

#endif /* ZCL_PRICE */


#ifdef ZCL_METERING
/* SE Metering */
typedef struct{
	u48  currentSummationDelivered;
}zcl_meteringAttr_t;


/* Metering */
zcl_meteringAttr_t g_zcl_meteringAttrs =
{
	.currentSummationDelivered 	= 0x0123456789ab,
};

const zclAttrInfo_t metering_attrTbl[] =
{
	{ ZCL_ATTRID_CURRENT_SUMMATION_DELIVERD,	ZCL_DATA_TYPE_UINT48,	ACCESS_CONTROL_READ,	(u8*)&g_zcl_meteringAttrs.currentSummationDelivered},
};

#define ZCL_METERING_ATTR_NUM	  sizeof(metering_attrTbl) / sizeof(zclAttrInfo_t)

#endif /* ZCL_METERING */

#ifdef ZCL_PREPAYMENT
/* Prepayment */
typedef struct{

	s32 creditRemaining;
	u8  debtLabel1[ZCL_DEBT_LABEL1_MAX_LENGTH];
	u16	currency;
} zcl_prepaymentAttr_t;

#define ZCL_SE_PREPAYMENT_DEBT_LABEL1	{11,'E','S','M','E',' ','D','e','b','t','1'}

zcl_prepaymentAttr_t g_zcl_prepayentAttrs =
{
	.creditRemaining = 0x87654321,
	.debtLabel1 = ZCL_SE_PREPAYMENT_DEBT_LABEL1,
	.currency = ISO_4217_CURRENCY_GDB
};

const zclAttrInfo_t prepayment_attrTbl[] =
{
	{ ZCL_ATTRID_CREDIT_REMAINING,	ZCL_DATA_TYPE_INT32,	ACCESS_CONTROL_READ,	(u8*)&g_zcl_prepayentAttrs.creditRemaining},
	{ ZCL_ATTRID_DEBT_LABEL1,		ZCL_DATA_TYPE_CHAR_STR,	ACCESS_CONTROL_READ,	(u8*)&g_zcl_prepayentAttrs.debtLabel1},
	{ ZCL_ATTRID_CURRENCY,			ZCL_DATA_TYPE_UINT16,	ACCESS_CONTROL_READ,	(u8*)&g_zcl_prepayentAttrs.currency},
};

#define ZCL_PREPAYMENT_ATTR_NUM	  sizeof(prepayment_attrTbl) / sizeof(zclAttrInfo_t)

#endif /* ZCL_PREPAYMENT */

/**
 *  @brief Definition for simple GW ZCL specific cluster
 */
const zcl_specClusterInfo_t g_esmeClusterList[] =
{
	{ZCL_CLUSTER_GEN_BASIC,						MANUFACTURER_CODE_NONE, ZCL_BASIC_ATTR_NUM, 	basic_attrTbl,  	zcl_basic_register,		esme_basicCb},
	{ZCL_CLUSTER_GEN_IDENTIFY,					MANUFACTURER_CODE_NONE, ZCL_IDENTIFY_ATTR_NUM,	identify_attrTbl,	zcl_identify_register,	esme_identifyCb},
#ifdef ZCL_TIME
	{ZCL_CLUSTER_GEN_TIME,						MANUFACTURER_CODE_NONE, ZCL_TIME_ATTR_NUM,		time_attrTbl,		zcl_time_register,		NULL},
#endif
#ifdef ZCL_PRICE
	{ZCL_CLUSTER_SE_PRICE,						MANUFACTURER_CODE_NONE, ZCL_PRICE_ATTR_NUM, 	price_attrTbl,  	zcl_price_register,		esme_priceCb},
#endif
#ifdef ZCL_METERING
	{ZCL_CLUSTER_SE_METERING,					MANUFACTURER_CODE_NONE, ZCL_METERING_ATTR_NUM,	metering_attrTbl,	zcl_metering_register,	NULL},
#endif
#ifdef ZCL_PREPAYMENT
	{ZCL_CLUSTER_SE_PREPAYMENT,					MANUFACTURER_CODE_NONE, ZCL_PREPAYMENT_ATTR_NUM,	prepayment_attrTbl,	zcl_prepayment_register,	NULL},
#endif
};

u8 ESME_CB_CLUSTER_NUM = (sizeof(g_esmeClusterList)/sizeof(g_esmeClusterList[0]));


/**********************************************************************
 * FUNCTIONS
 */




#endif	/* __PROJECT_ESME__ */
