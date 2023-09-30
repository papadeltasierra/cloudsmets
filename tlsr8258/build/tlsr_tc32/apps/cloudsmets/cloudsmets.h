/********************************************************************************************************
 * @file    cloudsmets.h
 *
 * @brief   This is the header file for cloudsmets
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

#ifndef _CLOUDSMETS_H_
#define _CLOUDSMETS_H_

/**********************************************************************
 * CONSTANT
 */
#define CLOUDSMETS_ENDPOINT  0x01

/**********************************************************************
 * TYPEDEFS
 */
typedef struct{
	u8 keyType; /* ERTIFICATION_KEY or MASTER_KEY key for touch-link or distribute network
	 	 	 	 SS_UNIQUE_LINK_KEY or SS_GLOBAL_LINK_KEY for distribute network */
	u8 key[16];	/* the key used */
}app_linkKey_info_t;

typedef struct{
	ev_timer_event_t *bdbFBTimerEvt;
	ev_timer_event_t *timerLedEvt;
    s32 Vbat;		//current voltage
	u32 keyPressedTime;

	u16 ledOnTime;
	u16 ledOffTime;
	u8 	oriSta;		//original state before blink
	u8 	sta;		//current state in blink
	u8 	times;		//blink times
	u8  state;

	u8  keyPressed;

	app_linkKey_info_t tcLinkKey;
}app_ctx_t;

/**
 *  @brief Defined for basic cluster attributes
 */
typedef struct{
	u8 	zclVersion;
	u8	appVersion;
	u8	stackVersion;
	u8	hwVersion;
	u8	manuName[ZCL_BASIC_MAX_LENGTH];
	u8	modelId[ZCL_BASIC_MAX_LENGTH];
	u8	powerSource;
	u8	deviceEnable;
}zcl_basicAttr_t;

/**
 *  @brief Defined for identify cluster attributes
 */
typedef struct{
	u16	identifyTime;
}zcl_identifyAttr_t;

/**********************************************************************
 * GLOBAL VARIABLES
 */
// !!PDS: not used.  extern app_ctx_t g_switchAppCtx;

extern bdb_appCb_t g_zbDemoBdbCb;

extern bdb_commissionSetting_t g_bdbCommissionSetting;

extern u8 CLOUDSMETS_CB_CLUSTER_NUM;
extern const zcl_specClusterInfo_t g_cloudsmetsClusterList[];
extern const af_simple_descriptor_t cloudsmets_simpleDesc;

/* Attributes */
extern zcl_basicAttr_t g_zcl_basicAttrs;
extern zcl_identifyAttr_t g_zcl_identifyAttrs;

#define zcl_pollCtrlAttrGet()	&g_zcl_pollCtrlAttrs
/**********************************************************************
 * FUNCTIONS
 */
status_t cloudsmets_basicCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t cloudsmets_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
void cloudsmets_zclProcessIncomingMsg(zclIncoming_t *pInHdlrMsg);

status_t cloudsmets_timeCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);

void cloudsmets_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf);
void cloudsmets_leaveIndHandler(nlme_leave_ind_t *pLeaveInd);
void cloudsmets_otaProcessMsgHandler(u8 evt, u8 status);

#endif /* _CLOUDSMETS_H_ */
