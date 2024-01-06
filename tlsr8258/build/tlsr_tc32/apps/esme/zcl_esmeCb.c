/********************************************************************************************************
 * @file    zcl_esmeCb.c
 *
 * @brief   This is the source file for zcl_esmeCb
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
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
#include "zb_api.h"
#include "zcl_include.h"
#include "esme.h"
#include "app_ui.h"
#if ZBHCI_EN
#include "zbhci.h"
#endif

/**********************************************************************
 * LOCAL CONSTANTS
 */



/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
#ifdef ZCL_READ
static void esme_zclReadRspCmd(zclIncoming_t *pInMsg);
#endif
#ifdef ZCL_WRITE
static void esme_zclWriteRspCmd(zclIncoming_t *pInMsg);
#endif
#ifdef ZCL_REPORT
static void esme_zclCfgReportCmd(zclCfgReportCmd_t *pCfgReportCmd);
static void esme_zclCfgReportRspCmd(zclIncoming_t *pInMsg);
static void esme_zclReportCmd(zclIncoming_t *pInMsg);
static void esme_zclCfgReadRspCmd(zclIncoming_t *pInMsg);
#endif
static void esme_zclDfltRspCmd(zclIncoming_t *pInMsg);

/**********************************************************************
 * GLOBAL VARIABLES
 */


/**********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZCL_IDENTIFY
static ev_timer_event_t *identifyTimerEvt = NULL;
static ev_timer_event_t *pricePublishPriceTimerEvt = NULL;
#endif


/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      esme_zclProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message.
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  None
 */
void esme_zclProcessIncomingMsg(zclIncoming_t *pInHdlrMsg)
{
//	printf("esme_zclProcessIncomingMsg\n");

	switch(pInHdlrMsg->hdr.cmd)
	{
#ifdef ZCL_READ
		case ZCL_CMD_READ_RSP:
			esme_zclReadRspCmd(pInHdlrMsg);
			break;
#endif
#ifdef ZCL_WRITE
		case ZCL_CMD_WRITE_RSP:
			esme_zclWriteRspCmd(pInHdlrMsg);
			break;
#endif
#ifdef ZCL_REPORT
		case ZCL_CMD_CONFIG_REPORT:
			esme_zclCfgReportCmd(pInHdlrMsg->attrCmd);
			break;
		case ZCL_CMD_CONFIG_REPORT_RSP:
			esme_zclCfgReportRspCmd(pInHdlrMsg);
			break;
		case ZCL_CMD_READ_REPORT_CFG_RSP:
			esme_zclCfgReadRspCmd(pInHdlrMsg);
			break;
		case ZCL_CMD_REPORT:
			esme_zclReportCmd(pInHdlrMsg);
			break;
#endif
		case ZCL_CMD_DEFAULT_RSP:
			esme_zclDfltRspCmd(pInHdlrMsg);
			break;
		default:
			break;
	}
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      esme_zclReadRspCmd
 *
 * @brief   Handler for ZCL Read Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void esme_zclReadRspCmd(zclIncoming_t *pInMsg)
{
#if ZBHCI_EN
	u8 array[64];
	memset(array, 0, 64);
	zclReadRspCmd_t *pReadRspCmd = pInMsg->attrCmd;

	u16 dataLen = 0;
	u8 *pBuf = array;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.src_short_addr);

	*pBuf++ = pInMsg->msg->indInfo.src_ep;
	*pBuf++ = pInMsg->msg->indInfo.dst_ep;

	*pBuf++ = pInMsg->hdr.seqNum;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.cluster_id);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.cluster_id);

	*pBuf++ = pReadRspCmd->numAttr;
	for(u8 i = 0; i < pReadRspCmd->numAttr; i++){
		*pBuf++ = HI_UINT16(pReadRspCmd->attrList[i].attrID);
		*pBuf++ = LO_UINT16(pReadRspCmd->attrList[i].attrID);
		*pBuf++ = pReadRspCmd->attrList[i].status;
		if(pReadRspCmd->attrList[i].status == ZCL_STA_SUCCESS){
			*pBuf++ = pReadRspCmd->attrList[i].dataType;
			dataLen = zcl_getAttrSize(pReadRspCmd->attrList[i].dataType, pReadRspCmd->attrList[i].data);
			memcpy(pBuf, pReadRspCmd->attrList[i].data, dataLen);
			if( (pReadRspCmd->attrList[i].dataType != ZCL_DATA_TYPE_LONG_CHAR_STR) && (pReadRspCmd->attrList[i].dataType != ZCL_DATA_TYPE_LONG_OCTET_STR) &&
				(pReadRspCmd->attrList[i].dataType != ZCL_DATA_TYPE_CHAR_STR) && (pReadRspCmd->attrList[i].dataType != ZCL_DATA_TYPE_OCTET_STR) &&
				(pReadRspCmd->attrList[i].dataType != ZCL_DATA_TYPE_STRUCT) ){
					ZB_LEBESWAP(pBuf, dataLen);
			}
			pBuf += dataLen;
		}
	}

   	zbhciTx(ZBHCI_CMD_ZCL_ATTR_READ_RSP, pBuf - array, array);
