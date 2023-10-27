/********************************************************************************************************
 * @file    zcl_cloudsmetsCb.c
 *
 * @brief   This is the source file for zcl_cloudsmetsCb
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
#include "zb_api.h"
#include "zcl_include.h"
#include "cloudsmets.h"
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
static void cloudsmets_zclReadRspCmd(zclIncoming_t *pInMsg);
#endif
#ifdef ZCL_WRITE
static void cloudsmets_zclWriteRspCmd(zclIncoming_t *pInMsg);
#endif
#ifdef ZCL_REPORT
static void cloudsmets_zclCfgReportCmd(zclCfgReportCmd_t *pCfgReportCmd);
static void cloudsmets_zclCfgReportRspCmd(zclIncoming_t *pInMsg);
static void cloudsmets_zclReportCmd(zclIncoming_t *pInMsg);
static void cloudsmets_zclCfgReadRspCmd(zclIncoming_t *pInMsg);
#endif
static void cloudsmets_zclDfltRspCmd(zclIncoming_t *pInMsg);

/**********************************************************************
 * GLOBAL VARIABLES
 */


/**********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZCL_IDENTIFY
static ev_timer_event_t *identifyTimerEvt = NULL;
#endif


/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      cloudsmets_zclProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message.
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  None
 */
void cloudsmets_zclProcessIncomingMsg(zclIncoming_t *pInHdlrMsg)
{
//	printf("cloudsmets_zclProcessIncomingMsg\n");

	switch(pInHdlrMsg->hdr.cmd)
	{
#ifdef ZCL_READ
		case ZCL_CMD_READ_RSP:
			cloudsmets_zclReadRspCmd(pInHdlrMsg);
			break;
#endif
#ifdef ZCL_WRITE
		case ZCL_CMD_WRITE_RSP:
			cloudsmets_zclWriteRspCmd(pInHdlrMsg);
			break;
#endif
#ifdef ZCL_REPORT
		case ZCL_CMD_CONFIG_REPORT:
			cloudsmets_zclCfgReportCmd(pInHdlrMsg->attrCmd);
			break;
		case ZCL_CMD_CONFIG_REPORT_RSP:
			cloudsmets_zclCfgReportRspCmd(pInHdlrMsg);
			break;
		case ZCL_CMD_READ_REPORT_CFG_RSP:
			cloudsmets_zclCfgReadRspCmd(pInHdlrMsg);
			break;
		case ZCL_CMD_REPORT:
			cloudsmets_zclReportCmd(pInHdlrMsg);
			break;
#endif
		case ZCL_CMD_DEFAULT_RSP:
			cloudsmets_zclDfltRspCmd(pInHdlrMsg);
			break;
		default:
			break;
	}
}

#ifdef ZCL_READ
/*********************************************************************
 * @fn      cloudsmets_zclReadRspCmd
 *
 * @brief   Handler for ZCL Read Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void cloudsmets_zclReadRspCmd(zclIncoming_t *pInMsg)
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
 * @fn      cloudsmets_zclWriteRspCmd
 *
 * @brief   Handler for ZCL Write Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void cloudsmets_zclWriteRspCmd(zclIncoming_t *pInMsg)
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
 * @fn      cloudsmets_zclDfltRspCmd
 *
 * @brief   Handler for ZCL Default Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void cloudsmets_zclDfltRspCmd(zclIncoming_t *pInMsg)
{
//    printf("cloudsmets_zclDfltRspCmd\n");
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
 * @fn      cloudsmets_zclCfgReportCmd
 *
 * @brief   Handler for ZCL Configure Report command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void cloudsmets_zclCfgReportCmd(zclCfgReportCmd_t *pCfgReportCmd)
{
//    printf("cloudsmets_zclCfgReportCmd\n");
}

/*********************************************************************
 * @fn      cloudsmets_zclCfgReportRspCmd
 *
 * @brief   Handler for ZCL Configure Report Response command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void cloudsmets_zclCfgReportRspCmd(zclIncoming_t *pInMsg)
{
//    printf("cloudsmets_zclCfgReportRspCmd\n");
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
 * @fn      cloudsmets_zclReportCmd
 *
 * @brief   Handler for ZCL Report command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */
