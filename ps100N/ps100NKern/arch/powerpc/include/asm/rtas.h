#ifndef _POWERPC_RTAS_H
#define _POWERPC_RTAS_H
#ifdef __KERNEL__

#include <linux/spinlock.h>
#include <asm/page.h>

/*
 * Definitions for talking to the RTAS on CHRP machines.
 *
 * Copyright (C) 2001 Peter Bergner
 * Copyright (C) 2001 PPC 64 Team, IBM Corp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#define RTAS_UNKNOWN_SERVICE (-1)
#define RTAS_INSTANTIATE_MAX (1ULL<<30) /* Don't instantiate rtas at/above this value */

/* Buffer size for ppc_rtas system call. */
#define RTAS_RMOBUF_MAX (64 * 1024)

/* RTAS return status codes */
#define RTAS_NOT_SUSPENDABLE	-9004
#define RTAS_BUSY		-2    /* RTAS Busy */
#define RTAS_EXTENDED_DELAY_MIN	9900
#define RTAS_EXTENDED_DELAY_MAX	9905

/*
 * In general to call RTAS use rtas_token("string") to lookup
 * an RTAS token for the given string (e.g. "event-scan").
 * To actually perform the call use
 *    ret = rtas_call(token, n_in, n_out, ...)
 * Where n_in is the number of input parameters and
 *       n_out is the number of output parameters
 *
 * If the "string" is invalid on this system, RTAS_UNKNOWN_SERVICE
 * will be returned as a token.  rtas_call() does look for this
 * token and error out gracefully so rtas_call(rtas_token("str"), ...)
 * may be safely used for one-shot calls to RTAS.
 *
 */

typedef u32 rtas_arg_t;

struct rtas_args {
	u32 token;
	u32 nargs;
	u32 nret; 
	rtas_arg_t args[16];
	rtas_arg_t *rets;     /* Pointer to return values in args[]. */
};  

struct rtas_t {
	unsigned long entry;		/* physical address pointer */
	unsigned long base;		/* physical address pointer */
	unsigned long size;
	raw_spinlock_t lock;
	struct rtas_args args;
	struct device_node *dev;	/* virtual address pointer */
};

/* RTAS event classes */
#define RTAS_INTERNAL_ERROR		0x80000000 /* set bit 0 */
#define RTAS_EPOW_WARNING		0x40000000 /* set bit 1 */
#define RTAS_POWERMGM_EVENTS		0x20000000 /* set bit 2 */
#define RTAS_HOTPLUG_EVENTS		0x10000000 /* set bit 3 */
#define RTAS_IO_EVENTS			0x08000000 /* set bit 4 */
#define RTAS_EVENT_SCAN_ALL_EVENTS	0xffffffff

/* RTAS event severity */
#define RTAS_SEVERITY_FATAL		0x5
#define RTAS_SEVERITY_ERROR		0x4
#define RTAS_SEVERITY_ERROR_SYNC	0x3
#define RTAS_SEVERITY_WARNING		0x2
#define RTAS_SEVERITY_EVENT		0x1
#define RTAS_SEVERITY_NO_ERROR		0x0

/* RTAS event disposition */
#define RTAS_DISP_FULLY_RECOVERED	0x0
#define RTAS_DISP_LIMITED_RECOVERY	0x1
#define RTAS_DISP_NOT_RECOVERED		0x2

/* RTAS event initiator */
#define RTAS_INITIATOR_UNKNOWN		0x0
#define RTAS_INITIATOR_CPU		0x1
#define RTAS_INITIATOR_PCI		0x2
#define RTAS_INITIATOR_ISA		0x3
#define RTAS_INITIATOR_MEMORY		0x4
#define RTAS_INITIATOR_POWERMGM		0x5

/* RTAS event target */
#define RTAS_TARGET_UNKNOWN		0x0
#define RTAS_TARGET_CPU			0x1
#define RTAS_TARGET_PCI			0x2
#define RTAS_TARGET_ISA			0x3
#define RTAS_TARGET_MEMORY		0x4
#define RTAS_TARGET_POWERMGM		0x5

