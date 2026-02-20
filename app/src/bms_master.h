/*
 * Copyright (c) The Libre Solar Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BMS_MASTER_H_
#define BMS_MASTER_H_

/** @file
 *
 * @brief BMS master control via ThingSet CAN
 *
 * Handles control commands received from a diyBMS v4 master controller over CAN bus.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * BMS control variables linked to ThingSet data objects
 */
extern bool bms_charge_enable;
extern float bms_target_voltage;
extern float bms_max_charge_current;
extern float bms_topping_voltage;
extern float bms_float_voltage;
extern uint32_t bms_heartbeat_timestamp;

/**
 * Initialize BMS master control
 *
 * Must be called once at startup after data_objects_init().
 */
void bms_master_init(void);

/**
 * Update BMS master control
 *
 * Called once per second from the charger control loop. Applies BMS limits to the charger
 * if active and not timed out.
 */
void bms_master_update(void);

/**
 * Callback when a BMS control value is updated via CAN
 *
 * Updates the BMS heartbeat timestamp to indicate an active BMS master.
 */
void bms_control_updated(void);

#ifdef __cplusplus
}
#endif

#endif /* BMS_MASTER_H_ */
