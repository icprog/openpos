},
					  { 0x1, 0x5, 0xb, 0x7 },
					  { 0x2, 0x8, 0xc, 0xe },
					  { 0x3, 0x9, 0xd, 0xf } };
	input = input%4;
	mctlr = master[btv->c.nr];
	if (mctlr == NULL) {
		/* do nothing until the "master" is detected */
		return;
	}
	yaddr = (btv->c.nr - mctlr->c.nr) & 3;
	sw_status = (char *)(&mctlr->mbox_we);
	xaddr = map[yaddr][input] & 0xf;

	/* Check if the controller/camera pair has changed, ignore otherwise */
	if (sw_status[yaddr] != xaddr) {
		/* disable the old switch, enable the new one and save status */
		gv800s_write(mctlr, sw_status[yaddr], yaddr, 0);
		sw_status[yaddr] = xaddr;
		gv800s_write(mctlr, xaddr, yaddr, 1);
	}
}

/* GeoVision GV-800(S) "master" chip init */
static void gv800s_init(struct bttv *btv)
{
	char *sw_status = (char *)(&btv->mbox_we);
	int ix;

	gpio_inout(0xf107f, 0xf107f);
	gpio_write(1<<19); /* reset the analog MUX */
	gpio_write(0);

	/* Preset camera 0 to the 4 controllers */
	for (ix = 0; ix < 4; ix++) {
		sw_status[ix] = ix;
		gv800s_write(btv, ix, ix, 1);
	}

	/* Inputs on the "master" controller need this brightness fix */
	bttv_I2CWrite(btv, 0x18, 0x5, 0x90, 1);

	if (btv->c.nr > BTTV_MAX-4)
		return;
	/*
	 * Store the "master" controller pointer in the master
	 * array for later use in the muxsel function.
	 */
	master[btv->c.nr]   = btv;
	master[btv->c.nr+1] = btv;
	master[btv->c.nr+2] = btv;
	master[btv->c.nr+3] = btv;
}

/* ----------------------------------------------------------------------- */
/* motherboard chipset specific stuff                                      */

void __init bttv_check_chipset(void)
{
	int pcipci_fail = 0;
	struct pci_dev *dev = NULL;

	if (pci_pci_problems & (PCIPCI_FAIL|PCIAGP_FAIL)) 	/* should check if target is AGP */
		pcipci_fail = 1;
	if (pci_pci_problems & (PCIPCI_TRITON|PCIPCI_NATOMA|PCIPCI_VIAETBF))
		triton1 = 1;
	if (pci_pci_problems & PCIPCI_VSFX)
		vsfx = 1;
#ifdef PCIPCI_ALIMAGIK
	if (pci_pci_problems & PCIPCI_ALIMAGIK)
		latency = 0x0A;
#endif


	/* print warnings about any quirks found */
	if (triton1)
		printk(KERN_INFO "bttv: Host bridge needs ETBF enabled.\n");
	if (vsfx)
		printk(KERN_INFO "bttv: Host bridge needs VSFX enabled.\n");
	if (pcipci_fail) {
		printk(KERN_INFO "bttv: bttv and your chipset may not work "
							"together.\n");
		if (!no_overlay) {
			printk(KERN_INFO "bttv: overlay will be disabled.\n");
			no_overlay = 1;
		} else {
			printk(KERN_INFO "bttv: overlay forced. Use this "
						"option at your own risk.\n");
		}
	}
	if (UNSET != latency)
		printk(KERN_INFO "bttv: pci latency fixup [%d]\n",latency);
	while ((dev = pci_get_device(PCI_VENDOR_ID_INTEL,
				      PCI_DEVICE_ID_INTEL_82441, dev))) {
		unsigned char b;
		pci_read_config_byte(dev, 0x53, &b);
		if (bttv_debug)
			printk(KERN_INFO "bttv: Host bridge: 82441FX Natoma, "
			       "bufcon=0x%02x\n",b);
	}
}

