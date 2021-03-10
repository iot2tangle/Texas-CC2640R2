/*
 * sensors.c
 *
 *  Created on: 6 mar 2021
 *      Author: Raul
 */


#include <ti/drivers/i2c/I2CCC26XX.h>
#include <stdint.h>
#include <board.h>
I2C_Handle handle;
uint8_t rxBuf[40];
I2C_Params params;


void init_mpu6060(){
    int status;
    I2C_Params_init(&params);
    params.transferMode=I2C_MODE_BLOCKING;
    params.bitRate=I2C_100kHz;
    I2C_Transaction i2cTrans;

    uint8_t txBufacc[]={0x6b,0x00};      // Transmit turn on
    // Initialize master I2C transaction structure
    i2cTrans.writeCount   = 2;
    i2cTrans.writeBuf     = txBufacc;
    i2cTrans.readCount    = 0;
    i2cTrans.readBuf      = rxBuf;
    i2cTrans.slaveAddress = 0x68;

    // Open I2C
    handle = I2C_open(Board_I2C0, &params);

    status=I2C_transfer(handle, &i2cTrans);
    I2C_close(handle);
}


void get_acceleration(int16_t *x, int16_t*y, int16_t*z){
    int status;
    I2C_Params_init(&params);
    params.transferMode=I2C_MODE_BLOCKING;
    params.bitRate=I2C_100kHz;
    I2C_Transaction i2cTrans;
    uint8_t txBufacc[]={0x3b};      // Transmit turn on
    // Initialize master I2C transaction structure
    i2cTrans.writeCount   = 1;
    i2cTrans.writeBuf     = txBufacc;
    i2cTrans.readCount    = 6;
    i2cTrans.readBuf      = rxBuf;
    i2cTrans.slaveAddress = 0x68;

    handle = I2C_open(Board_I2C0, NULL);
    status=I2C_transfer(handle, &i2cTrans);


    *x=rxBuf[0]<<8;
    *x|=rxBuf[1];
    *x=*x/16;

    *y=rxBuf[2]<<8;
    *y|=rxBuf[3];
    *y=*y/16;

    *z=rxBuf[4]<<8;
    *z|=rxBuf[5];
    *z=*z/16;
    I2C_close(handle);
}


void get_gyroscope(int16_t *x, int16_t*y, int16_t*z){
    int status;
    I2C_Params_init(&params);
    params.transferMode=I2C_MODE_BLOCKING;
    params.bitRate=I2C_100kHz;
    I2C_Transaction i2cTrans;
    uint8_t txBufacc[]={0x43};      // Transmit turn on
    // Initialize master I2C transaction structure
    i2cTrans.writeCount   = 1;
    i2cTrans.writeBuf     = txBufacc;
    i2cTrans.readCount    = 6;
    i2cTrans.readBuf      = rxBuf;
    i2cTrans.slaveAddress = 0x68;

    handle = I2C_open(Board_I2C0, NULL);
    status=I2C_transfer(handle, &i2cTrans);


    *x=rxBuf[0]<<8;
    *x|=rxBuf[1];
    *x=*x/16;

    *y=rxBuf[2]<<8;
    *y|=rxBuf[3];
    *y=*y/16;

    *z=rxBuf[4]<<8;
    *z|=rxBuf[5];
    *z=*z/16;
    I2C_close(handle);
}



void init_BH1750(){
    int status;
        I2C_Params_init(&params);
        params.transferMode=I2C_MODE_BLOCKING;
        params.bitRate=I2C_100kHz;
        I2C_Transaction i2cTrans;

        uint8_t txBufacc[]={0x01};      // Transmit turn on
        // Initialize master I2C transaction structure
        i2cTrans.writeCount   = 1;
        i2cTrans.writeBuf     = txBufacc;
        i2cTrans.readCount    = 0;
        i2cTrans.readBuf      = rxBuf;
        i2cTrans.slaveAddress = 0x23;

        // Open I2C
        handle = I2C_open(Board_I2C0, &params);

        status=I2C_transfer(handle, &i2cTrans);
        txBufacc[0]=0x10;
        status=I2C_transfer(handle, &i2cTrans);
        I2C_close(handle);
}



void get_light(uint16_t *light){
        int status;
        I2C_Params_init(&params);
        memset(rxBuf,0,sizeof(rxBuf));
        params.transferMode=I2C_MODE_BLOCKING;
        params.bitRate=I2C_100kHz;
        I2C_Transaction i2cTrans;
        uint8_t txBufacc[]={0};      // Transmit turn on
        // Initialize master I2C transaction structure
        i2cTrans.writeCount   = 0;
        i2cTrans.writeBuf     = txBufacc;
        i2cTrans.readCount    = 2;
        i2cTrans.readBuf      = rxBuf;
        i2cTrans.slaveAddress = 0x23;

        handle = I2C_open(Board_I2C0, NULL);
        status=I2C_transfer(handle, &i2cTrans);


        *light=rxBuf[0]<<8;
        *light|=rxBuf[1];
        *light=(*light>>2)*5;


        I2C_close(handle);
}




