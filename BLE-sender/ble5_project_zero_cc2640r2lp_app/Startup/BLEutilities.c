/*
 * BLEutilities.c
 *
 *  Created on: 7 mar 2021
 *      Author: Raul
 */



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t char_value[30];

void charValueInt(uint8_t * name, int value){

    char value_temp[5];
    int flagEmpty=0;
    memset(char_value, 0, sizeof(char_value));
    strcpy(char_value,(uint8_t*)"{\"");
    strcat(char_value, name);
    strcat(char_value,(uint8_t*)"\":\"");

    if(value<0){
        strcat(char_value,"-");
    value=-value;
    }else{
        strcat(char_value,"+");
    }

    value_temp[1]=value/1000;
    value_temp[2]=(value-value_temp[1]*1000)/100;
    value_temp[3]=(value-value_temp[2]*100-value_temp[1]*1000)/10;
    value_temp[4]=(value-value_temp[2]*100-value_temp[3]*10-value_temp[1]*1000);



    if(value_temp[1]==0){
        flagEmpty++;

        if(value_temp[2]==0){
            flagEmpty++;
            if(value_temp[3]==0){
                flagEmpty++;
            }
        }
    }
    memset(value_temp,0,sizeof(value_temp));
    sprintf(value_temp,"%d\"",value);
    strcat(char_value,value_temp);
    while(flagEmpty>0){
        flagEmpty--;
        strcat(char_value," ");
    }

    strcat(char_value,"}");




}

void charValueFloat(uint8_t * name, float value){
char value_temp[6];
int flagEmpty=0;
int newValue=(int16_t)(value);
memset(char_value, 0, sizeof(char_value));
strcpy(char_value,(uint8_t*)"{\"");
strcat(char_value, name);
strcat(char_value,(uint8_t*)"\":\"");

if(value<0){
    strcat(char_value,"-");
value=-value;
}else{
    strcat(char_value,"+");
}

value_temp[1]=value/1000;
value_temp[2]=(value-value_temp[1]*1000)/100;
value_temp[3]=(value-value_temp[2]*100-value_temp[1]*1000)/10;
value_temp[4]=(value-value_temp[2]*100-value_temp[3]*10-value_temp[1]*1000);



if(value_temp[1]==0){
    flagEmpty++;

    if(value_temp[2]==0){
        flagEmpty++;
        if(value_temp[3]==0){
            flagEmpty++;
        }
    }
}
memset(value_temp,0,sizeof(value_temp));
sprintf(value_temp,"%.2f\"",value);
strcat(char_value,value_temp);
while(flagEmpty>0){
    flagEmpty--;
    strcat(char_value," ");
}

strcat(char_value,"}");


}
