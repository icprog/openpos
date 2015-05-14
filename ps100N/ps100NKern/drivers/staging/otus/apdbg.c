/*
 * IPWireless 3G PCMCIA Network Driver
 *
 * Original code
 *   by Stephen Blackheath <stephen@blacksapphire.com>,
 *      Ben Martel <benm@symmetric.co.nz>
 *
 * Copyrighted as follows:
 *   Copyright (C) 2004 by Symmetric Systems Ltd (NZ)
 *
 * Various driver changes and rewrites, port to new kernels
 *   Copyright (C) 2006-2007 Jiri Kosina
 *
 * Misc code cleanups and updates
 *   Copyright (C) 2007 David Sterba
 */

#ifndef _IPWIRELESS_CS_NETWORK_H_
#define _IPWIRELESS_CS_NETWORK_H_

#include <linux/types.h>

struct ipw_network;
struct ipw_tty;
struct ipw_hardware;

/* Definitions of the different channels on the PCMCIA UE */
#define IPW_CHANNEL_RAS      0
#define IPW_CHANNEL_DIALLER  1
#define IPW_CHANNEL_CONSOLE  2
#define NO_OF_IPW_CHANNELS   5

void ipwireless_network_notify_control_line_change(struct ipw_network *net,
		unsigned int channel_idx, unsigned int control_lines,
		unsigned int control_mask);
void ipwireless_network_packet_received(struct ipw_network *net,
		unsigned int channel_idx, unsigned char *data,
		unsigned int length);
struct ipw_network *ipwireless_network_create(struct ipw_hardware *hw);
void ipwireless_network_free(struct ipw_network *net);
void ipwireless_associate_network_tty(struct ipw_network *net,
		unsigned int channel_idx, struct ipw_tty *tty);
void ipwireless_disassociate_network_ttys(struct ipw_network *net,
		unsigned int channel_idx);

void ipwireless_ppp_open(struct ipw_network *net);

void ipwireless_ppp_close(struct ipw_network *net);
int ipwireless_ppp_channel_index(struct ipw_network *net);
int ipwireless_ppp_unit_number(struct ipw_network *net);
int ipwireless_ppp_mru(const struct ipw_network *net);

#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          /*
 * Copyright (c) 2006, 2007, 2008 QLogic Corporation. All rights reserved.
 * Copyright (c) 2003, 2004, 2005, 2006 PathScale, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
 * This file contains all of the code that is specific to the
 * InfiniPath 7220 chip (except that specific to the SerDes)
 */

#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <rdma/ib_verbs.h>

#include "ipath_kernel.h"
#include "ipath_registers.h"
#include "ipath_7220.h"

static void ipath_setup_7220_setextled(struct ipath_devdata *, u64, u64);

static unsigned ipath_compat_ddr_negotiate = 1;

