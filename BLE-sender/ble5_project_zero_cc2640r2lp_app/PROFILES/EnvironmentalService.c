/**********************************************************************************************
 * Filename:       EnvironmentalService.c
 *
 * Description:    This file contains the implementation of the service.
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


/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <icall.h>

/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "EnvironmentalService.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
* GLOBAL VARIABLES
*/

// EnvironmentalService Service UUID
CONST uint8_t EnvironmentalServiceUUID[ATT_UUID_SIZE] =
{
 0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};

// Name UUID
CONST uint8_t EnvironmentalService_NameUUID[ATT_UUID_SIZE] =
{
 0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Light UUID
CONST uint8_t EnvironmentalService_LightUUID[ATT_UUID_SIZE] =
{
 0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Sound UUID
CONST uint8_t EnvironmentalService_SoundUUID[ATT_UUID_SIZE] =
{
 0x02,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Pressure UUID
CONST uint8_t EnvironmentalService_PressureUUID[ATT_UUID_SIZE] =
{
 0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Humidity UUID
CONST uint8_t EnvironmentalService_HumidityUUID[ATT_UUID_SIZE] =
{
 0x04,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Temperature UUID
CONST uint8_t EnvironmentalService_TemperatureUUID[ATT_UUID_SIZE] =
{
 0x05,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static EnvironmentalServiceCBs_t *pAppCBs = NULL;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t EnvironmentalServiceDecl = { ATT_UUID_SIZE, EnvironmentalServiceUUID };

// Characteristic "Name" Properties (for declaration)
static uint8_t EnvironmentalService_NameProps = GATT_PROP_READ;

// Characteristic "Name" Value variable
static uint8_t EnvironmentalService_NameVal[ENVIRONMENTALSERVICE_NAME_LEN] = {0};
// Characteristic "Light" Properties (for declaration)
static uint8_t EnvironmentalService_LightProps = GATT_PROP_READ;

// Characteristic "Light" Value variable
static uint8_t EnvironmentalService_LightVal[ENVIRONMENTALSERVICE_LIGHT_LEN] = {0};
// Characteristic "Sound" Properties (for declaration)
static uint8_t EnvironmentalService_SoundProps = GATT_PROP_READ;

// Characteristic "Sound" Value variable
static uint8_t EnvironmentalService_SoundVal[ENVIRONMENTALSERVICE_SOUND_LEN] = {0};
// Characteristic "Pressure" Properties (for declaration)
static uint8_t EnvironmentalService_PressureProps = GATT_PROP_READ;

// Characteristic "Pressure" Value variable
static uint8_t EnvironmentalService_PressureVal[ENVIRONMENTALSERVICE_PRESSURE_LEN] = {0};
// Characteristic "Humidity" Properties (for declaration)
static uint8_t EnvironmentalService_HumidityProps = GATT_PROP_READ;

// Characteristic "Humidity" Value variable
static uint8_t EnvironmentalService_HumidityVal[ENVIRONMENTALSERVICE_HUMIDITY_LEN] = {0};
// Characteristic "Temperature" Properties (for declaration)
static uint8_t EnvironmentalService_TemperatureProps = GATT_PROP_READ;

// Characteristic "Temperature" Value variable
static uint8_t EnvironmentalService_TemperatureVal[ENVIRONMENTALSERVICE_TEMPERATURE_LEN] = {0};

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t EnvironmentalServiceAttrTbl[] =
{
  // EnvironmentalService Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&EnvironmentalServiceDecl
  },
    // Name Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &EnvironmentalService_NameProps
    },
      // Name Characteristic Value
      {
        { ATT_UUID_SIZE, EnvironmentalService_NameUUID },
        GATT_PERMIT_READ,
        0,
        EnvironmentalService_NameVal
      },
    // Light Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &EnvironmentalService_LightProps
    },
      // Light Characteristic Value
      {
        { ATT_UUID_SIZE, EnvironmentalService_LightUUID },
        GATT_PERMIT_READ,
        0,
        EnvironmentalService_LightVal
      },
    // Sound Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &EnvironmentalService_SoundProps
    },
      // Sound Characteristic Value
      {
        { ATT_UUID_SIZE, EnvironmentalService_SoundUUID },
        GATT_PERMIT_READ,
        0,
        EnvironmentalService_SoundVal
      },
    // Pressure Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &EnvironmentalService_PressureProps
    },
      // Pressure Characteristic Value
      {
        { ATT_UUID_SIZE, EnvironmentalService_PressureUUID },
        GATT_PERMIT_READ,
        0,
        EnvironmentalService_PressureVal
      },
    // Humidity Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &EnvironmentalService_HumidityProps
    },
      // Humidity Characteristic Value
      {
        { ATT_UUID_SIZE, EnvironmentalService_HumidityUUID },
        GATT_PERMIT_READ,
        0,
        EnvironmentalService_HumidityVal
      },
    // Temperature Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &EnvironmentalService_TemperatureProps
    },
      // Temperature Characteristic Value
      {
        { ATT_UUID_SIZE, EnvironmentalService_TemperatureUUID },
        GATT_PERMIT_READ,
        0,
        EnvironmentalService_TemperatureVal
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t EnvironmentalService_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                           uint16_t maxLen, uint8_t method );
static bStatus_t EnvironmentalService_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                            uint8_t *pValue, uint16_t len, uint16_t offset,
                                            uint8_t method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t EnvironmentalServiceCBs =
{
  EnvironmentalService_ReadAttrCB,  // Read callback function pointer
  EnvironmentalService_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * EnvironmentalService_AddService- Initializes the EnvironmentalService service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t EnvironmentalService_AddService( uint8_t rspTaskId )
{
  uint8_t status;

  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( EnvironmentalServiceAttrTbl,
                                        GATT_NUM_ATTRS( EnvironmentalServiceAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &EnvironmentalServiceCBs );

  return ( status );
}

/*
 * EnvironmentalService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t EnvironmentalService_RegisterAppCBs( EnvironmentalServiceCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pAppCBs = appCallbacks;

    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

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
bStatus_t EnvironmentalService_SetParameter( uint8_t param, uint16_t len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case ENVIRONMENTALSERVICE_NAME_ID:
      if ( len == ENVIRONMENTALSERVICE_NAME_LEN )
      {
        memcpy(EnvironmentalService_NameVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ENVIRONMENTALSERVICE_LIGHT_ID:
      if ( len == ENVIRONMENTALSERVICE_LIGHT_LEN )
      {
        memcpy(EnvironmentalService_LightVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ENVIRONMENTALSERVICE_SOUND_ID:
      if ( len == ENVIRONMENTALSERVICE_SOUND_LEN )
      {
        memcpy(EnvironmentalService_SoundVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ENVIRONMENTALSERVICE_PRESSURE_ID:
      if ( len == ENVIRONMENTALSERVICE_PRESSURE_LEN )
      {
        memcpy(EnvironmentalService_PressureVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ENVIRONMENTALSERVICE_HUMIDITY_ID:
      if ( len == ENVIRONMENTALSERVICE_HUMIDITY_LEN )
      {
        memcpy(EnvironmentalService_HumidityVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case ENVIRONMENTALSERVICE_TEMPERATURE_ID:
      if ( len == ENVIRONMENTALSERVICE_TEMPERATURE_LEN )
      {
        memcpy(EnvironmentalService_TemperatureVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*
 * EnvironmentalService_GetParameter - Get a EnvironmentalService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t EnvironmentalService_GetParameter( uint8_t param, uint16_t *len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*********************************************************************
 * @fn          EnvironmentalService_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t EnvironmentalService_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                       uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                       uint16_t maxLen, uint8_t method )
{
  bStatus_t status = SUCCESS;

  // See if request is regarding the Name Characteristic Value
if ( ! memcmp(pAttr->type.uuid, EnvironmentalService_NameUUID, pAttr->type.len) )
  {
    if ( offset > ENVIRONMENTALSERVICE_NAME_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, ENVIRONMENTALSERVICE_NAME_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Light Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, EnvironmentalService_LightUUID, pAttr->type.len) )
  {
    if ( offset > ENVIRONMENTALSERVICE_LIGHT_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, ENVIRONMENTALSERVICE_LIGHT_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Sound Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, EnvironmentalService_SoundUUID, pAttr->type.len) )
  {
    if ( offset > ENVIRONMENTALSERVICE_SOUND_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, ENVIRONMENTALSERVICE_SOUND_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Pressure Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, EnvironmentalService_PressureUUID, pAttr->type.len) )
  {
    if ( offset > ENVIRONMENTALSERVICE_PRESSURE_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, ENVIRONMENTALSERVICE_PRESSURE_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Humidity Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, EnvironmentalService_HumidityUUID, pAttr->type.len) )
  {
    if ( offset > ENVIRONMENTALSERVICE_HUMIDITY_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, ENVIRONMENTALSERVICE_HUMIDITY_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Temperature Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, EnvironmentalService_TemperatureUUID, pAttr->type.len) )
  {
    if ( offset > ENVIRONMENTALSERVICE_TEMPERATURE_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, ENVIRONMENTALSERVICE_TEMPERATURE_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has READ permissions.
    *pLen = 0;
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  return status;
}


/*********************************************************************
 * @fn      EnvironmentalService_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t EnvironmentalService_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                        uint8_t *pValue, uint16_t len, uint16_t offset,
                                        uint8_t method )
{
  bStatus_t status  = SUCCESS;
  uint8_t   paramID = 0xFF;

  // See if request is regarding a Client Characterisic Configuration
  if ( ! memcmp(pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len) )
  {
    // Allow only notifications.
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY);
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has WRITE permissions.
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  // Let the application know something changed (if it did) by using the
  // callback it registered earlier (if it did).
  if (paramID != 0xFF)
    if ( pAppCBs && pAppCBs->pfnChangeCb )
    {
      uint16_t svcUuid = ENVIRONMENTALSERVICE_SERV_UUID;
      pAppCBs->pfnChangeCb(connHandle, svcUuid, paramID, len, pValue); // Call app function from stack task context.
    }
  return status;
}