/* RTAS event type */
#define RTAS_TYPE_RETRY			0x01
#define RTAS_TYPE_TCE_ERR		0x02
#define RTAS_TYPE_INTERN_DEV_FAIL	0x03
#define RTAS_TYPE_TIMEOUT		0x04
#define RTAS_TYPE_DATA_PARITY		0x05
#define RTAS_TYPE_ADDR_PARITY		0x06
#define RTAS_TYPE_CACHE_PARITY		0x07
#define RTAS_TYPE_ADDR_INVALID		0x08
#define RTAS_TYPE_ECC_UNCORR		0x09
#define RTAS_TYPE_ECC_CORR		0x0a
#define RTAS_TYPE_EPOW			0x40
#define RTAS_TYPE_PLATFORM		0xE0
#define RTAS_TYPE_IO			0xE1
#define RTAS_TYPE_INFO			0xE2
#define RTAS_TYPE_DEALLOC		0xE3
#define RTAS_TYPE_DUMP			0xE4
/* I don't add PowerMGM events right now, this is a different topic */ 
#define RTAS_TYPE_PMGM_POWER_SW_ON	0x60
#define RTAS_TYPE_PMGM_POWER_SW_OFF	0x61
#define RTAS_TYPE_PMGM_LID_OPEN		0x62
#define RTAS_TYPE_PMGM_LID_CLOSE	0x63
#define RTAS_TYPE_PMGM_SLEEP_BTN	0x64
#define RTAS_TYPE_PMGM_WAKE_BTN		0x65
#define RTAS_TYPE_PMGM_BATTERY_WARN	0x66
#define RTAS_TYPE_PMGM_BATTERY_CRIT	0x67
#define RTAS_TYPE_PMGM_SWITCH_TO_BAT	0x68
#define RTAS_TYPE_PMGM_SWITCH_TO_AC	0x69формація профілю Облікові записи  Місцезнаходження Безпека I}Устан. Моє місцезн., розблок. екрана, блок. SIM-карти, сховища обл. даних :dУстан. Моє місцезн., розбл. екрана, блок. схов. обл. даних Паролі 
Шифрування !Шифрувати телефон ����Ви можете шифрувати свої облікові записи, налаштування, завантажені додатки та їх дані, медіа-файли й інші файли. Якщо ви встановили блокування екрана (тобто ключ, числовий PIN-код або пароль) і зашифрували телефон, для його розшифрування потрібно буде розблоковувати екран під час кожного ввімкнення телефона. Розшифрувати телефон також можна шляхом відновлення заводських налаштувань, але в такому випадку буде стерто всі дані.

Шифрування триває не менше години. Перш ніж почати шифрування, переконайтеся, що акумулятор заряджено. Не можна відключати телефон до завершення процесу. Якщо перервати шифрування, буде втрачено деякі або всі дані. 'IЗарядіть акумулятор і повторіть спробу. 0ZПідключіть зарядний пристрій і повторіть спробу. 8eПотрібно ввести PIN-код або пароль для блокування екрана T��Щоб почати шифрування, потрібно встановити PIN-код або пароль для блокування екрана. B|Щоб підтвердити шифрування пристрою, намалюйте ключ розблокування. Зашифрувати? ���-Операція шифрування незворотна. Якщо її перервати, ви втратите дані. Шифрування триває не менше години, протягом чого ваш телефон перезавантажиться декілька разів. 7bЗачекайте, поки ваш телефон зашифрується. Виконано ^1%. AuЗачекайте, поки ваш телефон буде зашифровано. Залишилося часу: ^1 -Спробуйте ще через ^1 сек. d��Застереження. Після кількох невдалих спроб (^1) розблокувати пристрій із нього буде стерто всі дані. Введіть пароль *Шифрування не виконано �O�dШифрування перервано, його неможливо закінчити. Як наслідок, дані на телефоні більше не доступні. 

Щоб знову користуватися телефоном, потрібно відновити заводські налаштування. Налаштовуючи телефон після відновлення, ви матимете можливість відновити будь-які дані, резервні копії яких було збережено у вашому обліковому записі Google. 'Помилка дешифрування ��Ви ввели правильний пароль, але ваші дані пошкоджено. 

Щоб знову користуватися телефоном, відновіть заводські налаштування. Коли ви налаштовуватимете телефон після відновлення заводських налаштувань, ви зможете відновити всі дані, які зберігаються у вашому обліковому записі Google. 2Переключити метод введення !Блокування екрана 'Вибер. резерв. блокув. )Змінити блокув. екрана 0UЗмінити чи вимкнути кл