#endif
}
#endif	/* ZCL_READ */

#ifdef ZCL_WRITE
/*********************************************************************
 * @fn      esme_zclWriteRspCmd
 *
 * @brief   Handler for ZCL Write Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void esme_zclWriteRspCmd(zclIncoming_t *pInMsg)
{
#if ZBHCI_EN
	u8 array[64];
	memset(array, 0, 64);

	u8 *pBuf = array;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.src_short_addr);

	*pBuf++ = pInMsg->msg->indInfo.src_ep;
	*pBuf++ = pInMsg->msg->indInfo.dst_ep;

	*pBuf++ = pInMsg->hdr.seqNum;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.cluster_id);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.cluster_id);

	zclWriteRspCmd_t *pWriteRsp = (zclWriteRspCmd_t *)pInMsg->attrCmd;
	if(pInMsg->dataLen == 1){//the case of successful writing of all attributes
		*pBuf++ = pWriteRsp->attrList[0].status;
		*pBuf++ = 0xFF;
		*pBuf++ = 0xFF;
	}else{
		for(u8 i = 0; i < pWriteRsp->numAttr; i++){
			*pBuf++ = pWriteRsp->attrList[i].status;
			*pBuf++ = HI_UINT16(pWriteRsp->attrList[i].attrID);
			*pBuf++ = LO_UINT16(pWriteRsp->attrList[i].attrID);
		}
	}

	zbhciTx(ZBHCI_CMD_ZCL_ATTR_WRITE_RSP, pBuf - array, array);
#endif
}
#endif	/* ZCL_WRITE */

