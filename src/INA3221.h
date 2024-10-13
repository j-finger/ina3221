#ifndef INA3221_H
#define INA3221_H
#endif

#include <stdint.h>

// Register addresses for the INA3221 Power Monitor
// I have started you off with the register address table `8.6.2 Register Descriptions`. 

namespace INA3221REG {
	// Configuration Register
	// variable_type register_name = hex_address_value;
		
	// Sensor Measurements
	static constexpr uint8_t SHUNT_VOLTAGE_1 = 0x01;
	static constexpr uint8_t BUS_VOLTAGE_1 = 0x02;
	static constexpr uint8_t SHUNT_VOLTAGE_2 = 0x03;
	static constexpr uint8_t BUS_VOLTAGE_2 = 0x04;
	static constexpr uint8_t SHUNT_VOLTAGE_3 = 0x05;
	static constexpr uint8_t BUS_VOLTAGE_3 = 0x06;
	
	// Critical and Warning Limits
	static constexpr uint8_t CRITICAL_LIMIT_1 = 0x07;
	static constexpr uint8_t WARNING_LIMIT_1 = 0x08;
	static constexpr uint8_t CRITICAL_LIMIT_2 = 0x09;
	static constexpr uint8_t WARNING_LIMIT_2 = 0x0A;
	static constexpr uint8_t CRITICAL_LIMIT_3 = 0x0B;
	static constexpr uint8_t WARNING_LIMIT_3 = 0x0C;
  
	// Power Calculation
	static constexpr uint8_t SHUNT_VOLTAGE_SUM = 0x0D;
	static constexpr uint8_t SHUNT_VOLTAGE_SUM_LIMIT = 0x0E;
	
	// Mask/Enable
	static constexpr uint8_t MASK_ENABLE = 0x0F;
	
	// Alert Limit
	static constexpr uint8_t POWER_VALID_UPPER_LIMIT = 0x10;
	static constexpr uint8_t POWER_VALID_LOWER_LIMIT = 0x11;

	// Manufacturer and Die ID
	static constexpr uint8_t MANUFACTURER_ID = 0xFE;
	static constexpr uint8_t DIE_ID = 0xFF;
}


// You may also like to put simple settings here, so you don't have to remember the hex values. You may also like to simply just set these manually in the code. It's your choice

namespace INA3221SET {
	// variable_type register_name = hex_address_value;

    // Config register settings
    // TODO: What does this do? What is the objective?
    #define INA3221_CONFIG_ENABLE_CH1 (1 << 14)
    #define INA3221_CONFIG_ENABLE_CH2 (1 << 13)
    #define INA3221_CONFIG_ENABLE_CH3 (1 << 12)

    // Usually we just add basic settings here that we can just call quickly
    // They are usually simple settings combintions that we leave as binary values
    // For complex settings you put the logic in the class methods in the object

}
