*tmp;

	printk(KERN_DEBUG
	       "ACPI: Delete PCI Interrupt Routing Table for %04x:%02x\n",
	       pci_domain_nr(bus), bus->number);
	spin_lock(&acpi_prt_lock);
	list_for_each_entry_safe(entry, tmp, &acpi_prt_list, list) {
		if (pci_domain_nr(bus) == entry->id.segment
			&& bus->number == entry->id.bus) {
			list_del(&entry->list);
			kfree(entry);
		}
	}
	spin_unlock(&acpi_prt_lock);
}

/* --------------------------------------------------------------------------
                          PCI Interrupt Routing Support
   -------------------------------------------------------------------------- */
static struct acpi_prt_entry *acpi_pci_irq_lookup(struct pci_dev *dev, int pin)
{
	struct acpi_prt_entry *entry;
	struct pci_dev *bridge;
	u8 bridge_pin, orig_pin = pin;

	entry = acpi_pci_irq_find_prt_entry(dev, pin);
	if (entry) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found %s[%c] _PRT entry\n",
				  pci_name(dev), pin_name(pin)));
		return entry;
	}

	/* 
	 * Attempt to derive an IRQ for this device from a parent bridge's
	 * PCI interrupt routing entry (eg. yenta bridge and add-in card bridge).
	 */
	bridge = dev->bus->self;
	while (bridge) {
		pin = pci_swizzle_interrupt_pin(dev, pin);

		if ((bridge->class >> 8) == PCI_CLASS_BRIDGE_CARDBUS) {
			/* PC card has the same IRQ as its cardbridge */
			bridge_pin = bridge->pin;
			if (!bridge_pin) {
				ACPI_DEBUG_PRINT((ACPI_DB_INFO,
						  "No interrupt pin configured for device %s\n",
						  pci_name(bridge)));
				return NULL;
			}
			pin = bridge_pin;
		}

		entry = acpi_pci_irq_find_prt_entry(bridge, pin);
		if (entry) {
			ACPI_DEBUG_PRINT((ACPI_DB_INFO,
					 "Derived GSI for %s INT %c from %s\n",
					 pci_name(dev), pin_name(orig_pin),
					 pci_name(bridge)));
			return entry;
		}

		dev = bridge;
		bridge = dev->bus->self;
	}

	dev_warn(&dev->dev, "can't derive routing for PCI INT %c\n",
		 pin_name(orig_pin));
	return NULL;
}

int acpi_pci_irq_enable(struct pci_dev *dev)
{
	struct acpi_prt_entry *entry;
	int gsi;
	u8 pin;
	int triggering = ACPI_LEVEL_SENSITIVE;
	int polarity = ACPI_ACTIVE_LOW;
	char *link = NULL;
	char link_desc[16];
	int rc;

	pin = dev->pin;
	if (!pin) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "No interrupt pin configured for device %s\n",
				  pci_name(dev)));
		return 0;
	}

	entry = acpi_pci_irq_lookup(dev, pin);
	if (!entry) {
		/*
		 * IDE legacy mode controller IRQs are magic. Why do compat
		 * extensions always make such a nasty mess.
		 */
		if (dev->class >> 8 == PCI_CLASS_STORAGE_IDE &&
				(dev->class & 0x05) == 0)
			return 0;
	}

	if (entry) {
		if (entry->link)
			gsi = acpi_pci_link_allocate_irq(entry->link,
							 entry->index,
							 &triggering, &polarity,
							 &link);
		else
			gsi = entry->index;
	} else
		gsi = -1;

	/*
	 * No IRQ known to the ACPI subsystem - maybe the BIOS / 
	 * driver reported one, then use it. Exit in any case.
	 */
	if (gsi < 0) {
		dev_warn(&dev->dev, "PCI INT %c: no GSI", pin_name(pin));
		/* Interrupt Line values above 0xF are forbidden */
		if (dev->irq > 0 && (dev->irq <= 0xF)) {
			printk(" - using IRQ %d\n", dev->irq);
			acpi_register_gsi(&dev->dev, dev->irq,
					  ACPI_LEVEL_SENSITIVE,
					  ACPI_ACTIVE_LOW);
			return 0;
		} else {
			printk("\n");
			return 0;
		}
	}

	rc = acpi_register_gsi(&dev->dev, gsi, triggering, polarity);
	if (rc < 0) {
		dev_warn(&dev->dev, "PCI INT %c: failed to register GSI\n",
			 pin_name(pin));
		return rc;
	}
	dev->irq = rc;

	if (link)
		snprintf(link_desc, sizeof(link_desc), " -> Link[%s]", link);
	else
		link_desc[0] = '\0';

	dev_info(&dev->dev, "PCI INT %c%s -> GSI %u (%s, %s) -> IRQ %d\n",
		 pin_name(pin), link_desc, gsi,
		 (triggering == ACPI_LEVEL_SENSITIVE) ? "level" : "edge",
		 (polarity == ACPI_ACTIVE_LOW) ? "low" : "high", dev->irq);

	return 0;
}

/* FIXME: implement x86/x86_64 version */
void __attribute__ ((weak)) acpi_unregister_gsi(u32 i)
{
}

void acpi_pci_irq_disable(struct pci_dev *dev)
{
	struct acpi_prt_entry *entry;
	int gsi;
	u8 pin;

	pin = dev->pin;
	if (!pin)
		return;

	entry = acpi_pci_irq_lookup(dev, pin);
	if (!entry)
		return;

	if (entry->link)
		gsi = acpi_pci_link_free_irq(entry->link);
	else
		gsi = entry->index;

	/*
	 * TBD: It might be worth clearing dev->irq by magic constant
	 * (e.g. PCI_UNDEFINED_IRQ).
	 */

	dev_info(&dev->dev, "PCI INT %c disabled\n", pin_name(pin));
	acpi_unregister_gsi(gsi);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           