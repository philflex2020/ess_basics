#/bin/sh
pkill fims_echo


/usr/local/bin/fims/fims_echo -u /queryx/home/mystuff/uri -b '{"testOP_32_1":1234,"testOP_32_22":3222}' &
/usr/local/bin/fims/fims_echo -u /queryx/home/athings/uri -b '{"testOP_16_8":1608}' &
/usr/local/bin/fims/fims_echo -u /site/feedback -b '{
"active_power_fb":2
}'&

/usr/local/bin/fims/fims_echo -u /site/power -b '{
"pv_reactive_power":2
}' &

/usr/local/bin/fims/fims_echo -u /interfaces/hybridos -b '{

"present_enable_flag":0,
"present_disable_flag":0,
"present_standby_flag":0,
"site_state":0,
"alarm_status_flag":0,
"fault_status_flag":0,
"local_remote_source_flag":0,
"present_export_target_kW_cmd":0,
"present_features_kW_mode_cmd":0,
"present_features_kVAR_mode_cmd":0,
"peak_power_limit_status_flag":0,
"pfr_status_flag":0,
"solar_curtailment_state":0,
"present_manual_ess_kW_cmd":0,
"present_manual_solar_kW_cmd":0,
"present_ess_charge_control_target_soc":0,
"present_active_voltage_cmd":0,
"active_voltage_status_flag":0,
"present_power_factor_cmd":0,
"heartbeat_feedback":0,
"gross_active_power":0,
"gross_reactive_power":0,
"gross_apparent_power":0,
"net_active_power":0,
"net_reactive_power":0,
"net_apparent_power":0,
"ess_total_active_power":0,
"ess_total_reactive_power":0,
"ess_total_apparent_power":0,
"potential_ess_charge_kW":0,
"potential_ess_dischg_kW":0,
"ess_average_soc":0,
"ess_total_soc_abs":0,
"ess_total_soh":0,
"ess_total_soh_abs":0,
"ess_total_throughput":0,
"ess_decay_rate":0,
"bess_aux_active_power":0,
"bess_aux_reactive_power":0,
"available_ess_num":0,
"running_ess_num":0,
"solar_total_active_power":0,
"solar_total_reactive_power":0,
"solar_total_apparent_power":0,
"pv_aux_active_power":0,
"pv_aux_reactive_power":0,
"available_solar_num":0,
"running_solar_num":0,
"bess_active_power":0,
"bess_reactive_power":0,
"bess_apparent_power":0,
"pv_active_power":0,
"pv_reactive_power":0,
"pv_apparent_power":0,
"Timestamp":"06-04-2020 14:55:51.705450"
                 }' &

/usr/local/bin/fims/fims_echo -u /getUris/components/hybridos -b '{
"present_enable_flag":0,
"present_disable_flag":0,
"present_standby_flag":0,
"site_state":0,
"alarm_status_flag":0,
"fault_status_flag":0,
"local_remote_source_flag":0,
"present_export_target_kW_cmd":0,
"present_features_kW_mode_cmd":0,
"present_features_kVAR_mode_cmd":0,
"peak_power_limit_status_flag":0,
"pfr_status_flag":0,
"solar_curtailment_state":0,
"present_manual_ess_kW_cmd":0,
"present_manual_solar_kW_cmd":0,
"present_ess_charge_control_target_soc":0,
"present_active_voltage_cmd":0,
"active_voltage_status_flag":0,
"present_power_factor_cmd":0,
"heartbeat_feedback":0,
"gross_active_power":0,
"gross_reactive_power":0,
"gross_apparent_power":0,
"net_active_power":0,
"net_reactive_power":0,
"net_apparent_power":0,
"ess_total_active_power":0,
"ess_total_reactive_power":0,
"ess_total_apparent_power":0,
"potential_ess_charge_kW":0,
"potential_ess_dischg_kW":0,
"ess_average_soc":0,
"ess_total_soc_abs":0,
"ess_total_soh":0,
"ess_total_soh_abs":0,
"ess_total_throughput":0,
"ess_decay_rate":0,
"bess_aux_active_power":0,
"bess_aux_reactive_power":0,
"available_ess_num":0,
"running_ess_num":0,
"solar_total_active_power":0,
"solar_total_reactive_power":0,
"solar_total_apparent_power":0,
"pv_aux_active_power":0,
"pv_aux_reactive_power":0,
"available_solar_num":0,
"running_solar_num":0,
"bess_active_power":0,
"bess_reactive_power":0,
"bess_apparent_power":0,
"pv_active_power":0,
"pv_reactive_power":0,
"pv_apparent_power":0,
"Timestamp":"06-04-2020 14:55:51.705450"
                 }' &
/usr/local/bin/fims/fims_echo -u /assets/feeders/feed_6 -b '{
            "feeder_kW_slew_rate":1243,
            "remote_ess_charge_control_target_soc":5000,
            "remote_export_target_kW_cmd":6123
}' &

/usr/local/bin/fims/fims_echo -u /features/reactive_power -b '{
            "remote_features_kW_mode_cmd": 42,
            "remote_features_kVAR_mode_cmd":24
}' &

/usr/local/bin/fims/fims_echo -u /site/operation -b '{
           "remote_enable_flag":1,
           "remote_disable_flag":0,
           "remote_standby_flag":3
}' &