static void cloudsmets_zclReportCmd(zclIncoming_t *pInMsg)
{
//    printf("cloudsmets_zclReportCmd\n");
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
 * @fn      cloudsmets_zclCfgReadRspCmd
 *
 * @brief   Handler for ZCL Report Configure Read command.
 *
 * @param   pInHdlrMsg - incoming message to process
 *
 * @return  None
 */

static void cloudsmets_zclCfgReadRspCmd(zclIncoming_t *pInMsg)
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
 * @fn      cloudsmets_basicCb
 *
 * @brief   Handler for ZCL Basic Reset command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t cloudsmets_basicCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(cmdId == ZCL_CMD_BASIC_RESET_FAC_DEFAULT){
		//Reset all the attributes of all its clusters to factory defaults
		//zcl_nv_attr_reset();
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_BASIC */

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
static void cloudsmets_zclPublishPriceCmdHandler(zclIncomingAddrInfo_t *pAddrInfo, zcl_price_publishPriceCmd_t *pPublishPriceCmd)
{
#if ZBHCI_EN
	u8 payload[56];
	memset(payload, 0, 56);
	u8 *pBuf = payload;

	*pBuf++ = U32_BYTE0(pPublishPriceCmd->providerId);
	*pBuf++ = U32_BYTE1(pPublishPriceCmd->providerId);
	*pBuf++ = U32_BYTE2(pPublishPriceCmd->providerId);
	*pBuf++ = U32_BYTE3(pPublishPriceCmd->providerId);
	memcpy(pBuf, pPublishPriceCmd->rateLabel, ZCL_RATE_LABEL_MAX_LENGTH);
	pBuf += ZCL_RATE_LABEL_MAX_LENGTH;
	*pBuf++ = U32_BYTE0(pPublishPriceCmd->issuerEventId);
	*pBuf++ = U32_BYTE1(pPublishPriceCmd->issuerEventId);
	*pBuf++ = U32_BYTE2(pPublishPriceCmd->issuerEventId);
	*pBuf++ = U32_BYTE3(pPublishPriceCmd->issuerEventId);
	*pBuf++ = U32_BYTE0(pPublishPriceCmd->currentTime);
	*pBuf++ = U32_BYTE1(pPublishPriceCmd->currentTime);
	*pBuf++ = U32_BYTE2(pPublishPriceCmd->currentTime);
	*pBuf++ = U32_BYTE3(pPublishPriceCmd->currentTime);
	*pBuf++ = pPublishPriceCmd->unitsOfMeasure;
	*pBuf++ = U16_BYTE0(pPublishPriceCmd->currency);
	*pBuf++ = U16_BYTE1(pPublishPriceCmd->currency);
	*pBuf++ = pPublishPriceCmd->priceTrailingDigitAndPriceTier;
	*pBuf++ = pPublishPriceCmd->numPriceTiersAndRegisterTier;
	*pBuf++ = U32_BYTE0(pPublishPriceCmd->startTime);
	*pBuf++ = U32_BYTE1(pPublishPriceCmd->startTime);
	*pBuf++ = U32_BYTE2(pPublishPriceCmd->startTime);
	*pBuf++ = U32_BYTE3(pPublishPriceCmd->startTime);
	*pBuf++ = U16_BYTE0(pPublishPriceCmd->durationInMins);
	*pBuf++ = U16_BYTE1(pPublishPriceCmd->durationInMins);
	*pBuf++ = pPublishPriceCmd->price;
	*pBuf++ = pPublishPriceCmd->priceRatio;
	*pBuf++ = U32_BYTE0(pPublishPriceCmd->generationPrice);
	*pBuf++ = U32_BYTE1(pPublishPriceCmd->generationPrice);
	*pBuf++ = U32_BYTE2(pPublishPriceCmd->generationPrice);
	*pBuf++ = U32_BYTE3(pPublishPriceCmd->generationPrice);
	*pBuf++ = pPublishPriceCmd->generationpriceRatio;
	*pBuf++ = U32_BYTE0(pPublishPriceCmd->alternateCostDelivered);
	*pBuf++ = U32_BYTE1(pPublishPriceCmd->alternateCostDelivered);
	*pBuf++ = U32_BYTE2(pPublishPriceCmd->alternateCostDelivered);
	*pBuf++ = U32_BYTE3(pPublishPriceCmd->alternateCostDelivered);
	*pBuf++ = pPublishPriceCmd->alternateCodeUnit;
	*pBuf++ = pPublishPriceCmd->alternateCostTrailingDigit;
	*pBuf++ = pPublishPriceCmd->numBlockThresholds;
	*pBuf++ = pPublishPriceCmd->priceControl;
	*pBuf++ = pPublishPriceCmd->numGenerationTiers;
	*pBuf++ = pPublishPriceCmd->generationTier;
	*pBuf++ = pPublishPriceCmd->extendedNumPriceTiers;
	*pBuf++ = pPublishPriceCmd->extendedPriceTiers;
	*pBuf++ = pPublishPriceCmd->extendedRegisterTier;

	zbhciTx(ZBHCI_CMD_ZCL_GROUP_ADD_RSP, 56, payload);
#endif
}

/*********************************************************************
 * @fn      cloudsmets_priceCb
 *
 * @brief   Handler for ZCL Smart Energy publish Price command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t cloudsmets_priceCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == CLOUDSMETS_ENDPOINT){
		if(pAddrInfo->dirCluster == ZCL_FRAME_SERVER_CLIENT_DIR){
			switch(cmdId){
				case ZCL_CMD_PUBLISH_PRICE:
					cloudsmets_zclPublishPriceCmdHandler(pAddrInfo, (zcl_price_publishPriceCmd_t *)cmdPayload);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_PRICE */

#ifdef ZCL_IDENTIFY
s32 cloudsmets_zclIdentifyTimerCb(void *arg)
{
	if(g_zcl_identifyAttrs.identifyTime <= 0){
		identifyTimerEvt = NULL;
		return -1;
	}
	g_zcl_identifyAttrs.identifyTime--;
	return 0;
}

void cloudsmets_zclIdentifyTimerStop(void)
{
	if(identifyTimerEvt){
		TL_ZB_TIMER_CANCEL(&identifyTimerEvt);
	}
}

/*********************************************************************
 * @fn      cloudsmets_zclIdentifyCmdHandler
 *
 * @brief   Handler for ZCL Identify command. This function will set blink LED.
 *
 * @param   endpoint
 * @param   srcAddr
 * @param   identifyTime
 *
 * @return  None
 */
void cloudsmets_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime)
{
	g_zcl_identifyAttrs.identifyTime = identifyTime;

	if(identifyTime == 0){
		cloudsmets_zclIdentifyTimerStop();
		light_blink_stop(LED_POWER);
	}else{
		if(!identifyTimerEvt){
			light_blink_start(LED_POWER, identifyTime, 500, 500);
			identifyTimerEvt = TL_ZB_TIMER_SCHEDULE(cloudsmets_zclIdentifyTimerCb, NULL, 1000);
		}
	}
}

/*********************************************************************
 * @fn      cloudsmets_zcltriggerCmdHandler
 *
 * @brief   Handler for ZCL trigger command.
 *
 * @param   pTriggerEffect
 *
 * @return  None
 */
static void cloudsmets_zcltriggerCmdHandler(zcl_triggerEffect_t *pTriggerEffect)
{
	u8 effectId = pTriggerEffect->effectId;
	// u8 effectVariant = pTriggerEffect->effectVariant;

	switch(effectId){
		case IDENTIFY_EFFECT_BLINK:
			light_blink_start(LED_POWER, 1, 500, 500);
			break;
		case IDENTIFY_EFFECT_BREATHE:
			light_blink_start(LED_POWER, 15, 300, 700);
			break;
		case IDENTIFY_EFFECT_OKAY:
			light_blink_start(LED_POWER, 2, 250, 250);
			break;
		case IDENTIFY_EFFECT_CHANNEL_CHANGE:
			light_blink_start(LED_POWER, 1, 500, 7500);
			break;
		case IDENTIFY_EFFECT_FINISH_EFFECT:
			light_blink_start(LED_POWER, 1, 300, 700);
			break;
		case IDENTIFY_EFFECT_STOP_EFFECT:
			light_blink_stop(LED_POWER);
			break;
		default:
			break;
	}
}

/*********************************************************************
 * @fn      cloudsmets_zclIdentifyQueryRspCmdHandler
 *
 * @brief   Handler for ZCL Identify command. This function will set blink LED.
 *
 * @param   srcAddr
 * @param   cmdPayload
 *
 * @return  None
 */
static void cloudsmets_zclIdentifyQueryRspCmdHandler(zclIncomingAddrInfo_t *pAddrInfo, zcl_identifyRspCmd_t *cmdPayload)
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
status_t cloudsmets_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == CLOUDSMETS_ENDPOINT){
		if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
			switch(cmdId){
				case ZCL_CMD_IDENTIFY:
					cloudsmets_zclIdentifyCmdHandler(pAddrInfo->dstEp, pAddrInfo->srcAddr, ((zcl_identifyCmd_t *)cmdPayload)->identifyTime);
					break;
				case ZCL_CMD_TRIGGER_EFFECT:
					cloudsmets_zcltriggerCmdHandler((zcl_triggerEffect_t *)cmdPayload);
					break;
				default:
					break;
			}
		}else{
			if(cmdId == ZCL_CMD_IDENTIFY_QUERY_RSP){
				cloudsmets_zclIdentifyQueryRspCmdHandler(pAddrInfo, (zcl_identifyRspCmd_t *)cmdPayload);
			}
		}
	}

	return ZCL_STA_SUCCESS;
}

#endif	/* ZCL_IDENTIFY */

#endif  /* __PROJECT_CLOUDSMETS__ */


