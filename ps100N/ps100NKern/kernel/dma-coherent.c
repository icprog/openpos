tat_count_tx = sizeof(struct igb_tx_queue_stats) / sizeof(u64);
	int stat_count_rx = sizeof(struct igb_rx_queue_stats) / sizeof(u64);
	int j;
	int i;

	igb_update_stats(adapter);
	for (i = 0; i < IGB_GLOBAL_STATS_LEN; i++) {
		char *p = (char *)adapter+igb_gstrings_stats[i].stat_offset;
		data[i] = (igb_gstrings_stats[i].sizeof_stat ==
			sizeof(u64)) ? *(u64 *)p : *(u32 *)p;
	}
	for (j = 0; j < adapter->num_tx_queues; j++) {
		int k;
		queue_stat = (u64 *)&adapter->tx_ring[j].tx_stats;
		for (k = 0; k < stat_count_tx; k++)
			data[i + k] = queue_stat[k];
		i += k;
	}
	for (j = 0; j < adapter->num_rx_queues; j++) {
		int k;
		queue_stat = (u64 *)&adapter->rx_ring[j].rx_stats;
		for (k = 0; k < stat_count_rx; k++)
			data[i + k] = queue_stat[k];
		i += k;
	}
}

static void igb_get_strings(struct net_device *netdev, u32 stringset, u8 *data)
{
	struct igb_adapter *adapter = netdev_priv(netdev);
	u8 *p = data;
	int i;

	switch (stringset) {
	case ETH_SS_TEST:
		memcpy(data, *igb_gstrings_test,
			IGB_TEST_LEN*ETH_GSTRING_LEN);
		break;
	case ETH_SS_STATS:
		for (i = 0; i < IGB_GLOBAL_STATS_LEN; i++) {
			memcpy(p, igb_gstrings_stats[i].stat_string,
			       ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < adapter->num_tx_queues; i++) {
			sprintf(p, "tx_queue_%u_packets", i);
			p += ETH_GSTRING_LEN;
			sprintf(p, "tx_queue_%u_bytes", i);
			p += ETH_GSTRING_LEN;
		}
		for (i = 0; i < adapter->num_rx_queues; i++) {
			sprintf(p, "rx_queue_%u_packets", i);
			p += ETH_GSTRING_LEN;
			sprintf(p, "rx_queue_%u_bytes", i);
			p += ETH_GSTRING_LEN;
			sprintf(p, "rx_queue_%u_drops", i);
			p += ETH_GSTRING_LEN;
		}
/*		BUG_ON(p - data != IGB_STATS_LEN * ETH_GSTRING_LEN); */
		break;
	}
}

static struct ethtool_ops igb_ethtool_ops = {
	.get_settings           = igb_get_settings,
	.set_settings           = igb_set_settings,
	.get_drvinfo            = igb_get_drvinfo,
	.get_regs_len           = igb_get_regs_len,
	.get_regs               = igb_get_regs,
	.get_wol                = igb_get_wol,
	.set_wol                = igb_set_wol,
	.get_msglevel           = igb_get_msglevel,
	.set_msglevel           = igb_set_msglevel,
	.nway_reset             = igb_nway_reset,
	.get_link               = ethtool_op_get_link,
	.get_eeprom_len         = igb_get_eeprom_len,
	.get_eeprom             = igb_get_eeprom,
	.set_eeprom             = igb_set_eeprom,
	.get_ringparam          = igb_get_ringparam,
	.set_ringparam          = igb_set_ringparam,
	.get_pauseparam         = igb_get_pauseparam,
	.set_pauseparam         = igb_set_pauseparam,
	.get_rx_csum            = igb_get_rx_csum,
	.set_rx_csum            = igb_set_rx_csum,
	.get_tx_csum            = igb_get_tx_csum,
	.set_tx_csum            = igb_set_tx_csum,
	.get_sg                 = ethtool_op_get_sg,
	.set_sg                 = ethtool_op_set_sg,
	.get_tso                = ethtool_op_get_tso,
	.set_tso                = igb_set_tso,
	.self_test              = igb_diag_test,
	.get_strings            = igb_get_strings,
	.phys_id                = igb_phys_id,
	.get_sset_count         = igb_get_sset_count,
	.get_ethtool_stats      = igb_get_ethtool_stats,
	.get_coalesce           = igb_get_coalesce,
	.set_coalesce           = igb_set_coalesce,
};

void igb_set_ethtool_ops(struct net_device *netdev)
{
	SET_ETHTOOL_OPS(netdev, &igb_ethtool_ops);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 /*
    Auvitek AU8522 QAM/8VSB demodulator driver

    Copyright (C) 2008 Steven Toth <stoth@linuxtv.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that i