int8_t dig_H6;
uint16_t dig_T1,dig_P1,dig_H1,dig_H3;
int16_t dig_T2,dig_T3,dig_P2,dig_P3,dig_P4,dig_P5,dig_P6,dig_P7,dig_P8,dig_P9,dig_H2,dig_H4,dig_H5;
void init_BME280(){
    int status;
        I2C_Params_init(&params);
        params.transferMode=I2C_MODE_BLOCKING;
        params.bitRate=I2C_100kHz;
        I2C_Transaction i2cTrans;

        uint8_t txBufacc[]={0xF2, 0x02,
                            0xF4,0x4b,
                            0xf5,0x04
       };      // Transmit turn on
        // Initialize master I2C transaction structure
        i2cTrans.writeCount   = sizeof(txBufacc);
        i2cTrans.writeBuf     = txBufacc;
        i2cTrans.readCount    = 0;
        i2cTrans.readBuf      = rxBuf;
        i2cTrans.slaveAddress = 0x76;

        // Open I2C
        handle = I2C_open(Board_I2C0, &params);

        status=I2C_transfer(handle, &i2cTrans);




        memset(rxBuf,0,sizeof(rxBuf));
        //ora leggo i parametri 88-E6 32 byte
        txBufacc[0]=0x88;
        i2cTrans.writeCount   = 1;
        i2cTrans.writeBuf     = txBufacc;
        i2cTrans.readCount    = 26;
        i2cTrans.readBuf      = rxBuf;
        i2cTrans.slaveAddress = 0x76;
        status=I2C_transfer(handle, &i2cTrans);

        txBufacc[0]=0xE1;
        i2cTrans.writeCount   = 1;
        i2cTrans.writeBuf     = txBufacc;
        i2cTrans.readCount    = 7;
        i2cTrans.readBuf      = rxBuf+26;
        i2cTrans.slaveAddress = 0x76;
        status=I2C_transfer(handle, &i2cTrans);

        dig_T1=(rxBuf[1]<<8)|rxBuf[0];
        dig_T2=(rxBuf[3]<<8)|rxBuf[2];
        dig_T3=(rxBuf[5]<<8)|rxBuf[4];

        dig_P1=(rxBuf[7]<<8)|rxBuf[6];
        dig_P2=(rxBuf[9]<<8)|rxBuf[8];
        dig_P3=(rxBuf[11]<<8)|rxBuf[10];
        dig_P4=(rxBuf[13]<<8)|rxBuf[12];
        dig_P5=(rxBuf[15]<<8)|rxBuf[14];
        dig_P6=(rxBuf[17]<<8)|rxBuf[16];
        dig_P7=(rxBuf[19]<<8)|rxBuf[18];
        dig_P8=(rxBuf[21]<<8)|rxBuf[20];
        dig_P9=(rxBuf[23]<<8)|rxBuf[22];

        dig_H1=rxBuf[25];
        dig_H2=(rxBuf[27]<<8)|rxBuf[26];
        dig_H3=rxBuf[28];
        dig_H4=(rxBuf[29]<<4)|(rxBuf[30]&0x0F);
        dig_H5=(rxBuf[31]<<4)|(rxBuf[30]>>4);
        dig_H6=rxBuf[32];


        I2C_close(handle);
}

int32_t t_fine;
int32_t BME280_compensate_T_int32(int32_t adc_T){
    int32_t var1,var2,T;
    var1=((((adc_T>>3)-((int32_t)dig_T1<<1)))*((int32_t)dig_T2))>>11;
    var2=(((((adc_T>>4)-((int32_t)dig_T1))*((adc_T>>4)-((int32_t)dig_T1)))>>12)*((int32_t)dig_T3))>>14;
    t_fine=var1+var2;
    T=(t_fine*5+128)>>8;
    return T;
}



uint32_t BME280_compensate_H(int32_t adc_H)
{
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
        ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r *
        ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
        ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    v_x1_u32r = v_x1_u32r>>12;
    return (uint32_t)((v_x1_u32r * 1000)>>10);
}


uint32_t BME280_compensate_P_int32(int32_t adc_P)
{
    int32_t var1, var2;
uint32_t p;
  var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
  var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
  var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
  var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
  var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
  if (p < 0x80000000) {
    p = (p << 1) / ((uint32_t)var1);
  } else {
    p = (p / (uint32_t)var1) * 2;
  }
  var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
  return p;
}

int32_t tfinal,pfinal,hfinal;
void get_environment(float *humidity,float *temperature,float *pressure){
        int status;
        I2C_Params_init(&params);
        memset(rxBuf,0,sizeof(rxBuf));
        params.transferMode=I2C_MODE_BLOCKING;
        params.bitRate=I2C_100kHz;
        I2C_Transaction i2cTrans;
        uint8_t txBufacc[]={0xF7};      // Transmit turn on
        // Initialize master I2C transaction structure
        i2cTrans.writeCount   = 1;
        i2cTrans.writeBuf     = txBufacc;
        i2cTrans.readCount    = 8;
        i2cTrans.readBuf      = rxBuf;
        i2cTrans.slaveAddress = 0x76;

        handle = I2C_open(Board_I2C0, NULL);
        status=I2C_transfer(handle, &i2cTrans);


        int32_t press;
        uint32_t hum;
        int32_t temp;

        press=(rxBuf[0]<<12)|(rxBuf[1]<<4)|(rxBuf[2]>>4);
        temp=(rxBuf[3]<<12)|(rxBuf[4]<<4)|(rxBuf[5]>>4);
        hum=(rxBuf[6]<<8)|(rxBuf[7]);

        tfinal=BME280_compensate_T_int32(temp);
        hfinal=BME280_compensate_H(hum);
        pfinal=BME280_compensate_P_int32(press);
        I2C_close(handle);

        *humidity=((float)(hfinal))/1000;
        *temperature=((float)(tfinal))/100;
        *pressure=((float)(pfinal))/100;
}