module_param_named(compat_ddr_negotiate, ipath_compat_ddr_negotiate, uint,
			S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(compat_ddr_negotiate,
		"Attempt pre-IBTA 1.2 DDR speed negotiation");

static unsigned ipath_sdma_fetch_arb = 1;
module_param_named(fetch_arb, ipath_sdma_fetch_arb, uint, S_IRUGO);
MODULE_PARM_DESC(fetch_arb, "IBA7220: change SDMA descriptor arbitration");

/*
 * This file contains almost all the chip-specific register information and
 * access functions for the QLogic InfiniPath 7220 PCI-Express chip, with the
 * exception of SerDes support, which in in ipath_sd7220.c.
 *
 * This lists the InfiniPath registers, in the actual chip layout.
 * This structure should never be directly accessed.
 */
struct _infinipath_do_not_use_kernel_regs {
	unsigned long long Revision;
	unsigned long long Control;
	unsigned long long PageAlign;
	unsigned long long PortCnt;
	unsigned long long DebugPortSelect;
	unsigned long long DebugSigsIntSel; /* was Reserved0;*/
	unsigned long long SendRegBase;
	unsigned long long UserRegBase;
	unsigned long long CounterRegBase;
	unsigned long long Scratch;
	unsigned long long EEPROMAddrCmd; /* was Reserved1; */
	unsigned long long EEPROMData; /* was Reserved2; */
	unsigned long long IntBlocked;
	unsigned long long IntMask;
	unsigned long long IntStatus;
	unsigned long long IntClear;
	unsigned long long ErrorMask;
	unsigned long long ErrorStatus;
	unsigned long long ErrorClear;
	unsigned long long HwErrMask;
	unsigned long long HwErrStatus;
	unsigned long long HwErrClear;
	unsigned long long HwDiagCtrl;
	unsigned long long MDIO;
	unsigned long long IBCStatus;
	unsigned long long IBCCtrl;
	unsigned long long ExtStatus;
	unsigned long long ExtCtrl;
	unsigned long long GPIOOut;
	unsigned long long GPIOMask;
	unsigned long long GPIOStatus;
	unsigned long long GPIOClear;
	unsigned long long RcvCtrl;
	unsigned long long RcvBTHQP;
	unsigned long long RcvHdrSize;
	unsigned long long RcvHdrCnt;
	unsigned long long RcvHdrEntSize;
	unsigned long long RcvTIDBase;
	unsigned long long RcvTIDCnt;
	unsigned long long RcvEgrBase;
	unsigned long long RcvEgrCnt;
	unsigned long long RcvBufBase;
	unsigned long long RcvBufSize;
	unsigned long long RxIntMemBase;
	unsigned long long RxIntMemSize;
	unsigned long long RcvPartitionKey;
	unsigned long long RcvQPMulticastPort;
	unsigned long long RcvPktLEDCnt;
	unsigned long long IBCDDRCtrl;
	unsigned long long HRTBT_GUID;
	unsigned long long IB_SDTEST_IF_TX;
	unsigned long long IB_SDTEST_IF_RX;
	unsigned long long IBCDDRCtrl2;
	unsigned long long IBCDDRStatus;
	unsigned long long JIntReload;
	unsigned long long IBNCModeCtrl;
	unsigned long long SendCtrl;
	unsigned long long SendBufBase;
	unsigned long long SendBufSize;
	unsigned long long SendBufCnt;
	unsigned long long SendAvailAddr;
	unsigned long long TxIntMemBase;
	unsigned long long TxIntMemSize;
	unsigned long long SendDmaBase;
	unsigned long long SendDmaLenGen;
	unsigned long long SendDmaTail;
	unsigned long long SendDmaHead;
	unsigned long long SendDmaHeadAddr;
	unsigned long long SendDmaBufMask0;
	unsigned long long SendDmaBufMask1;
	unsigned long long SendDmaBufMask2;
	unsigned long long SendDmaStatus;
	unsigned long long SendBufferError;
	unsigned long long SendBufferErrorCONT1;
	unsigned long long SendBufErr2; /* was Reserved6SBE[0/6] */
	unsigned long long Reserved6L[2];
	unsigned long long AvailUpdCount;
	unsigned long long RcvHdrAddr0;
	unsigned long long RcvHdrAddrs[16]; /* Why enumerate? */
	unsigned long long Reserved7hdtl; /* Align next to 300 */
	unsigned long long RcvHdrTailAddr0; /* 300, like others */
	unsigned long long RcvHdrTailAddrs[16];
	unsigned long long Reserved9SW[7]; /* was [8]; we have 17 ports */
	unsigned long long IbsdEpbAccCtl; /* IB Serdes EPB access control */
	unsigned long long IbsdEpbTransReg; /* IB Serdes EPB Transaction */
	unsigned long long Reserved10sds; /* was SerdesStatus on */
	unsigned long long XGXSConfig;
	unsigned long long IBSerDesCtrl; /* Was IBPLLCfg on Monty */
	unsigned long long EEPCtlStat; /* for "boot" EEPROM/FLASH */
	unsigned long long EEPAddrCmd;
	unsigned long long EEPData;
	unsigned long long PcieEpbAccCtl;
	unsigned long long PcieEpbTransCtl;
	unsigned long long EfuseCtl; /* E-Fuse control */
	unsigned long long EfuseData[4];
	unsigned long long ProcMon;
	/* this chip moves following two from previous 200, 208 */
	unsigned long long PCIeRBufTestReg0;
	unsigned long long PCIeRBufTestReg1;
	/* added for this chip */
	unsigned long long PCIeRBufTestReg2;
	unsigned long long PCIeRBufTestReg3;
	/* added for this chip, debug only */
	unsigned long long SPC_JTAG_ACCESS_REG;
	unsigned long long LAControlReg;
	unsigned long long GPIODebugSelReg;
	unsigned long long DebugPortValueReg;
	/* added for this chip, DMA */
	unsigned long long SendDmaBufUsed[3];
	unsigned long long SendDmaReqTagUsed;
	/*
	 * added for this chip, EFUSE: note that these program 64-bit
	 * words 2 and 3 */
	unsigned long long efuse_pgm_data[2];
	unsigned long long Reserved11LAalign[10]; /* Skip 4B0..4F8 */
	/* we have 30 regs for DDS and RXEQ in IB SERDES */
	unsigned long long SerDesDDSRXEQ[30];
	unsigned long long Reserved12LAalign[2]; /* Skip 5F0, 5F8 */
	/* added for LA debug support */
	unsigned long long LAMemory[32];
};

struct _infinipath_do_not_use_counters {
	__u64 LBIntCnt;
	__u64 LBFlowStallCnt;
	__u64 TxSDmaDescCnt;	/* was Reserved1 */
	__u64 TxUnsupVLErrCnt;
	__u64 TxDataPktCnt;
	__u64 TxFlowPktCnt;
	__u64 TxDwordCnt;
	__u64 TxLenErrCnt;
	__u64 TxMaxMinLenErrCnt;
	__u64 TxUnderrunCnt;
	__u64 TxFlowStallCnt;
	__u64 TxDroppedPktCnt;
	__u64 RxDroppedPktCnt;
	__u64 RxDataPktCnt;
	__u64 RxFlowPktCnt;
	__u64 RxDwordCnt;
	__u64 RxLenErrCnt;
	__u64 RxMaxMinLenErrCnt;
	__u64 RxICRCErrCnt;
	__u64 RxVCRCErrCnt;
	__u64 RxFlowCtrlErrCnt;
	__u64 RxBadFormatCnt;
	__u64 RxLinkProblemCnt;
	__u64 RxEBPCnt;
	__u64 RxLPCRCErrCnt;
	__u64 RxBufOvflCnt;
	__u64 RxTIDFullErrCnt;
	__u64 RxTIDValidErrCnt;
	__u64 RxPKeyMismatchCnt;
	__u64 RxP0HdrEgrOvflCnt;
	__u64 RxP1HdrEgrOvflCnt;
	__u64 RxP2HdrEgrOvflCnt;
	__u64 RxP3HdrEgrOvflCnt;
	__u64 RxP4HdrEgrOvflCnt;
	__u64 RxP5HdrEgrOvflCnt;
	__u64 RxP6HdrEgrOvflCnt;
	__u64 RxP7HdrEgrOvflCnt;
	__u64 RxP8HdrEgrOvflCnt;
	__u64 RxP9HdrEgrOvflCnt;	/* was Reserved6 */
	__u64 RxP10HdrEgrOvflCnt;	/* was Reserved7 */
	__u64 RxP11HdrEgrOvflCnt;	/* new for IBA7220 */
	__u64 RxP12HdrEgrOvflCnt;	/* new for IBA7220 */
	__u64 RxP13HdrEgrOvflCnt;	/* new for IBA7220 */
	__u64 RxP14HdrEgrOvflCnt;	/* new for IBA7220 */
	__u64 RxP15HdrEgrOvflCnt;	/* new for IBA7220 */
	__u64 RxP16HdrEgrOvflCnt;	/* new for IBA7220 */
	__u64 IBStatusChangeCnt;
	__u64 IBLinkErrRecoveryCnt;
	__u64 IBLi