/*
 * bme280.h
 *
 *  Created on: Jun 15, 2023
 *      Author: pickle
 */

#ifndef BME280_BME280_H_
#define BME280_BME280_H_

#define BME280_ADDRESS	(0x77<<1)

#define BME280_S32_t	int32_t
#define BME280_U32_t	uint32_t
#define BME280_S64_t	int64_t


#include "stm32u5xx_hal.h"
#include "stdlib.h"
#include "string.h"



struct bme280_comp_data{
	uint32_t pressure;
	uint32_t temperature;
	uint32_t humidity;
};

struct bme280_uncomp_data{
	uint32_t pressure;
	uint32_t temperature;
	uint32_t humidity;
};


/*void bme280_parse_sensor_data(const uint8_t *reg_data, struct bme280_uncomp_data *uncomp_data);*/
void bme280_init(void);
void BMEReadRaw(struct bme280_comp_data *comp_data);
void bme280_isready(void);
void TrimRead(void);

BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T);
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P);
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H);
BME280_U32_t BME280_compensate_P_int32(BME280_S32_t adc_P);



#endif /* BME280_BME280_H_ */