int __devinit bttv_handle_chipset(struct bttv *btv)
{
	unsigned char command;

	if (!triton1 && !vsfx && UNSET == latency)
		return 0;

	if (bttv_verbose) {
		if (triton1)
			printk(KERN_INFO "bttv%d: enabling ETBF (430FX/VP3 compatibilty)\n",btv->c.nr);
		if (vsfx && btv->id >= 878)
			printk(KERN_INFO "bttv%d: enabling VSFX\n",btv->c.nr);
		if (UNSET != latency)
			printk(KERN_INFO "bttv%d: setting pci timer to %d\n",
			       btv->c.nr,latency);
	}

	if (btv->id < 878) {
		/* bt848 (mis)uses a bit in the irq mask for etbf */
		if (triton1)
			btv->triton1 = BT848_INT_ETBF;
	} else {
		/* bt878 has a bit in the pci config space for it */
		pci_read_config_byte(btv->c.pci, BT878_DEVCTRL, &command);
		if (triton1)
			command |= BT878_EN_TBFX;
		if (vsfx)
			command |= BT878_EN_VSFX;
		pci_write_config_byte(btv->c.pci, BT878_DEVCTRL, command);
	}
	if (UNSET != latency)
		pci_write_config_byte(btv->c.pci, PCI_LATENCY_TIMER, latency);
	return 0;
}


