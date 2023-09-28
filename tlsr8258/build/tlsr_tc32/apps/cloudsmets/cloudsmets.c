/********************************************************************************************************
 * @file    cloudsmets.c
 *
 * @brief   This is the source file for cloudsmets
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
#include "bdb.h"
#include "ota.h"
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
 * GLOBAL VARIABLES
 */
app_ctx_t g_switchAppCtx;


/**********************************************************************
 * We need callbacks for the ZigBee network functions so that we can learn
 * the IEEE address of the ESME/gas proxy so we can query it.
 */
const zdo_appIndCb_t appCbLst = {
	bdb_zdoStartDevCnf,//start device cnf cb
	NULL,//reset cnf cb
	NULL,//device announce indication cb
	cloudsmets_leaveIndHandler,//leave ind cb
	cloudsmets_leaveCnfHandler,//leave cnf cb
	NULL,//nwk update ind cb
	NULL,//permit join ind cb
	NULL,//nlme sync cnf cb
	NULL,//tc join ind cb
	NULL,//tc detects that the frame counter is near limit
};


/**
 *  @brief Definition for BDB finding and binding cluster
 */
u16 bdb_findBindClusterList[] =
{
	ZCL_CLUSTER_GEN_ON_OFF,
};

/**
 *  @brief Definition for BDB finding and binding cluster number
 */
#define FIND_AND_BIND_CLUSTER_NUM		(sizeof(bdb_findBindClusterList)/sizeof(bdb_findBindClusterList[0]))

??? Not sure  we need this

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

#if TOUCHLINK_SUPPORT
	.touchlinkEnable = 1,												/* enable touch-link */
#else
	.touchlinkEnable = 0,												/* disable touch-link */
#endif
	.touchlinkChannel = DEFAULT_CHANNEL, 								/* touch-link default operation channel for target */
	.touchlinkLqiThreshold = 0xA0,			   							/* threshold for touch-link scan req/resp command */
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
	/* Initialize ZB stack */
	zb_init();

	/* Register stack CB */
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
#if ZCL_POLL_CTRL_SUPPORT
	af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_PERIODICALLY);
#else
	af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_WHEN_STIMULATED);
#endif

    /* Initialize ZCL layer */
	/* Register Incoming ZCL Foundation command/response messages */
	zcl_init(cloudsmets_zclProcessIncomingMsg);

	/* register endPoint */
	af_endpointRegister(CLOUDSMETS_ENDPOINT, (af_simple_descriptor_t *)&cloudsmets_simpleDesc, zcl_rx_handler, NULL);

	/* Register ZCL specific cluster information */
	zcl_register(CLOUDSMETS_ENDPOINT, SAMPLE_SWITCH_CB_CLUSTER_NUM, (zcl_specClusterInfo_t *)g_cloudsmetsClusterList);

}



void led_init(void)
{
	led_off(LED_GREEN);
	led_off(LED_RED);
}

static void cloudsmetsSysException(void)
{
	/* Always peform full reset if something goes wrong. */
	SYSTEM_RESET();
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
	/* Initialize LEDs*/
	led_init();

	/*****************************************************************
	 * CloudSMETS controls the tlsr8258 from the ESP32-C3 over the zbhci interface.
	 */
	zbhciInit();

	if(!isRetention){
		/* Initialize Stack */
		stack_init();

		/* Initialize user application */
		user_app_init();

		/* Register exception handler. */
		sys_exceptHandlerRegister(cloudsmetsSysException);

		/* User's Task */
		ev_on_poll(EV_POLL_HCI, zbhciTask);

		/* There are not keys etc so no need for an app_task. */

		/* Load the pre-install code from flash */
		if(bdb_preInstallCodeLoad(&g_switchAppCtx.tcLinkKey.keyType, g_switchAppCtx.tcLinkKey.key) == RET_OK){
			g_bdbCommissionSetting.linkKey.tcLinkKey.keyType = g_switchAppCtx.tcLinkKey.keyType;
			g_bdbCommissionSetting.linkKey.tcLinkKey.key = g_switchAppCtx.tcLinkKey.key;
		}

		??? What is all of this?  What is BDB?

		bdb_findBindMatchClusterSet(FIND_AND_BIND_CLUSTER_NUM, bdb_findBindClusterList);

		/* Initialize BDB */
		u8 repower = drv_pm_deepSleep_flag_get() ? 0 : 1;
		bdb_init((af_simple_descriptor_t *)&cloudsmets_simpleDesc, &g_bdbCommissionSetting, &g_zbDemoBdbCb, repower);
	}else{
		/* Re-config phy when system recovery from deep sleep with retention */
		mac_phyReconfig();
	}
}

#endif  /* __PROJECT_CLOUDSMETS__ */
