/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_err.h"
#include "esp_zigbee_type.h"
#include "zcl/esp_zigbee_zcl_common.h"

/**
 * @brief  Create an empty attribute list.
 *
 * @note  This attribute list is ready for certain cluster id to add / update attribute refer to esp_zb_xx_cluster_add_attr() and esp_zb_cluster_update_attr().
 * @note  Attribute list groups up a cluster.
 *
 * @param[in] cluster_id The cluster id for attribute list refer to esp_zb_zcl_cluster_id
 *
 * @return pointer to  @ref esp_zb_attribute_list_s
 *
 */
esp_zb_attribute_list_t *esp_zb_zcl_attr_list_create(uint16_t cluster_id);

/**
 * @brief  Get ZCL attribute descriptor.
 *
 * @note  Getting the local attribute from the specific endpoint and cluster.
 *
 * @param[in] endpoint The endpoint
 * @param[in] cluster_id Cluster id for attribute list refer to esp_zb_zcl_cluster_id
 * @param[in] cluster_role Cluster role of this cluster, either server or client role refer to esp_zb_zcl_cluster_role
 * @param[in] attr_id Attribute id
 *
 * @return pointer to  @ref esp_zb_zcl_attr_s
 *
 */
esp_zb_zcl_attr_t *esp_zb_zcl_get_attribute(uint8_t endpoint, uint16_t cluster_id, uint8_t cluster_role, uint16_t attr_id);

/**
 * @brief  Set ZCL attribute value.
 *
 * @note  Set the local attribute from the specific endpoint, cluster and attribute.
 *
 * @param[in] endpoint The endpoint
 * @param[in] cluster_id Cluster id for attribute list refer to esp_zb_zcl_cluster_id
 * @param[in] cluster_role Cluster role of this cluster, either server or client role refer to esp_zb_zcl_cluster_role
 * @param[in] attr_id Attribute id
 * @param[in] value_p pointer to new value
 * @param[in] check The access method of attribute whether is required to check or not
 *
 * @note Please look up the Zigbee Cluster Library for the access method of attribute
 * @return zcl status refer to esp_zb_zcl_status_t
 *
 */
esp_zb_zcl_status_t esp_zb_zcl_set_attribute_val(uint8_t endpoint, uint16_t cluster_id, uint8_t cluster_role,
        uint16_t attr_id, void *value_p, bool check);

/**
 * @brief Add an attribute in basic cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_basic_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in power config cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 * @note Attribute for battery information, battery settings 2 and 3 sets haven't supported yet.
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_power_config_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in identify cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_identify_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in groups cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_groups_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in scenes cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_scenes_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in on_off cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_on_off_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in on_off switch config cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_on_off_switch_config_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in level cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_level_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in color control cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_color_control_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in time cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_time_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in binary input cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_binary_input_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in shade config cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_shade_config_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in door lock cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_door_lock_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in IAS zone cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_ias_zone_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Set the IAS zone CIE address for IAS zone server
 * 
 * @param[in] endpoint A 8-bit endpoint ID which the IAS zone cluster attach
 * @param[in] cie_ieee_addr The 8-byte IEEE address will be regarded as the IAS message destination address  
 * @return  
 *      - ESP_OK on success
 *      - ESP_FAIL The CIE address has been set, invalid argument or IAS zone cluster does not exist
 * 
 */
esp_err_t esp_zb_ias_zone_cluster_set_cie_address(uint8_t endpoint, esp_zb_ieee_addr_t cie_ieee_addr);
/**
 * @brief Add an attribute in temperature measurement cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_temperature_meas_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in humidity measurement cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_humidity_meas_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute and variables in OTA client cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_ota_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in illuminance measurement cluster
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_illuminance_meas_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in pressure measurement cluster
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_pressure_meas_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in electrical measurement cluster
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_electrical_meas_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in window covering cluster
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_window_covering_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in occupancy sensor cluster
 * 
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 * 
 */
esp_err_t esp_zb_occupancy_sensing_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in thermostat cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_thermostat_cluster_add_attr(esp_zb_attribute_list_t* attr_list, uint16_t attr_id, void* value_p);

/**
 * @brief Add an attribute in fan control cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_fan_control_cluster_add_attr(esp_zb_attribute_list_t* attr_list, uint16_t attr_id, void* value_p);

/**
 * @brief Add an attribute in thermostat ui configuration cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_thermostat_ui_config_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in analog input cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_analog_input_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in analog output cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_analog_output_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in analog value cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_analog_value_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in carbon dioxide measurement cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_carbon_dioxide_measurement_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in pm2.5 measurement cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_pm2_5_measurement_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in multistate value cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id to be added
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
 *
 */
esp_err_t esp_zb_multistate_value_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

/**
 * @brief Add an attribute in a specified cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] cluster_id The cluster ID to which the attribute will be added, refer to esp_zb_zcl_cluster_id_t
 * @param[in] attr_id An attribute id to be added
 * @param[in] attr_type Type of attribute to be added, refer to esp_zb_zcl_attr_type_t
 * @param[in] attr_access Access type of attribute to be added, refer to esp_zb_zcl_attr_access_t
 * @param[in] value_p A pointer to attribute value wants to add
 *
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if attribute is existed or attribute type is unsupported
 *
 */
esp_err_t esp_zb_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t cluster_id, uint16_t attr_id, uint8_t attr_type, uint8_t attr_access, void *value_p);

/**
* @brief Add customized attribute in customized cluster.
*
* @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
* @param[in] attr_id An attribute id to be added
* @param[in] attr_type Type of attribute to be added, refer to esp_zb_zcl_attr_type_t
* @param[in] attr_access Access type of attribute to be added, refer to esp_zb_zcl_attr_access_t
* @param[in] value_p A pointer to attribute value wants to add
*
* @return
* - ESP_OK on success
* - ESP_ERR_INVALID_ARG if attribute is existed or unsupported
*
*/
esp_err_t esp_zb_custom_cluster_add_custom_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, uint8_t attr_type, uint8_t attr_access, void *value_p);

/**
 * @brief Update an attribute in a specific cluster.
 *
 * @param[in] attr_list A pointer to attribute list @ref esp_zb_attribute_list_s
 * @param[in] attr_id  An attribute id which wants to update
 * @param[in] value_p A pointer to attribute value wants to update
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if attribute list not initialized
 *      - ESP_ERR_NOT_FOUND the request update attribute is not found
 *
 */
esp_err_t esp_zb_cluster_update_attr(esp_zb_attribute_list_t *attr_list, uint16_t attr_id, void *value_p);

#ifdef __cplusplus
}
#endif
