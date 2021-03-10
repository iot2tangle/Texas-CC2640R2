/**********************************************************************************************
 * Filename:       InertialService.h
 *
 * Description:    This file contains the InertialService service definitions and
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


#ifndef _INERTIALSERVICE_H_
#define _INERTIALSERVICE_H_

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
#define INERTIALSERVICE_SERV_UUID 0xBABE

//  Characteristic defines
#define INERTIALSERVICE_NAME_ID   0
#define INERTIALSERVICE_NAME_UUID 0xBEEF
#define INERTIALSERVICE_NAME_LEN  18

//  Characteristic defines
#define INERTIALSERVICE_ACCX_ID   1
#define INERTIALSERVICE_ACCX_UUID 0xBEEF
#define INERTIALSERVICE_ACCX_LEN  26

//  Characteristic defines
#define INERTIALSERVICE_ACCY_ID   2
#define INERTIALSERVICE_ACCY_UUID 0xBEEF
#define INERTIALSERVICE_ACCY_LEN  26

//  Characteristic defines
#define INERTIALSERVICE_ACCZ_ID   3
#define INERTIALSERVICE_ACCZ_UUID 0xBEEF
#define INERTIALSERVICE_ACCZ_LEN  26

//  Characteristic defines
#define INERTIALSERVICE_GYROX_ID   4
#define INERTIALSERVICE_GYROX_UUID 0xBEEF
#define INERTIALSERVICE_GYROX_LEN  18

//  Characteristic defines
#define INERTIALSERVICE_GYROY_ID   5
#define INERTIALSERVICE_GYROY_UUID 0xBEEF
#define INERTIALSERVICE_GYROY_LEN  18

//  Characteristic defines
#define INERTIALSERVICE_GYROZ_ID   6
#define INERTIALSERVICE_GYROZ_UUID 0xBEEF
#define INERTIALSERVICE_GYROZ_LEN  18

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
typedef void (*InertialServiceChange_t)(uint16_t connHandle, uint16_t svcUuid, uint8_t paramID, uint16_t len, uint8_t *pValue);

typedef struct
{
  InertialServiceChange_t        pfnChangeCb;  // Called when characteristic value changes
  InertialServiceChange_t        pfnCfgChangeCb;
} InertialServiceCBs_t;



/*********************************************************************
 * API FUNCTIONS
 */


/*
 * InertialService_AddService- Initializes the InertialService service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t InertialService_AddService( uint8_t rspTaskId);

/*
 * InertialService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t InertialService_RegisterAppCBs( InertialServiceCBs_t *appCallbacks );

/*
 * InertialService_SetParameter - Set a InertialService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t InertialService_SetParameter(uint8_t param, uint16_t len, void *value);

/*
 * InertialService_GetParameter - Get a InertialService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t InertialService_GetParameter(uint8_t param, uint16_t *len, void *value);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _INERTIALSERVICE_H_ */
