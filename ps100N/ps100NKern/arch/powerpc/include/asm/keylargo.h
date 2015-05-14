#ifndef _ASM_POWERPC_KEYLARGO_H
#define _ASM_POWERPC_KEYLARGO_H
#ifdef __KERNEL__
/*
 * keylargo.h: definitions for using the "KeyLargo" I/O controller chip.
 *
 */

/* "Pangea" chipset has keylargo device-id 0x25 while core99
 * has device-id 0x22. The rev. of the pangea one is 0, so we
 * fake an artificial rev. in keylargo_rev by oring 0x100
 */
#define KL_PANGEA_REV		0x100

/* offset from base for feature control registers */
#define KEYLARGO_MBCR		0x34	/* KL Only, Media bay control/status */
#define KEYLARGO_FCR0		0x38
#define KEYLARGO_FCR1		0x3c
#define KEYLARGO_FCR2		0x40
#define KEYLARGO_FCR3		0x44
#define KEYLARGO_FCR4		0x48
#define KEYLARGO_FCR5		0x4c	/* Pangea only */

/* K2 aditional FCRs */
#define K2_FCR6			0x34
#define K2_FCR7			0x30
#define K2_FCR8			0x2c
#define K2_FCR9			0x28
#define K2_FCR10		0x24

/* GPIO registers */
#define KEYLARGO_GPIO_LEVELS0		0x50
#define KEYLARGO_GPIO_LEVELS1		0x54
#define KEYLARGO_GPIO_EXTINT_0		0x58
#define KEYLARGO_GPIO_EXTINT_CNT	18
#define KEYLARGO_GPIO_0			0x6A
#define KEYLARGO_GPIO_CNT		17
#define KEYLARGO_GPIO_EXTINT_DUAL_EDGE	0x80
#define KEYLARGO_GPIO_OUTPUT_ENABLE	0x04
#define KEYLARGO_GPIO_OUTOUT_DATA	0x01
#define KEYLARGO_GPIO_INPUT_DATA	0x02

/* K2 does only extint GPIOs and does 51 of them */
#define K2_GPIO_EXTINT_0		0x58
#define K2_GPIO_EXTINT_CNT		51

/* Specific GPIO regs */

#define KL_GPIO_MODEM_RESET		(KEYLARGO_GPIO_0+0x03)
#define KL_GPIO_MODEM_POWER		(KEYLARGO_GPIO_0+0x02) /* Pangea */

#define KL_GPIO_SOUND_POWER		(KEYLARGO_GPIO_0+0x05)

/* Hrm... this one is only to be used on Pismo. It seeem to also
 * control the timebase enable on other machines. Still to be
 * experimented... --BenH.
 */
#define KL_GPIO_FW_CABLE_POWER		(KEYLARGO_GPIO_0+0x09)
#define KL_GPIO_TB_ENABLE		(KEYLARGO_GPIO_0+0x09)

#define KL_GPIO_ETH_PHY_RESET		(KEYLARGO_GPIO_0+0x10)

#define KL_GPIO_EXTINT_CPU1		(KEYLARGO_GPIO_0+0x0a)
#define KL_GPIO_EXTINT_CPU1_ASSERT	0x04
#define KL_GPIO_EXTINT_CPU1_RELEASE	0x38

#define KL_GPIO_RESET_CPU0		(KEYLARGO_GPIO_EXTINT_0+0x03)
#define KL_GPIO_RESET_CPU1		(KEYLARGO_GPIO_EXTINT_0+0x04)
#define KL_GPIO_RESET_CPU2		(KEYLARGO_GPIO_EXTINT_0+0x0f)
#define KL_GPIO_RESET_CPU3		(KEYLARGO_GPIO_EXTINT_0+0x10)

#define KL_GPIO_PMU_MESSAGE_IRQ		(KEYLARGO_GPIO_EXTINT_0+0x09)
#define KL_GPIO_PMU_MESSAGE_BIT		KEYLARGO_GPIO_INPUT_DATA

#define KL_GPIO_MEDIABAY_IRQ		(KEYLARGO_GPIO_EXTINT_0+0x0e)

#define KL_GPIO_AIRPORT_0		(KEYLARGO_GPIO_EXTINT_0+0x0a)
#define KL_GPIO_AIRPORT_1		(KEYLARGO_GPIO_EXTINT_0+0x0d)
#define KL_GPIO_AIRPORT_2		(KEYLARGO_GPIO_0+0x0d)
#define KL_GPIO_AIRPORT_3		(KEYLARGO_GPIO_0+0x0e)
#define KL_GPIO_AIRPORT_4		(KEYLARGO_GPIO_0+0x0f)

/*
 * Bits in feature control register. Those bits different for K2 are
 * listed separately
 */