/*********************************************************************
 * @fn      esme_zclDfltRspCmd
 *
 * @brief   Handler for ZCL Default Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void esme_zclDfltRspCmd(zclIncoming_t *pInMsg)
{
//    printf("esme_zclDfltRspCmd\n");
#if 0
	u8 array[16];
	memset(array, 0, 16);

	u8 *pBuf = array;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.src_short_addr);

	*pBuf++ = pInMsg->msg->indInfo.src_ep;
	*pBuf++ = pInMsg->msg->indInfo.dst_ep;

	*pBuf++ = pInMsg->hdr.seqNum;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.cluster_id);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.cluster_id);

	zclDefaultRspCmd_t *pDefaultRsp = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
	*pBuf++ = pDefaultRsp->commandID;
	*pBuf++ = pDefaultRsp->statusCode;

	zbhciTx(ZBHCI_CMD_ZCL_DEFAULT_RSP, pBuf - array, array);
#endif
}

#ifdef ZCL_REPORT
/*********************************************************************
 * @fn      esme_zclCfgReportCmd
 *
 * @brief   Handler for ZCL Configure Report command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void esme_zclCfgReportCmd(zclCfgReportCmd_t *pCfgReportCmd)
{
//    printf("esme_zclCfgReportCmd\n");
}

/*********************************************************************
 * @fn      esme_zclCfgReportRspCmd
 *
 * @brief   Handler for ZCL Configure Report Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void esme_zclCfgReportRspCmd(zclIncoming_t *pInMsg)
{
//    printf("esme_zclCfgReportRspCmd\n");
#if ZBHCI_EN
	zclCfgReportRspCmd_t *pCfgReportRspCmd = (zclCfgReportRspCmd_t*)pInMsg->attrCmd;
	u8 array[64];
	memset(array, 0, 64);

	u8 *pBuf = array;
	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.src_short_addr);

	*pBuf++ = pInMsg->msg->indInfo.src_ep;
	*pBuf++ = pInMsg->msg->indInfo.dst_ep;

	*pBuf++ = pInMsg->hdr.seqNum;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.cluster_id);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.cluster_id);

	*pBuf++ = pCfgReportRspCmd->numAttr;

	for(u8 i = 0; i < pCfgReportRspCmd->numAttr; i++){
		*pBuf++ = pCfgReportRspCmd->attrList[i].status;
		*pBuf++ = pCfgReportRspCmd->attrList[i].direction;
		*pBuf++ = HI_UINT16(pCfgReportRspCmd->attrList[i].attrID);
		*pBuf++ = LO_UINT16(pCfgReportRspCmd->attrList[i].attrID);
	}

	zbhciTx(ZBHCI_CMD_ZCL_CONFIG_REPORT_RSP, pBuf - array, array);
#endif
}

/*********************************************************************
 * @fn      esme_zclReportCmd
 *
 * @brief   Handler for ZCL Report command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void esme_zclReportCmd(zclIncoming_t *pInMsg)
{
//    printf("esme_zclReportCmd\n");
#if ZBHCI_EN
	zclReportCmd_t *pReportCmd = (zclReportCmd_t *)pInMsg->attrCmd;
	u16 dataLen = 0;

	u8 bufLen = 8;//srcAddr + srcEp + dstEp + seqNum + clusterId + numAttr
	for(u8 i = 0; i < pReportCmd->numAttr; i++){
		bufLen += 3;//attrID + dataType

		dataLen = zcl_getAttrSize(pReportCmd->attrList[i].dataType, pReportCmd->attrList[i].attrData);
		bufLen += dataLen;
	}

	u8 *pBuf = ev_buf_allocate(bufLen);
	if(!pBuf){
		return;
	}

	memset(pBuf, 0, bufLen);

	u8 *pData = pBuf;
	*pData++ = HI_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pData++ = LO_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pData++ = pInMsg->msg->indInfo.src_ep;
	*pData++ = pInMsg->msg->indInfo.dst_ep;

	*pData++ = pInMsg->hdr.seqNum;

	*pData++ = HI_UINT16(pInMsg->msg->indInfo.cluster_id);
	*pData++ = LO_UINT16(pInMsg->msg->indInfo.cluster_id);

	*pData++ = pReportCmd->numAttr;
	for(u8 i = 0; i < pReportCmd->numAttr; i++){
		*pData++ = HI_UINT16(pReportCmd->attrList[i].attrID);
		*pData++ = LO_UINT16(pReportCmd->attrList[i].attrID);
		*pData++ = pReportCmd->attrList[i].dataType;
		dataLen = zcl_getAttrSize(pReportCmd->attrList[i].dataType, pReportCmd->attrList[i].attrData);
		memcpy(pData, pReportCmd->attrList[i].attrData, dataLen);
		if( (pReportCmd->attrList[i].dataType != ZCL_DATA_TYPE_LONG_CHAR_STR) && (pReportCmd->attrList[i].dataType != ZCL_DATA_TYPE_LONG_OCTET_STR) &&
			(pReportCmd->attrList[i].dataType != ZCL_DATA_TYPE_CHAR_STR) && (pReportCmd->attrList[i].dataType != ZCL_DATA_TYPE_OCTET_STR) &&
			(pReportCmd->attrList[i].dataType != ZCL_DATA_TYPE_STRUCT) ){
				ZB_LEBESWAP(pData, dataLen);
		}
		pData += dataLen;
	}

	zbhciTx(ZBHCI_CMD_ZCL_REPORT_MSG_RCV, pData - pBuf, pBuf);

	ev_buf_free(pBuf);
#endif
}

/*********************************************************************
 * @fn      esme_zclCfgReadRspCmd
 *
 * @brief   Handler for ZCL Report Configure Read command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */

