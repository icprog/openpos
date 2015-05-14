K 25
svn:wc:ra_dav:version-url
V 79
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce
END
iforce-usb.c
K 25
svn:wc:ra_dav:version-url
V 92
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/iforce-usb.c
END
Kconfig
K 25
svn:wc:ra_dav:version-url
V 87
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/Kconfig
END
iforce-packets.c
K 25
svn:wc:ra_dav:version-url
V 96
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/iforce-packets.c
END
iforce-ff.c
K 25
svn:wc:ra_dav:version-url
V 91
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/iforce-ff.c
END
iforce-serio.c
K 25
svn:wc:ra_dav:version-url
V 94
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/iforce-serio.c
END
iforce-main.c
K 25
svn:wc:ra_dav:version-url
V 93
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/iforce-main.c
END
Makefile
K 25
svn:wc:ra_dav:version-url
V 88
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/Makefile
END
iforce.h
K 25
svn:wc:ra_dav:version-url
V 88
/svn/YF-110510-01/!svn/ver/1153/soft/trunk/kernel/drivers/input/joystick/iforce/iforce.h
END
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      INDX( 	                 (   �   �      '                       �?     x d     �?     :� `<���gY$
���`<��utǙ���      
               M a k e f i l e . s v n - b a s e                   x d     �?     :� `<���gY$
���`<��utǙ���      
               M a k e f i l e . s v n - b a s e                   x d     �?     :� `<���gY$
���`<��utǙ���      
               M a k e f i l e . s v n - b a s e                   x d     �?     :� `<���gY$
���`<��utǙ���    ' 
               M a k e f i l e . s v n - b a s e                   x d     �?     :� `<���gY$
���`<��utǙ���      
               M a k e f i l e . s v n - b a s e                   �?     :� `<���gY$
���`<��utǙ���      
               M a k e f i l e . s v n - b a s e                                                                                                                                                                                                                 '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ' /*
 *  Fast C2P (Chunky-to-Planar) Conversion
 *
 *  Copyright (C) 2003-2008 Geert Uytterhoeven
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive
 *  for more details.
 */

#include <linux/module.h>
#include <linux/string.h>

#include <asm/unaligned.h>

#include "c2p.h"
#include "c2p_core.h"


    /*
     *  Perform a full C2P step on 16 8-bit pixels, stored in 4 32-bit words
     *  containing
     *    - 16 8-bit chunky pixels on input
     *    - permutated planar data (2 planes per 32-bit word) on output
     */

static void c2p_16x8(u32 d[4])
{
	transp4(d, 8, 2);
	transp4(d, 1, 2);
	transp4x(d, 16, 2);
	transp4x(d, 2, 2);
	transp4(d, 4, 1);
}


    /*
     *  Array containing the permutation indices of the planar data after c2p
     */

static const int perm_c2p_16x8[4] = { 1, 3, 0, 2 };


    /*
     *  Store a full block of iplan2 data after c2p conversion
     */

static inline void store_iplan2(void *dst, u32 bpp, u32 d[4])
{
	int i;

	for (i = 0; i < bpp/2; i++, dst += 4)
		put_unaligned_be32(d[perm_c2p_16x8[i]], dst);
}


    /*
     *  Store a partial block of iplan2 data after c2p conversion
     */

static inline void store_iplan2_masked(void *dst, u32 bpp, u32 d[4], u32 mask)
{
	int i;

	for (i = 0; i < bpp/2; i++, dst += 4)
		put_unaligned_be32(comp(d[perm_c2p_16x8[i]],
					get_unaligned_be32(dst), mask),
				   dst);
}


    /*
     *  c2p_iplan2 - Copy 8-bit chunky image data to an interleaved planar
     *  frame buffer with 2 bytes of interleave
     *  @dst: Starting address of the planar frame buffer
     *  @dx: Horizontal destination offset (in pixels)
     *  @dy: Vertical destination offset (in pixels)
     *  @width: Image width (in pixels)
     *  @height: Image height (in pixels)
     *  @dst_nextline: Frame buffer offset to the next line (in bytes)
     *  @src_nextline: Image offset to the next line (in bytes)
     *  @bpp: Bits per pixel of the planar frame buffer (2, 4, or 8)
     */

void c2p_iplan2(void *dst, const void *src, u32 dx, u32 dy, u32 width,
		u32 height, u32 dst_nextline, u32 src_nextline, u32 bpp)
{
	union {
		u8 pixels[16];
		u32 words[4];
	} d;
	u32 dst_idx, first, last, w;
	const u8 *c;
	void *p;

	dst += dy*dst_nextline+(dx & ~15)*bpp;
	dst_idx = dx % 16;
	first = 0xffffU >> dst_idx;
	first |= first << 16;
	last = 0xffffU ^ (0xffffU >> ((dst_idx+width) % 16));
	last |= last << 16;
	while (height--) {
		c = src;
		p = dst;
		w = width;
		if (dst_idx+width <= 16) {
			/* Single destination word */
			first &= last;
			memset(d.pixels, 0, sizeof(d));
			memcpy(d.pixels+dst_idx, c, width);
			c += width;
			c2p_16x8(d.words);
			store_iplan2_masked(p, bpp, d.words, first);
			p += bpp*2;
		} else {
			/* Multiple destination words */
			w = width;
			/* Leading bits */
			if (dst_idx) {
				w = 16 - dst_idx;
				memset(d.pixels, 0, dst_idx);
				memcpy(d.pixels+dst_idx, c, w);
				c += w;
				c2p_16x8(d.words);
				store_iplan2_masked(p, bpp, d.words, first);
				p += bpp*2;
				w = width-w;
			}
			/* Main chunk */
			while (w >= 16) {
				memcpy(d.pixels, c, 16);
				c += 16;
				c2p_16x8(d.words);
				store_iplan2(p, bpp, d.words);
				p += bpp*2;
				w -= 16;
			}
			/* Trailing bits */
			w %= 16;
			if (w > 0) {
				memcpy(d.pixels, c, w);
				memset(d.pixels+w, 0, 16-w);
				c2p_16x8(d.words);
				store_iplan2_masked(p, bpp, d.words, last);
			}
		}
		src += src_nextline;
		dst += dst_nextline;
	}
}
EXPORT_SYMBOL_GPL(c2p_iplan2);

MODULE_LICENSE("GPL");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              C�r @      ��  
     ��f�  P      �4� �     0��5  ,     �!M� �      i�       `HSn        C�> @     ��	  
     ���O  P      �4� �     0��       �!M�  �      i�       `H�'  (      C�= @     ��	  
     ��&T  P      �4? �     0�	       �!��  �      i�       `H�n  (      C�u @     Ҵ  
     ����  P      �4� �     0��7       �!M� �      ij       `Hso  (      C�| @     ��  
     ����  P      �4� �     0�Y7       �!M� �      i�       `Ho  (      C�y @     ��  
     ���v  P      �4� �     0��7       �!; �      i�       `H�'  (      C�� @     ��	  
     ��&l  P      �4� �     0�       �!͹ �      i�       `H�n  (      C�v @     Ҽ  
     ��&�  P      �4� �     0��7       �!M�  �      i�       `H�o  (      C�} @     Ҝ  
     ��&�  P      �4� �     0�i7       �!ͻ �      i�       `H3o  (      C�z @     ��  
     ����  P      �4� �     0��7       �!M�  �      i�       `H�3  (      C�? @     ҄  
     ��&T  P      �4c �     0��7       �!Ϳ �      i�       `Hs'  (      C�< @     ��	       ��fN  @      �4w       0��       �!�*       iX	 �      `H�J        CX        ��       ��&�  @      �4#       0�)       �!MK �      iB�     `H*@ 4      C�P�     ҄
      ���N� h      �4s@     0�)9       �!M� �      i��     `H�'@ 4      C�T �     Ҝ       ��fT  �      �4� �     0�I9  0     �!�� �     iZ       `HS'  \      C�9 �     Ҕ       ��&O  �      �4/@     0�	       �!��  �      ib �     `H�6  4      C�> �     ��       ��f�  �      �4q �     0��  .     �!�  �     i�        `H3  l      C=  �     Ҁ       ���  �      �45 �     0�q  8     �!�  �     i�        `H3  l      C=  �     Ҁ       ���  �      �45 �     0�q  8     �!�  �     ih       `Hss  l      C�]  �     ҈       ��&  �      �4�  �     0��  8     �!�  �     ih       `Hss  l      C�]  �     ҈       ��&  �      �4�  �     0��  8     �!(  �     i�       `H#  l      C�  �     Ҥ       ��&  �      �4) �     0��	  8     �!(  �     i�       `H#  l      C�  �     Ҥ       ��&  �      �4) �     0��	  8     �!O  �     i�       `H�  l      C��  �     ��       ���/  �      �4� �     0�Y  >     �!O  �     i�       `H�  l      C��  �     ��       ���/  �      �4� �     0�Y  >     �!�l  �     ip       `H�  l      C��  �     Ҥ       ���=  �      �4� �     0��  8     �!�  �     iL       `H�"  l      C� �     ��       ��&L  �      �4i �     0�y  >     �!�j         C��         �4o       0�Y  �      `H�  �     ���-  �      �46       0��	  8     �!�H  �     il       `H  p      C�  �     ��       ��&  �      �4�        0��  8     �!M:  �     iv       `H�
  p      C�S  �     ��       ���  �      �4�        0��  8     �!M~  �     i�       `HS  p      C�7        �4a        if       `HS  l      C��  `     Ҥ       ���4  �      �4w �     0�I  6     �!MJ  �     iR �     `H#  l      C��  `     Ҥ       ��F  �      �4�  �     0�  6     �!�� �     i� �     `H#  l      C�� `     ��       ��  �      �4s  �     0��9  6     �!(  �     i�  �     `H3  l      C��  `     ��       ��&M  �      �4; �     0�i  6     �,    2�,L��	&G�C�(���(��)�B)��(��*�+��(Ԁ�(̀B(��)@��D(L�E(ـ�+T�U(Ѐ�(�� F �     y  }  C�H����� �P�0��� s�u C�p�p;"��sE���An8W��\�v� b���sG����%�!� �!܁�A�8��\�v��y�vp�t��w��rpz �q@�p��!�)�YH�v�v(0�7�5X0�Q�Pb��sP�����+�p�	r;�9(�� 砠����� ���0��PC�{p�p;X@��;��91p.¹#��`���@C�cp
z;b��sP����*�˂��(8g���v�sW���%؁�a���a��s�n8�����v0Dp���"�� � ��;�p
t;�9(�� �`����p.�;��"� �@��+��p�	x;b���sG�����D�ށ�X8����v�sN���AA{8��\�w� !y��Y��p��r����sP���Ay�\�{�zz(�!������*����`nAn8'AnAn8'�nAn8'�nAn8'C�+p�p;b�\�sE ���� � ��!
Ι�n8woZ�x(�/*
 * Copyright (c) 2000-2001 Adaptec Inc.
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
 *
 * String handling code courtesy of Gerard Roudier's <groudier@club-internet.fr>
 * sym driver.
 *
 * $Id: //depot/aic7xxx/linux/drivers/scsi/aic7xxx/aic7xxx_proc.c#29 $
 */
#include "aic7xxx_osm.h"
#include "aic7xxx_inline.h"
#include "aic7xxx_93cx6.h"

static void	copy_mem_info(struct info_str *info, char *data, int len);
static int	copy_info(struct info_str *info, char *fmt, ...);
static void	ahc_dump_target_state(struct ahc_softc *ahc,
				      struct info_str *info,
				      u_int our_id, char channel,
				      u_int target_id, u_int target_offset);
static void	ahc_dump_device_state(struct info_str *info,
				      struct scsi_device *dev);
static int	ahc_proc_write_seeprom(struct ahc_softc *ahc,
				       char *buffer, int length);

/*
 * Table of syncrates that don't follow the "divisible by 4"
 * rule. This table will be expanded in future SCSI specs.
 */
static const struct {
	u_int period_factor;
	u_int period;	/* in 100ths of ns */
} scsi_syncrates[] = {
	{ 0x08, 625 },	/* FAST-160 */
	{ 0x09, 1250 },	/* FAST-80 */
	{ 0x0a, 2500 },	/* FAST-40 40MHz */
	{ 0x0b, 3030 },	/* FAST-40 33MHz */
	{ 0x0c, 5000 }	/* FAST-20 */
};

/*
 * Return the frequency in kHz corresponding to the given
 * sync period factor.
 */
static u_int
ahc_calc_syncsrate(u_int period_factor)
{
	int i;

	/* See if the period is in the "exception" table */
	for (i = 0; i < ARRAY_SIZE(scsi_syncrates); i++) {

		if (period_factor == scsi_syncrates[i].period_factor) {
			/* Period in kHz */
			return (100000000 / scsi_syncrates[i].period);
		}
	}

	/*
	 * Wasn't in the table, so use the standard
	 * 4 times conversion.
	 */
	return (10000000 / (period_factor * 4 * 10));
}


static void
copy_mem_info(struct info_str *info, char *data, int len)
{
	if (info->pos + len > info->offset + info->length)
		len = info->offset + info->length - info->pos;

	if (info->pos + len < info->offset) {
		info->pos += len;
		return;
	}

	if (info->pos < info->offset) {
		off_t partial;

		partial = info->offset - info->pos;
		data += partial;
		info->pos += partial;
		len  -= partial;
	}

	if (len > 0) {
		memcpy(info->buffer, data, len);
		info->pos += len;
		info->buffer += len;
	}
}

static int
copy_info(struct info_str *info, char *fmt, ...)
{
	va_list args;
	char buf[256];
	int len;

	va_start(args, fmt);
	len = vsprintf(buf, fmt, args);
	va_end(args);

	copy_mem_info(info, buf, len);
	return (len);
}

static void
ahc_format_transinfo(struct info_str *info, struct ahc_transinfo *tinfo)
{
	u_int speed;
	u_int freq;
	u_int mb;

        speed = 3300;
        freq = 0;
	if (tinfo->offset != 0) {
		freq = ahc_calc_syncsrate(tinfo->period);
		speed = freq;
	}
	speed *= (0x01 << tinfo->width);
        mb = speed / 1000;
        if (mb > 0)
		copy_info(info, "%d.%03dMB/s transfers", mb, speed % 1000);
        else
		copy_info(info, "%dKB/s transfers", speed);

	if (freq != 0) {
		copy_info(info, " (%d.%03dMHz%s, offset %d",
			 freq / 1000, freq % 1000,
			 (tinfo->ppr_options & MSG_EXT_PPR_DT_REQ) != 0
			 ? " DT" : "", tinfo->offset);
	}

	if (tinfo->width > 0) {
		if (freq != 0) {
			copy_info(info, ", ");
		} else {
			copy_info(info, " (");
		}
		copy_info(info, "%dbit)", 8 * (0x01 << tinfo->width));
	} else if (freq != 0) {
		copy_info(info, ")");
	}
	copy_info(info, "\n");
}

static void
ahc_dump_target_state(struct ahc_softc *ahc, struct info_str *info,
		      u_int our_id, char channel, u_int target_id,
		      u_int target_offset)
{
	struct	scsi_target *starget;
	struct	ahc_initiator_tinfo *tinfo;
	struct	ahc_tmode_tstate *tstate;
	int	lun;

	tinfo = ahc_fetch_transinfo(ahc, channel, our_id,
				    target_id, &tstate);
	if ((ahc->features & AHC_TWIN) != 0)
		copy_info(info, "Channel %c ", channel);
	copy_info(info, "Target %d Negotiation Settings\n", target_id);
	copy_info(info, "\tUser: ");
	ahc_format_transinfo(info, &tinfo->user);
	starget = ahc->platform_data->starget[target_offset];
	if (!starget)
		return;

	copy_info(info, "\tGoal: ");
	ahc_format_transinfo(info, &tinfo->goal);
	copy_info(info, "\tCurr: ");
	ahc_format_transinfo(info, &tinfo->curr);

	for (lun = 0; lun < AHC_NUM_LUNS; lun++) {
		struct scsi_device *sdev;

		sdev = scsi_device_lookup_by_target(starget, lun);

		if (sdev == NULL)
			continue;

		ahc_dump_device_state(info, sdev);
	}
}

static void
ahc_dump_device_state(struct info_str *info, struct scsi_device *sdev)
{
	struct ahc_linux_device *dev = scsi_transport_device_data(sdev);

	copy_info(info, "\tChannel %c Target %d Lun %d Settings\n",
		  sdev->sdev_target->channel + 'A',
		  sdev->sdev_target->id, sdev->lun);

	copy_info(info, "\t\tCommands Queued %ld\n", dev->commands_issued);
	copy_info(info, "\t\tCommands Active %d\n", dev->active);
	copy_info(info, "\t\tCommand Openings %d\n", dev->openings);
	copy_info(info, "\t\tMax Tagged Openings %d\n", dev->maxtags);
	copy_info(info, "\t\tDevice Queue Frozen Count %d\n", dev->qfrozen);
}

static int
ahc_proc_write_seeprom(struct ahc_softc *ahc, char *buffer, int length)
{
	struct seeprom_descriptor sd;
	int have_seeprom;
	u_long s;
	int paused;
	int written;

	/* Default to failure. */
	written = -EINVAL;
	ahc_lock(ahc, &s);
	paused = ahc_is_paused(ahc);
	if (!paused)
		ahc_pause(ahc);

	if (length != sizeof(struct seeprom_config)) {
		printf("ahc_proc_write_seeprom: incorrect buffer size\n");
		goto done;
	}

	have_seeprom = ahc_verify_cksum((struct seeprom_config*)buffer);
	if (have_seeprom == 0) {
		printf("ahc_proc_write_seeprom: cksum verification failed\n");
		goto done;
	}

	sd.sd_ahc = ahc;
#if AHC_PCI_CONFIG > 0
	if ((ahc->chip & AHC_PCI) != 0) {
		sd.sd_control_offset = SEECTL;
		sd.sd_status_offset = SEECTL;
		sd.sd_dataout_offset = SEECTL;
		if (ahc->flags & AHC_LARGE_SEEPROM)
			sd.sd_chip = C56_66;
		else
			sd.sd_chip = C46;
		sd.sd_MS = SEEMS;
		sd.sd_RDY = SEERDY;
		sd.sd_CS = SEECS;
		sd.sd_CK = SEECK;
		sd.sd_DO = SEEDO;
		sd.sd_DI = SEEDI;
		have_seeprom = ahc_acquire_seeprom(ahc, &sd);
	} else
#endif
	if ((ahc->chip & AHC_VL) != 0) {
		sd.sd_control_offset = SEECTL_2840;
		sd.sd_status_offset = STATUS_2840;
		sd.sd_dataout_offset = STATUS_2840;		
		sd.sd_chip = C46;
		sd.sd_MS = 0;
		sd.sd_RDY = EEPROM_TF;
		sd.sd_CS = CS_2840;
		sd.sd_CK = CK_2840;
		sd.sd_DO = DO_2840;
		sd.sd_DI = DI_2840;
		have_seeprom = TRUE;
	} else {
		printf("ahc_proc_write_seeprom: unsupported adapter type\n");
		goto done;
	}

	if (!have_seeprom) {
		printf("ahc_proc_write_seeprom: No Serial EEPROM\n");
		goto done;
	} else {
		u_int start_addr;

		if (ahc->seep_config == NULL) {
			ahc->seep_config = malloc(sizeof(*ahc->seep_config),
						  M_DEVBUF, M_NOWAIT);
			if (ahc->seep_config == NULL) {
				printf("aic7xxx: Unable to allocate serial "
				       "eeprom buffer.  Write failing\n");
				goto done;
			}
		}
		printf("aic7xxx: Writing Serial EEPROM\n");
		start_addr = 32 * (ahc->channel - 'A');
		ahc_write_seeprom(&sd, (u_int16_t *)buffer, start_addr,
				  sizeof(struct seeprom_config)/2);
		ahc_read_seeprom(&sd, (uint16_t *)ahc->seep_config,
				 start_addr, sizeof(struct seeprom_config)/2);
#if AHC_PCI_CONFIG > 0
		if ((ahc->chip & AHC_VL) == 0)
			ahc_release_seeprom(&sd);
#endif
		written = length;
	}

done:
	if (!paused)
		ahc_unpause(ahc);
	ahc_unlock(ahc, &s);
	return (written);
}

/*
 * Return information to handle /proc support for the driver.
 */
int
ahc_linux_proc_info(struct Scsi_Host *shost, char *buffer, char **start,
		    off_t offset, int length, int inout)
{
	struct	ahc_softc *ahc = *(struct ahc_softc **)shost->hostdata;
	struct	info_str info;
	char	ahc_info[256];
	u_int	max_targ;
	u_int	i;
	int	retval;

	 /* Has data been written to the file? */ 
	if (inout == TRUE) {
		retval = ahc_proc_write_seeprom(ahc, buffer, length);
		goto done;
	}

	if (start)
		*start = buffer;

	info.buffer	= buffer;
	info.length	= length;
	info.offset	= offset;
	info.pos	= 0;

	copy_info(&info, "Adaptec AIC7xxx driver version: %s\n",
		  AIC7XXX_DRIVER_VERSION);
	copy_info(&info, "%s\n", ahc->description);
	ahc_controller_info(ahc, ahc_info);
	copy_info(&info, "%s\n", ahc_info);
	copy_info(&info, "Allocated SCBs: %d, SG List Length: %d\n\n",
		  ahc->scb_data->numscbs, AHC_NSEG);


	if (ahc->seep_config == NULL)
		copy_info(&info, "No Serial EEPROM\n");
	else {
		copy_info(&info, "Serial EEPROM:\n");
		for (i = 0; i < sizeof(*ahc->seep_config)/2; i++) {
			if (((i % 8) == 0) && (i != 0)) {
				copy_info(&info, "\n");
			}
			copy_info(&info, "0x%.4x ",
				  ((uint16_t*)ahc->seep_config)[i]);
		}
		copy_info(&info, "\n");
	}
	copy_info(&info, "\n");

	max_targ = 16;
	if ((ahc->features & (AHC_WIDE|AHC_TWIN)) == 0)
		max_targ = 8;

	for (i = 0; i < max_targ; i++) {
		u_int our_id;
		u_int target_id;
		char channel;

		channel = 'A';
		our_id = ahc->our_id;
		target_id = i;
		if (i > 7 && (ahc->features & AHC_TWIN) != 0) {
			channel = 'B';
			our_id = ahc->our_id_b;
			target_id = i % 8;
		}

		ahc_dump_target_state(ahc, &info, our_id,
				      channel, target_id, i);
	}
	retval = info.pos > info.offset ? info.pos - info.offset : 0;
done:
	return (retval);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          /*
 * This file contains code to reset and initialize USB host controllers.
 * Some of it includes work-arounds for PCI hardware and BIOS quirks.
 * It may need to run early during booting -- before USB would normally
 * initialize -- to ensure that Linux doesn't use any legacy modes.
 *
 *  Copyright (c) 1999 Martin Mares <mj@ucw.cz>
 *  (and others)
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include "pci-quirks.h"
#include "xhci-ext-caps.h"


#define UHCI_USBLEGSUP		0xc0		/* legacy support */
#define UHCI_USBCMD		0		/* command register */
#define UHCI_USBINTR		4		/* interrupt register */
#define UHCI_USBLEGSUP_RWC	0x8f00		/* the R/WC bits */
#define UHCI_USBLEGSUP_RO	0x5040		/* R/O and reserved bits */
#define UHCI_USBCMD_RUN		0x0001		/* RUN/STOP bit */
#define UHCI_USBCMD_HCRESET	0x0002		/* Host Controller reset */
#define UHCI_USBCMD_EGSM	0x0008		/* Global Suspend Mode */
#define UHCI_USBCMD_CONFIGURE	0x0040		/* Config Flag */
#define UHCI_USBINTR_RESUME	0x0002		/* Resume interrupt enable */

#define OHCI_CONTROL		0x04
#define OHCI_CMDSTATUS		0x08
#define OHCI_INTRSTATUS		0x0c
#define OHCI_INTRENABLE		0x10
#define OHCI_INTRDISABLE	0x14
#define OHCI_OCR		(1 << 3)	/* ownership change request */
#define OHCI_CTRL_RWC		(1 << 9)	/* remote wakeup connected */
#define OHCI_CTRL_IR		(1 << 8)	/* interrupt routing */
#define OHCI_INTR_OC		(1 << 30)	/* ownership change */

#define EHCI_HCC_PARAMS		0x08		/* extended capabilities */
#define EHCI_USBCMD		0		/* command register */
#define EHCI_USBCMD_RUN		(1 << 0)	/* RUN/STOP bit */
#define EHCI_USBSTS		4		/* status register */
#define EHCI_USBSTS_HALTED	(1 << 12)	/* HCHalted bit */
#define EHCI_USBINTR		8		/* interrupt register */
#define EHCI_CONFIGFLAG		0x40		/* configured flag register */
#define EHCI_USBLEGSUP		0		/* legacy support register */
#define EHCI_USBLEGSUP_BIOS	(1 << 16)	/* BIOS semaphore */
#define EHCI_USBLEGSUP_OS	(1 << 24)	/* OS semaphore */
#define EHCI_USBLEGCTLSTS	4		/* legacy control/status */
#define EHCI_USBLEGCTLSTS_SOOE	(1 << 13)	/* SMI on ownership change */


/*
 * Make sure the controller is completely inactive, unable to
 * generate interrupts or do DMA.
 */
void uhci_reset_hc(struct pci_dev *pdev, unsigned long base)
{
	/* Turn off PIRQ enable and SMI enable.  (This also turns off the
	 * BIOS's USB Legacy Support.)  Turn off all the R/WC bits too.
	 */
	pci_write_config_word(pdev, UHCI_USBLEGSUP, UHCI_USBLEGSUP_RWC);

	/* Reset the HC - this will force us to get a
	 * new notification of any already connected
	 * ports due to the virtual disconnect that it
	 * implies.
	 */
	outw(UHCI_USBCMD_HCRESET, base + UHCI_USBCMD);
	mb();
	udelay(5);
	if (inw(base + UHCI_USBCMD) & UHCI_USBCMD_HCRESET)
		dev_warn(&pdev->dev, "HCRESET not completed yet!\n");

	/* Just to be safe, disable interrupt requests and
	 * make sure the controller is stopped.
	 */
	outw(0, base + UHCI_USBINTR);
	outw(0, base + UHCI_USBCMD);
}
EXPORT_SYMBOL_GPL(uhci_reset_hc);

/*
 * Initialize a controller that was newly discovered or has just been
 * resumed.  In either case we can't be sure of its previous state.
 *
 * Returns: 1 if the controller was reset, 0 otherwise.
 */
int uhci_check_and_reset_hc(struct pci_dev *pdev, unsigned long base)
{
	u16 legsup;
	unsigned int cmd, intr;

	/*
	 * When restarting a suspended controller, we expect all the
	 * settings to be the same as we left them:
	 *
	 *	PIRQ and SMI disabled, no R/W bits set in USBLEGSUP;
	 *	Controller is stopped and configured with EGSM set;
	 *	No interrupts enabled except possibly Resume Detect.
	 *
	 * If any of these conditions are violated we do a complete reset.
	 */
	pci_read_config_word(pdev, UHCI_USBLEGSUP, &legsup);
	if (legsup & ~(UHCI_USBLEGSUP_RO | UHCI_USBLEGSUP_RWC)) {
		dev_dbg(&pdev->dev, "%s: legsup = 0x%04x\n",
				__func__, legsup);
		goto reset_needed;
	}

	cmd = inw(base + UHCI_USBCMD);
	if ((cmd & UHCI_USBCMD_RUN) || !(cmd & UHCI_USBCMD_CONFIGURE) ||
			!(cmd & UHCI_USBCMD_EGSM)) {
		dev_dbg(&pdev->dev, "%s: cmd = 0x%04x\n",
				__func__, cmd);
		goto reset_needed;
	}

	intr = inw(base + UHCI_USBINTR);
	if (intr & (~UHCI_USBINTR_RESUME)) {
		dev_dbg(&pdev->dev, "%s: intr = 0x%04x\n",
				__func__, intr);
		goto reset_needed;
	}
	return 0;

reset_needed:
	dev_dbg(&pdev->dev, "Performing full reset\n");
	uhci_reset_hc(pdev, base);
	return 1;
}
EXPORT_SYMBOL_GPL(uhci_check_and_reset_hc);

static inline int io_type_enabled(struct pci_dev *pdev, unsigned int mask)
{
	u16 cmd;
	return !pci_read_config_word(pdev, PCI_COMMAND, &cmd) && (cmd & mask);
}

#define pio_enabled(dev) io_type_enabled(dev, PCI_COMMAND_IO)
#define mmio_enabled(dev) io_type_enabled(dev, PCI_COMMAND_MEMORY)

static void __devinit quirk_usb_handoff_uhci(struct pci_dev *pdev)
{
	unsigned long base = 0;
	int i;

	if (!pio_enabled(pdev))
		return;

	for (i = 0; i < PCI_ROM_RESOURCE; i++)
		if ((pci_resource_flags(pdev, i) & IORESOURCE_IO)) {
			base = pci_resource_start(pdev, i);
			break;
		}

	if (base)
		uhci_check_and_reset_hc(pdev, base);
}

static int __devinit mmio_resource_enabled(struct pci_dev *pdev, int idx)
{
	return pci_resource_start(pdev, idx) && mmio_enabled(pdev);
}

static void __devinit quirk_usb_handoff_ohci(struct pci_dev *pdev)
{
	void __iomem *base;

	if (!mmio_resource_enabled(pdev, 0))
		return;

	base = pci_ioremap_bar(pdev, 0);
	if (base == NULL)
		return;

/* On PA-RISC, PDC can leave IR set incorrectly; ignore it there. */
#ifndef __hppa__
{
	u32 control = readl(base + OHCI_CONTROL);
	if (control & OHCI_CTRL_IR) {
		int wait_time = 500; /* arbitrary; 5 seconds */
		writel(OHCI_INTR_OC, base + OHCI_INTRENABLE);
		writel(OHCI_OCR, base + OHCI_CMDSTATUS);
		while (wait_time > 0 &&
				readl(base + OHCI_CONTROL) & OHCI_CTRL_IR) {
			wait_time -= 10;
			msleep(10);
		}
		if (wait_time <= 0)
			dev_warn(&pdev->dev, "OHCI: BIOS handoff failed"
					" (BIOS bug?) %08x\n",
					readl(base + OHCI_CONTROL));

		/* reset controller, preserving RWC */
		writel(control & OHCI_CTRL_RWC, base + OHCI_CONTROL);
	}
}
#endif

	/*
	 * disable interrupts
	 */
	writel(~(u32)0, base + OHCI_INTRDISABLE);
	writel(~(u32)0, base + OHCI_INTRSTATUS);

	iounmap(base);
}

static void __devinit quirk_usb_disable_ehci(struct pci_dev *pdev)
{
	int wait_time, delta;
	void __iomem *base, *op_reg_base;
	u32	hcc_params, val;
	u8	offset, cap_length;
	int	count = 256/4;
	int	tried_handoff = 0;

	if (!mmio_resource_enabled(pdev, 0))
		return;

	base = pci_ioremap_bar(pdev, 0);
	if (base == NULL)
		return;

	cap_length = readb(base);
	op_reg_base = base + cap_length;

	/* EHCI 0.96 and later may have "extended capabilities"
	 * spec section 5.1 explains the bios handoff, e.g. for
	 * booting from USB disk or using a usb keyboard
	 */
	hcc_params = readl(base + EHCI_HCC_PARAMS);
	offset = (hcc_params >> 8) & 0xff;
	while (offset && --count) {
		u32		cap;
		int		msec;

		pci_read_config_dword(pdev, offset, &cap);
		switch (cap & 0xff) {
		case 1:			/* BIOS/SMM/... handoff support */
			if ((cap & EHCI_USBLEGSUP_BIOS)) {
				dev_dbg(&pdev->dev, "EHCI: BIOS handoff\n");

#if 0
/* aleksey_gorelov@phoenix.com reports that some systems need SMI forced on,
 * but that seems dubious in general (the BIOS left it off intentionally)
 * and is known to prevent some systems from booting.  so we won't do this
 * unless maybe we can determine when we're on a system that needs SMI forced.
 */
				/* BIOS workaround (?): be sure the
				 * pre-Linux code receives the SMI
				 */
				pci_read_config_dword(pdev,
						offset + EHCI_USBLEGCTLSTS,
						&val);
				pci_write_config_dword(pdev,
						offset + EHCI_USBLEGCTLSTS,
						val | EHCI_USBLEGCTLSTS_SOOE);
#endif

				/* some systems get upset if this semaphore is
				 * set for any other reason than forcing a BIOS
				 * handoff..
				 */
				pci_write_config_byte(pdev, offset + 3, 1);
			}

			/* if boot firmware now owns EHCI, spin till
			 * it hands it over.
			 */
			msec = 1000;
			while ((cap & EHCI_USBLEGSUP_BIOS) && (msec > 0)) {
				tried_handoff = 1;
				msleep(10);
				msec -= 10;
				pci_read_config_dword(pdev, offset, &cap);
			}

			if (cap & EHCI_USBLEGSUP_BIOS) {
				/* well, possibly buggy BIOS... try to shut
				 * it down, and hope nothing goes too wrong
				 */
				dev_warn(&pdev->dev, "EHCI: BIOS handoff failed"
						" (BIOS bug?) %08x\n", cap);
				pci_write_config_byte(pdev, offset + 2, 0);
			}

			/* just in case, always disable EHCI SMIs */
			pci_write_config_dword(pdev,
					offset + EHCI_USBLEGCTLSTS,
					0);

			/* If the BIOS ever owned the controller then we
			 * can't expect any power sessions to remain intact.
			 */
			if (tried_handoff)
				writel(0, op_reg_base + EHCI_CONFIGFLAG);
			break;
		case 0:			/* illegal reserved capability */
			cap = 0;
			/* FALLTHROUGH */
		default:
			dev_warn(&pdev->dev, "EHCI: unrecognized capability "
					"%02x\n", cap & 0xff);
			break;
		}
		offset = (cap >> 8) & 0xff;
	}
	if (!count)
		dev_printk(KERN_DEBUG, &pdev->dev, "EHCI: capability loop?\n");

	/*
	 * halt EHCI & disable its interrupts in any case
	 */
	val = readl(op_reg_base + EHCI_USBSTS);
	if ((val & EHCI_USBSTS_HALTED) == 0) {
		val = readl(op_reg_base + EHCI_USBCMD);
		val &= ~EHCI_USBCMD_RUN;
		writel(val, op_reg_base + EHCI_USBCMD);

		wait_time = 2000;
		delta = 100;
		do {
			writel(0x3f, op_reg_base + EHCI_USBSTS);
			udelay(delta);
			wait_time -= delta;
			val = readl(op_reg_base + EHCI_USBSTS);
			if ((val == ~(u32)0) || (val & EHCI_USBSTS_HALTED)) {
				break;
			}
		} while (wait_time > 0);
	}
	writel(0, op_reg_base + EHCI_USBINTR);
	writel(0x3f, op_reg_base + EHCI_USBSTS);

	iounmap(base);

	return;
}

/*
 * handshake - spin reading a register until handshake completes
 * @ptr: address of hc register to be read
 * @mask: bits to look at in result of read
 * @done: value of those bits when handshake succeeds
 * @wait_usec: timeout in microseconds
 * @delay_usec: delay in microseconds to wait between polling
 *
 * Polls a register every delay_usec microseconds.
 * Returns 0 when the mask bits have the value done.
 * Returns -ETIMEDOUT if this condition is not true after
 * wait_usec microseconds have passed.
 */
static int handshake(void __iomem *ptr, u32 mask, u32 done,
		int wait_usec, int delay_usec)
{
	u32	result;

	do {
		result = readl(ptr);
		result &= mask;
		if (result == done)
			return 0;
		udelay(delay_usec);
		wait_usec -= delay_usec;
	} while (wait_usec > 0);
	return -ETIMEDOUT;
}

/**
 * PCI Quirks for xHCI.
 *
 * Takes care of the handoff between the Pre-OS (i.e. BIOS) and the OS.
 * It signals to the BIOS that the OS wants control of the host controller,
 * and then waits 5 seconds for the BIOS to hand over control.
 * If we timeout, assume the BIOS is broken and take control anyway.
 */
static void __devinit quirk_usb_handoff_xhci(struct pci_dev *pdev)
{
	void __iomem *base;
	int ext_cap_offset;
	void __iomem *op_reg_base;
	u32 val;
	int timeout;

	if (!mmio_resource_enabled(pdev, 0))
		return;

	base = ioremap_nocache(pci_resource_start(pdev, 0),
				pci_resource_len(pdev, 0));
	if (base == NULL)
		return;

	/*
	 * Find the Legacy Support Capability register -
	 * this is optional for xHCI host controllers.
	 */
	ext_cap_offset = xhci_find_next_cap_offset(base, XHCI_HCC_PARAMS_OFFSET);
	do {
		if (!ext_cap_offset)
			/* We've reached the end of the extended capabilities */
			goto hc_init;
		val = readl(base + ext_cap_offset);
		if (XHCI_EXT_CAPS_ID(val) == XHCI_EXT_CAPS_LEGACY)
			break;
		ext_cap_offset = xhci_find_next_cap_offset(base, ext_cap_offset);
	} while (1);

	/* If the BIOS owns the HC, signal that the OS wants it, and wait */
	if (val & XHCI_HC_BIOS_OWNED) {
		writel(val & XHCI_HC_OS_OWNED, base + ext_cap_offset);

		/* Wait for 5 seconds with 10 microsecond polling interval */
		timeout = handshake(base + ext_cap_offset, XHCI_HC_BIOS_OWNED,
				0, 5000, 10);

		/* Assume a buggy BIOS and take HC ownership anyway */
		if (timeout) {
			dev_warn(&pdev->dev, "xHCI BIOS handoff failed"
					" (BIOS bug ?) %08x\n", val);
			writel(val & ~XHCI_HC_BIOS_OWNED, base + ext_cap_offset);
		}
	}

	/* Disable any BIOS SMIs */
	writel(XHCI_LEGACY_DISABLE_SMI,
			base + ext_cap_offset + XHCI_LEGACY_CONTROL_OFFSET);

hc_init:
	op_reg_base = base + XHCI_HC_LENGTH(readl(base));

	/* Wait for the host controller to be ready before writing any
	 * operational or runtime registers.  Wait 5 seconds and no more.
	 */
	timeout = handshake(op_reg_base + XHCI_STS_OFFSET, XHCI_STS_CNR, 0,
			5000, 10);
	/* Assume a buggy HC and start HC initialization anyway */
	if (timeout) {
		val = readl(op_reg_base + XHCI_STS_OFFSET);
		dev_warn(&pdev->dev,
				"xHCI HW not ready after 5 sec (HC bug?) "
				"status = 0x%x\n", val);
	}

	/* Send the halt and disable interrupts command */
	val = readl(op_reg_base + XHCI_CMD_OFFSET);
	val &= ~(XHCI_CMD_RUN | XHCI_IRQS);
	writel(val, op_reg_base + XHCI_CMD_OFFSET);

	/* Wait for the HC to halt - poll every 125 usec (one microframe). */
	timeout = handshake(op_reg_base + XHCI_STS_OFFSET, XHCI_STS_HALT, 1,
			XHCI_MAX_HALT_USEC, 125);
	if (timeout) {
		val = readl(op_reg_base + XHCI_STS_OFFSET);
		dev_warn(&pdev->dev,
				"xHCI HW did not halt within %d usec "
				"status = 0x%x\n", XHCI_MAX_HALT_USEC, val);
	}

	iounmap(base);
}

static void __devinit quirk_usb_early_handoff(struct pci_dev *pdev)
{
	if (pdev->class == PCI_CLASS_SERIAL_USB_UHCI)
		quirk_usb_handoff_uhci(pdev);
	else if (pdev->class == PCI_CLASS_SERIAL_USB_OHCI)
		quirk_usb_handoff_ohci(pdev);
	else if (pdev->class == PCI_CLASS_SERIAL_USB_EHCI)
		quirk_usb_disable_ehci(pdev);
	else if (pdev->class == PCI_CLASS_SERIAL_USB_XHCI)
		quirk_usb_handoff_xhci(pdev);
}
DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, quirk_usb_early_handoff);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         for the elf file format,
 * we need to keep a linked list of every threads pr_status and then create
 * a single section for them in the final core file.
 */
static int elf_dump_thread_status(long signr, struct elf_thread_status *t)
{
	int sz = 0;
	struct task_struct *p = t->thread;
	t->num_notes = 0;

	fill_prstatus(&t->prstatus, p, signr);
	elf_core_copy_task_regs(p, &t->prstatus.pr_reg);	
	
	fill_note(&t->notes[0], "CORE", NT_PRSTATUS, sizeof(t->prstatus),
		  &(t->prstatus));
	t->num_notes++;
	sz += notesize(&t->notes[0]);

	if ((t->prstatus.pr_fpvalid = elf_core_copy_task_fpregs(p, NULL,
								&t->fpu))) {
		fill_note(&t->notes[1], "CORE", NT_PRFPREG, sizeof(t->fpu),
			  &(t->fpu));
		t->num_notes++;
		sz += notesize(&t->notes[1]);
	}

#ifdef ELF_CORE_COPY_XFPREGS
	if (elf_core_copy_task_xfpregs(p, &t->xfpu)) {
		fill_note(&t->notes[2], "LINUX", ELF_CORE_XFPREG_TYPE,
			  sizeof(t->xfpu), &t->xfpu);
		t->num_notes++;
		sz += notesize(&t->notes[2]);
	}
#endif	
	return sz;
}

struct elf_note_info {
	struct memelfnote *notes;
	struct elf_prstatus *prstatus;	/* NT_PRSTATUS */
	struct elf_prpsinfo *psinfo;	/* NT_PRPSINFO */
	struct list_head thread_list;
	elf_fpregset_t *fpu;
#ifdef ELF_CORE_COPY_XFPREGS
	elf_fpxregset_t *xfpu;
#endif
	int thread_status_size;
	int numnote;
};

static int fill_note_info(struct elfhdr *elf, int phdrs,
			  struct elf_note_info *info,
			  long signr, struct pt_regs *regs)
{
#define	NUM_NOTES	6
	struct list_head *t;

	info->notes = NULL;
	info->prstatus = NULL;
	info->psinfo = NULL;
	info->fpu = NULL;
#ifdef ELF_CORE_COPY_XFPREGS
	info->xfpu = NULL;
#endif
	INIT_LIST_HEAD(&info->thread_list);

	info->notes = kmalloc(NUM_NOTES * sizeof(struct memelfnote),
			      GFP_KERNEL);
	if (!info->notes)
		return 0;
	info->psinfo = kmalloc(sizeof(*info->psinfo), GFP_KERNEL);
	if (!info->psinfo)
		return 0;
	info->prstatus = kmalloc(sizeof(*info->prstatus), GFP_KERNEL);
	if (!info->prstatus)
		return 0;
	info->fpu = kmalloc(sizeof(*info->fpu), GFP_KERNEL);
	if (!info->fpu)
		return 0;
#ifdef ELF_CORE_COPY_XFPREGS
	info->xfpu = kmalloc(sizeof(*info->xfpu), GFP_KERNEL);
	if (!info->xfpu)
		return 0;
#endif

	info->thread_status_size = 0;
	if (signr) {
		struct core_thread *ct;
		struct elf_thread_status *ets;

		for (ct = current->mm->core_state->dumper.next;
						ct; ct = ct->next) {
			ets = kzalloc(sizeof(*ets), GFP_KERNEL);
			if (!ets)
				return 0;

			ets->thread = ct->task;
			list_add(&ets->list, &info->thread_list);
		}

		list_for_each(t, &info->thread_list) {
			int sz;

			ets = list_entry(t, struct elf_thread_status, list);
			sz = elf_dump_thread_status(signr, ets);
			info->thread_status_size += sz;
		}
	}
	/* now collect the dump for the current */
	memset(info->prstatus, 0, sizeof(*info->prstatus));
	fill_prstatus(info->prstatus, current, signr);
	elf_core_copy_regs(&info->prstatus->pr_reg, regs);

	/* Set up header */
	fill_elf_header(elf, phdrs, ELF_ARCH, ELF_CORE_EFLAGS, ELF_OSABI);

	/*
	 * Set up the notes in similar form to SVR4 core dumps made
	 * with info from their /proc.
	 */

	fill_note(info->notes + 0, "CORE", NT_PRSTATUS,
		  sizeof(*info->prstatus), info->prstatus);
	fill_psinfo(info->psinfo, current->group_leader, current->mm);
	fill_note(info->notes + 1, "CORE", NT_PRPSINFO,
		  sizeof(*info->psinfo), info->psinfo);

	info->numnote = 2;

	fill_auxv_note(&info->notes[info->numnote++], current->mm);

	/* Try to dump the FPU. */
	info->prstatus->pr_fpvalid = elf_core_copy_task_fpregs(current, regs,
							       info->fpu);
	if (info->prstatus->pr_fpvalid)
		fill_note(info->notes + info->numnote++,
			  "CORE", NT_PRFPREG, sizeof(*info->fpu), info->fpu);
#ifdef ELF_CORE_COPY_XFPREGS
	if (elf_core_copy_task_xfpregs(current, info->xfpu))
		fill_note(info->notes + info->numnote++,
			  "LINUX", ELF_CORE_XFPREG_TYPE,
			  sizeof(*info->xfpu), info->xfpu);
#endif

	return 1;

#undef NUM_NOTES
}

static size_t get_note_info_size(struct elf_note_info *info)
{
	int sz = 0;
	int i;

	for (i = 0; i < info->numnote; i++)
		sz += notesize(info->notes + i);

	sz += info->thread_status_size;

	return sz;
}

static int write_note_info(struct elf_note_info *info,
			   struct file *file, loff_t *foffset)
{
	int i;
	struct list_head *t;

	for (i = 0; i < info->numnote; i++)
		if (!writenote(info->notes + i, file, foffset))
			return 0;

	/* write out the thread status notes section */
	list_for_each(t, &info->thread_list) {
		struct elf_thread_status *tmp =
				list_entry(t, struct elf_thread_status, list);

		for (i = 0; i < tmp->num_notes; i++)
			if (!writenote(&tmp->notes[i], file, foffset))
				return 0;
	}

	return 1;
}

static void free_note_info(struct elf_note_info *info)
{
	while (!list_empty(&info->thread_list)) {
		struct list_head *tmp = info->thread_list.next;
		list_del(tmp);
		kfree(list_entry(tmp, struct elf_thread_status, list));
	}

	kfree(info->prstatus);
	kfree(info->psinfo);
	kfree(info->notes);
	kfree(info->fpu);
#ifdef ELF_CORE_COPY_XFPREGS
	kfree(info->xfpu);
#endif
}

#endif

static struct vm_area_struct *first_vma(struct task_struct *tsk,
					struct vm_area_struct *gate_vma)
{
	struct vm_area_struct *ret = tsk->mm->mmap;

	if (ret)
		return ret;
	return gate_vma;
}
/*
 * Helper function for iterating across a vma list.  It ensures that the caller
 * will visit `gate_vma' prior to terminating the search.
 */
static struct vm_area_struct *next_vma(struct vm_area_struct *this_vma,
					struct vm_area_struct *gate_vma)
{
	struct vm_area_struct *ret;

	ret = this_vma->vm_next;
	if (ret)
		return ret;
	if (this_vma == gate_vma)
		return NULL;
	return gate_vma;
}

/*
 * Actual dumper
 *
 * This is a two-pass process; first we find the offsets of the bits,
 * and then they are actually written out.  If we run out of core limit
 * we just truncate.
 */
static int elf_core_dump(long signr, struct pt_regs *regs, struct file *file, unsigned long limit)
{
	int has_dumped = 0;
	mm_segment_t fs;
	int segs;
	size_t size = 0;
	struct vm_area_struct *vma, *gate_vma;
	struct elfhdr *elf = NULL;
	loff_t offset = 0, dataoff, foffset;
	unsigned long mm_flags;
	struct elf_note_info info;

	/*
	 * We no longer stop all VM operations.
	 * 
	 * This is because those proceses that could possibly change map_count
	 * or the mmap / vma pages are now blocked in do_exit on current
	 * finishing this core dump.
	 *
	 * Only ptrace can touch these memory addresses, but it doesn't change
	 * the map_count or the pages allocated. So no possibility of crashing
	 * exists while dumping the mm->vm_next areas to the core file.
	 */
  
	/* alloc memory for large data structures: too large to be on stack */
	elf = kmalloc(sizeof(*elf), GFP_KERNEL);
	if (!elf)
		goto out;
	/*
	 * The number of segs are recored into ELF header as 16bit value.
	 * Please check DEFAULT_MAX_MAP_COUNT definition when you modify here.
	 */
	segs = current->mm->map_count;
#ifdef ELF_CORE_EXTRA_PHDRS
	segs += ELF_CORE_EXTRA_PHDRS;
#endif

	gate_vma = get_gate_vma(current);
	if (gate_vma != NULL)
		segs++;

	/*
	 * Collect all the non-memory information about the process for the
	 * notes.  This also sets up the file header.
	 */
	if (!fill_note_info(elf, segs + 1, /* including notes section */
			    &info, signr, regs))
		goto cleanup;

	has_dumped = 1;
	current->flags |= PF_DUMPCORE;
  
	fs = get_fs();
	set_fs(KERNEL_DS);

	DUMP_WRITE(elf, sizeof(*elf));
	offset += sizeof(*elf);				/* Elf header */
	offset += (segs + 1) * sizeof(struct elf_phdr); /* Program headers */
	foffset = offset;

	/* Write notes phdr entry */
	{
		struct elf_phdr phdr;
		size_t sz = get_note_info_size(&info);

		sz += elf_coredump_extra_notes_size();

		fill_elf_note_phdr(&phdr, sz, offset);
		offset += sz;
		DUMP_WRITE(&phdr, sizeof(phdr));
	}

	dataoff = offset = roundup(offset, ELF_EXEC_PAGESIZE);

	/*
	 * We must use the same mm->flags while dumping core to avoid
	 * inconsistency between the program headers and bodies, otherwise an
	 * unusable core file can be generated.
	 */
	mm_flags = current->mm->flags;

	/* Write program headers for segments dump */
	for (vma = first_vma(current, gate_vma); vma != NULL;
			vma = next_vma(vma, gate_vma)) {
		struct elfrequest, we won't lose much by setting the
		 * revision level down to 2.  The only devices that would be
		 * affected are those with sparse LUNs. */
		if (sdev->scsi_level > SCSI_2)
			sdev->sdev_target->scsi_level =
					sdev->scsi_level = SCSI_2;

		/* USB-IDE bridges tend to report SK = 0x04 (Non-recoverable
		 * Hardware Error) when any low-level error occurs,
		 * recoverable or not.  Setting this flag tells the SCSI
		 * midlayer to retry such commands, which frequently will
		 * succeed and fix the error.  The worst this can lead to
		 * is an occasional series of retries that will all fail. */
		sdev->retry_hwerror = 1;

		/* USB disks should allow restart.  Some drives spin down
		 * automatically, requiring a START-STOP UNIT command. */
		sdev->allow_restart = 1;

		/* Some USB cardreaders have trouble reading an sdcard's last
		 * sector in a larger then 1 sector read, since the performance
		 * impact is negible we set this flag for all USB disks */
		sdev->last_sector_bug = 1;

		/* Enable last-sector hacks for single-target devices using
		 * the Bulk-only transport, unless we already know the
		 * capacity will be decremented or is correct. */
		if (!(us->fflags & (US_FL_FIX_CAPACITY | US_FL_CAPACITY_OK |
					US_FL_SCM_MULT_TARG)) &&
				us->protocol == US_PR_BULK)
			us->use_last_sector_hacks = 1;
	} else {

		/* Non-disk-type devices don't need to blacklist any pages
		 * or to force 192-byte transfer lengths for MODE SENSE.
		 * But they do need to use MODE SENSE(10). */
		sdev->use_10_for_ms = 1;
	}

	/* The CB and CBI transports have no way to pass LUN values
	 * other than the bits in the second byte of a CDB.  But those
	 * bits don't get set to the LUN value if the device reports
	 * scsi_level == 0 (UNKNOWN).  Hence such devices must necessarily
	 * be single-LUN.
	 */
	if ((us->protocol == US_PR_CB || us->protocol == US_PR_CBI) &&
			sdev->scsi_level == SCSI_UNKNOWN)
		us->max_lun = 0;

	/* Some devices choke when they receive a PREVENT-ALLOW MEDIUM
	 * REMOVAL command, so suppress those commands. */
	if (us->fflags & US_FL_NOT_LOCKABLE)
		sdev->lockable = 0;

	/* this is to satisfy the compiler, tho I don't think the 
	 * return code is ever checked 