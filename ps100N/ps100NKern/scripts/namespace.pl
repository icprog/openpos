#!/usr/bin/perl -w
#
#	namespace.pl.  Mon Aug 30 2004
#
#	Perform a name space analysis on the linux kernel.
#
#	Copyright Keith Owens <kaos@ocs.com.au>.  GPL.
#
#	Invoke by changing directory to the top of the kernel object
#	tree then namespace.pl, no parameters.
#
#	Tuned for 2.1.x kernels with the new module handling, it will
#	work with 2.0 kernels as well.
#
#	Last change 2.6.9-rc1, adding support for separate source and object
#	trees.
#
#	The source must be compiled/assembled first, the object files
#	are the primary input to this script.  Incomplete or missing
#	objects will result in a flawed analysis.  Compile both vmlinux
#	and modules.
#
#	Even with complete objects, treat the result of the analysis
#	with caution.  Some external references are only used by
#	certain architectures, others with certain combinations of
#	configuration parameters.  Ideally the source should include
#	something like
#
#	#ifndef CONFIG_...
#	static
#	#endif
#	symbol_definition;
#
#	so the symbols are defined as static unless a particular
#	CONFIG_... requires it to be external.
#
#	A symbol that is suffixed with '(export only)' has these properties
#
#	* It is global.
#	* It is marked EXPORT_SYMBOL or EXPORT_SYMBOL_GPL, either in the same
#	  source file or a different source file.
#	* Given the current .config, nothing uses the symbol.
#
#	The symbol is a candidate for conversion to static, plus removal of the
#	export.  But be careful that a different .config might use the symbol.
#
#
#	Name space analysis and cleanup is an iterative process.  You cannot
#	expect to find all the problems in a single pass.
#
#	* Identify possibly unnecessary global declarations, verify that they
#	  really are unnecessary and change them to static.
#	* Compile and fix up gcc warnings about static, removing dead symbols
#	  as necessary.
#	* make clean and rebuild with different configs (especially
#	  CONFIG_MODULES=n) to see which symbols are being defined when the
#	  config does not require them.  These symbols bloat the kernel object
#	  for no good reason, which is frustrating for embedded systems.
#	* Wrap config sensitive symbols in #ifdef CONFIG_foo, as long as the
#	  code does not get too ugly.
#	* Repeat the name space analysis until you can live with with the
#	  result.
#

require 5;	# at least perl 5
use strict;
use File::Find;

my $nm = ($ENV{'NM'} || "nm") . " -p";
my $objdump = ($ENV{'OBJDUMP'} || "objdump") . " -s -j .comment";
my $srctree = "";
my $objtree = "";
$srctree = "$ENV{'srctree'}/" if (exists($ENV{'srctree'}));
$objtree = "$ENV{'objtree'}/" if (exists($ENV{'objtree'}));

if ($#ARGV != -1) {
	print STDERR "usage: $0 takes no parameters\n";
	die("giving up\n");
}

my %nmdata = ();	# nm data for each object
my %def = ();		# all definitions for each name
my %ksymtab = ();	# names that appear in __ksymtab_
my %ref = ();		# $ref{$name} exists if there is a true external reference to $name
my %export = ();	# $export{$name} exists if there is an EXPORT_... of $name

&find(\&linux_objects, '.');	# find the objects and do_nm on them
&list_multiply_defined();
&resolve_external_references();
&list_extra_externals();

exit(0);