static void esme_zclCfgReadRspCmd(zclIncoming_t *pInMsg)
{
#if ZBHCI_EN
	zclReadReportCfgRspCmd_t *pReadCfgRspCmd = (zclReadReportCfgRspCmd_t *)pInMsg->attrCmd;
	u8 array[64];
	memset(array, 0, 64);

	u16 dataLen = 0;
	u8 *pBuf = array;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.src_short_addr);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.src_short_addr);

	*pBuf++ = pInMsg->msg->indInfo.src_ep;
	*pBuf++ = pInMsg->msg->indInfo.dst_ep;

	*pBuf++ = pInMsg->hdr.seqNum;

	*pBuf++ = HI_UINT16(pInMsg->msg->indInfo.cluster_id);
	*pBuf++ = LO_UINT16(pInMsg->msg->indInfo.cluster_id);

	*pBuf++ = pReadCfgRspCmd->numAttr;

	for(u8 i = 0; i < pReadCfgRspCmd->numAttr; i++){
		*pBuf++ = pReadCfgRspCmd->attrList[i].status;
		*pBuf++ = pReadCfgRspCmd->attrList[i].direction;
		*pBuf++ = HI_UINT16(pReadCfgRspCmd->attrList[i].attrID);
		*pBuf++ = LO_UINT16(pReadCfgRspCmd->attrList[i].attrID);

		if(pReadCfgRspCmd->attrList[i].direction == ZCL_SEND_ATTR_REPORTS){
			*pBuf++ = pReadCfgRspCmd->attrList[i].dataType;
			*pBuf++ = HI_UINT16(pReadCfgRspCmd->attrList[i].minReportInt);
			*pBuf++ = LO_UINT16(pReadCfgRspCmd->attrList[i].minReportInt);
			*pBuf++ = HI_UINT16(pReadCfgRspCmd->attrList[i].maxReportInt);
			*pBuf++ = LO_UINT16(pReadCfgRspCmd->attrList[i].maxReportInt);

			if(zcl_analogDataType(pReadCfgRspCmd->attrList[i].dataType)){
				dataLen = zcl_getAttrSize(pReadCfgRspCmd->attrList[i].dataType, pReadCfgRspCmd->attrList[i].reportableChange);
				memcpy(pBuf, pReadCfgRspCmd->attrList[i].reportableChange, dataLen);
				pBuf += dataLen;
			}
		}else{
			*pBuf++ = HI_UINT16(pReadCfgRspCmd->attrList[i].timeoutPeriod);
			*pBuf++ = LO_UINT16(pReadCfgRspCmd->attrList[i].timeoutPeriod);
		}
	}

	zbhciTx(ZBHCI_CMD_ZCL_READ_REPORT_CFG_RSP, pBuf - array, array);
#endif
}
#endif	/* ZCL_REPORT */

#ifdef ZCL_BASIC
/*********************************************************************
 * @fn      esme_basicCb
 *
 * @brief   Handler for ZCL Basic Reset command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t esme_basicCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(cmdId == ZCL_CMD_BASIC_RESET_FAC_DEFAULT){
		//Reset all the attributes of all its clusters to factory defaults
		//zcl_nv_attr_reset();
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_BASIC */

#ifdef ZCL_IDENTIFY
s32 esme_zclIdentifyTimerCb(void *arg)
{
	if(g_zcl_identifyAttrs.identifyTime <= 0){
		identifyTimerEvt = NULL;
		return -1;
	}
	g_zcl_identifyAttrs.identifyTime--;
	return 0;
}

void esme_zclIdentifyTimerStop(void)
{
	if(identifyTimerEvt){
		TL_ZB_TIMER_CANCEL(&identifyTimerEvt);
	}
}

/*********************************************************************
 * @fn      esme_zclIdentifyCmdHandler
 *
 * @brief   Handler for ZCL Identify command. This function will set blink LED.
 *
 * @param   endpoint
 * @param   srcAddr
 * @param   identifyTime
 *
 * @return  None
 */
void esme_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime)
{
	g_zcl_identifyAttrs.identifyTime = identifyTime;

	if(identifyTime == 0){
		esme_zclIdentifyTimerStop();
		light_blink_stop();
	}else{
		if(!identifyTimerEvt){
			light_blink_start(identifyTime, 500, 500);
			identifyTimerEvt = TL_ZB_TIMER_SCHEDULE(esme_zclIdentifyTimerCb, NULL, 1000);
		}
	}
}

/*********************************************************************
 * @fn      esme_zcltriggerCmdHandler
 *
 * @brief   Handler for ZCL trigger command.
 *
 * @param   pTriggerEffect
 *
 * @return  None
 */
static void esme_zcltriggerCmdHandler(zcl_triggerEffect_t *pTriggerEffect)
{
	u8 effectId = pTriggerEffect->effectId;
//	u8 effectVariant = pTriggerEffect->effectVariant;

	switch(effectId){
		case IDENTIFY_EFFECT_BLINK:
			light_blink_start(1, 500, 500);
			break;
		case IDENTIFY_EFFECT_BREATHE:
			light_blink_start(15, 300, 700);
			break;
		case IDENTIFY_EFFECT_OKAY:
			light_blink_start(2, 250, 250);
			break;
		case IDENTIFY_EFFECT_CHANNEL_CHANGE:
			light_blink_start(1, 500, 7500);
			break;
		case IDENTIFY_EFFECT_FINISH_EFFECT:
			light_blink_start(1, 300, 700);
			break;
		case IDENTIFY_EFFECT_STOP_EFFECT:
			light_blink_stop();
			break;
		default:
			break;
	}
}

/*********************************************************************
 * @fn      esme_zclIdentifyQueryRspCmdHandler
 *
 * @brief   Handler for ZCL Identify command. This function will set blink LED.
 *
 * @param   srcAddr
 * @param   cmdPayload
 *
 * @return  None
 */
