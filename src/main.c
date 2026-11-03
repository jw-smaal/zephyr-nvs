/*
 * Copyright 2026 Jan-Willem Smaal <usenet@gispen.org>
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file main.c
 * @brief Minimal NVRAM write and read-back example for QEMU Cortex-M0
 * @author Jan-Willem Smaal <usenet@gispen.org>
 * @date 2026-11-03
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/kvss/nvs.h>
#include <zephyr/sys/printk.h>

#define STORAGE_NODE     user_nvram_partition
#define NVS_SECTOR_COUNT 2U
#define KEY_TEXT_ID      1U
#define TEXT_MESSAGE     "hello nvram Hallo Hallo"
#define TEXT_BUF_SIZE    32U

int main(void)
{
	struct nvs_fs fs;
	struct flash_pages_info info;
	char read_buf[TEXT_BUF_SIZE];

	fs.flash_device = PARTITION_DEVICE(STORAGE_NODE);
	fs.offset = PARTITION_OFFSET(STORAGE_NODE);
	(void)flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	fs.sector_size = info.size;
	printk("======================\n");
	printk("info.size:%d \n", info.size);
	printk("info.index:%d \n", info.index);
	fs.sector_count = NVS_SECTOR_COUNT;
	printk("fs.sector_size:%d \n", fs.sector_size);

	(void)nvs_mount(&fs);

	/* Write string to NVRAM */
	int ret = nvs_write(&fs, KEY_TEXT_ID, TEXT_MESSAGE, sizeof(TEXT_MESSAGE));
	if (ret < 0 ) {
		printk("Cannot write nvs\n");
	}

	/* Read string back from NVRAM */
	ret = nvs_read(&fs, KEY_TEXT_ID, read_buf, sizeof(read_buf));
	if (ret < 0 ) {
		printk("Cannot read nvs\n");
	}


	printk("read %d bytes\n", ret);
	printk("Read from NVRAM: %s\n", read_buf);
	printk("======================\n");

	return 0;
}