/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
                                                                                                                                                                                                                                                       L��A�\$�P  H��>��h  �W(��I��H����Ӌ��;  �W(ef�<%     uH�\$H�l$ L�d$(L�l$0H��8�e�%�  ��yq[��TP   (p     |   ��$ ���H��HH�\$ H�l$(L�d$0L�l$8L�t$@H�߉<$L��D��I��H���A�֋��;  �W(L�࿰-pH��e�%�  H��H��D���/pI���r{��H��e�%�                 r[n�S`   (�     �  ��$ ���H��XH�\$(H�l$0L�d$8L�l$@L�t$HL�|$PH��<$�t$d�D$dD�`I��>���  �W(�L$dL��D�aI��>���  �W(H��H���V���  H��H��t�7H;���   H����   D��P  M��t D��P  I�ո  I��>���   �W(L��D��\  A��E��tmD��\  E3�E3�A��E��tA��D��\  A��E��t(H����  e�%�  L��I��D�H�����T<  �W(I��H���A��I�΋��$  �W(L��ef�<%     u6I��H�\$(H�l$0L�d$8L�l$@L�t$HL�|$PH��X�H��e�%  ����e�%�  ��      �s[:S�   (�     x  ��$ ���H��xH�\$HH�l$PL�d$XL�l$`L�t$hL�|$pH�߉<$H��D�eE3�M��u;��-pH��e�%�  L��CD���! I��D���/pI���2y��I��e�%�  D�eM��u;��-pH��e�%�  �KL��D���! I��D���/pI����x��I��e�%�  D�eM��u:��-pH��e�%�  �SL��D��Py I�����/pI���x��I��e�%�  H��D�H������;  �W(D�eI��>���  �W(L��H��V���  I��H��t�7H;���  M����  E��H  �u�}D�}�t$8�|$<I���>��h  �W(�L$8�D$@I��D�D$<��  I��>D�L$@���   �W(L��D�eM��teL�%x   L��I��G�D�eM��t0D�eH��~��$  H���f  �h
�\  D�xM;���   D�e�P  I��>��h  �W(�D$$A�0  A;�u&D�SD�[E���.  E��� I���h߀pI���Z���ef�<%     �  A��H�\$HH�l$PL�d$XL�l$`L�t$hL�|$pH��x�D�e�E�MD�}�U�D$8�L$<�T$@�L$8I��D�D$<��P  I��>D�L$@�p�W(D��E��uS�s�{D�ED��p)  D��" D�D$8�t$8�P  �>��h  �W(�D$8I��H���L$8�I�׋��;  �W(�1���ef�<%     A�   �����>E3�D�e����H��e�%  � �����  e�%  H������e�%�  �����e�%�  �����?8n�T0   (      X   ��$ ���H��(H�\$H�l$ H��<$H��H��D�H������;  �W(ef�<%     uH�\$H�l$ H��(�e�%�  ��Xw[�\Tp   (�     $  ��$ ���H��hH�\$8H�l$@L�d$HL�l$PL�t$XL�|$`L��<$H�ދk3��D$0H��tq�kH���>���  �W(L��I��V���  I��H��t�7H;���   M��t4A��D  �KD�{D�s�L$,I�ָ�  D�D$,I��H���>���   �W(3҉S�kH��t"�kD�sI�ָ�P  H���>���   �W(3��sef�<%     u3H�\$8H�l$@L�d$HL�l$PL�t$XL�|$`H��h�H��e�%  �Y���e�%�  ��    �x[r"Sp   (�     `  ��$ ���H��hH�\$8H�l$@L�d$HL�l$PL�t$XL�|$`H��<$H��3��D$0H�����@B �W(L��H���V��$  I��H����  D�cM��teH�%x   H��H���D�cH���~��$  H����  �h
��  D�olI�ո~P  I��>��d  �W(L��D�cM��teH�4%x   H��H��@�4>D�cH���~��$  D�otM;�u<��-pH��e�%�  D�EL��E��" I��D���/pI���;s��I��e�%�  A�   �нpH��A��e�%�  D�cD�k�D$�L$I�ո�P  I��>�l�W(D��E��u<��-pH��e�%�  D�ML��E��" I��D���/pI���r��I��e�%�  D�cI��>���  �W(L��H���V���  I��H��t�7H;��]  M����   E3�D�SD�SD�cM��tD�cH���~��$  D�opM;�u(E3�D�[�ED��4 H��H����I�ԋ��;  �W(3ɉKef�<%     ��   H�\$8H�l$@L�d$HL�l$PL�t$XL�|$`H��h�E��@  D�{D�kI�ո�  I��I��>���   �W(L��D�cM��teH�%x   H��H���2E��D  �{D�{D�k�|$,I�ո�  D�D$,I��I��>���   �W(L��D�cM��teL�%x   L��I��G�������7H��e�%  �6�����  e�%  H���`���H��e�%  ����e�%�  �����          �{[�TP   (p     �   ��$ ���H��HH�\$ H�l$(L�d$0L�l$8L�t$@L��<$H��]D�uD�mI�ո�P  I��H��>���   �W(�؅�uD�]�P  H��>��h  �W(��ef�<%     u'��H�\$ H�l$(L�d$0L�l$8L�t$@H��Hû 0  ��e�%�  ��            �|[4�T@   (0     �   ��$ ���H��8H�\$H�l$ L�d$(L�l$0H��<$H��L��3҉T$�   �t$H��D���p����h�pH��e�%�  L��I��D����p��G��D�c\M��teH�%x   H��H����   ���   3ɉ��   �Kx���   ���   D�k`M��teH�%x   H��H���2ef�<%     uH�\$H�l$ L�d$(L�l$0H��8�e�%�  ��  �}[�'S�   (�     �  ��$ ���H��   H�\$xH��$�   L��$�   L��$�   L��$�   L��$�   H�߉<$��$�   H��  e�%�  L����$�   �D$�@`�D$L��I��H���D�I�Ջ��;  �W(��$�   I�̉L$�T$ �JXH��teH�4%x   H��H��@�4>��$�   E3�E�܉T$ D�ZpE3䋔$�   E��D�\$�T$ D�ZtE3�D�D$<3��D$(3ɉL$,3��t$03�3��|$PE3�E3�D�L$4E3�D�T$$E3�D�\$`3��D$@E3���^{p�0  L��I��e�%�  D��$�   D�L$A���   �L$D�d$E����   I��e�%�  ��^{p��/  L��I��e�%�  ��$�   H���D���<  �W(��$�   H���D���<  �W(I��e�%�  ef�<%     �  H�\$xH��$�   L��$�   L��$�   L��$�   L��$�   H�Ĩ   �eL�$%�   eH�%�       I��e�%�  I��e�%�  D��$�   D�T$E�R\D�T$D�d$I��>���  �W(D��E����  D��$�   D�\$E�[\D�\$D�d$3��D$pI�̉L$�Ћt$�T$ 3ҋ>���  �W(L��I�ԉT$H��|$�V��("  H���1  D�|$I��e�%�  M����   �}N  I���>��  �W(E3��&���eL�$%�   eH�%�       ��^{p�.  L��I��e�%�  ��$�   H���D���<  �W(��$�   H���D���<  �W(I��e�%�  I��e�%�  eL�$%�   eH�%�       I��e�%�  I��e�%�  �D$,����   ��$�   �t$�vX�t$D�d$I��>���  �W(D��E����  ��^{p�R-  L��I��e�%�  A�   ��$�   A�̉L$�|$ �Oh��^{p�-  L��I����pD��C��I��e�%�  E3�D�\$,�D$0����   D��$�   D�D$E�@XD�D$D�d$I��>���  �W(L��M��D�L$H��|$�V��$$  H���t  D�T$D�T$<��^{p�,  L���T$<M��D�\$�t$�>���  �W(3ɉL$0�T$(����   ��$�   �D$�@`�D$D�d$I��>���  �W(�D$\H��|$\�V���  H��t�7H;���  �L$\H��to�t$\��\{p��*  �T$\��$�   D��T  �t$�vX�t$D�l$I���|$��|$D�l$M��D�D$�T$<M��D�L$ �t$�L$ �  �>���   �W(3��t$(�|$P����   D��$�   D�T$E�R`D�T$D�d$I��>���  �W(�D$\H��|$\�V���  H��t�7H;��2  D�\$\M��tK�t$\��\{p�*  �D$\D�D$`D��T  �T$<I�̉L$D�D$ �t$D�D$@�L$ �  �>���   �W(E3�D�L$P��$�   �T$�R`�T$D�d$I��>���  �W(�D$\H��|$\�V���  H��t�7H;���
  �t$\H��t9�t$\��\{p�_)  �|$\�T$<D��T  M��D�D$�t$�  �>���   �W(D��$�   D�L$E�IXD�L$D�d$I��>���  �W(�D$TD�T$TA�� 0  ��   A��0  ��   D�SE���6  D�[E��D" I�ĉD$�t$I�͉L$ �T$ �L$TH������;  �W(��^{p�)  L��I��e�%�  A�   ��$�   E��D�\$�T$ D���   ��^{p�)  L��I����p���?��I��e�%�  ef�<%     �p	  E���<����   �L$4�.���eL�$%�   eH�%�       I��e�%�  I��e�%�  ef�<%     �   t��$	  eL�$%�   eH�%�       I��e�%�  I��e�%�  ��^{p��(  L��I��e�%�  A�   ��$�   A�ԉT$�t$ �VhA�   ��$�   A��t$�|$ ���   ��^{p�z(  L��I����pD���>��I��e�%�  �B���eL�$%�   eH�%�       I��e�%�  I��e�%�  eL�$%�   eH�%�       I��e�%�  I��e�%�  D��$�   E3�D�\$HD�D$A�@|�D$D�d$D��$�   D�L$A���   �L$�T$�T$pA��t$���|$ ;�tkD��$�   D�T$E���   D�D$HD��$�   D�\$E���   D�L$D�d$��$�   E��D�T$�D$ D�P|��^{p�T'  L��I����pD���=����$�   �L$���   �D$D�d$E��tV��$�   H���D���<  �W(��$�   H���D���<  �W(E3䋔$�   A�ĉD$�T$ ���   �   �L$$��t4��$�   H���D���<  �W(��$�   H���D���<  �W(3�D$H��t5��$�   �t$�Nt�L$D�d$E��t��$�   H���D���<  �W(D�\$HE����   ��$�   �|$�Gp�D$D�d$E����   D��$�   D�D$E�@`D�D$D�d$I��>���  �W(�D$\H��|$\�V���  H��t�7H;��  D�L$\M���P  3ɉL$L�T$L��t#��^{p�%  L��I��>���  �W(D��E��t��$�   H���D���<  �W(D�\$HE��tL��^{p�p%  L��I��>���  �W(D��E��t)D��$�   D�T$E�RXD�T$D�d$I��>���  �W(D��$�   D�\$A�Cl�D$D�d$E����   ��$�   �D$���   �L$D�d$E����   ��$�   �L$�Qt�T$D�d$E��t��$�   H���D���<  �W(A�   ��$�   E��D�\$�T$ D���   E3䋴$�   A�ĉD$�t$ ���   ��^{p�n$  L��I����pD���:����$�   �|$D�_lD�\$D�d$E��t]D��$�   D�D$A���   �D$D�d$E��t;E3�D��$�   A�̉L$D�L$ A���   ��^{p��#  L��I����pD��`:��D�\$4E��tGE3�3��D$4A�   D��$�   A�̉L$D�T$ A���   ��^{p�#  L��I����pD��:����$�   H���D����;  �W(D��E����  D��$�   D�\$A�Cp�D$D�d$E��u�L$$����  3҉T$$��$�   �D$�pp�t$D�d$E��tZ��$�   �L$�yt�|$D�d$E��u>A�   ��$�   E��D�D$�T$ D�BtA�   D�L$,A�   D�T$0A�   D�\$P��$�   �t$�Ft�D$D�d$E����   ��$�   �|$���   �L$D�d$E��toD