#define KL_MBCR_MB0_PCI_ENABLE		0x00000800	/* exist ? */
#define KL_MBCR_MB0_IDE_ENABLE		0x00001000
#define KL_MBCR_MB0_FLOPPY_ENABLE	0x00002000	/* exist ? */
#define KL_MBCR_MB0_SOUND_ENABLE	0x00004000	/* hrm... */
#define KL_MBCR_MB0_DEV_MASK		0x00007800
#define KL_MBCR_MB0_DEV_POWER		0x00000400
#define KL_MBCR_MB0_DEV_RESET		0x00000200
#define KL_MBCR_MB0_ENABLE		0x00000100
#define KL_MBCR_MB1_PCI_ENABLE		0x08000000	/* exist ? */
#define KL_MBCR_MB1_IDE_ENABLE		0x10000000
#define KL_MBCR_MB1_FLOPPY_ENABLE	0x20000000	/* exist ? */
#define KL_MBCR_MB1_SOUND_ENABLE	0x40000000	/* hrm... */
#define KL_MBCR_MB1_DEV_MASK		0x78000000
#define KL_MBCR_MB1_DEV_POWER		0x04000000
#define KL_MBCR_MB1_DEV_RESET		0x02000000
#define KL_MBCR_MB1_ENABLE		0x01000000

