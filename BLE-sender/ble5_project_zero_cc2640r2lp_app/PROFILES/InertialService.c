/**********************************************************************************************
 * Filename:       InertialService.c
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

#include "InertialService.h"

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

// InertialService Service UUID
CONST uint8_t InertialServiceUUID[ATT_UUID_SIZE] =
{
 0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};

// Name UUID
CONST uint8_t InertialService_NameUUID[ATT_UUID_SIZE] =
{
 0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Accx UUID
CONST uint8_t InertialService_AccxUUID[ATT_UUID_SIZE] =
{
 0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Accy UUID
CONST uint8_t InertialService_AccyUUID[ATT_UUID_SIZE] =
{
 0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// Accz UUID
CONST uint8_t InertialService_AcczUUID[ATT_UUID_SIZE] =
{
 0x03,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// gyrox UUID
CONST uint8_t InertialService_GyroxUUID[ATT_UUID_SIZE] =
{
 0x04,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// gyroy UUID
CONST uint8_t InertialService_GyroyUUID[ATT_UUID_SIZE] =
{
 0x05,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};
// gyroz UUID
CONST uint8_t InertialService_GyrozUUID[ATT_UUID_SIZE] =
{
 0x06,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x05,0x00
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static InertialServiceCBs_t *pAppCBs = NULL;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t InertialServiceDecl = { ATT_UUID_SIZE, InertialServiceUUID };

// Characteristic "Name" Properties (for declaration)
static uint8_t InertialService_NameProps = GATT_PROP_READ;

// Characteristic "Name" Value variable
static uint8_t InertialService_NameVal[INERTIALSERVICE_NAME_LEN] = {0};
// Characteristic "Accx" Properties (for declaration)
static uint8_t InertialService_AccxProps = GATT_PROP_READ;

// Characteristic "Accx" Value variable
static uint8_t InertialService_AccxVal[INERTIALSERVICE_ACCX_LEN] = {0};
// Characteristic "Accy" Properties (for declaration)
static uint8_t InertialService_AccyProps = GATT_PROP_READ;

// Characteristic "Accy" Value variable
static uint8_t InertialService_AccyVal[INERTIALSERVICE_ACCY_LEN] = {0};
// Characteristic "Accz" Properties (for declaration)
static uint8_t InertialService_AcczProps = GATT_PROP_READ;

// Characteristic "Accz" Value variable
static uint8_t InertialService_AcczVal[INERTIALSERVICE_ACCZ_LEN] = {0};
// Characteristic "Gyrox" Properties (for declaration)
static uint8_t InertialService_GyroxProps = GATT_PROP_READ;

// Characteristic "Gyrox" Value variable
static uint8_t InertialService_GyroxVal[INERTIALSERVICE_GYROX_LEN] = {0};
// Characteristic "Gyroy" Properties (for declaration)
static uint8_t InertialService_GyroyProps = GATT_PROP_READ;

// Characteristic "Gyroy" Value variable
static uint8_t InertialService_GyroyVal[INERTIALSERVICE_GYROY_LEN] = {0};
// Characteristic "Gyroz" Properties (for declaration)
static uint8_t InertialService_GyrozProps = GATT_PROP_READ;

// Characteristic "Gyroz" Value variable
static uint8_t InertialService_GyrozVal[INERTIALSERVICE_GYROZ_LEN] = {0};

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t InertialServiceAttrTbl[] =
{
  // InertialService Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&InertialServiceDecl
  },
    // Name Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_NameProps
    },
      // Name Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_NameUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_NameVal
      },
    // Accx Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_AccxProps
    },
      // Accx Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_AccxUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_AccxVal
      },
    // Accy Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_AccyProps
    },
      // Accy Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_AccyUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_AccyVal
      },
    // Accz Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_AcczProps
    },
      // Accz Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_AcczUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_AcczVal
      },
    // Gyrox Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_GyroxProps
    },
      // Gyrox Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_GyroxUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_GyroxVal
      },
    // Gyroy Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_GyroyProps
    },
      // Gyroy Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_GyroyUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_GyroyVal
      },
    // Gyroz Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &InertialService_GyrozProps
    },
      // Gyroz Characteristic Value
      {
        { ATT_UUID_SIZE, InertialService_GyrozUUID },
        GATT_PERMIT_READ,
        0,
        InertialService_GyrozVal
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t InertialService_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                           uint16_t maxLen, uint8_t method );
static bStatus_t InertialService_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                            uint8_t *pValue, uint16_t len, uint16_t offset,
                                            uint8_t method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t InertialServiceCBs =
{
  InertialService_ReadAttrCB,  // Read callback function pointer
  InertialService_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * InertialService_AddService- Initializes the InertialService service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t InertialService_AddService( uint8_t rspTaskId )
{
  uint8_t status;

  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( InertialServiceAttrTbl,
                                        GATT_NUM_ATTRS( InertialServiceAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &InertialServiceCBs );

  return ( status );
}

/*
 * InertialService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t InertialService_RegisterAppCBs( InertialServiceCBs_t *appCallbacks )
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
 * InertialService_SetParameter - Set a InertialService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t InertialService_SetParameter( uint8_t param, uint16_t len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case INERTIALSERVICE_NAME_ID:
      if ( len == INERTIALSERVICE_NAME_LEN )
      {
        memcpy(InertialService_NameVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case INERTIALSERVICE_ACCX_ID:
      if ( len == INERTIALSERVICE_ACCX_LEN )
      {
        memcpy(InertialService_AccxVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case INERTIALSERVICE_ACCY_ID:
      if ( len == INERTIALSERVICE_ACCY_LEN )
      {
        memcpy(InertialService_AccyVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case INERTIALSERVICE_ACCZ_ID:
      if ( len == INERTIALSERVICE_ACCZ_LEN )
      {
        memcpy(InertialService_AcczVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case INERTIALSERVICE_GYROX_ID:
      if ( len == INERTIALSERVICE_GYROX_LEN )
      {
        memcpy(InertialService_GyroxVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case INERTIALSERVICE_GYROY_ID:
      if ( len == INERTIALSERVICE_GYROY_LEN )
      {
        memcpy(InertialService_GyroyVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case INERTIALSERVICE_GYROZ_ID:
      if ( len == INERTIALSERVICE_GYROZ_LEN )
      {
        memcpy(InertialService_GyrozVal, value, len);
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
 * InertialService_GetParameter - Get a InertialService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t InertialService_GetParameter( uint8_t param, uint16_t *len, void *value )
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
 * @fn          InertialService_ReadAttrCB
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
static bStatus_t InertialService_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                       uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                       uint16_t maxLen, uint8_t method )
{
  bStatus_t status = SUCCESS;

  // See if request is regarding the Name Characteristic Value
if ( ! memcmp(pAttr->type.uuid, InertialService_NameUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_NAME_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_NAME_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Accx Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, InertialService_AccxUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_ACCX_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_ACCX_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Accy Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, InertialService_AccyUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_ACCY_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_ACCY_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Accz Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, InertialService_AcczUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_ACCZ_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_ACCZ_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Gyrox Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, InertialService_GyroxUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_GYROX_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_GYROX_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Gyroy Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, InertialService_GyroyUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_GYROY_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_GYROY_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Gyroz Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, InertialService_GyrozUUID, pAttr->type.len) )
  {
    if ( offset > INERTIALSERVICE_GYROZ_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, INERTIALSERVICE_GYROZ_LEN - offset);  // Transmit as much as possible
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
 * @fn      InertialService_WriteAttrCB
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
static bStatus_t InertialService_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
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
      uint16_t svcUuid = INERTIALSERVICE_SERV_UUID;
      pAppCBs->pfnChangeCb(connHandle, svcUuid, paramID, len, pValue); // Call app function from stack task context.
    }
  return status;
}
