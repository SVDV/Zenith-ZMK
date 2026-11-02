/*
 * Battery-drain diagnostic: the left half draws ~385 uA asleep vs a few uA on
 * the right. An external QSPI flash that fails to initialise is never put into
 * deep power-down and draws hundreds of uA, so report its state every 10 s
 * over USB logging (no need to catch the boot messages).
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zenith_qspi_diag, LOG_LEVEL_INF);

#define FLASH_NODE DT_NODELABEL(p25q16h)

#if DT_NODE_HAS_STATUS(FLASH_NODE, okay) && defined(CONFIG_NORDIC_QSPI_NOR)
static const struct device *const flash_dev = DEVICE_DT_GET(FLASH_NODE);
#else
static const struct device *const flash_dev = NULL;
#endif

static void report(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(report_work, report);

static void report(struct k_work *work) {
    if (flash_dev == NULL) {
        LOG_WRN("QSPI DIAG: no QSPI flash driver in this build");
    } else if (!device_is_ready(flash_dev)) {
        LOG_ERR("QSPI DIAG: FLASH NOT READY - init failed at boot");
    } else {
        uint8_t id[3] = {0};
        int err = -ENOTSUP;
#ifdef CONFIG_FLASH_JESD216_API
        err = flash_read_jedec_id(flash_dev, id);
#endif
        LOG_INF("QSPI DIAG: flash ready, JEDEC id %02x %02x %02x (err %d, expected 85 60 15)",
                id[0], id[1], id[2], err);
    }

    k_work_reschedule(&report_work, K_SECONDS(10));
}

static int qspi_diag_init(void) {
    k_work_reschedule(&report_work, K_SECONDS(5));
    return 0;
}

SYS_INIT(qspi_diag_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
