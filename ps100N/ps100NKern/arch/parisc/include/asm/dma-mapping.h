arge */
	if (machine_is_mx51_3ds())
		CHECK_ERROR(pmic_write(REG_CHARGE, 0xB40003));

	pm_power_off = mc13892_power_off;

	return PMIC_SUCCESS;
}

unsigned int pmic_get_active_events(unsigned int *active_events)
{
	unsigned int count = 0;
	unsigned int status0, status1;
	int bit_set;

	pmic_read(REG_INT_STATUS0, &status0);
	pmic_read(REG_INT_STATUS1, &status1);
	pmic_write(REG_INT_STATUS0, status0);
	pmic_write(REG_INT_STATUS1, status1);
	status0 &= events_enabled0;
	status1 &= events_enabled1;

	while (status0) {
		bit_set = ffs(status0) - 1;
		*(active_events + count) = bit_set;
		count++;
		status0 ^= (1 << bit_set);
	}
	while (status1) {
		bit_set = ffs(status1) - 1;
		*(active_events + count) = bit_set + 24;
		count++;
		status1 ^= (1 << bit_set);
	}

	return count;
}

#define EVENT_MASK_0			0x387fff
#define EVENT_MASK_1			0x1177eb

int pmic_event_unmask(type_event event)
{
	unsigned int event_mask = 0;
	unsigned int mask_reg = 0;
	unsigned int event_bit = 0;
	int ret;

	if (event < EVENT_1HZI) {
		mask_reg = REG_INT_MASK0;
		event_mask = EVENT_MASK_0;
		event_bit = (1 << event);
		events_enabled0 |= event_bit;
	} else {
		event -= 24;
		mask_reg = REG_INT_MASK1;
		event_mask = EVENT_MASK_1;
		event_bit = (1 << event);
		events_enabled1 |= event_bit;
	}

	if ((event_bit & event_mask) == 0) {
		pr_debug("Error: unmasking a reserved/unused event\n");
		return PMIC_ERROR;
	}

	ret = pmic_write_reg(mask_reg, 0, event_bit);

	pr_debug("Enable Event : %d\n", event);

	return ret;
}

int pmic_event_mask(type_event event)
{
	unsigned int event_mask = 0;
	unsigned int mask_reg = 0;
	unsigned int event_bit = 0;
	int ret;

	if (event < EVENT_1HZI) {
		mask_reg = REG_INT_MASK0;
		event_mask = EVENT_MASK_0;
		event_bit = (1 << event);
		events_enabled0 &= ~event_bit;
	} else {
		event -= 24;
		mask_reg = REG_INT_MASK1;
		event_mask = EVENT_MASK_1;
		event_bit = (1 << event);
		events_enabled1 &= ~event_bit;
	}

	if ((event_bit & event_mask) == 0) {
		pr_debug("Error: masking a reserved/unused event\n");
		return PMIC_ERROR;
	}

	ret = pmic_write_reg(mask_reg, event_bit, event_bit);

	pr_debug("Disable Event : %d\n", event);

	return ret;
}

/*!
 * This function returns the PMIC version in system.
 *
 * @param 	ver	pointer to the pmic_version_t structure
 *
 * @return       This function returns PMIC version.
 */
void pmic_get_revision(pmic_version_t *ver)
{
	int rev_id = 0;
	int rev1 = 0;
	int rev2 = 0;
	int finid = 0;
	int icid = 0;

	ver->id = PMIC_MC13892;
	pmic_read(REG_IDENTIFICATION, &rev_id);

	rev1 = (rev_id & 0x018) >> 3;
	rev2 = (rev_id & 0x007);
	icid = (rev_id & 0x01C0) >> 6;
	finid = (rev_id & 0x01E00) >> 9;

	ver->revision = ((rev1 * 10) + rev2);
	printk(KERN_INFO "mc13892 Rev %d.%d FinVer %x detected\n", rev1,
	       rev2, finid);
}

