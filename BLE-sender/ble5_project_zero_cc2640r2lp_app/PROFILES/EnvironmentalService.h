/**********************************************************************************************
 * Filename:       EnvironmentalService.h
 *
 * Description:    This file contains the EnvironmentalService service definitions and
 *                 prototypes.
 *
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************************************/


#ifndef _ENVIRONMENTALSERVICE_H_
#define _ENVIRONMENTALSERVICE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
* CONSTANTS
*/
// Service UUID
#define ENVIRONMENTALSERVICE_SERV_UUID 0xBABE

//  Characteristic defines
#define ENVIRONMENTALSERVICE_NAME_ID   7
#define ENVIRONMENTALSERVICE_NAME_UUID 0xBEEF
#define ENVIRONMENTALSERVICE_NAME_LEN  24

//  Characteristic defines
#define ENVIRONMENTALSERVICE_LIGHT_ID   8
#define ENVIRONMENTALSERVICE_LIGHT_UUID 0xBEEF
#define ENVIRONMENTALSERVICE_LIGHT_LEN  17

//  Characteristic defines
#define ENVIRONMENTALSERVICE_SOUND_ID   9
#define ENVIRONMENTALSERVICE_SOUND_UUID 0xBEEF
#define ENVIRONMENTALSERVICE_SOUND_LEN  16

//  Characteristic defines
#define ENVIRONMENTALSERVICE_PRESSURE_ID   10
#define ENVIRONMENTALSERVICE_PRESSURE_UUID 0xBEEF
#define ENVIRONMENTALSERVICE_PRESSURE_LEN  23

//  Characteristic defines
#define ENVIRONMENTALSERVICE_HUMIDITY_ID   11
#define ENVIRONMENTALSERVICE_HUMIDITY_UUID 0xBEEF
#define ENVIRONMENTALSERVICE_HUMIDITY_LEN  23

//  Characteristic defines
#define ENVIRONMENTALSERVICE_TEMPERATURE_ID   12
#define ENVIRONMENTALSERVICE_TEMPERATURE_UUID 0xBEEF
#define ENVIRONMENTALSERVICE_TEMPERATURE_LEN  26

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*EnvironmentalServiceChange_t)(uint16_t connHandle, uint16_t svcUuid, uint8_t paramID, uint16_t len, uint8_t *pValue);

typedef struct
{
  EnvironmentalServiceChange_t        pfnChangeCb;  // Called when characteristic value changes
  EnvironmentalServiceChange_t        pfnCfgChangeCb;
} EnvironmentalServiceCBs_t;



/*********************************************************************
 * API FUNCTIONS
 */


/*
 * EnvironmentalService_AddService- Initializes the EnvironmentalService service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t EnvironmentalService_AddService( uint8_t rspTaskId);

/*
 * EnvironmentalService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t EnvironmentalService_RegisterAppCBs( EnvironmentalServiceCBs_t *appCallbacks );

/*
 * EnvironmentalService_SetParameter - Set a EnvironmentalService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t EnvironmentalService_SetParameter(uint8_t param, uint16_t len, void *value);

/*
 * EnvironmentalService_GetParameter - Get a EnvironmentalService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t EnvironmentalService_GetParameter(uint8_t param, uint16_t *len, void *value);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _ENVIRONMENTALSERVICE_H_ */
