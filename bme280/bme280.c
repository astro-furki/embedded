/*
 * bme280.c
 *
 *  Created on: Jun 15, 2023
 *      Author: pickle
 */

#include "bme280.h"
#include "inttypes.h"

extern I2C_HandleTypeDef hi2c1;

//uint8_t TrimParam[36];

uint16_t 	dig_T1,
			dig_P1,
			dig_H1, dig_H3;

int16_t 	dig_T2, dig_T3,
			dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9,
			dig_H2, dig_H4, dig_H5, dig_H6;

void bme280_isready(void){
	while(HAL_I2C_IsDeviceReady(&hi2c1, BME280_ADDRESS, 1, 100)!=HAL_OK){
		;
	}
}

void bme280_init(void){

	//device reset
	uint8_t reset_data = 0xB6;
	HAL_I2C_Mem_Write(&hi2c1, BME280_ADDRESS, 0xE0, 1, &reset_data, 1, 1000);
	HAL_Delay(10);

	//control humidity
	uint8_t ctrl_hum = 0x01;
	HAL_I2C_Mem_Write(&hi2c1, BME280_ADDRESS, 0xF2, 1, &ctrl_hum, 1, 1000);
	HAL_Delay(10);

	//control temperature and pressure
	uint8_t ctrl_meas = 0x57;
	HAL_I2C_Mem_Write(&hi2c1, BME280_ADDRESS, 0xF4, 1, &ctrl_meas, 1, 1000);
	HAL_Delay(10);

	//config of tstandby, IIR, SPI
	uint8_t dev_config = 0x10;
	HAL_I2C_Mem_Write(&hi2c1, BME280_ADDRESS, 0xF5, 1, &dev_config, 1, 1000);
	HAL_Delay(10);

}

void BMEReadRaw(struct bme280_comp_data *comp_data) {
	//bme280_init();
	uint8_t RawData[8];
	struct bme280_uncomp_data uncomp_data;

	uint8_t reg = 0xF7;
	// Read the Registers 0xF7 to 0xFE
	//BME280_IsDataReady();
	HAL_I2C_Master_Transmit(&hi2c1, BME280_ADDRESS, &reg, 1, 10);

	HAL_I2C_Master_Receive(&hi2c1, BME280_ADDRESS /*| 0x01*/, RawData, 8, 10);
	TrimRead();

	/* Calculate the Raw data for the parameters
	 * Here the Pressure and Temperature are in 20 bit format and humidity in 16 bit format
	 */
	uncomp_data.pressure = (RawData[0] << 12) | (RawData[1] << 4)
			| (RawData[2] >> 4);
	uncomp_data.temperature = (RawData[3] << 12) | (RawData[4] << 4)
			| (RawData[5] >> 4);
	uncomp_data.humidity = (RawData[6] << 8) | (RawData[7]);

	comp_data->pressure =(BME280_compensate_P_int32(uncomp_data.pressure));
	comp_data->temperature = (BME280_compensate_T_int32(uncomp_data.temperature));
	comp_data->humidity = (bme280_compensate_H_int32(uncomp_data.humidity));

}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BME280_S32_t t_fine;
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T) {
	BME280_S32_t var1, var2, T;
	var1 =((((adc_T >> 3) - ((BME280_S32_t) dig_T1 << 1))) * ((BME280_S32_t)dig_T2))>> 11;
	var2 =(((((adc_T >> 4) - ((BME280_S32_t) dig_T1)) * ((adc_T>>4) - ((BME280_S32_t)dig_T1)))>> 12)*((BME280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

void TrimRead(void){
	uint8_t trimdata[32];

	HAL_I2C_Mem_Read(&hi2c1, BME280_ADDRESS, 0x88, 1, trimdata, 25, HAL_MAX_DELAY);

	HAL_I2C_Mem_Read(&hi2c1, BME280_ADDRESS, 0xE1, 1, ((uint8_t *)trimdata)+25, 7, HAL_MAX_DELAY);

	dig_T1 = (trimdata[1]<<8) | trimdata[0];
	dig_T2 = (trimdata[3]<<8) | trimdata[2];
	dig_T3 = (trimdata[5]<<8) | trimdata[4];
	dig_P1 = (trimdata[7]<<8) | trimdata[6];
	dig_P2 = (trimdata[9]<<8) | trimdata[8];
	dig_P3 = (trimdata[11]<<8) | trimdata[10];
	dig_P4 = (trimdata[13]<<8) | trimdata[12];
	dig_P5 = (trimdata[15]<<8) | trimdata[14];
	dig_P6 = (trimdata[17]<<8) | trimdata[16];
	dig_P7 = (trimdata[19]<<8) | trimdata[18];
	dig_P8 = (trimdata[21]<<8) | trimdata[20];
	dig_P9 = (trimdata[23]<<8) | trimdata[22];
	dig_H1 = trimdata[24];
	dig_H2 = (trimdata[26]<<8) | trimdata[25];
	dig_H3 = trimdata[27];
	dig_H4 = (trimdata[28]<<4) | (trimdata[29] & 0x0F);
	dig_H5 = (trimdata[30]<<4) | (trimdata[29]>>4);
	dig_H6 = trimdata[31];
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P) {
	BME280_S64_t var1, var2, p;
	var1 = ((BME280_S64_t)t_fine) - 128000;
	var2 = var1 * var1 * (BME280_S64_t)dig_P6;
	var2 = var2 + ((var1*(BME280_S64_t)dig_P5)<<17);
	var2 = var2 + (((BME280_S64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (BME280_S64_t)dig_P3)>>8) + ((var1 * (BME280_S64_t)dig_P2)<<12);
	var1 = (((((BME280_S64_t)1)<<47)+var1))*((BME280_S64_t)dig_P1)>>33;

	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((BME280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((BME280_S64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7)<<4);
	p = p/256;
	return (BME280_U32_t) p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H) {
	BME280_S32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((BME280_S32_t) 76800));
	v_x1_u32r =(((((adc_H << 14) - (((BME280_S32_t) dig_H4) << 20) - (((BME280_S32_t)dig_H5) * v_x1_u32r)) + ((BME280_S32_t)16384)) >> 15)* (((((((v_x1_u32r * ((BME280_S32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((BME280_S32_t)dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) * ((BME280_S32_t)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t) dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (BME280_U32_t) (v_x1_u32r >> 12);
}

BME280_U32_t BME280_compensate_P_int32(BME280_S32_t adc_P) {
	BME280_S32_t var1, var2;
	BME280_U32_t p;
	var1 = (((BME280_S32_t) t_fine) >> 1) - (BME280_S32_t) 64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((BME280_S32_t) dig_P6);
	var2 = var2 + ((var1 * ((BME280_S32_t) dig_P5)) << 1);
	var2 = (var2 >> 2) + (((BME280_S32_t) dig_P4) << 16);
	var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((BME280_S32_t) dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((BME280_S32_t) dig_P1)) >> 15);
	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}
	p = (((BME280_U32_t) (((BME280_S32_t) 1048576) - adc_P) - (var2 >> 12)))
			* 3125;
	if (p < 0x80000000) {
		p = (p << 1) / ((BME280_U32_t) var1);
	} else {
		p = (p / (BME280_U32_t) var1) * 2;
	}
	var1 = (((BME280_S32_t) dig_P9)
			* ((BME280_S32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
	var2 = (((BME280_S32_t) (p >> 2)) * ((BME280_S32_t) dig_P8)) >> 13;
	p = (BME280_U32_t) ((BME280_S32_t) p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}
