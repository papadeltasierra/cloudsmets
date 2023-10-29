/********************************************************************************************************
 * @file    esme.c
 *
 * @brief   This is the source file for esme
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
#include "bdb.h"
#include "ota.h"
#include "gp.h"
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
 * GLOBAL VARIABLES
 */
app_ctx_t g_appGwCtx;


#if ZBHCI_EN
extern mac_appIndCb_t macAppIndCbList;
#endif

#if defined(ZCL_OTA) || defined(ZCL_HCI_OTA)
//running code firmware information
ota_preamble_t esme_otaInfo = {
	.fileVer 			= FILE_VERSION,
	.imageType 			= IMAGE_TYPE,
	.manufacturerCode 	= MANUFACTURER_CODE_TELINK,
};
#endif


//Must declare the application call back function which used by ZDO layer
const zdo_appIndCb_t appCbLst = {
	bdb_zdoStartDevCnf,					//start device cnf cb
	NULL,								//reset cnf cb
	esme_devAnnHandler,				//device announce indication cb
	esme_leaveIndHandler,			//leave ind cb
	esme_leaveCnfHandler,			//leave cnf cb
	esme_nwkUpdateIndicateHandler,	//nwk update ind cb
	NULL,								//permit join ind cb
	NULL,								//nlme sync cnf cb
	esme_tcJoinIndHandler,			//tc join ind cb
	esme_tcFrameCntReachedHandler,	//tc detects that the frame counter is near limit
};


/**
 *  @brief Definition for bdb commissioning setting
 */
bdb_commissionSetting_t g_bdbCommissionSetting = {
	.linkKey.tcLinkKey.keyType = SS_GLOBAL_LINK_KEY,
	.linkKey.tcLinkKey.key = (u8 *)tcLinkKeyCentralDefault,       		//can use unique link key stored in NV

	.linkKey.distributeLinkKey.keyType = MASTER_KEY,
	.linkKey.distributeLinkKey.key = (u8 *)linkKeyDistributedMaster,  	//use linkKeyDistributedCertification before testing

	.linkKey.touchLinkKey.keyType = MASTER_KEY,
	.linkKey.touchLinkKey.key = (u8 *)touchLinkKeyMaster,   			//use touchLinkKeyCertification before testing

	.touchlinkEnable = 0,			 									//disable touch link for coordinator
};

/**********************************************************************
 * LOCAL VARIABLES
 */


/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      stack_init
 *
 * @brief   This function initialize the ZigBee stack and related profile. If HA/ZLL profile is
 *          enabled in this application, related cluster should be registered here.
 *
 * @param   None
 *
 * @return  None
 */
void stack_init(void)
{
	zb_init();

#if ZBHCI_EN
	zb_macCbRegister((mac_appIndCb_t *)&macAppIndCbList);
#endif
	zb_zdoCbRegister((zdo_appIndCb_t *)&appCbLst);
}

/*********************************************************************
 * @fn      user_app_init
 *
 * @brief   This function initialize the application(Endpoint) information for this node.
 *
 * @param   None
 *
 * @return  None
 */
void user_app_init(void)
{
	af_nodeDescManuCodeUpdate(MANUFACTURER_CODE_TELINK);

    /* Initialize ZCL layer */
	/* Register Incoming ZCL Foundation command/response messages */
	zcl_init(esme_zclProcessIncomingMsg);

	/* Register endPoint */
	af_endpointRegister(ESME_ENDPOINT, (af_simple_descriptor_t *)&esme_simpleDesc, zcl_rx_handler, esme_dataSendConfirm);

	/* Register ZCL specific cluster information */
	zcl_register(ESME_ENDPOINT, ESME_CB_CLUSTER_NUM, (zcl_specClusterInfo_t *)g_esmeClusterList);

#if ZCL_OTA_SUPPORT || ZCL_HCI_OTA_SUPPORT
	/* Note that ESME is always a client (actually we update over the HCI). */
    ota_init(OTA_TYPE_CLIENT, (af_simple_descriptor_t *)&esme_simpleDesc, &esme_otaInfo, NULL);
#endif
}

void led_init(void)
{
	led_off(LED_PERMIT);
	light_init();
}

void app_task(void)
{
	static bool assocPermit = 0;
	// Checking whether the logic is somehow wrong.
#if 0
	if(assocPermit != zb_getMacAssocPermit()){
		assocPermit = zb_getMacAssocPermit();
#else
	if (assocPermit != (g_zbInfo.macPib.associationPermit == 1)) {
		assocPermit = (g_zbInfo.macPib.associationPermit == 1);
#endif
		if(assocPermit){
			led_on(LED_PERMIT);
		}else{
			led_off(LED_PERMIT);
		}
	}

#if 0 //PDS
	if(BDB_STATE_GET() == BDB_STATE_IDLE){
		app_key_handler();
	}
#endif
}

static void esmeSysException(void)
{
#if 1
	SYSTEM_RESET();
#else
	while(1){
		gpio_toggle(LED_POWER);
		WaitMs(100);
	}
#endif
}



/*********************************************************************
 * @fn      user_init
 *
 * @brief   User level initialization code.
 *
 * @param   isRetention - if it is waking up with ram retention.
 *
 * @return  None
 */
void user_init(bool isRetention)
{
	(void)isRetention;

#if defined(MCU_CORE_8258) || defined(MCU_CORE_8278) || defined(MCU_CORE_B91)
	extern u8 firmwareCheckWithUID(void);
	if(firmwareCheckWithUID()){
		while(1);
	}
#endif

	/* Initialize LEDs*/
	led_init();

#if PA_ENABLE
	/* external RF PA used */
	rf_paInit(PA_TX, PA_RX);
#endif

	/* Initialize Stack */
	stack_init();

	sys_exceptHandlerRegister(esmeSysException);

	/* Initialize user application */
	user_app_init();

	/* User's Task */
#if ZBHCI_EN
    /*
     * define ZBHCI_USB_PRINT, ZBHCI_USB_CDC or ZBHCI_UART as 1 in app_cfg.h
     * if needing to enable ZBHCI_EN
     *
     * */
    zbhciInit();
	ev_on_poll(EV_POLL_HCI, zbhciTask);
#endif
	ev_on_poll(EV_POLL_IDLE, app_task);

	//af_nodeDescStackRevisionSet(20);

	/*
	 * bdb initialization start,
	 * once initialization is done, the g_zbDemoBdbCb.bdbInitCb() will be called
	 *
	 * */
    bdb_init((af_simple_descriptor_t *)&esme_simpleDesc, &g_bdbCommissionSetting, &g_zbDemoBdbCb, 1);
}

#endif  /* __PROJECT_ESME__ */

