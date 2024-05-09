// --- includes ----------------------------------------------------------------
#include "flash_system.h"
#include <zephyr/logging/log.h>

// --- logging settings --------------------------------------------------------
LOG_MODULE_REGISTER(flash_system_m);

// --- static variables definitions --------------------------------------------
static struct nvs_fs fs;

// --- functions definitions ---------------------------------------------------
void flash_system_init(void)
{
    int rc = 0;
	struct flash_pages_info info;

	/* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	3 sectors
	 *	starting at NVS_PARTITION_OFFSET
	 */
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		printk("Flash device %s is not ready\n", fs.flash_device->name);
		return;
	}
	fs.offset = NVS_PARTITION_OFFSET;
	rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		printk("Unable to get page info\n");
		return;
	}
	fs.sector_size = info.size;
	fs.sector_count = 3U;

	rc = nvs_mount(&fs);
	if (rc) {
		printk("Flash Init failed\n");
		return;
	}
}

// File system handle getter
struct nvs_fs* get_file_system_handle(void)
{
    return &fs;
}