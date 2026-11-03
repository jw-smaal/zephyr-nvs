/*
 * Copyright 2026 Jan-Willem Smaal <usenet@gispen.org>
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file main.c
 * @brief Simple NVRAM write and read-back test for QEMU Cortex-M0
 * @author Jan-Willem Smaal <usenet@gispen.org>
 * @date 2026-11-03
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/kvss/nvs.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

#define STORAGE_NODE       storage_partition
#define NVS_SECTOR_COUNT   4U
#define KEY_COUNTER_ID     1U
#define KEY_TEXT_ID        2U
#define TEXT_BUF_SIZE      32U
#define REBOOT_WAIT_MS     2000U

int main(void)
{
	struct nvs_fs fs;
	struct flash_pages_info info;
	uint32_t counter = 0U;
	char write_buf[TEXT_BUF_SIZE];
	char read_buf[TEXT_BUF_SIZE];
	ssize_t rc;

	fs.flash_device = PARTITION_DEVICE(STORAGE_NODE);
	if (!device_is_ready(fs.flash_device)) {
		printk("Flash device not ready\n");
		return -ENODEV;
	}

	fs.offset = PARTITION_OFFSET(STORAGE_NODE);
	if (flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info) != 0) {
		printk("Unable to get page info\n");
		return -EIO;
	}

	fs.sector_size = info.size;
	fs.sector_count = NVS_SECTOR_COUNT;

	if (nvs_mount(&fs) != 0) {
		printk("NVS mount failed\n");
		return -EIO;
	}

	/* Read previous counter from NVRAM (starts at 0 if not found) */
	rc = nvs_read(&fs, KEY_COUNTER_ID, &counter, sizeof(counter));
	if (rc < 0) {
		counter = 0U;
	}

	/* Increment and write counter to NVRAM */
	counter = counter + 1U;
	(void)nvs_write(&fs, KEY_COUNTER_ID, &counter, sizeof(counter));

	/* Format "hello<N>" and write it to NVRAM */
	snprintf(write_buf, sizeof(write_buf), "hello%u", (unsigned int)counter);
	(void)nvs_write(&fs, KEY_TEXT_ID, write_buf, strlen(write_buf) + 1U);

	/* Read the string back from NVRAM and print it */
	memset(read_buf, 0, sizeof(read_buf));
	rc = nvs_read(&fs, KEY_TEXT_ID, read_buf, sizeof(read_buf));
	if (rc > 0) {
		printk("%s\n", read_buf);
	}

	k_msleep(REBOOT_WAIT_MS);
	sys_reboot(SYS_REBOOT_COLD);

	return 0;
}