void mc13892_power_off(void)
{
	unsigned int value;

	pmic_read_reg(REG_POWER_CTL0, &value, 0xffffff);

	value |= 0x000008;

	pmic_write_reg(REG_POWER_CTL0, value, 0xffffff);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       /*
 * Copyright (c) 1996, 2003 VIA Networking, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * File: iocmd.h
 *
 * Purpose: Handles the viawget ioctl private interface functions
 *
 * Author: Lyndon Chen
 *
 * Date: May 8, 2002
 *
 */

#ifndef __IOCMD_H__
#define __IOCMD_H__

#if !defined(__TTYPE_H__)
#include "ttype.h"
#endif


/*---------------------  Export Definitions -------------------------*/

#if !defined(DEF)
#define DEF
#endif

//typedef int BOOL;
//typedef uint32_t u32;
//typedef uint16_t u16;
//typedef uint8_t u8;


// ioctl Command code
#define MAGIC_CODE	                 0x3142
#define IOCTL_CMD_TEST	            (SIOCDEVPRIVATE + 0)
#define IOCTL_CMD_SET			    (SIOCDEVPRIVATE + 1)
#define IOCTL_CMD_HOSTAPD           (SIOCDEVPRIVATE + 2)
#define IOCTL_CMD_WPA               (SIOCDEVPRIVATE + 3)


typedef enum tagWMAC_CMD {

    WLAN_CMD_BSS_SCAN,
    WLAN_CMD_BSS_JOIN,
    WLAN_CMD_DISASSOC,
    WLAN_CMD_SET_WEP,
    WLAN_CMD_GET_LINK,
    WLAN_CMD_GET_LISTLEN,
    WLAN_CMD_GET_LIST,
    WLAN_CMD_GET_MIB,
    WLAN_CMD_GET_STAT,
    WLAN_CMD_STOP_MAC,
    WLAN_CMD_START_MAC,
    WLAN_CMD_AP_START,
    WLAN_CMD_SET_HOSTAPD,
    WLAN_CMD_SET_HOSTAPD_STA,
    WLAN_CMD_SET_802_1X,
    WLAN_CMD_SET_HOST_WEP,
    WLAN_CMD_SET_WPA,
    WLAN_CMD_GET_NODE_CNT,
     WLAN_CMD_ZONETYPE_SET,
    WLAN_CMD_GET_NODE_LIST

} WMAC_CMD, DEF* PWMAC_CMD;

	typedef enum tagWZONETYPE {
  ZoneType_USA=0,
  ZoneType_Japan=1,
  ZoneType_Europe=2
}WZONETYPE;

#define ADHOC	0
#define INFRA	1
#define BOTH	2
#define AP	    3

#define ADHOC_STARTED	   1
#define ADHOC_JOINTED	   2


#define PHY80211a 	    0
#define PHY80211b       1
#define PHY80211g       2

#define SSID_ID                0
#define SSID_MAXLEN            32
#define BSSID_LEN              6
#define WEP_NKEYS              4
#define WEP_KEYMAXLEN          29
#define WEP_40BIT_LEN          5
#define WEP_104BIT_LEN         13
#define WEP_232BIT_LEN         16


// Ioctl interface structure
// Command structure
//
#pragma pack(1)
typedef struct tagSCmdRequest {
	U8 	    name[16];
	void	*data;
	U16	    wResult;
	U16     wCmdCode;
} SCmdRequest, *PSCmdRequest;


//
// Scan
//

typedef struct tagSCmdScan {

    U8	    ssid[SSID_MAXLEN + 2];

} SCmdScan, *PSCmdScan;


//
// BSS Join
//

typedef struct tagSCmdBSSJoin {

    U16	    wBSSType;
    U16     wBBPType;
    U8	    ssid[SSID_MAXLEN + 2];
    U32	    uChannel;
    BOOL    bPSEnable;
    BOOL    bShareKeyAuth;

} SCmdBSSJoin, *PSCmdBSSJoin;

typedef struct tagSCmdZoneTypeSet {

 BOOL       bWrite;
 WZONETYPE  ZoneType;

} SCmdZoneTypeSet, *PSCmdZoneTypeSet;

#ifdef WPA_SM_Transtatus
typedef struct tagSWPAResult {
         char	ifname[100];
         U8		proto;
         U8   key_mgmt;
         U8   eap_type;
         BOOL authenticated;
} SWPAResult, *PSWPAResult;
#endif


typedef struct tagSCmdStartAP {

    U16	    wBSSType;
    U16     wBBPType;
    U8	    ssid[SSID_MAXLEN + 2];
    U32 	uChannel;
    U32     uBeaconInt;
    BOOL    bShareKeyAuth;
    U8      byBasicRate;

} SCmdStartAP, *PSCmdStartAP;


typedef struct tagSCmdSetWEP {

    BOOL    bEnableWep;
    U8      byKeyIndex;
    U8      abyWepKey[WEP_NKEYS][WEP_KEYMAXLEN];
    BOOL    bWepKeyAvailable[WEP_NKEYS];
    U32     auWepKeyLength[WEP_NKEYS];

} SCmdSetWEP, *PSCmdSetWEP;



typedef struct tagSBSSIDItem {

	U32	    uChannel;
    U8      abyBSSID[BSSID_LEN];
    U8      abySSID[SSID_MAXLEN + 1];
    //2006-1116-01,<M