sub linux_objects
{
	# Select objects, ignoring objects which are only created by
	# merging other objects.  Also ignore all of modules, scripts
	# and compressed.  Most conglomerate objects are handled by do_nm,
	# this list only contains the special cases.  These include objects
	# that are linked from just one other object and objects for which
	# there is really no permanent source file.
	my $basename = $_;
	$_ = $File::Find::name;
	s:^\./::;
	if (/.*\.o$/ &&
		! (
		m:/built-in.o$:
		|| m:arch/x86/kernel/vsyscall-syms.o$:
		|| m:arch/ia64/ia32/ia32.o$:
		|| m:arch/ia64/kernel/gate-syms.o$:
		|| m:arch/ia64/lib/__divdi3.o$:
		|| m:arch/ia64/lib/__divsi3.o$:
		|| m:arch/ia64/lib/__moddi3.o$:
		|| m:arch/ia64/lib/__modsi3.o$:
		|| m:arch/ia64/lib/__udivdi3.o$:
		|| m:arch/ia64/lib/__udivsi3.o$:
		|| m:arch/ia64/lib/__umoddi3.o$:
		|| m:arch/ia64/lib/__umodsi3.o$:
		|| m:arch/ia64/scripts/check_gas_for_hi/* Driver for USB Mass Storage compliant devices
 * Protocol Functions Header File
 *
 * Current development and maintenance by:
 *   (c) 1999, 2000 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *
 * This driver is based on the 'USB Mass Storage Class' document. This
 * describes in detail the protocol used to communicate with such
 * devices.  Clearly, the designers had SCSI and ATAPI commands in
 * mind when they created this document.  The commands are all very
 * similar to commands in the SCSI-II and ATAPI specifications.
 *
 * It is important to note that in a number of cases this class
 * exhibits class-specific exemptions from the USB specification.
 * Notably the usage of NAK, STALL and ACK differs from the norm, in
 * that they are used to communicate wait, failed and OK on commands.
 *
 * Also, for certain devices, the interrupt endpoint is used to convey
 * status of a command.
 *
 * Please see http://www.one-eyed-alien.net/~mdharm/linux-usb for more
 * information about this driver.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/* Protocol handling routines */
extern void usb_stor_pad12_command(struct scsi_cmnd*, struct us_data*);
extern void usb_stor_ufi_command(struct scsi_cmnd*, struct us_data*);
extern void usb_stor_transparent_scsi_command(struct scsi_cmnd*,
		struct us_data*);

/* struct scsi_cmnd transfer buffer access utilities */
enum xfer_buf_dir	{TO_XFER_BUF, FROM_XFER_BUF};

extern unsigned int usb_stor_access_xfer_buf(unsigned char *buffer,
	unsigned int buflen, struct scsi_cmnd *srb, struct scatterlist **,
	unsigned int *offset, enum xfer_buf_dir dir);

extern void usb_stor_set_xfer_buf(unsigned char *buffer,
	unsigned int buflen, struct scsi_cmnd *srb);
#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           #ifndef _IP22_ZILOG_H
#define _IP22_ZILOG_H

#include <asm/byteorder.h>

struct zilog_channel {
#ifdef __BIG_ENDIAN
	volatile unsigned char unused0[3];
	volatile unsigned char control;
	volatile unsigned char unused1[3];
	volatile unsigned char data;
#else /* __LITTLE_ENDIAN */
	volatile unsigned char control;
	volatile unsigned char unused0[3];
	volatile unsigned char data;
	volatile unsigned char unused1[3];
#endif
};

struct zilog_layout {
	struct zilog_channel channelB;
	struct zilog_channel channelA;
};

#define NUM_ZSREGS    16

/* Conversion routines to/from brg time constants from/to bits
 * per second.
 */
#define BRG_TO_BPS(brg, freq) ((freq) / 2 / ((brg) + 2))
#define BPS_TO_BRG(bps, freq) ((((freq) + (bps)) / (2 * (bps))) - 2)

/* The Zilog register set */

#define	FLAG	0x7e

/* Write Register 0 */
#define	R0	0		/* Register selects */
#define	R1	1
#define	R2	2
#define	R3	3
#define	R4	4
#define	R5	5
#define	R6	6
#define	R7	7
#define	R8	8
#define	R9	9
#define	R10	10
#define	R11	11
#define	R12	12
#define	R13	13
#define	R14	14
#define	R15	15

#define	NULLCODE	0	/* Null Code */
#define	POINT_HIGH	0x8	/* Select upper half of registers */
#define	RES_EXT_INT	0x10	/* Reset Ext. Status Interrupts */
#define	SEND_ABORT	0x18	/* HDLC Abort */
#define	RES_RxINT_FC	0x20	/* Reset RxINT on First Character */
#define	RES_Tx_P	0x28	/* Reset TxINT Pending */
#define	ERR_RES		0x30	/* Error Reset */
#define	RES_H_IUS	0x38	/* Reset highest IUS */

#define	RES_Rx_CRC	0x40	/* Reset Rx CRC Checker */
#define	RES_Tx_CRC	0x80	/* Reset Tx CRC Checker */
#define	RES_EOM_L	0xC0	/* Reset EOM latch */

/* Write Register 1 */

#define	EXT_INT_ENAB	0x1	/* Ext Int Enable */
#define	TxINT_ENAB	0x2	/* Tx Int Enable */
#define	PAR_SPEC	0x4	/* Parity is special condition */

#define	RxINT_DISAB	0	/* Rx Int Disable */
#define	RxINT_FCERR	0x8	/* Rx Int on First Character Only or Error */
#define	INT_ALL_Rx	0x10	/* Int on all Rx Characters or error */
#define	INT_ERR_Rx	0x18	/* Int on error only */
#define RxINT_MASK	0x18

#define	WT_RDY_RT	0x20	/* Wait/Ready on R/T */
#define	WT_FN_RDYFN	0x40	/* Wait/FN/Ready FN */
#define	WT_RDY_ENAB	0x80	/* Wait/Ready Enable */

/* Write Register #2 (Interrupt Vector) */

/* Write Register 3 */

#define	RxENAB  	0x1	/* Rx Enable */
#define	SYNC_L_INH	0x2	/* Sync Character Load Inhibit */
#define	ADD_SM		0x4	/* Address Search Mode (SDLC) */
#define	RxCRC_ENAB	0x8	/* Rx CRC Enable */
#define	ENT_HM		0x10	/* Enter Hunt Mode */
#define	AUTO_ENAB	0x20	/* Auto Enables */
#define	Rx5		0x0	/* Rx 5 Bits/Character */
#define	Rx7		0x40	/* Rx 7 Bits/Character */
#define	Rx6		0x80	/* Rx 6 Bits/Character */
#define	Rx8		0xc0	/* Rx 8 Bits/Character */
#define RxN_MASK	0xc0

/* Write Register 4 */

#define	PAR_ENAB	0x1	/* Parity Enable */
#define	PAR_EVEN	0x2	/* Parity Even/Odd* */

#define	SYNC_ENAB	0	/* Sync Modes Enable */
#define	SB1		0x4	/* 1 stop bit/char */
#define	SB15		0x8	/* 1.5 stop bits/char */
#define	SB2		0xc	/* 2 stop bits/char */

#define	MONSYNC		0	/* 8 Bit Sync character */
#define	BISYNC		0x10	/* 16 bit sync character */
#define	SDLC		0x20	/* SDLC Mode (01111110 Sync Flag) */
#define	EXTSYNC		0x30	/* External Sync Mode */

#define	X1CLK		0x0	/* x1 clock mode */
#define	X16CLK		0x40	/* x16 clock mode */
#define	X32CLK		0x80	/* x32 clock mode */
#define	X64CLK		0xC0	/* x64 clock mode */
#define XCLK_MASK	0xC0

/* Write Register 5 */

#define	TxCRC_ENAB	0x1	/* Tx CRC Enable */
#define	RTS		0x2	/* RTS */
#define	SDLC_CRC	0x4	/* SDLC/CRC-16 */
#define	TxENAB		0x8	/* Tx Enable */
#define	SND_BRK		0x10	/* Send Break */
#define	Tx5		0x0	/* Tx 5 bits (or less)/character */
#define	Tx7		0x20	/* Tx 7 bits/character */
#define	Tx6		0x40	/* Tx 6 bits/character */
#define	Tx8		0x60	/* Tx 8 bits/character */
#define TxN_MASK	0x60
#define	DTR		0x80	/* DTR */

/* Write Register 6 (Sync bits 0-7/SDLC Address Field) */

/* Write Register 7 (Sync bits 8-15/SDLC 01111110) */

/* Write Register 8 (transmit buffer) */

/* Write Register 9 (Master interrupt control) */
#define	VIS	1	/* Vector Includes Status */
#define	NV	2	/* No Vector */
#define	DLC	4	/* Disable Lower Chain */
#define	MIE	8	/* Master Interrupt Enable */
#define	STATHI	0x10	/* Status high */
#define	NORESET	0	/* No reset on write to R9 */
#define	CHRB	0x40	/* Reset channel B */
#define	CHRA	0x80	/* Reset channel A */
#define	FHWRES	0xc0	/* Force hardware reset */

/* Write Register 10 (misc control bits) */
#define	BIT6	1	/* 6 bit/8bit sync */
#define	LOOPMODE 2	/* SDLC Loop mode */
#define	ABUNDER	4	/* Abort/flag on SDLC xmit underrun */
#define	MARKIDLE 8	/* Mark/flag on idle */
#define	GAOP	0x10	/* Go active on poll */
#define	NRZ	0	/* NRZ mode */
#define	NRZI	0x20	/* NRZI mode */
#define	FM1	0x40	/* FM1 (transition = 1) */
#define	FM0	0x60	/* FM0 (transition = 0) */
#define	CRCPS	0x80	/* CRC Preset I/O */

/* Write Register 11 (Clock Mode control) */
#define	TRxCXT	0	/* TRxC = Xtal output */
#define	TRxCTC	1	/* TRxC = Transmit clock */
#define	TRxCBR	2	/* TRxC = BR Generator Output */
#define	TRxCDP	3	/* TRxC = DPLL output */
#define	TRxCOI	4	/* TRxC O/I */
#define	TCRTxCP	0	/* Transmit clock = RTxC pin */
#define	TCTRxCP	8	/* Transmit clock = TRxC pin */
#defin