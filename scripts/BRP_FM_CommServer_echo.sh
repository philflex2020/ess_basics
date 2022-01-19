#/bin/sh

pkill fims_echo
/usr/local/bin/fims/fims_echo -u /xxinterfaces/outstation -b '{
"Controllable_Load_Emercency_Down_Ramp_Rate":1,
"Controllable_Load_Emercency_Up_Ramp_Rate":2,
"Controllable_Load_Normal_Down_Ramp_Rate":3,
"Controllable_Load_Normal_Up_Ramp_Rate":4,
"Controllable_Load_Scheduled_Power_Consumption":5,
"Controllable_Load_Scheduled_Power_Consumption_+2_hours":6,
"Controllable_Load_Virtual_Breaker_Status_-_PSEUDO_LOAD":7,
"Generation_Emercency_Down_Ramp_Rate":8,
"Generation_Emergency_Up_Ramp_Rate":9,
"Generation_Maximum_Operating_Charge_Power_Limit":10,
"Generation_Maximum_Operating_Discharge_Power_Limit":11,
"Generation_Maximum_Operating_State_of_Charge":12,
"Generation_Minimum_Operating_State_of_Charge":13,
"Generation_Normal_Down_Ramp_Rate":14,
"Generation_Normal_Up_Ramp_Rate":15,
"Generation_Plant_Auxiliary_Load_MVAR":16,
"Generation_Plant_Auxiliary_Load_MW":17,
"Generation_Resource_Breaker_Status":18,
"Generation_Resource_Switches_":19,
"Generation_State_of_Charge":20,
"Generation_Virtual_Breaker_Status_-_PSEUDO_GEN":21,
"Generation_Voltage":22
                 }' &

/usr/local/bin/fims/fims_echo -u /sites/odessa -b '{
"breaker_status":1001
}'&

/usr/local/bin/fims/fims_echo -u /metrics/odessa -b '{
"gen_net_mw":600,
"gen_net_mvar":601,
"gen_gross_mw":602,
"gen_gross_mvar":603,
"gen_normal_up_ramp_rate":604,
"gen_normal_down_ramp_rate":605,
"gen_emergency_up_ramp_rate":606,
"gen_emergency_down_ramp_rate":607,
"gen_max_operating_soc":608,
"gen_min_operating_soc":609,
"gen_soc":610,
"gen_max_discharge_mw":611,
"gen_max_charge_mw":612,
"gen_line_flows_mw":613,
"gen_line_flows_mvar":614,
"gen_voltage":615,
"gen_aux_mw":616,
"gen_aux_mvar":617,
"load_net_mw":630,
"load_net_mvar":631,
"load_scheduled_mw":632,
"load_scheduled_mw_2hr":633,
"load_normal_up_ramp_rate":634,
"load_normal_down_ramp_rate":635,
"load_emergency_up_ramp_rate":636,
"load_emergency_down_ramp_rate":637
"gen_avr_status":638,
"gen_ds1_status":639,
"gen_pseudo_switch_status":640,
"load_pseudo_switch_status"641
}' &

/usr/local/bin/fims/fims_echo -u /metrics/alvin -b '{
"gen_net_mw":100,
"gen_net_mvar":101,
"gen_gross_mw":102,
"gen_gross_mvar":103,
"gen_normal_up_ramp_rate":104,
"gen_normal_down_ramp_rate":105,
"gen_emergency_up_ramp_rate":106,
"gen_emergency_down_ramp_rate":107,
"gen_max_operating_soc":108,
"gen_min_operating_soc":109,
"gen_soc":110,
"gen_max_discharge_mw":111,
"gen_max_charge_mw":112,
"gen_line_flows_mw":113,
"gen_line_flows_mvar":114,
"gen_voltage":115,
"gen_aux_mw":116,
"gen_aux_mvar":117,
"gen_52m_status":118,
"gen_avr_status":119,
"gen_ds1_status":120,
"gen_pseudo_switch_status":121,
"load_pseudo_switch_status"122,
"load_net_mw":130,
"load_net_mvar":131,
"load_scheduled_mw":132,
"load_scheduled_mw_2hr":133,
"load_normal_up_ramp_rate":134,
"load_normal_down_ramp_rate":135,
"load_emergency_up_ramp_rate":136,
"load_emergency_down_ramp_rate":137
}' &


