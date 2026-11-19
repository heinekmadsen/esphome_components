#include "point_catalog.h"

namespace esphome {
namespace auto_nilan {

// Unit helpers (UTF-8 degree sign kept explicit so source encoding does not matter).
#define U_TEMP "\xC2\xB0" "C"

#define HR REG_HOLDING
#define IR REG_INPUT

#define PT(key_, name_, addr_, reg_, kind_, scale_, flags_, min_, max_, unit_, opts_, cap_) \
  {key_, name_, addr_, reg_, kind_, scale_, flags_, min_, max_, unit_, opts_, cap_}

// ---------------------------------------------------------------------------
// CTS602 (full + Light share this catalog; Light simply fails the optional
// probes and its missing feature registers are filtered by capabilities).
// Ordered by register type + address so commissioning can use block reads.
// ---------------------------------------------------------------------------
static const PointDef CTS602_POINTS[] = {
    // --- Device / identity -------------------------------------------------
    PT("bus_version", "Bus version", 0, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("app_id", "Application ID", 4, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("hw_type", "Hardware type", 5, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),

    // --- Digital inputs ----------------------------------------------------
    PT("user_function", "User function active", 100, IR, KIND_BINARY, 0, PF_OPTIONAL, 0, 1, "", nullptr, 0),
    PT("air_filter_alarm", "Air filter alarm", 101, IR, KIND_BINARY, 0, PF_NONE, 0, 1, "", nullptr, 0),
    PT("door_open", "Door contact", 102, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("smoke_alarm", "Smoke alarm", 103, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("motor_thermo", "Motor thermo fuse", 104, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("frost_overheat", "Frost / overheat", 105, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("airflow_monitor", "Airflow monitor", 106, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("high_pressure", "High pressure switch", 107, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("low_pressure", "Low pressure switch", 108, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("boiler", "Hot water boiling", 109, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, CAP_HOT_WATER),
    PT("three_way_valve", "Hot water 3-way valve", 110, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, CAP_HOT_WATER),
    PT("defrost_hotgas", "Hotgas defrost", 111, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, CAP_COMPRESSOR),
    PT("defrost_thermostat", "Defrost thermostat", 112, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, CAP_COMPRESSOR),
    PT("user_function_2", "User function 2 active", 113, IR, KIND_BINARY, 0, PF_OPTIONAL, 0, 1, "", nullptr, 0),
    PT("damper_closed", "Damper closed", 114, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("damper_opened", "Damper opened", 115, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("fc_thermo_alarm", "FC / thermo alarm", 116, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("brine_pressostat", "Brine pressostat", 117, IR, KIND_BINARY, 0, PF_OPTIONAL | PF_DIAG, 0, 1, "", nullptr, CAP_COMPRESSOR),

    // --- Temperatures ------------------------------------------------------
    PT("t0_controller", "Controller temperature", 200, IR, KIND_SENSOR, -2, PF_SIGNED | PF_DIAG, 0, 0, U_TEMP, nullptr, 0),
    PT("t1_intake", "Intake temperature", 201, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t2_inlet", "Inlet temperature", 202, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t3_exhaust", "Exhaust temperature", 203, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t4_outlet", "Outlet temperature", 204, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t5_cond", "Condenser temperature", 205, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL | PF_DIAG, 0, 0, U_TEMP, nullptr, CAP_COOLING),
    PT("t6_evap", "Evaporator temperature", 206, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL | PF_DIAG, 0, 0, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("t7_inlet", "Inlet temperature after heater", 207, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t8_outdoor", "Outdoor temperature", 208, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t9_heater", "Heater surface temperature", 209, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL | PF_DIAG, 0, 0, U_TEMP, nullptr, CAP_HEATING),
    PT("t10_external", "External room temperature", 210, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL, 0, 0, U_TEMP, nullptr, 0),
    PT("t11_hotwater_top", "Hot water top temperature", 211, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, CAP_HOT_WATER),
    PT("t12_hotwater_bottom", "Hot water bottom temperature", 212, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, CAP_HOT_WATER),
    PT("t13_return", "Central heating return temperature", 213, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("t14_supply", "Central heating supply temperature", 214, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("t15_room", "Room temperature (panel)", 215, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL, 0, 0, U_TEMP, nullptr, 0),
    PT("t16_anode", "Hot water anode temperature", 216, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL | PF_DIAG, 0, 0, U_TEMP, nullptr, CAP_HOT_WATER),
    PT("t17_preheat", "Preheater / earth tube temperature", 217, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, CAP_PREHEAT),
    PT("t18_pressure_pipe", "Pressure pipe temperature", 218, IR, KIND_SENSOR, -2, PF_SIGNED | PF_OPTIONAL | PF_DIAG, 0, 0, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("rh", "Humidity", 221, IR, KIND_SENSOR, -2, PF_NONE, 0, 0, "%", nullptr, CAP_RH),
    PT("co2", "CO2", 222, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "ppm", nullptr, CAP_CO2),

    // --- Alarms ------------------------------------------------------------
    PT("alarm_status", "Active alarms", 400, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_1_code", "Alarm 1 code", 401, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_1_date", "Alarm 1 date", 402, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_1_time", "Alarm 1 time", 403, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_2_code", "Alarm 2 code", 404, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_2_date", "Alarm 2 date", 405, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_2_time", "Alarm 2 time", 406, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_3_code", "Alarm 3 code", 407, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_3_date", "Alarm 3 date", 408, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_3_time", "Alarm 3 time", 409, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),

    // --- Actual control ----------------------------------------------------
    PT("run_act", "Actual on/off", 1000, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("mode_act", "Actual operation mode", 1001, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "Off|Heat|Cool|Auto|Service", 0),
    PT("state_display", "Control state", 1002, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "Off|Shift|Stop|Start|Standby|Ventilation stop|Ventilation|Heating|Cooling|Hot water|Legionella|Cooling + hot water|Central heating|Defrost|Frost secure|Service|Alarm|Heating + hot water", 0),
    PT("sec_in_state", "Seconds in state", 1003, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "s", nullptr, 0),

    // --- Actual airflow ----------------------------------------------------
    PT("vent_set_act", "Ventilation step", 1100, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "", nullptr, 0),
    PT("inlet_step_act", "Inlet fan step", 1101, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("exhaust_step_act", "Exhaust fan step", 1102, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("days_since_filter", "Days since filter change", 1103, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "days", nullptr, 0),
    PT("days_to_filter", "Days to next filter change", 1104, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "days", nullptr, 0),

    // --- Actual temperatures / capacity -----------------------------------
    PT("is_summer", "Summer mode", 1200, IR, KIND_BINARY, 0, PF_NONE, 0, 1, "", nullptr, 0),
    PT("temp_inlet_set", "Inlet temperature request", 1201, IR, KIND_SENSOR, -2, PF_SIGNED | PF_DIAG, 0, 0, U_TEMP, nullptr, 0),
    PT("temp_control", "Controlled temperature", 1202, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("temp_room", "Actual room temperature", 1203, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("heat_exchange_efficiency", "Heat exchange efficiency", 1204, IR, KIND_SENSOR, -2, PF_NONE, 0, 0, "%", nullptr, 0),
    PT("capacity_requested", "Requested capacity", 1205, IR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, 0),
    PT("capacity_actual", "Actual capacity", 1206, IR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, 0),

    // --- Feature presence --------------------------------------------------
    PT("compressor_type", "Compressor type", 1500, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "None|On/Off|Modulated hotgas valve", CAP_COMPRESSOR),
    PT("hotwater_type", "Hot water type", 1700, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "None|Hot water only|With ventilation|With central heating", CAP_HOT_WATER),
    PT("hotwater_anode", "Hot water anode state", 1701, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "Off|OK|Service|Error", CAP_HOT_WATER),
    PT("central_heat_setpoint", "Central heating setpoint", 1800, IR, KIND_SENSOR, -2, PF_SIGNED | PF_DIAG, 0, 0, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("preheat_block_remain", "Preheater block remaining", 2100, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "s", nullptr, CAP_PREHEAT),

    // --- DPT pressure / airflow -------------------------------------------
    PT("dpt_in_session", "DPT in session", 2200, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, CAP_PRESSURE),
    PT("dpt_airflow_1", "DPT airflow 1", 2201, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "m3/h", nullptr, CAP_PRESSURE),
    PT("dpt_airflow_2", "DPT airflow 2", 2202, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "m3/h", nullptr, CAP_PRESSURE),
    PT("dpt_pressure_1", "DPT pressure 1", 2221, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "Pa", nullptr, CAP_PRESSURE),
    PT("dpt_pressure_2", "DPT pressure 2", 2222, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "Pa", nullptr, CAP_PRESSURE),

    // --- Display / derived inputs -----------------------------------------
    PT("bypass_open", "Bypass damper open", 3000, IR, KIND_BINARY, 0, PF_NONE, 0, 1, "", nullptr, CAP_BYPASS),
    PT("airheat_capacity", "After-heater capacity", 3001, IR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, CAP_HEATING),
    PT("exchanger_defrost", "Exchanger defrost", 3002, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, CAP_COMPRESSOR),
    PT("co2_enable", "CO2 sensor enabled", 3003, IR, KIND_BINARY, 0, PF_DIAG | PF_DISABLED, 0, 1, "", nullptr, 0),
    PT("room_reduce", "Low room temperature reduce", 3004, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("last_damper_test_day", "Last damper test", 3005, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("winter_reduce", "Low outdoor temperature reduce", 3007, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("temp_setpoint_actual", "Resulting temperature setpoint", 3008, IR, KIND_SENSOR, -2, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("temp_control_master", "Master controlled temperature", 3009, IR, KIND_SENSOR, -2, PF_SIGNED | PF_DIAG, 0, 0, U_TEMP, nullptr, 0),
    PT("alarm_log_event", "Alarm log event", 3050, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_log_ctr_state", "Alarm log control state", 3075, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", "Off|Shifting|Stopping|Starting|Standby|Ventilation stop|Air exchange|Air heat|Air cool|Hot water|Legionella|Air cool + hot water|Central heat|Defrost|Frost|Service|Alarm shutdown|Air heat + hot water", 0),
    PT("rh_average", "Humidity average", 3100, IR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, CAP_RH),
    PT("expansion_board_id", "Expansion board ID", 3101, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("vent_state", "Ventilation state", 3102, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "Off|Normal|Low humidity|High humidity|High CO2|Low room temperature|Low outdoor temperature", 0),

    // --- Fan outputs / actual speed ---------------------------------------
    PT("exhaust_speed_out", "Exhaust fan output", 200, HR, KIND_SENSOR, -2, PF_NONE, 0, 0, "%", nullptr, 0),
    PT("inlet_speed_out", "Inlet fan output", 201, HR, KIND_SENSOR, -2, PF_NONE, 0, 0, "%", nullptr, 0),
    PT("airheat_capacity_out", "After-heater output", 202, HR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, CAP_HEATING),
    PT("central_heat_capacity_out", "Central heater output", 203, HR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, CAP_CENTRAL_HEAT),
    PT("compressor_capacity_out", "Compressor output", 204, HR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, CAP_COMPRESSOR),
    PT("preheat_capacity_out", "Preheater output", 205, HR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, CAP_PREHEAT),
    PT("rotor_exchanger", "Rotor exchanger", 206, HR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "%", nullptr, 0),

    // --- Commands ----------------------------------------------------------
    PT("reset_alarm", "Reset alarm", 400, HR, KIND_BUTTON, 0, PF_WRITABLE | PF_DIAG, 0, 255, "", nullptr, 0),

    // --- Program / user functions -----------------------------------------
    PT("week_program", "Week program", 500, HR, KIND_SELECT, 0, PF_WRITABLE, 0, 4, "", "None|Program 1|Program 2|Program 3|Erase", 0),
    PT("user_function_active", "User function active", 600, HR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "None|Extend|Inlet|Exhaust|External heater offset|Ventilate|Cooker Hood", 0),
    PT("user_function_set", "User function", 601, HR, KIND_SELECT, 0, PF_WRITABLE, 0, 6, "", "None|Extend|Inlet|Exhaust|External heater offset|Ventilate|Cooker Hood", 0),
    PT("user_time_set", "User function time", 602, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_TIME_HHMM, 0, 480, "min", nullptr, 0),
    PT("user_vent_set", "User function ventilation step", 603, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 4, "", nullptr, 0),
    PT("user_temp_set", "User function temperature", 604, HR, KIND_NUMBER, 0, PF_WRITABLE, 15, 30, U_TEMP, nullptr, 0),
    PT("user_offset_set", "User function temperature offset", 605, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_SIGNED, -10, 10, U_TEMP, nullptr, 0),
    PT("user2_function_active", "User function 2 active", 610, HR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", "None|Extend|Inlet|Exhaust|External heater offset|Ventilate|Cooker Hood", 0),
    PT("user2_function_set", "User function 2", 611, HR, KIND_SELECT, 0, PF_WRITABLE, 0, 6, "", "None|Extend|Inlet|Exhaust|External heater offset|Ventilate|Cooker Hood", 0),
    PT("user2_time_set", "User function 2 time", 612, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_TIME_HHMM, 0, 480, "min", nullptr, 0),
    PT("user2_vent_set", "User function 2 ventilation step", 613, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 4, "", nullptr, 0),
    PT("user2_temp_set", "User function 2 temperature", 614, HR, KIND_NUMBER, 0, PF_WRITABLE, 15, 30, U_TEMP, nullptr, 0),
    PT("user2_offset_set", "User function 2 temperature offset", 615, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_SIGNED, -10, 10, U_TEMP, nullptr, 0),

    // --- Control -----------------------------------------------------------
    PT("model_type", "Machine type", 1000, HR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("run_set", "On/off", 1001, HR, KIND_SWITCH, 0, PF_WRITABLE, 0, 1, "", nullptr, 0),
    PT("mode_set", "Operation mode", 1002, HR, KIND_SELECT, 0, PF_WRITABLE, 0, 4, "", "Off|Heat|Cool|Auto|Service", 0),
    PT("vent_set", "Ventilation step", 1003, HR, KIND_SELECT, 0, PF_WRITABLE, 0, 4, "", "Off|1|2|3|4", 0),
    PT("temp_set", "Temperature setpoint", 1004, HR, KIND_NUMBER, -2, PF_WRITABLE, 5, 35, U_TEMP, nullptr, 0),
    PT("service_mode", "Service mode", 1005, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 8, "", "Off|Defrost|Flaps|Inlet|Exhaust|Compressor|Heating|Hot water|Central heat", 0),
    PT("service_percent", "Service capacity", 1006, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("preset", "Factory preset", 1007, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG | PF_DISABLED, 0, 3, "", "Ready|Standard|Backup|Restore", 0),

    // --- Airflow settings --------------------------------------------------
    PT("air_exchange_mode", "Air exchange mode", 1100, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "Energy|Comfort|ComfortWater", 0),
    PT("cooling_vent", "Cooling ventilation step", 1101, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 4, "", nullptr, CAP_COOLING),
    PT("damper_test_select", "Damper test day", 1102, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 7, "", "Off|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday", 0),
    PT("damper_test_state", "Damper test state", 1104, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 6, "", "Off|Standby|Start|Closing|Opening|OK|Error", 0),
    PT("filter_alarm_type", "Filter alarm type", 1105, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 5, "", "Pressure guard|30 days|90 days|180 days|360 days|70 days + pressure", 0),

    // --- Temperature control ----------------------------------------------
    PT("cooling_set", "Cooling setpoint offset", 1200, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 8, "", "Off|+0 C|+1 C|+2 C|+3 C|+4 C|+5 C|+7 C|+10 C", CAP_COOLING),
    PT("temp_min_summer", "Minimum supply temp summer", 1201, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("temp_min_winter", "Minimum supply temp winter", 1202, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("temp_max_summer", "Maximum supply temp summer", 1203, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("temp_max_winter", "Maximum supply temp winter", 1204, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("summer_winter_limit", "Summer/winter limit", 1205, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 30, U_TEMP, nullptr, 0),
    PT("night_day_limit", "Night cooling outdoor limit", 1206, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 40, U_TEMP, nullptr, 0),
    PT("night_setpoint", "Night cooling room setpoint", 1207, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 10, 30, U_TEMP, nullptr, 0),
    PT("sensor_select", "Control temperature sensor", 1208, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 3, "", "User panel|External sensor|Inlet channel|Exhaust channel", 0),
    PT("heat_select", "Heat source", 1209, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 4, "", "No heating|Heat pump only|HP + afterheat|Afterheat only|Afterheat + HP", 0),

    // --- Compressor --------------------------------------------------------
    PT("cond_temp_min", "Condenser temperature low curve", 1500, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 100, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("cond_temp_max", "Condenser temperature high curve", 1501, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 100, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("compressor_start_temp", "Compressor start temperature", 1502, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, -20, 30, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("compressor_restart_time", "Compressor stop time", 1503, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 60, "min", nullptr, CAP_COMPRESSOR),

    // --- Hot water ---------------------------------------------------------
    PT("hotwater_temp_top", "Hot water top setpoint", 1700, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 80, U_TEMP, nullptr, CAP_HOT_WATER),
    PT("hotwater_temp_bottom", "Hot water bottom setpoint", 1701, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 80, U_TEMP, nullptr, CAP_HOT_WATER),
    PT("hotwater_priority", "Hot water priority", 1702, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", "Water|Inlet", CAP_HOT_WATER),
    PT("hotwater_scald_temp", "Hot water scald protection", 1703, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 90, U_TEMP, nullptr, CAP_HOT_WATER),
    PT("hotwater_heat_type", "Hot water electric supplement", 1704, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", "Off|Electric", CAP_HOT_WATER),
    PT("hotwater_legionella", "Legionella day", 1705, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 7, "", "Off|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday", CAP_HOT_WATER),
    PT("hotwater_bypass_offset", "Hot water bypass offset", 1706, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 30, U_TEMP, nullptr, CAP_HOT_WATER),

    // --- Central heating ---------------------------------------------------
    PT("central_heat_offset", "Central heating offset", 1800, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, -5, 5, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("central_heat_select", "Central heating mode", 1801, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "Pump/frost only|Always central heating|Only when room temp low", CAP_CENTRAL_HEAT),
    PT("central_heat_supply_min", "Central heating min supply", 1802, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 80, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("central_heat_supply_max", "Central heating max supply", 1803, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 80, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("central_heat_supply_offset", "Central heating curve offset", 1804, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, -5, 5, U_TEMP, nullptr, CAP_CENTRAL_HEAT),
    PT("central_heat_curve", "Central heating curve", 1805, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 1, 10, "", nullptr, CAP_CENTRAL_HEAT),
    PT("central_heat_pump_mode", "Central heating pump mode", 1806, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", "When heating active|Continuous", CAP_CENTRAL_HEAT),
    PT("central_heat_type", "Central heating heat type", 1807, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 3, "", "Off|Electric|Heat pump|Both", CAP_CENTRAL_HEAT),
    PT("central_heat_reg_time", "Central heating regulation time", 1808, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 1, 25, "s", nullptr, CAP_CENTRAL_HEAT),

    // --- IAQ control -------------------------------------------------------
    PT("rh_vent_low", "Humidity low ventilation step", 1910, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 4, "", nullptr, CAP_RH),
    PT("rh_vent_high", "Humidity high ventilation step", 1911, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 4, "", nullptr, CAP_RH),
    PT("rh_limit_low", "Humidity low limit", 1912, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 15, 45, "%", nullptr, CAP_RH),
    PT("rh_timeout", "Humidity high ventilation time", 1913, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 180, "min", nullptr, CAP_RH),
    PT("co2_vent_high", "CO2 high ventilation step", 1920, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 4, "", nullptr, CAP_CO2),
    PT("co2_limit_normal", "CO2 normal limit", 1921, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 400, 2000, "ppm", nullptr, CAP_CO2),
    PT("co2_limit_high", "CO2 high limit", 1922, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 400, 2000, "ppm", nullptr, CAP_CO2),

    // --- Preheater ---------------------------------------------------------
    PT("preheat_block", "Block preheater", 2100, HR, KIND_BUTTON, 0, PF_WRITABLE | PF_DIAG | PF_DISABLED, 0, 1, "", nullptr, CAP_PREHEAT),

    // --- Installation settings --------------------------------------------
    PT("cooling_vent_high", "Cooling high ventilation step", 4001, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 4, "", nullptr, CAP_COOLING),
    PT("winter_temp", "Winter ventilation outdoor limit", 4002, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_SIGNED, -20, 10, U_TEMP, nullptr, 0),
    PT("winter_vent", "Winter ventilation step", 4003, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 4, "", nullptr, 0),
    PT("airheat_type", "After-heater type", 4005, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "No additional heat|Electrical|Electric on binary relays", CAP_HEATING),
    PT("airheat_delay", "After-heater delay", 4006, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 60, "min", nullptr, CAP_HEATING),
    PT("temp_min_summer_set", "Summer min supply setting", 4007, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("temp_max_summer_set", "Summer max supply setting", 4008, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("temp_min_winter_set", "Winter min supply setting", 4009, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("temp_max_winter_set", "Winter max supply setting", 4010, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 5, 50, U_TEMP, nullptr, 0),
    PT("room_deadband", "Room temperature deadband", 4011, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 10, U_TEMP, nullptr, 0),
    PT("room_low_limit", "Low room temperature limit", 4012, HR, KIND_NUMBER, -2, PF_WRITABLE | PF_DIAG, 0, 20, U_TEMP, nullptr, 0),
    PT("airheat_select", "After-heater activation", 4013, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, CAP_HEATING),
    PT("inlet_min", "Minimum inlet level", 4015, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "", nullptr, 0),
    PT("exhaust_min", "Minimum exhaust level", 4016, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "", nullptr, 0),
    PT("exhaust_max", "Maximum exhaust level", 4017, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "", nullptr, 0),
    PT("start_delay", "Fan start delay", 4018, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 60, "min", nullptr, 0),
    PT("bypass_walk_time", "Bypass damper walk time", 4019, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 600, "s", nullptr, CAP_BYPASS),
    PT("restart_mode", "External fire alarm restart", 4040, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "Off|HPLP|Continue", 0),
    PT("power_save", "Power saving", 4041, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("inlet_scale", "Inlet fan scaling", 4098, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 200, "%", nullptr, 0),
    PT("exhaust_scale", "Exhaust fan scaling", 4099, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 200, "%", nullptr, 0),
    PT("inlet_speed_1", "Inlet speed step 1", 4100, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("inlet_speed_2", "Inlet speed step 2", 4101, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("inlet_speed_3", "Inlet speed step 3", 4102, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("inlet_speed_4", "Inlet speed step 4", 4103, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("exhaust_speed_1", "Exhaust speed step 1", 4104, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("exhaust_speed_2", "Exhaust speed step 2", 4105, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("exhaust_speed_3", "Exhaust speed step 3", 4106, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("exhaust_speed_4", "Exhaust speed step 4", 4107, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 0, 100, "%", nullptr, 0),
    PT("iaq_type", "Indoor air quality control", 4108, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "Off|Humidity|Humidity + CO2", 0),
    PT("analog_out_type", "Heater output type", 4109, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG | PF_DISABLED, 0, 4, "", "Period|0-10 V|0/5/10 V|Relay 3|Type 4", 0),
};

// ---------------------------------------------------------------------------
// CTS400 (ES1077)
// ---------------------------------------------------------------------------
static const PointDef CTS400_POINTS[] = {
    PT("d1", "Digital input D1", 20, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("d2", "Digital input D2", 21, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("d3", "Digital input D3", 22, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("bypass_open", "Bypass damper open", 23, IR, KIND_BINARY, 0, PF_NONE, 0, 1, "", nullptr, CAP_BYPASS),
    PT("extract_air", "Extract air", 24, IR, KIND_SENSOR, -1, PF_NONE, 0, 0, "%", nullptr, 0),
    PT("supply_air", "Supply air", 25, IR, KIND_SENSOR, -1, PF_NONE, 0, 0, "%", nullptr, 0),
    PT("after_heat", "Status after heating element", 26, IR, KIND_SENSOR, -1, PF_DIAG, 0, 0, "%", nullptr, CAP_HEATING),
    PT("t1_outdoor", "Outdoor temperature", 27, IR, KIND_SENSOR, -1, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t2_inlet", "Inlet temperature", 28, IR, KIND_SENSOR, -1, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t3_exhaust", "Exhaust temperature", 29, IR, KIND_SENSOR, -1, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("t4_outlet", "Outlet temperature", 30, IR, KIND_SENSOR, -1, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("rh", "Humidity", 31, IR, KIND_SENSOR, -1, PF_NONE, 0, 0, "%", nullptr, CAP_RH),
    PT("analog_input", "Analog input", 32, IR, KIND_SENSOR, -2, PF_DIAG, 0, 0, "V", nullptr, 0),
    PT("t7_inlet", "Inlet temperature after heater", 45, IR, KIND_SENSOR, -1, PF_SIGNED, 0, 0, U_TEMP, nullptr, 0),
    PT("rh_average", "Average humidity", 46, IR, KIND_SENSOR, -1, PF_DIAG, 0, 0, "%", nullptr, CAP_RH),
    PT("co2", "CO2", 47, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "ppm", nullptr, CAP_CO2),
    PT("voc", "VOC", 48, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "ppm", nullptr, CAP_VOC),
    PT("filter_alarm", "Filter change alarm", 49, IR, KIND_BINARY, 0, PF_NONE, 0, 1, "", nullptr, 0),
    PT("alarm_status", "Alarm status", 50, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_1_code", "Alarm code 1", 51, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_2_code", "Alarm code 2", 52, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("alarm_3_code", "Alarm code 3", 53, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("info_1", "Info alarm 1", 56, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("info_2", "Info alarm 2", 57, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("info_3", "Info alarm 3", 58, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "", nullptr, 0),
    PT("fan_level", "Fan level", 63, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "", nullptr, 0),
    PT("rh_24h_ok", "Humidity 24h OK", 64, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, CAP_RH),
    PT("fan_override", "Fan level override", 65, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("high_humidity", "High humidity level", 66, IR, KIND_SENSOR, -1, PF_DIAG, 0, 0, "%", nullptr, CAP_RH),
    PT("high_humidity_time", "High humidity time", 70, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "s", nullptr, CAP_RH),
    PT("season", "Season", 72, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "", "Summer|Winter", 0),
    PT("after_heat_on", "After heating active", 74, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, CAP_HEATING),
    PT("filter_timer", "Filter timer", 77, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("damper_test_age", "Fire damper test age", 87, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "h", nullptr, 0),
    PT("deicing", "De-icing", 91, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("d1_active_time", "D1 active time", 96, IR, KIND_SENSOR, 0, PF_DIAG | PF_DISABLED, 0, 0, "s", nullptr, 0),
    PT("fan_level_room_temp", "Fan level by room temperature", 97, IR, KIND_SENSOR, 0, PF_DIAG, 0, 0, "", nullptr, 0),
    PT("fire_damper_open", "Fire damper open", 100, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("fire_damper_closed", "Fire damper closed", 106, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("fire_input", "Fire input", 108, IR, KIND_BINARY, 0, PF_DIAG, 0, 1, "", nullptr, 0),
    PT("filter_days_back", "Filter days back", 110, IR, KIND_SENSOR, 0, PF_NONE, 0, 0, "days", nullptr, 0),

    PT("reset_alarm", "Reset alarm", 30, HR, KIND_BUTTON, 0, PF_WRITABLE | PF_DIAG | PF_DISABLED, 0, 1, "", nullptr, 0),
    PT("rh_low_limit", "Low humidity limit", 31, HR, KIND_NUMBER, -1, PF_WRITABLE, 15, 45, "%", nullptr, CAP_RH),
    PT("fan_rh_low", "Fan step at low humidity", 32, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 3, "", nullptr, CAP_RH),
    PT("fan_rh_high", "Fan step at high humidity", 33, HR, KIND_NUMBER, 0, PF_WRITABLE, 2, 4, "", nullptr, CAP_RH),
    PT("rh_timeout", "High humidity timeout", 34, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 180, "min", nullptr, CAP_RH),
    PT("co2_limit_high", "High CO2 limit", 35, HR, KIND_NUMBER, 0, PF_WRITABLE, 500, 2000, "ppm", nullptr, CAP_CO2),
    PT("voc_limit_high", "High VOC limit", 36, HR, KIND_NUMBER, 0, PF_WRITABLE, 500, 2000, "ppm", nullptr, CAP_VOC),
    PT("temp_set", "Room temperature setpoint", 37, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_SIGNED, 10, 30, U_TEMP, nullptr, 0),
    PT("reg_deadband", "Regulation deadband", 38, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_DIAG, 0, 4, U_TEMP, nullptr, 0),
    PT("deice_start_t4", "De-icing start T4", 39, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_DIAG, 1, 5, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("deice_stop_t4", "De-icing stop T4", 40, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_DIAG, 5, 10, U_TEMP, nullptr, CAP_COMPRESSOR),
    PT("deice_max_time", "Maximum de-icing time", 41, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 5, 60, "min", nullptr, CAP_COMPRESSOR),
    PT("deselect_t2", "Deselect T2 sensor", 42, HR, KIND_SWITCH, 0, PF_WRITABLE, 0, 1, "", nullptr, 0),
    PT("deice_interval", "Time between de-icing", 43, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 15, 760, "min", nullptr, CAP_COMPRESSOR),
    PT("season_temp", "Summer / winter limit", 45, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_DIAG, 5, 20, U_TEMP, nullptr, 0),
    PT("fire_reset", "Fire thermostat reset", 46, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("filter_alarm_panel", "Filter alarm on panel", 47, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("extra_sensor", "Extra sensor", 48, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "None|VOC|CO2", 0),
    PT("filter_days", "Days between filter change", 50, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 360, "days", nullptr, 0),
    PT("reset_filter", "Reset filter timer", 51, HR, KIND_BUTTON, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("heat_option", "Heating option", 53, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 1, 4, "", "Normal|Water|Electric|Type 4", 0),
    PT("min_supply_temp", "Minimum supply temperature", 57, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_SIGNED, 10, 20, U_TEMP, nullptr, CAP_HEATING),
    PT("max_supply_temp", "Maximum supply temperature", 58, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_SIGNED, 10, 50, U_TEMP, nullptr, CAP_HEATING),
    PT("supply_level_1", "Supply air level 1", 59, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("supply_level_2", "Supply air level 2", 60, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("supply_level_3", "Supply air level 3", 61, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("supply_level_4", "Supply air level 4", 62, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("extract_level_1", "Extract air level 1", 63, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("extract_level_2", "Extract air level 2", 64, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("extract_level_3", "Extract air level 3", 65, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("extract_level_4", "Extract air level 4", 66, HR, KIND_NUMBER, -1, PF_WRITABLE, 20, 100, "%", nullptr, 0),
    PT("vent_set", "Ventilation step", 69, HR, KIND_NUMBER, 0, PF_WRITABLE, 1, 4, "", nullptr, 0),
    PT("unit_stop", "Stop unit", 70, HR, KIND_SWITCH, 0, PF_WRITABLE, 0, 1, "", nullptr, 0),
    PT("user1_time_set", "User function 1 time", 71, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 180, "min", nullptr, 0),
    PT("fan_speed_lock", "Lock fan speed level", 72, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("unit_lock_on", "Unit cannot turn off", 73, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("damper_test_interval", "Fire damper test interval", 75, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 2, "", "7 days|14 days|28 days", 0),
    PT("supply_deicing", "Supply fan during de-icing", 78, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
    PT("user1_vent_set", "User function 1 ventilation step", 79, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 4, "", nullptr, 0),
    PT("high_level", "High fan level", 80, HR, KIND_NUMBER, 0, PF_WRITABLE | PF_DIAG, 2, 4, "", nullptr, 0),
    PT("low_room_stop", "Stop at low room temperature", 81, HR, KIND_NUMBER, -1, PF_WRITABLE | PF_DIAG, 0, 20, U_TEMP, nullptr, 0),
    PT("extra_input", "Extra input", 82, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG, 0, 3, "", "Off|User selection 2|Filter guard|Type 3", 0),
    PT("output_type", "Output type", 83, HR, KIND_SELECT, 0, PF_WRITABLE | PF_DIAG | PF_DISABLED, 0, 4, "", "Off|10V alarm|0-10V alarm|10V|Type 4", 0),
    PT("user2_time_set", "User function 2 time", 84, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 180, "min", nullptr, 0),
    PT("user2_vent_set", "User function 2 ventilation step", 85, HR, KIND_NUMBER, 0, PF_WRITABLE, 0, 4, "", nullptr, 0),
    PT("fire_polarity", "Fire input polarity", 86, HR, KIND_SWITCH, 0, PF_WRITABLE | PF_DIAG, 0, 1, "", nullptr, 0),
};

const PointDef *catalog_points(Profile profile) {
  switch (profile) {
    case PROFILE_CTS602:
      return CTS602_POINTS;
    case PROFILE_CTS400:
      return CTS400_POINTS;
    default:
      return nullptr;
  }
}

size_t catalog_size(Profile profile) {
  switch (profile) {
    case PROFILE_CTS602:
      return sizeof(CTS602_POINTS) / sizeof(CTS602_POINTS[0]);
    case PROFILE_CTS400:
      return sizeof(CTS400_POINTS) / sizeof(CTS400_POINTS[0]);
    default:
      return 0;
  }
}

const char *profile_name(Profile profile) {
  switch (profile) {
    case PROFILE_CTS602:
      return "cts602";
    case PROFILE_CTS400:
      return "cts400";
    default:
      return "unknown";
  }
}

static const char *const MODEL_NAMES[] = {
    "None",
    "Test",
    "VPL 10 uden koel",
    "VPL 15 uden koel",
    "VPL 15 med koel",
    "VPL 25 med 3 hastigheder uden koel",
    "VPL 25 med 3 hastigheder med koel",
    "VPL 28 2 hastigheder uden koel",
    "VPL 28 med 2 hastigheder med koel",
    "VP 18 med kryds monteret oven paa anlaeg uden koel",
    "VP 18 med kryds monteret oven paa anlaeg med koel",
    "VP 18 Compact og Compact P uden koel",
    "VP 18 Compact og Compact P med koel",
    "Comfort anlaeg (Comfort 300 LR)",
    "CT 150 anlaeg med 1-2-3 omskifter",
    "VLX som koerer VAV",
    "VLX med 2 trin",
    "VLX med 3 trin",
    "VP 18 uden koel",
    "VP 18 med koel",
    "VP 18 med elkedel uden koel",
    "VP 18 med elkedel og koel",
    "VGU 250 brugsvands varmepumpe",
    "VGU 250 brugsvands varmepumpe med elkedel",
    "VPL 25 uden koel",
    "VPL 25 med koel",
    "VPM 120-560",
    "Comfort 1200 - 4000",
    "VP 20 Compact gorona",
    "VLX med CTS 602 print",
    "Compact P Nordic",
    "Comfort Nordic",
    "VP 18 Version 1",
    "Combi 300",
    "Compact med 4-vejsventil uden koel",
};

const char *model_name(uint16_t model) {
  if (model < (sizeof(MODEL_NAMES) / sizeof(MODEL_NAMES[0])))
    return MODEL_NAMES[model];
  return "Unknown";
}

bool model_has_cooling(uint16_t model) {
  switch (model) {
    case 4:
    case 6:
    case 8:
    case 10:
    case 12:
    case 19:
    case 21:
    case 25:
      return true;
    default:
      return false;
  }
}

uint16_t caps_from_model(uint16_t model) {
  uint16_t caps = CAP_HEATING;
  if (model_has_cooling(model))
    caps |= CAP_COOLING;
  return caps;
}

const char *kind_name(uint8_t kind) {
  switch (kind) {
    case KIND_SENSOR:
      return "sensor";
    case KIND_BINARY:
      return "binary";
    case KIND_NUMBER:
      return "number";
    case KIND_SELECT:
      return "select";
    case KIND_SWITCH:
      return "switch";
    case KIND_BUTTON:
      return "button";
    default:
      return "sensor";
  }
}

const char *cap_name(uint16_t cap) {
  switch (cap) {
    case CAP_CO2:
      return "co2";
    case CAP_RH:
      return "rh";
    case CAP_VOC:
      return "voc";
    case CAP_COOLING:
      return "cooling";
    case CAP_HEATING:
      return "heating";
    case CAP_HOT_WATER:
      return "hot_water";
    case CAP_COMPRESSOR:
      return "compressor";
    case CAP_BYPASS:
      return "bypass";
    case CAP_CENTRAL_HEAT:
      return "central_heat";
    case CAP_PREHEAT:
      return "preheat";
    case CAP_EXPANSION:
      return "expansion";
    case CAP_PRESSURE:
      return "pressure";
    default:
      return "";
  }
}

}  // namespace auto_nilan
}  // namespace esphome

