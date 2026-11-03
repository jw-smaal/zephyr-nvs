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
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nvram_test, LOG_LEVEL_INF);

#define STORAGE_NODE     user_nvram_partition
#define NVS_SECTOR_COUNT 2U
#define KEY_TEXT_ID      1U
#define TEXT_MESSAGE     "hello nvram Hallo Hallo"
#define TEXT_BUF_SIZE    32U
#define HEXDUMP_DATA_LEN 64U
#define HEXDUMP_ATE_LEN  32U

int main(void)
{
	struct nvs_fs fs;
	struct flash_pages_info info;
	char read_buf[TEXT_BUF_SIZE];
	int ret;

	fs.flash_device = PARTITION_DEVICE(STORAGE_NODE);
	fs.offset = PARTITION_OFFSET(STORAGE_NODE);
	(void)flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	fs.sector_size = info.size;
	LOG_INF("======================");
	LOG_INF("info.size:%u", (unsigned int)info.size);
	LOG_INF("info.index:%u", (unsigned int)info.index);
	fs.sector_count = NVS_SECTOR_COUNT;
	LOG_INF("fs.sector_size:%u", (unsigned int)fs.sector_size);

	(void)nvs_mount(&fs);

	/* Write string to NVRAM */
	ret = nvs_write(&fs, KEY_TEXT_ID, TEXT_MESSAGE, sizeof(TEXT_MESSAGE));
	if (ret < 0) {
		LOG_ERR("Cannot write nvs");
	}

	/* Read string back from NVRAM */
	ret = nvs_read(&fs, KEY_TEXT_ID, read_buf, sizeof(read_buf));
	if (ret < 0) {
		LOG_ERR("Cannot read nvs");
	}

	LOG_INF("read %d bytes", ret);
	LOG_INF("Read from NVRAM: %s", read_buf);

	/* Dump first 64 bytes of sector 0 (payload data area at bottom of sector) */
	LOG_HEXDUMP_INF((const void *)fs.offset,
			HEXDUMP_DATA_LEN,
			"NVS Sector 0 Data (bottom)");

	/* Dump last 32 bytes of sector 0 (ATE metadata table at top of sector) */
	LOG_HEXDUMP_INF((const void *)(fs.offset + fs.sector_size - HEXDUMP_ATE_LEN),
			HEXDUMP_ATE_LEN,
			"NVS Sector 0 ATEs (top)");

	LOG_INF("======================");

	return 0;
}