#define KL0_SCC_B_INTF_ENABLE		0x00000001	/* (KL Only) */
#define KL0_SCC_A_INTF_ENABLE		0x00000002
#define KL0_SCC_SLOWPCLK		0x00000004
#define KL0_SCC_RESET			0x00000008
#define KL0_SCCA_ENABLE			0x00000010
#define KL0_SCCB_ENABLE			0x00000020
#define KL0_SCC_CELL_ENABLE		0x00000040
#define KL0_IRDA_HIGH_BAND		0x00000100	/* (KL Only) */
#define KL0_IRDA_SOURCE2_SEL		0x00000200	/* (KL Only) */
#define KL0_IRDA_SOURCE1_SEL		0x00000400	/* (KL Only) */
#define KL0_PG_USB0_PMI_ENABLE		0x00000400	/* (Pangea/Intrepid Only) */
#define KL0_IRDA_RESET			0x00000800	/* (KL Onl   4S z���       ,   HS m���g    A�A�A�a�A�A�   (   xS ����P    A�F�C AC�A�     �S Ȥ��%    A�C ]C�8   �S ͤ��~    A�A�A�A�C0rA�A�A�A�    S ���
          S ���          (S ����6    A�t�  0   DS ���_    A�A�A�C SA�C�A�    xS >���;       (   �S e���Y    A�A�N@FA�A�  (   �S ����Z    A�A�C RA�A�     �S ����F    A�N sC�(   S ���-    A�A�A�e�C�A�   0S ���&          DS ����          XS ���          lS ����       (   �S ���Q    A�A�F�FA�A� <   �S ����    A�A�A�A�C��A�A�A�A�   8   �S ^����    A�A�A�A�C0�A�A�A�A�<   (S ����Y   A�A�A�A�NLBA�A�A�A�   <   hS ɧ��Y   A�A�A�A�NLBA�A�A�A�       �S ���    A�N	A�      �S ީ��    A�N	A�  0   �S ڪ���   A�A�A�C �A�A�A�<   $S W���R   A�A�A�A�CPFA�A�A�A�   4   dS i���   A�A�A�
C�A�EAA�A�4   �S 7���   A�A�A�
C�A�EAA�A�   �S ���    A�]�  (   �S ���:    A�A�C rA�A�   0   S ���k    A�A�A�C aA�A�A� (   PS M���\    A�A�C TA�A�  8   |S }���}    A�A�A�A�C0qA�A�A�A�(   �S ����D    A�F�A�u�A�A�8   �S ְ���    A�A�A�A�CP�A�A�A�A�(    S 2���G    A�A�C A�A�      LS M���7       (   `S p���:    A�A�C rA�A�   8   �S ~���   A�A�A�A�C@�A�A�A�A�(   �S J���l    A�C�C bA�A�  0   �S ����z    A�A�A�C pA�A�A�    (S в��       0   <S ɲ���   A�A�A�C �A�A�A�   pS Y���+    C g (   �S l���F    A�A�C0~A�A�      �S ����          �S u���          �S d���       (   �S S���U    A�A�N BA�A�  (   	S |���:    A�A�C rA�A�   (   H	S ����:    A�A�C rA�A�      t	S ����5    A�C mC�    �	S ����    A�F�T�A� 8   �	S ����z    A�F�A�A�C0iA�A�A�A�8   �	S ����    A�F�A�A�Cp�A�A�A�A�8   0
S B����    A�F�A�A�C`�A�A�A�A�   l
S ����          �
S ����          �
S ����          �
S ����       0   �
S r���e    A�A�A�N PA�A�A� (   �
S ����:    A�A�C rA�A�   <   S ����w   A�A�A�A�F�hA�A�A�A�     \S ���5       0   pS 	����    A�A�A�C �A�A�A� <   �S y����   A�A�A�F�F��A�A�A�A�     �S  ���          �S ���          S ���           S ���       0   4S ����B    A�A�A�N0mA�A�A�     hS 
���          |S ����          �S ���          �S ޾��          �S Ҿ��          �S ʾ��          �S ����          �S ����          S ����          S ����          0S ����          DS ����          XS ����          lS ~���          �S l���          �S \���          �S L���          �S <���          �S ,���          �S ���          �S 
���          S ����       (    S ���9    A�A�N fA�A�   (   LS ����*    A�A�C bA�A�   8   xS �����    A�A�A�A�C0�A�A�A�A�8   �S T����    A�A�A�A�CP�A�A�A�A�0   �S ����W   A�A�A�CpMA�A�A�(   $S ֿ��I    A�A�C0AA�A�  (   PS ����I    A�A�C0AA�A�     |S ���          �S ���          �S ���          �S ���:    A�x�  (   �S ,���C    A�A�N pA�A�   (    S D���*    A�A�C bA�A�   8   ,S B���   A�A�A�A�C@�A�A�A�A�8   hS 
����    A�A�A�A�C@�A�A�A�A�0   �S �����    A�A�A�C �A�A�A� (   �S F���t    A�A�N aA�A�  (   S ����M    A�A�N zA�A�      0S ����%          DS ����'    Cc    \S ����'    Cc    tS ����%    Ca    �S ����%          �S ���%    Ca    �S ���           �S ���&    A�C `A�8   �S "����    A�A�A�A�C0�A�A�A�A�(   (S v���D    A�A�C |A�A�   0   TS ����G    A�A�A�C }A�A�A�     �S ����7    A�u�  (   �S ����]    A�C�N HA�A�  0   �S �����    A�A�A�N �C�A�A�    S c���1    A�C kA�   $S t���,    A�C fA�P   DS �����    A�A�A�A�C0|
A�A�A�A�ANC�A�A�A�  0   �S ����=    A�A�A�C sA�A�A�  0   �S ����:    A�A�A�C pA�A�A�  (    S ����:    A�A�N gA�A�   8   ,S �����    A�F�A�A�C@�A�A�A�A�8   hS ����L    A�F�A�A�C0{A�A�A�A� <   �S ����
   A�A�A�A�F��A�A�A�A�  8   �S n����    A�A�A�A�C0�A�A�A�A�8    S ���   A�A�A�A�N@�A�A�A�A�    \S ����=    A�A�y�A� 8   �S �����    A�A�A�A�C0�A�A�A�A�(   �S ����D    A�A�C |A�A�      �S ����D    A�C ~A�    S ����L    A�C FA�   (   ,S ����0    A�A�C dA�A�   0   XS �����    A�A�A�C �A�A�A� P   �S j����   A�A�A�A�N0�
A�A�A�A�E�A�A�A�A� <   �S ����.   A�A�A�A�N0A�A�A�A�       S ����5    A�C oA�0   @S �����    A�A�A�C0{A�A�A�    tS ����5    A�H jA�0   �S  ���Z    A�A�A�C PA�A�A� 0   �S &���c    A�F�A�C TA�A�A� 8   �S U����    A�A�A�A�C0�C�A�A�A�(   8S ����t    A�A�N0aA�A�  P   dS ?����    A�A�A�A�C0�
A�A�A�A�ESA�A�A�A�  8   �S ����]    A�A�A�C�N0BC�A�A�A�<   �S ����A   A�A�A�A�C@5A�A�A�A�   (   4S ����|    A�C�HPmA�A�  (   `S ���T    A�C�H0EA�A�  <   �S C����   A�A�A�A�N�rA�A�A�A�     �S ����          �S ����          �S ����"    A�N QA�8   S �����    A�A�A�A�N0�C�A�A�A�(   PS ����    A�A�N �A�A�     |S ����$    A�C ^A�(   �S �����    A�A�C �A�A�     �S ���-    R�N KA�<   �S ����    A�C�A�A�C@h
A�A�A�A�A <   (S h����   A�C�A�A�C`~A�A�A�A�   0   hS ����{    A�C�A�N0dA�A�A� <   �S �����   A�A�A�C�EP�A�A�A�A�   <   �S _���j   A�A�A�A�Cp\C�A�A�A�   <   S ����2   A�C�A�A