num_outputs = 1;
	dinfo->output[i].type = INTELFB_OUTPUT_ANALOG;

	/* setup the DDC bus for analog output */
	intelfb_setup_i2c_bus(dinfo, &dinfo->output[i].ddc_bus, GPIOA,
			      "CRTDDC_A", I2C_CLASS_DDC);
	i++;

	/* need to add the output busses for each device
	   - this function is very incomplete
	   - i915GM has LVDS and TVOUT for example
	*/
	switch(dinfo->chipset) {
	case INTEL_830M:
	case INTEL_845G:
	case INTEL_854:
	case INTEL_855GM:
	case INTEL_865G:
		dinfo->output[i].type = INTELFB_OUTPUT_DVO;
		intelfb_setup_i2c_bus(dinfo, &dinfo->output[i].ddc_bus,
				      GPIOD, "DVODDC_D", I2C_CLASS_DDC);
		intelfb_setup_i2c_bus(dinfo, &dinfo->output[i].i2c_bus,
				      GPIOE, "DVOI2C_E", 0);
		i++;
		break;
	case INTEL_915G:
	case INTEL_915GM:
		/* has some LVDS + tv-out */
	case INTEL_945G:
	case INTEL_945GM:
	case INTEL_945GME:
	case INTEL_965G:
	case INTEL_965GM:
		/* SDVO ports have a single control bus - 2 devices */
		dinfo->output[i].type = INTELFB_OUTPUT_SDVO;
		intelfb_setup_i2c_bus(dinfo, &dinfo->output[i].i2c_bus,
				      GPIOE, "SDVOCTRL_E", 0);
		/* TODO: initialize the SDVO */
		/* I830SDVOInit(pScrn, i, DVOB); */
		i++;

		/* set up SDVOC */
		dinfo->output[i].type = INTELFB_OUTPUT_SDVO;
		dinfo->output[i].i2c_bus = dinfo->output[i - 1].i2c_bus;
		/* TODO: initialize the SDVO */
		/* I830SDVOInit(pScrn, i, DVOC); */
		i++;
		break;
	}
	dinfo->num_outputs = i;
}

void intelfb_delete_i2c_busses(struct intelfb_info *dinfo)
{
	int i;

	for (i = 0; i < MAX_OUTPUTS; i++) {
		if (dinfo->output[i].i2c_bus.dinfo) {
			i2c_del_adapter(&dinfo->output[i].i2c_bus.adapter);
			dinfo->output[i].i2c_bus.dinfo = NULL;
		}
		if (dinfo->output[i].ddc_bus.dinfo) {
			i2c_del_adapter(&dinfo->output[i].ddc_bus.adapter);
			dinfo->output[i].ddc_bus.dinfo = NULL;
		}
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           