_WAITING_DMA_DONE_INT	FIELD32(0x01000000)

/*
 * RX descriptor format for RX Ring.
 */

/*
 * Word0
 * CIPHER_ERROR: 1:ICV error, 2:MIC error, 3:invalid key.
 * KEY_INDEX: Decryption key actually used.
 */
#define RXD_W0_OWNER_NIC		FIELD32(0x00000001)
#define RXD_W0_DROP			FIELD32(0x00000002)
#define RXD_W0_UNICAST_TO_ME		FIELD32(0x00000004)
#define RXD_W0_MULTICAST		FIELD32(0x00000008)
#define RXD_W0_BROADCAST		FIELD32(0x00000010)
#define RXD_W0_MY_BSS			FIELD32(0x00000020)
#define RXD_W0_CRC_ERROR		FIELD32(0x00000040)
#define RXD_W0_OFDM			FIELD32(0x00000080)
#define RXD_W0_CIPHER_ERROR		FIELD32(0x00000300)
#define RXD_W0_KEY_INDEX		FIELD32(0x0000fc00)
#define RXD_W0_DATABYTE_COUNT		FIELD32(0x0fff0000)
#define RXD_W0_CIPHER_ALG		FIELD32(0xe0000000)

/*
 * WORD1
 * SIGNAL: RX raw data rate reported by BBP.
 * RSSI: RSSI reported by BBP.
 */
#define RXD_W1_SIGNAL			FIELD32(0x000000ff)
#define RXD_W1_RSSI_AGC			FIELD32(0x00001f00)
#define RXD_W1_RSSI_LNA			FIELD32(0x00006000)
#define RXD_W1_FRAME_OFFSET		FIELD32(0x7f000000)

/*
 * Word2
 * IV: Received IV of originally encrypted.
 */
#define RXD_W2_IV			FIELD32(0xffffffff)

/*
 * Word3
 * EIV: Received EIV of originally encrypted.
 */
#define RXD_W3_EIV			FIELD32(0xffffffff)

/*
 * Word4
 * ICV: Received ICV of originally encrypted.
 * NOTE: This is a guess, the official definition is "reserved"
 */
#define RXD_W4_ICV			FIELD32(0xffffffff)

/*
 * the above 20-byte is called RXINFO and will be DMAed to MAC RX block
 * and passed to the HOST driver.
 * The following fields are for DMA block and HOST usage only.
 * Can't be touched by ASIC MAC block.
 */

/*
 * Word5
 */
#define RXD_W5_RESERVED			FIELD32(0xffffffff)

/*
 * Macro's for converting txpower from EEPROM to mac80211 value
 * and from mac80211 value to register value.
 */
#define MIN_TXPOWER	0
#define MAX_TXPOWER	31
#define DEFAULT_TXPOWER	24

#define TXPOWER_FROM_DEV(__txpower) \
	(((u8)(__txpower)) > MAX_TXPOWER) ? DEFAULT_TXPOWER : (__txpower)

#define TXPOWER_TO_DEV(__txpower) \
	clamp_t(char, __txpower, MIN_TXPOWER, MAX_TXPOWER)

#endif /* RT73USB_H */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            /*******************************************************************************
 *
 * Module Name: nseval - Object evaluation, includes control method execution
 *
 ******************************************************************************/

/*
 * Copyright (C) 2000 - 2008, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#include <acpi/acpi.h>
#include "accommon.h"
#include "acparser.h"
#include "acinterp.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_NAMESPACE
ACPI_MODULE_NAME("nseval")

/*******************************************************************************
 *
 * FUNCTION:    acpi_ns_evaluate
 *
 * PARAMETERS:  Info            - Evaluation info block, contains:
 *                  prefix_node     - Prefix or Method/Object Node to execute
 *                  Pathname        - Name of method to execute, If NULL, the
 *                                    Node is the object to execute
 *                  Parameters      - List of parameters to pass to the method,
 *                                    terminated by NULL. Params itself may be
 *                                    NULL if no parameters are being passed.
 *                  return_object   - Where to put method's return value (if
 *                                    any). If NULL, no value is returned.
 *                  parameter_type  - Type of Parameter list
 *                  return_object   - Where to put method's return value (if
 *                                    any). If NULL, no value is returned.
 *                  Flags           - ACPI_IGNORE_RETURN_VALUE to delete return
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Execute a control method or return the current value of an
 *              ACPI namespace object.
 *
 * MUTEX:       Locks interpreter
 *
 ******************************************************************************/
acpi_status acpi_ns_evaluate(struct acpi_evaluate_info * info)
{
	acpi_status status;
	struct acpi_namespace_node *node;

	ACPI_FUNCTION_TRACE(ns_evaluate);

	if (!info) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	/* Initialize the return value to an invalid object */

	info->return_object = NULL;
	info->param_count = 0;

	/*
	 * Get the actual namespace node for the target object. Handles these cases:
	 *
	 * 1) Null node, Pathname (absolute path)
	 * 2) Node, Pathname (path relative to Node)
	 * 3) Node, Null Pathname
	 */
	status = acpi_ns_get_node(info->prefix_node, info->pathname,
				  ACPI_NS_NO_UPSEARCH, &info->resolved_node);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/*
	 * For a metho