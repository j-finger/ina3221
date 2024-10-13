#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <string>


#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdio_usb.h"

#include "json.hpp" // JSON library for settings

#include "INA3221.h"

#define DEVICE_ID "INA3221" // Placeholder Device ID

/* ----- I2C MACROS ----- */

// I2C pins and instance
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_PORT i2c0
#define INA3221_ADDRESS 0x40 // INA3221 I2C address (assuming A0 pin is set to GND)

const uint32_t baud_rate = 100000; // 100 kHz I2C

// This allows us to not need to prefix the INA3221 register addresses with `INA3221REG::`
using namespace INA3221REG;
using namespace INA3221SET;

using json = nlohmann::json;

using namespace std;


/* ----- Power Data Struct ----- */

struct powerData {
    uint32_t timestamp; // uint32_t - note that this should match your timestamp size when read from the register i.e. if its 24 bits, use uint24_t, if its 16 bits, use uint16_t. etc...
    float vcc_voltage;
    float vdd_voltage;
    float vddio_voltage;
    float vcc_voltage_drop;
    float vdd_voltage_drop;
    float vddio_voltage_drop;
};

/* ----- INA3221 Class ----- */

class INA3221 {
public:	
    INA3221(i2c_inst_t* i2c_port, uint32_t baud_rate, const std::string& device_id)
        : i2c_port_(i2c_port), baud_rate_(baud_rate), device_id_(device_id), i2c_address_(INA3221_ADDRESS) {
            
        ina3221_i2c_init(i2c_port, baud_rate);
        setup_power_monitor_default_settings();
    }


	powerData readTotalPower(){
		// Objective: read registers and put data into struct and return struct
		
		powerData data; // we declare the temporary stuct to hold the data

		data.vcc_voltage = readBusVoltage(1); // set default values
        data.vdd_voltage = readBusVoltage(2); // set default values
        data.vddio_voltage = readBusVoltage(3); // set default values

        data.vcc_voltage_drop = readShuntVoltage(1); // set default values
        data.vdd_voltage_drop = readShuntVoltage(2); // set default values
        data.vddio_voltage_drop = readShuntVoltage(3); // set default values

        data.timestamp = readTimestamp(); // set default values

		return data; // we return the struct as the function output
	}

    float readShuntVoltage(uint8_t channel){
        
        uint16_t rawShuntVoltage;
        // First bit is the sign bit, so we need to mask it out
        // The next 15 bits are the actual value. 
        // The data is in a 13-bit format, so we need to shift it to the right by 3 bits to get the actual value

        // read shunt voltage from channel
        if (channel == 1) {
            read_register(SHUNT_VOLTAGE_1, rawShuntVoltage);
        } else if (channel == 2) {
            read_register(SHUNT_VOLTAGE_2, rawShuntVoltage);
        } else if (channel == 3) {
            read_register(SHUNT_VOLTAGE_3, rawShuntVoltage);
        } else {
            // error handling
            printf("Invalid channel number\n");
            return 0;
        }

        // Check the sign bit
        int sign;
        uint8_t sign_bit = rawShuntVoltage >> 15; // shift it to the right by 15 bits to get the 16th bit
        if (sign_bit == 1) { // positive value
            sign = 1;
        } else { // negative value
            sign = -1;
        }

        // Getting the actual value - Convert raw readings to voltage
        // TODO: check the scaling factor from the datasheet
        uint16_t shunt_v = rawShuntVoltage >> 3; // shift it to the right by 3 bits
        float shunt_voltage = (float)sign * (float)rawShuntVoltage * 0.0000025;  // Based on datasheet scaling


        return shuntVoltage;
    }

