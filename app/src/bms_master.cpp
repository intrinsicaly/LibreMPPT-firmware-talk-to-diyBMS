/*
 * Copyright (c) The Libre Solar Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if CONFIG_BMS_MASTER

#include "bms_master.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bms_master, CONFIG_BMS_MASTER_LOG_LEVEL);

#include "bat_charger.h"
#include "helper.h"
#include "setup.h"

extern Charger charger;
extern BatConf bat_conf;

/* BMS control variables linked to ThingSet data objects */
bool bms_charge_enable = true;
float bms_target_voltage = 0.0f;
float bms_max_charge_current = 0.0f;
float bms_topping_voltage = 0.0f;
float bms_float_voltage = 0.0f;
uint32_t bms_heartbeat_timestamp = 0;

void bms_master_init(void)
{
    charger.bms_master_enabled = IS_ENABLED(CONFIG_BMS_MASTER_ENABLED);
    charger.bms_timeout_seconds = CONFIG_BMS_MASTER_TIMEOUT_SECONDS;
    charger.bms_node_id = CONFIG_BMS_MASTER_NODE_ID;
    LOG_INF("BMS master control initialized (node_id=0x%04X, timeout=%us)",
            charger.bms_node_id, charger.bms_timeout_seconds);
}

void bms_master_update(void)
{
    if (!charger.bms_master_enabled) {
        return;
    }

    uint32_t now = uptime();

    /* Check for BMS communication timeout */
    if (bms_heartbeat_timestamp == 0
        || (now - bms_heartbeat_timestamp) > charger.bms_timeout_seconds)
    {
        if (!charger.bms_timeout_detected) {
            LOG_WRN("BMS master timeout detected - reverting to local control");
            charger.bms_timeout_detected = true;
        }
        return;
    }

    if (charger.bms_timeout_detected) {
        LOG_INF("BMS master communication restored");
        charger.bms_timeout_detected = false;
    }

    /* Transfer BMS ThingSet values into charger struct */
    charger.bms_charge_enabled = bms_charge_enable;

    if (bms_target_voltage > 0.0f) {
        /* BMS voltage limit is ANDed with local limit (most restrictive wins) */
        charger.bms_voltage_limit =
            MIN(bms_target_voltage, bat_conf.absolute_max_voltage);
    }
    else {
        charger.bms_voltage_limit = 0.0f;
    }

    if (bms_max_charge_current > 0.0f) {
        /* BMS current limit is ANDed with local limit (most restrictive wins) */
        charger.bms_current_limit =
            MIN(bms_max_charge_current, bat_conf.charge_current_max);
    }
    else {
        charger.bms_current_limit = 0.0f;
    }

    charger.time_last_bms_msg = now;

    LOG_DBG("BMS update: V_lim=%.2fV, I_lim=%.2fA, chg_en=%d",
            (double)charger.bms_voltage_limit,
            (double)charger.bms_current_limit,
            charger.bms_charge_enabled);
}

void bms_control_updated(void)
{
    bms_heartbeat_timestamp = uptime();
    LOG_DBG("BMS control value updated: V=%.2f, I=%.2f, en=%d",
            (double)bms_target_voltage, (double)bms_max_charge_current, bms_charge_enable);
}

#endif /* CONFIG_BMS_MASTER */