static void esme_zclIdentifyQueryRspCmdHandler(zclIncomingAddrInfo_t *pAddrInfo, zcl_identifyRspCmd_t *cmdPayload)
{
#if ZBHCI_EN
	u8 array[8];
	memset(array, 0, 8);

	u8 *pBuf = array;

	*pBuf++ = HI_UINT16(pAddrInfo->srcAddr);
	*pBuf++ = LO_UINT16(pAddrInfo->srcAddr);
	*pBuf++ = pAddrInfo->srcEp;
	*pBuf++ = pAddrInfo->dstEp;
	*pBuf++ = pAddrInfo->seqNum;

	*pBuf++ = HI_UINT16(cmdPayload->timeout);
	*pBuf++ = LO_UINT16(cmdPayload->timeout);

	zbhciTx(ZBHCI_CMD_ZCL_IDENTIFY_QUERY_RSP, pBuf - array, array);
#endif
}

/*********************************************************************
 * @fn      sampleLight_identifyCb
 *
 * @brief   Handler for ZCL Identify command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t esme_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == ESME_ENDPOINT){
		if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
			switch(cmdId){
				case ZCL_CMD_IDENTIFY:
					esme_zclIdentifyCmdHandler(pAddrInfo->dstEp, pAddrInfo->srcAddr, ((zcl_identifyCmd_t *)cmdPayload)->identifyTime);
					break;
				case ZCL_CMD_TRIGGER_EFFECT:
					esme_zcltriggerCmdHandler((zcl_triggerEffect_t *)cmdPayload);
					break;
				default:
					break;
			}
		}else{
			if(cmdId == ZCL_CMD_IDENTIFY_QUERY_RSP){
				esme_zclIdentifyQueryRspCmdHandler(pAddrInfo, (zcl_identifyRspCmd_t *)cmdPayload);
			}
		}
	}

	return ZCL_STA_SUCCESS;
}

#endif	/* ZCL_IDENTIFY */

#ifdef ZCL_PRICE

/*********************************************************************
 * @fn      cloudsmets_zclPublishPriceCmdHandler
 *
 * @brief   Handler for ZCL SE Price publish price command.
 *
 * @param   pAddreInfo
 * @param   pPublishPriceCmd
 *
 * @return  None
 */

s32 esme_zclPublishPriceTimerCb(void *arg)
{
    epInfo_t *dstEpInfo = (epInfo_t *)arg;

    zcl_price_publishPriceCmd(ESME_ENDPOINT, dstEpInfo, TRUE, &g_zcl_pricePublishPriceCmd);
	pricePublishPriceTimerEvt = NULL;

	// -1 stops the timer.
	return -1;
}

static void esme_zclGetCurrentPriceCmdHandler(zclIncomingAddrInfo_t *pAddrInfo, zcl_price_getCurrentPriceCmd_t *pPublishPriceCmd)
{
	/*
	 * We ignore the payload and just schedule a timer to send the current
	 * price information shortly.
	 *
	 * However as do need to route the request back to where it came from which
	 * means we need to know the address information.  We rely on there only
	 * being a single reuqest being processed at a time and use a static variable
	 * to hold this information.
	 */
	static epInfo_t dstEp;

	// We need to send the response
	if(!pricePublishPriceTimerEvt){
		/*
		 * Set the destination address, which will be the source address for this
		 * command.
		 */
	    dstEp.dstAddrMode = APS_SHORT_DSTADDR_WITHEP;
		dstEp.dstAddr.shortAddr = pAddrInfo->srcAddr;
		dstEp.dstEp = pAddrInfo->srcEp;
        dstEp.profileId = pAddrInfo->profileId;
		dstEp.txOptions |= APS_TX_OPT_ACK_TX;
		if(pAddrInfo->apsSec){
			dstEp.txOptions |= APS_TX_OPT_SECURITY_ENABLED;
		}
		pricePublishPriceTimerEvt = TL_ZB_TIMER_SCHEDULE(esme_zclPublishPriceTimerCb, &dstEp, 100);
	}
}

/*********************************************************************
 * @fn      esme_priceCb
 *
 * @brief   Handler for ZCL Smart Energy get current price command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t esme_priceCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == ESME_ENDPOINT){
		// Note that this is "server from client".
		if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
			switch(cmdId){
				case ZCL_CMD_GET_CURRENT_PRICE:
					esme_zclGetCurrentPriceCmdHandler(pAddrInfo, (zcl_price_getCurrentPriceCmd_t *)cmdPayload);
					break;
				default:
					break;
			}
		}
	}
	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_PRICE */

#endif  /* __PROJECT_ESME__ */

