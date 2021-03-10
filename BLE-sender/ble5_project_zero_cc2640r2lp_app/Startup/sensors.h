/*
 * sensors.h
 *
 *  Created on: 6 mar 2021
 *      Author: Raul
 */

#ifndef STARTUP_SENSORS_H_
#define STARTUP_SENSORS_H_


void init_mpu6060();
void get_acceleration(int16_t *x, int16_t*y, int16_t*z);
void get_gyroscope(int16_t *x, int16_t*y, int16_t*z);

void init_BH1750();
void get_light(uint16_t *light);

void get_environment(float *humidity,float *temperature,float *pressure);
uint32_t BME280_compensate_P_int32(int32_t adc_P);
uint32_t BME280_compensate_H(int32_t adc_H);
int32_t BME280_compensate_T_int32(int32_t adc_T);
void init_BME280();

#endif /* STARTUP_SENSORS_H_ */