    float readBusVoltage(uint8_t channel){
        // First bit is the sign bit, so we need to mask it out
        // The next 15 bits are the actual value. 
        // The data is in a 13-bit format, so we need to shift it to the right by 3 bits to get the actual value
        
        uint16_t rawBusVoltage;
        if (channel == 1) {
            read_register(BUS_VOLTAGE_1, rawBusVoltage);
        } else if (channel == 2) {
            read_register(BUS_VOLTAGE_2, rawBusVoltage);
        } else if (channel == 3) {
            read_register(BUS_VOLTAGE_3, rawBusVoltage);
        } else {
            // error handling
            printf("Invalid channel number\n");
            return 0;
        }

        // Check the sign bit
        int sign;
        uint8_t sign_bit = rawBusVoltage >> 15; // shift it to the right by 15 bits to get the 16th bit
        if (sign_bit == 1) { // positive value
            sign = 1;
        } else { // negative value
            sign = -1;
        }

        // Getting the actual value - Convert raw readings to voltage
        // TODO: check the scaling factor from the datasheet
        uint16_t bus_v = rawBusVoltage >> 3; // shift it to the right by 3 bits
        float bus_voltage = (float)sign * (float)bus_v * 0.00125; // Based on datasheet scaling

        return bus_voltage;
    }

    uint32_t readTimestamp(){
        // read timestamp from register
        uint32_t timestamp;
        read_registers(TIMESTAMP, &timestamp, sizeof(timestamp));
        return timestamp;
    }

	// Example convert power data to JSON
	json jsonify_data(const powerData& data_in) {
        // Pass in the power data struct, device id and time - just use placeholders for device id and time
        json power_json; // create json object

		// Assign values to JSON object
        power_json["device"] = device_id_; // Note this uses the device id set in the constructor
        power_json["power"]["timestamp"] = data.timestamp;
        power_json["power"]["vcc_voltage"] = data.vcc;
        power_json["power"]["vdd_voltage"] = data.vdd;
        power_json["power"]["vddio_voltage"] = data.vddio;
        power_json["power"]["vcc_voltage_drop"] = data.vcc_drop;
        power_json["power"]["vdd_voltage_drop"] = data.vdd_drop;
        power_json["power"]["vddio_voltage_drop"] = data.vddio_drop;
        
	    // return JSON object
        return power_json;
    }

private: 
	const string device_id_;
	uint32_t baud_rate_;
	i2c_inst_t* i2c_port_;
    uint8_t i2c_address_;

	/* ----- I2C ----- */

	void ina3221_i2c_init(i2c_inst_t* i2c_port, uint32_t baud_rate){
            // Initialize I2C
            // TODO: check how these are done in the Pico SDK and confirm the below code

            // 100 kHz I2C
            i2c_init(i2c_port_, baud_rate_);  // This function should be from the Pico SDK

            gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
            gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

            // Enable pull-up/down on SDA and SCL lines according to how the INA3221 is connected in the schematic
            gpio_pull_up(I2C_SDA_PIN);
            gpio_pull_up(I2C_SCL_PIN);
	}

    /* ----- Register Functions ----- */

	void write_register(uint16_t reg_addr, uint16_t data) {
		// Single write register function goes here
		// This function returns nothing (i.e. using `void` as the function return type) because we are only writing one register. 
        
        // TODO: make this match the method used in the Pico SDK
	}

	uint16_t read_register(uint16_t reg_addr) {
		// Single read register function goes here
		// Because we are only reading one register (which is generally 8 bits long), we know the length of the return data. So we can just return the data directly.

        // TODO: make this match the method used in the Pico SDK
	}

	void read_registers(uint16_t reg_addr, uint16_t* data, size_t length) {
		// multiple register read function goes here. 
		// Because we are reading multiple registers at once, we don't know how long the return data is going to be. So instead we pass in a pointer to where we are going to store the output data and this function just writes to that memory address. That's why we use `void` as the return type. 

        // TODO: make this match the method used in the Pico SDK
        return 0;
	}

    /* ----- Power Monitor Functions ----- */

	void setup_power_monitor_default_settings(){
        // setup power monitor settings to default
        // write to registers to set settings

        // TODO: This should be just the default settings for the INA3221 you determined from the datasheet
    }

}


/* ----- Main ----- */

int main() {

    // Initialize USB Serial output and all other functions for Pico
    stdio_init_all();

    // INA3221 initialization
    INA3221 ina3221(I2C_PORT, baud_rate, DEVICE_ID);

    while (true) {
        sleep_ms(2000);  // Delay for a second before next reading

        powerData data = ina3221.readTotalPower(); // read power data
        
        json power_json = ina3221.jsonify_data(data); // convert power data to JSON
        
        printf("JSON: %s\n", power_json.dump(4).c_str()); // print JSON to serial

        // TODO: Test the functionality for reading power data for each channel
        
    }

    return 0;
}
