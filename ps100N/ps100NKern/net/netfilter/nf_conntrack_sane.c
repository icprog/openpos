		}
			break;

		case 0x0b:
			/* tag 'Inputs': according to Hauppauge this is specific
			to each driver family, so no good assumptions can be
			made. */
			break;

		/* case 0x0c: tag 'Balun' */
		/* case 0x0d: tag 'Teletext' */

		case 0x0e:
			/* tag: 'Radio' */
			tvee->has_radio = eeprom_data[i+1];
			break;

		case 0x0f:
			/* tag 'IRInfo' */
			tvee->has_ir = 1 | (eeprom_data[i+1] << 1);
			break;

		/* case 0x10: tag 'VBIInfo' */
		/* case 0x11: tag 'QCInfo' */
		/* case 0x12: tag 'InfoBits' */

		default:
			tveeprom_dbg("Not sure what to do with tag [%02x]\n",
					tag);
			/* dump the rest of the packet? */
		}
	}

	if (!done) {
		tveeprom_warn("Ran out of data!\n");
		return;
	}

	if (tvee->revision != 0) {
		tvee->rev_str[0] = 32 + ((tvee->revision >> 18) & 0x3f);
		tvee->rev_str[1] = 32 + ((tvee->revision >> 12) & 0x3f);
		tvee->rev_str[2] = 32 + ((tvee->revision >>  6) & 0x3f);
		tvee->rev_str[3] = 32 + (tvee->revision & 0x3f);
		tvee->rev_str[4] = 0;
	}

	if (hasRadioTuner(tuner1) && !tvee->has_radio) {
		tveeprom_info("The eeprom says no radio is present, but the tuner type\n");
		tveeprom_info("indicates otherwise. I will assume that radio is present.\n");
		tvee->has_radio = 1;
	}

	if (tuner1 < sizeof(hauppauge_tuner)/sizeof(struct HAUPPAUGE_TUNER)) {
		tvee->tuner_type = hauppauge_tuner[tuner1].id;
		t_name1 = hauppauge_tuner[tuner1].name;
	} else {
		t_name1 = "unknown";
	}

	if (tuner2 < sizeof(hauppauge_tuner)/sizeof(struct HAUPPAUGE_TUNER)) {
		tvee->tuner2_type = hauppauge_tuner[tuner2].id;
		t_name2 = hauppauge_tuner[tuner2].name;
	} else {
		t_name2 = "unknown";
	}

	tvee->tuner_hauppauge_model = tuner1;
	tvee->tuner2_hauppauge_model = tuner2;
	tvee->tuner_formats = 0;
	tvee->tuner2_formats = 0;
	for (i = j = 0; i < 8; i++) {
		if (t_format1 & (1 << i)) {
			tvee->tuner_formats |= hauppauge_tuner_fmt[i].id;
			t_fmt_name1[j++] = hauppauge_tuner_fmt[i].name;
		}
	}
	for (i = j = 0; i < 8; i++) {
		if (t_format2 & (1 << i)) {
			tvee->tuner2_formats |= hauppauge_tuner_fmt[i].id;
			t_fmt_name2[j++] = hauppauge_tuner_fmt[i].name;
		}
	}

	tveeprom_info("Hauppauge model %d, rev %s, serial# %d\n",
		tvee->model, tvee->rev_str, tvee->serial_number);
	if (tvee->has_MAC_address == 1)
		tveeprom_info("MAC address is %02X-%02X-%02X-%02X-%02X-%02X\n",
			tvee->MAC_address[0], tvee->MAC_address[1],
			tvee->MAC_address[2], tvee->MAC_address[3],
			tvee->MAC_address[4], tvee->MAC_address[5]);
	tveeprom_info("tuner model is %s (idx %d, type %d)\n",
		t_name1, tuner1, tvee->tuner_type);
	tveeprom_info("TV standards%s%s%s%s%s%s%s%s (eeprom 0x%02x)\n",
		t_fmt_name1[0], t_fmt_name1[1], t_fmt_name1[2],
		t_fmt_name1[3],	t_fmt_name1[4], t_fmt_name1[5],
		t_fmt_name1[6], t_fmt_name1[7],	t_format1);
	if (tuner2)
		tveeprom_info("second tuner model is %s (idx %d, type %d)\n",
					t_name2, tuner2, tvee->tuner2_type);
	if (t_format2)
		tveeprom_info("TV standards%s%s%s%s%s%s%s%s (eeprom 0x%02x)\n",
			t_fmt_name2[0], t_fmt_name2[1], t_fmt_name2[2],
			t_fmt_name2[3],	t_fmt_name2[4], t_fmt_name2[5],
			t_fmt_name2[6], t_fmt_name2[7], t_format2);
	if (audioic < 0) {
		tveeprom_info("audio processor is unknown (no idx)\n");
		tvee->audio_processor = V4L2_IDENT_UNKNOWN;
	} else {
		if (audioic < ARRAY_SIZE(audioIC))
			tveeprom_info("audio processor is %s (idx %d)\n",
					audioIC[audioic].name, audioic);
		else
			tveeprom_info("audio processor is unknown (idx %d)\n",
								audioic);
	}
	if (tvee->decoder_processor)
		tveeprom_info("decoder processor is %s (idx %d)\n",
			STRM(decoderIC, tvee->decoder_processor),
			tvee->decoder_processor);
	if (tvee->has_ir)
		tveeprom_info("has %sradio, has %sIR receiver, has %sIR transmitter\n",
				tvee->has_radio ? "" : "no ",
				(tvee->has_ir & 2) ? "" : "no ",
				(tvee->has_ir & 4) ? "" : "no ");
	else
		tveeprom_info("has %sradio\n",
				tvee->has_radio ? "" : "no ");
}
EXPORT_SYMBOL(tveeprom_hauppauge_analog);

/* ----------------------------------------------------------------------- */
/* generic helper functions                                                */

int tveeprom_read(struct i2c_client *c, unsigned char *eedata, int len)
{
	unsigned char buf;
	int err;

	buf = 0;
	err = i2c_master_send(c, &buf, 1);
	if (err != 1) {
		tveeprom_info("Huh, no eeprom present (err=%d)?\n", err);
		return -1;
	}
	err = i2c_master_recv(c, eedata, len);
	if (err != len) {
		tveeprom_warn("i2c eeprom read error (err=%d)\n", err);
		return -1;
	}
	if (debug) {
		int i;

		tveeprom_info("full 256-byte eeprom dump:\n");
		for (i = 0; i < len; i++) {
			if (0 == (i % 16))
				tveeprom_info("%02x:", i);
			printk(KERN_CONT " %02x", eedata[i]);
			if (15 == (i % 16))
				printk(KERN_CONT "\n");
		}
	}
	return 0;
}
EXPORT_SYMBOL(tveeprom_read);

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   