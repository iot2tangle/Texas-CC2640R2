/******************************************************************************

   @file  project_zero.c

   @brief This file contains the Project Zero sample application for use
        with the CC2650 Bluetooth Low Energy Protocol Stack.

   Group: CMCU, LPRF
   Target Device: cc2640r2

 ******************************************************************************
   
 Copyright (c) 2015-2021, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
   
   
 *****************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <string.h>

#if !(defined __TI_COMPILER_VERSION__)
#include <intrinsics.h>
#endif

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/drivers/utils/List.h>
#include <ti/drivers/GPIO.h>

//#include <xdc/runtime/Log.h> // Comment this in to use xdc.runtime.Log
#include <uartlog/UartLog.h>  // Comment out if using xdc Log

#include <ti/display/AnsiColor.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

#include <icall.h>
#include <bcomdef.h>
/* This Header file contains all BLE API and icall structure definition */
#include <icall_ble_api.h>

/* Bluetooth Profiles */
#include <devinfoservice.h>

/* Application specific includes */
#include <Board.h>

#include <project_zero.h>
#include <util.h>
#include "EnvironmentalService.h"
#include "InertialService.h"

#include DeviceFamily_constructPath(driverlib/aon_batmon.h)


#include "config.h"
#include "sensors.h"
#include "BLEutilities.h"
#include <ti/drivers/i2c/I2CCC26XX.h>
/*********************************************************************
 * MACROS
 */

// Spin if the expression is not true
#define APP_ASSERT(expr) if(!(expr)) {project_zero_spin();}

#define UTIL_ARRTOHEX_REVERSE     1
#define UTIL_ARRTOHEX_NO_REVERSE  0

/*********************************************************************
 * CONSTANTS
 */
// Task configuration
#define PZ_TASK_PRIORITY                     1

#ifndef PZ_TASK_STACK_SIZE
#define PZ_TASK_STACK_SIZE                   2048
#endif

// Internal Events for RTOS application
#define PZ_ICALL_EVT                         ICALL_MSG_EVENT_ID  // Event_Id_31
#define PZ_APP_MSG_EVT                       Event_Id_30

// Bitwise OR of all RTOS events to pend on
#define PZ_ALL_EVENTS                        (PZ_ICALL_EVT | \
                                              PZ_APP_MSG_EVT)

// Types of messages that can be sent to the user application task from other
// tasks or interrupts. Note: Messages from BLE Stack are sent differently.
#define PZ_SERVICE_WRITE_EVT     0  /* A characteristic value has been written     */
#define PZ_SERVICE_CFG_EVT       1  /* A characteristic configuration has changed  */
#define PZ_UPDATE_CHARVAL_EVT    2  /* Request from ourselves to update a value    */
#define PZ_BUTTON_DEBOUNCED_EVT  3  /* A button has been debounced with new value  */
#define PZ_PAIRSTATE_EVT         4  /* The pairing state is updated                */
#define PZ_PASSCODE_EVT          5  /* A pass-code/PIN is requested during pairing */
#define PZ_ADV_EVT               6  /* A subscribed advertisement activity         */
#define PZ_START_ADV_EVT         7  /* Request advertisement start from task ctx   */
#define PZ_SEND_PARAM_UPD_EVT    8  /* Request parameter update req be sent        */
#define PZ_CONN_EVT              9  /* Connection Event End notice                 */
#define APP_MSG_PERIODIC_TIMER   10

// General discoverable mode: advertise indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) for parameter update request
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     12

// Maximum connection interval (units of 1.25ms, 800=1000ms) for  parameter update request
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     36

// Slave latency to use for parameter update request
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) for parameter update request
#define DEFAULT_DESIRED_CONN_TIMEOUT          200

// Supervision timeout conversion rate to miliseconds
#define CONN_TIMEOUT_MS_CONVERSION            10

// Connection interval conversion rate to miliseconds
#define CONN_INTERVAL_MS_CONVERSION           1.25

// Pass parameter updates to the app for it to decide.
#define DEFAULT_PARAM_UPDATE_REQ_DECISION     GAP_UPDATE_REQ_PASS_TO_APP

// Delay (in ms) after connection establishment before sending a parameter update requst
#define PZ_SEND_PARAM_UPDATE_DELAY            6000

/*********************************************************************
 * TYPEDEFS
 */
// Struct for messages sent to the application task
typedef struct
{
    uint8_t event;
    void    *pData;
} pzMsg_t;

// Struct for messages about characteristic data
typedef struct
{
    uint16_t svcUUID; // UUID of the service
    uint16_t dataLen; //
    uint8_t paramID; // Index of the characteristic
    uint8_t data[]; // Flexible array member, extended to malloc - sizeof(.)
} pzCharacteristicData_t;

// Struct for message about sending/requesting passcode from peer.
typedef struct
{
    uint16_t connHandle;
    uint8_t uiInputs;
    uint8_t uiOutputs;
    uint32_t numComparison;
} pzPasscodeReq_t;

// Struct for message about a pending parameter update request.
typedef struct
{
    uint16_t connHandle;
} pzSendParamReq_t;

// Struct for message about button state
typedef struct
{
    PIN_Id pinId;
    uint8_t state;
} pzButtonState_t;

// Container to store passcode data when passing from gapbondmgr callback
// to app event. See the pfnPairStateCB_t documentation from the gapbondmgr.h
// header file for more information on each parameter.
typedef struct
{
    uint8_t state;
    uint16_t connHandle;
    uint8_t status;
} pzPairStateData_t;

// Container to store passcode data when passing from gapbondmgr callback
// to app event. See the pfnPasscodeCB_t documentation from the gapbondmgr.h
// header file for more information on each parameter.
typedef struct
{
    uint8_t deviceAddr[B_ADDR_LEN];
    uint16_t connHandle;
    uint8_t uiInputs;
    uint8_t uiOutputs;
    uint32_t numComparison;
} pzPasscodeData_t;

// Container to store advertising event data when passing from advertising
// callback to app event. See the respective event in GapAdvScan_Event_IDs
// in gap_advertiser.h for the type that pBuf should be cast to.
typedef struct
{
    uint32_t event;
    void *pBuf;
} pzGapAdvEventData_t;

// List element for parameter update and PHY command status lists
typedef struct
{
    List_Elem elem;
    uint16_t *connHandle;
} pzConnHandleEntry_t;

// Connected device information
typedef struct
{
    uint16_t connHandle;                    // Connection Handle
    Clock_Struct* pUpdateClock;             // pointer to clock struct
    bool phyCngRq;                          // Set to true if PHY change request is in progress
    uint8_t currPhy;                        // The active PHY for a connection
    uint8_t rqPhy;                          // The requested PHY for a connection
    uint8_t phyRqFailCnt;                   // PHY change request fail count
} pzConnRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Task configuration
Task_Struct pzTask;
#if defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(appTaskStack, 8)
#else
#pragma data_alignment=8
#endif
uint8_t appTaskStack[PZ_TASK_STACK_SIZE];

/*********************************************************************
 * LOCAL VARIABLES
 */




// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Event globally used to post local events and pend on system and
// local events.
static ICall_SyncHandle syncEvent;

// Queue object used for app messages
static Queue_Struct appMsgQueue;
static Queue_Handle appMsgQueueHandle;

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "Texas2Tangle";

// Advertisement data
static uint8_t advertData[] =
{
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // complete name
    13, // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'T',
    'e',
    'x',
    'a',
    's',
    '2',
    't',
    'a',
    'n',
    'g',
    'l',
    'e'
};

// Scan Response Data

// Advertising handles
static uint8_t advHandleLegacy;

// Per-handle connection info
static pzConnRec_t connList[MAX_NUM_BLE_CONNS];

// List to store connection handles for set phy command status's
static List_List setPhyCommStatList;

// List to store connection handles for queued param updates
static List_List paramUpdateList;



/*
 * Initial LED pin configuration table
 *   - LEDs Board_PIN_LED0 & Board_PIN_LED1 are off.
 */
PIN_Config ledPinTable[] = {
    Board_PIN_RLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL |
    PIN_DRVSTR_MAX,
    Board_PIN_GLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL |
    PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/*
 * Application button pin configuration table:
 *   - Buttons interrupts are configured to trigger on falling edge.
 */
PIN_Config buttonPinTable[] = {
    Board_PIN_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    Board_PIN_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    PIN_TERMINATE
};

// Clock objects for debouncing the buttons
static Clock_Struct button0DebounceClock;
static Clock_Struct button1DebounceClock;
static Clock_Handle button0DebounceClockHandle;
static Clock_Handle button1DebounceClockHandle;

// State of the buttons
static uint8_t button0State = 0;
static uint8_t button1State = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/* Task functions */
static void ProjectZero_init(void);
static void ProjectZero_taskFxn(UArg a0,
                                UArg a1);

/* Event message processing functions */
static void ProjectZero_processStackEvent(uint32_t stack_event);
static void ProjectZero_processApplicationMessage(pzMsg_t *pMsg);
static uint8_t ProjectZero_processGATTMsg(gattMsgEvent_t *pMsg);
static void ProjectZero_processGapMessage(gapEventHdr_t *pMsg);
static void ProjectZero_processHCIMsg(ICall_HciExtEvt *pMsg);
static void ProjectZero_processPairState(pzPairStateData_t *pPairState);
static void ProjectZero_processPasscode(pzPasscodeReq_t *pReq);
static void ProjectZero_processCmdCompleteEvt(hciEvt_CmdComplete_t *pMsg);
static void ProjectZero_processAdvEvent(pzGapAdvEventData_t *pEventData);

/* Profile value change handlers */


/* Stack or profile callback function */
static void ProjectZero_advCallback(uint32_t event,
                                    void *pBuf,
                                    uintptr_t arg);
static void ProjectZero_passcodeCb(uint8_t *pDeviceAddr,
                                   uint16_t connHandle,
                                   uint8_t uiInputs,
                                   uint8_t uiOutputs,
                                   uint32_t numComparison);
static void ProjectZero_pairStateCb(uint16_t connHandle,
                                    uint8_t state,
                                    uint8_t status);


/* Connection handling functions */
static uint8_t ProjectZero_getConnIndex(uint16_t connHandle);
static uint8_t ProjectZero_clearConnListEntry(uint16_t connHandle);
static uint8_t ProjectZero_addConn(uint16_t connHandle);
static uint8_t ProjectZero_removeConn(uint16_t connHandle);
static void ProjectZero_updatePHYStat(uint16_t eventCode,
                                      uint8_t *pMsg);
static void ProjectZero_handleUpdateLinkParamReq(
    gapUpdateLinkParamReqEvent_t *pReq);
static void ProjectZero_sendParamUpdate(uint16_t connHandle);
static void ProjectZero_handleUpdateLinkEvent(gapLinkUpdateEvent_t *pEvt);
static void ProjectZero_paramUpdClockHandler(UArg arg);
static void ProjectZero_processConnEvt(Gap_ConnEventRpt_t *pReport);

/* Button handling functions */
static void buttonDebounceSwiFxn(UArg buttonId);
static void buttonCallbackFxn(PIN_Handle handle,
                              PIN_Id pinId);



/* Utility functions */
static status_t ProjectZero_enqueueMsg(uint8_t event,
                                   void *pData);
static char * util_arrtohex(uint8_t const *src,
                            uint8_t src_len,
                            uint8_t       *dst,
                            uint8_t dst_len,
                            uint8_t reverse);
static char * util_getLocalNameStr(const uint8_t *advData, uint8_t len);
static void ProjectZero_processL2CAPMsg(l2capSignalEvent_t *pMsg);

/*********************************************************************
 * EXTERN FUNCTIONS
 */
extern void AssertHandler(uint8_t assertCause,
                          uint8_t assertSubcause);


extern uint8_t char_value[100];

uint16_t light;
int16_t accx,accy,accz;
int16_t gyrox,gyroy,gyroz;
uint8_t sound;
float pressure;
float humidity;
float temperature;
static Clock_Struct myClock;

void updateSensors(){

    sound=GPIO_read(Board_GPIO_BTN1);
    if(!sound){
        EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_SOUND_ID, ENVIRONMENTALSERVICE_SOUND_LEN, "{\"acoustic\":\"1\"}");
    }else{
        EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_SOUND_ID, ENVIRONMENTALSERVICE_SOUND_LEN, "{\"acoustic\":\"0\"}");
    }
    get_light(&light);
    get_acceleration(&accx, &accy, &accz);
    get_gyroscope(&gyrox, &gyroy, &gyroz);
    charValueInt("acceleration x", accx);
    InertialService_SetParameter(INERTIALSERVICE_ACCX_ID, INERTIALSERVICE_ACCX_LEN, char_value);
    charValueInt("acceleration y", accy);
    InertialService_SetParameter(INERTIALSERVICE_ACCY_ID, INERTIALSERVICE_ACCY_LEN, char_value);
    charValueInt("acceleration z", accz);
    InertialService_SetParameter(INERTIALSERVICE_ACCZ_ID, INERTIALSERVICE_ACCZ_LEN, char_value);
    charValueInt("gyro x", gyrox);
    InertialService_SetParameter(INERTIALSERVICE_GYROX_ID, INERTIALSERVICE_GYROX_LEN, char_value);
    charValueInt("gyro y", gyroy);
    InertialService_SetParameter(INERTIALSERVICE_GYROY_ID, INERTIALSERVICE_GYROY_LEN, char_value);
    charValueInt("gyro z", gyroz);
    InertialService_SetParameter(INERTIALSERVICE_GYROZ_ID, INERTIALSERVICE_GYROZ_LEN, char_value);
    charValueInt("light", light);
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_LIGHT_ID, ENVIRONMENTALSERVICE_LIGHT_LEN, char_value);
    get_environment(&humidity, &temperature, &pressure);
    charValueFloat("Pressure", pressure);
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_PRESSURE_ID, ENVIRONMENTALSERVICE_PRESSURE_LEN, char_value);
    charValueFloat("Humidity", humidity);
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_HUMIDITY_ID, ENVIRONMENTALSERVICE_HUMIDITY_LEN, char_value);

    if(INTERNAL_TEMPERATURE){
        AONBatMonEnable();
        temperature=AONBatMonTemperatureGetDegC();
        AONBatMonDisable();
    }
    charValueFloat("Temperature", temperature);
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_TEMPERATURE_ID, ENVIRONMENTALSERVICE_TEMPERATURE_LEN, char_value);



}

/*********************************************************************
 * PROFILE CALLBACKS
 */
// GAP Bond Manager Callbacks
static gapBondCBs_t ProjectZero_BondMgrCBs =
{
    ProjectZero_passcodeCb,     // Passcode callback
    ProjectZero_pairStateCb     // Pairing/Bonding state Callback
};

/*
 * Callbacks in the user application for events originating from BLE services.
 */
// LED Service callback handler.
// The type LED_ServiceCBs_t is defined in led_service.h



/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn     project_zero_spin
 *
 * @brief   Spin forever
 */
static void project_zero_spin(void)
{
  volatile uint8_t x = 0;;

  while(1)
  {
    x++;
  }
}

void myClockSwiFxn(uintptr_t arg0)
{
  // Can't call blocking TI-RTOS calls or BLE APIs from here.
  // .. Send a message to the Task that something is afoot.
    ProjectZero_enqueueMsg(APP_MSG_PERIODIC_TIMER, NULL);// Not sending any data here, just a signal
}


/*********************************************************************
 * @fn      ProjectZero_createTask
 *
 * @brief   Task creation function for the Project Zero.
 */
void ProjectZero_createTask(void)
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = appTaskStack;
    taskParams.stackSize = PZ_TASK_STACK_SIZE;
    taskParams.priority = PZ_TASK_PRIORITY;

    Task_construct(&pzTask, ProjectZero_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * @fn      ProjectZero_init
 *
 * @brief   Called during initialization and contains application
 *          specific initialization (ie. hardware initialization/setup,
 *          table initialization, power up notification, etc), and
 *          profile initialization/setup.
 */
static void ProjectZero_init(void)
{
    // ******************************************************************
    // NO STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
    // ******************************************************************
    // Register the current thread as an ICall dispatcher application
    // so that the application can send and receive messages.
    ICall_registerApp(&selfEntity, &syncEvent);

    // Initialize queue for application messages.
    // Note: Used to transfer control to application thread from e.g. interrupts.
    Queue_construct(&appMsgQueue, NULL);
    appMsgQueueHandle = Queue_handle(&appMsgQueue);

    // ******************************************************************
    // Hardware initialization
    // ******************************************************************



    // Set the Device Name characteristic in the GAP GATT Service
    // For more information, see the section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

    // Configure GAP for param update
    {
        uint16_t paramUpdateDecision = DEFAULT_PARAM_UPDATE_REQ_DECISION;

        // Pass all parameter update requests to the app for it to decide
        GAP_SetParamValue(GAP_PARAM_LINK_UPDATE_DECISION, paramUpdateDecision);
    }

    // Setup the GAP Bond Manager. For more information see the GAP Bond Manager
    // section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    {
        // Don't send a pairing request after connecting (the peer device must
        // initiate pairing)
        uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        // Use authenticated pairing: require passcode.
        uint8_t mitm = TRUE;
        // This device only has display capabilities. Therefore, it will display the
        // passcode during pairing. However, since the default passcode is being
        // used, there is no need to display anything.
        uint8_t ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
        // Request bonding (storing long-term keys for re-encryption upon subsequent
        // connections without repairing)
        uint8_t bonding = TRUE;

        GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t),
                                &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES, sizeof(uint8_t),
                                &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED, sizeof(uint8_t),
                                &bonding);
    }



    // clockParams is only used during init and can be on the stack.
    Clock_Params myClockParams;
    // Insert default params
    Clock_Params_init(&myClockParams);
    // Set a period, so it times out periodically without jitter
    myClockParams.period = UPDATE_SENSORS_TIMEOUT * (1000/Clock_tickPeriod), // 500 ms, conversion from ms to clock ticks.
    // Initialize the clock object / Clock_Struct previously added globally.
    Clock_construct(&myClock, myClockSwiFxn,
                    0, // Initial delay before first timeout
                    &myClockParams);



    // ******************************************************************
    // BLE Service initialization
    // ******************************************************************
    GGS_AddService(GATT_ALL_SERVICES);         // GAP GATT Service
    GATTServApp_AddService(GATT_ALL_SERVICES); // GATT Service
    DevInfo_AddService();                      // Device Information Service

    EnvironmentalService_AddService(selfEntity);
    uint8_t EnvironmentalService_Name_initVal[ENVIRONMENTALSERVICE_NAME_LEN] = "{\"Name\":\"Environmental\"}";
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_NAME_ID, ENVIRONMENTALSERVICE_NAME_LEN, EnvironmentalService_Name_initVal);
    uint8_t EnvironmentalService_Light_initVal[ENVIRONMENTALSERVICE_LIGHT_LEN] = {0};
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_LIGHT_ID, ENVIRONMENTALSERVICE_LIGHT_LEN, EnvironmentalService_Light_initVal);
    uint8_t EnvironmentalService_Sound_initVal[ENVIRONMENTALSERVICE_SOUND_LEN] = {0};
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_SOUND_ID, ENVIRONMENTALSERVICE_SOUND_LEN, EnvironmentalService_Sound_initVal);
    uint8_t EnvironmentalService_Pressure_initVal[ENVIRONMENTALSERVICE_PRESSURE_LEN] = {0};
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_PRESSURE_ID, ENVIRONMENTALSERVICE_PRESSURE_LEN, EnvironmentalService_Pressure_initVal);
    uint8_t EnvironmentalService_Humidity_initVal[ENVIRONMENTALSERVICE_HUMIDITY_LEN] = {0};
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_HUMIDITY_ID, ENVIRONMENTALSERVICE_HUMIDITY_LEN, EnvironmentalService_Humidity_initVal);
    uint8_t EnvironmentalService_Temperature_initVal[ENVIRONMENTALSERVICE_TEMPERATURE_LEN] = {0};
    EnvironmentalService_SetParameter(ENVIRONMENTALSERVICE_TEMPERATURE_ID, ENVIRONMENTALSERVICE_TEMPERATURE_LEN, EnvironmentalService_Temperature_initVal);



    InertialService_AddService(selfEntity);
    uint8_t InertialService_Name_initVal[INERTIALSERVICE_NAME_LEN] = "{\"Name\":\"MPU6050\"}";
    InertialService_SetParameter(INERTIALSERVICE_NAME_ID, INERTIALSERVICE_NAME_LEN, InertialService_Name_initVal);
    uint8_t InertialService_Accx_initVal[INERTIALSERVICE_ACCX_LEN] = {0};
    InertialService_SetParameter(INERTIALSERVICE_ACCX_ID, INERTIALSERVICE_ACCX_LEN, InertialService_Accx_initVal);
    uint8_t InertialService_Accy_initVal[INERTIALSERVICE_ACCY_LEN] = {0};
    InertialService_SetParameter(INERTIALSERVICE_ACCY_ID, INERTIALSERVICE_ACCY_LEN, InertialService_Accy_initVal);
    uint8_t InertialService_Accz_initVal[INERTIALSERVICE_ACCZ_LEN] = {0};
    InertialService_SetParameter(INERTIALSERVICE_ACCZ_ID, INERTIALSERVICE_ACCZ_LEN, InertialService_Accz_initVal);
    uint8_t InertialService_gyrox_initVal[INERTIALSERVICE_GYROX_LEN] = {0};
    InertialService_SetParameter(INERTIALSERVICE_GYROX_ID, INERTIALSERVICE_GYROX_LEN, InertialService_gyrox_initVal);
    uint8_t InertialService_gyroy_initVal[INERTIALSERVICE_GYROY_LEN] = {0};
    InertialService_SetParameter(INERTIALSERVICE_GYROY_ID, INERTIALSERVICE_GYROY_LEN, InertialService_gyroy_initVal);
    uint8_t InertialService_gyroz_initVal[INERTIALSERVICE_GYROZ_LEN] = {0};
    InertialService_SetParameter(INERTIALSERVICE_GYROZ_ID, INERTIALSERVICE_GYROZ_LEN, InertialService_gyroz_initVal);


    // Start Bond Manager and register callback
    VOID GAPBondMgr_Register(&ProjectZero_BondMgrCBs);

    // Register with GAP for HCI/Host messages. This is needed to receive HCI
    // events. For more information, see the HCI section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    GAP_RegisterForMsgs(selfEntity);

    // Register for GATT local events and ATT Responses pending for transmission
    GATT_RegisterForMsgs(selfEntity);

    // Set default values for Data Length Extension
    // Extended Data Length Feature is already enabled by default
    {
      // Set initial values to maximum, RX is set to max. by default(251 octets, 2120us)
      // Some brand smartphone is essentially needing 251/2120, so we set them here.
      #define APP_SUGGESTED_PDU_SIZE 251 //default is 27 octets(TX)
      #define APP_SUGGESTED_TX_TIME 2120 //default is 328us(TX)

      // This API is documented in hci.h
      // See the LE Data Length Extension section in the BLE5-Stack User's Guide for information on using this command:
      // http://software-dl.ti.com/lprf/ble5stack-latest/
      HCI_LE_WriteSuggestedDefaultDataLenCmd(APP_SUGGESTED_PDU_SIZE, APP_SUGGESTED_TX_TIME);
    }

    // Initialize GATT Client, used by GAPBondMgr to look for RPAO characteristic for network privacy
    GATT_InitClient();

    // Initialize Connection List
    ProjectZero_clearConnListEntry(CONNHANDLE_ALL);

    //Initialize GAP layer for Peripheral role and register to receive GAP events
    GAP_DeviceInit(GAP_PROFILE_PERIPHERAL, selfEntity, ADDRMODE_PUBLIC, NULL);

    Clock_start(Clock_handle(&myClock));
}

/*********************************************************************
 * @fn      ProjectZero_taskFxn
 *
 * @brief   Application task entry point for the Project Zero.
 *
 * @param   a0, a1 - not used.
 */
static void ProjectZero_taskFxn(UArg a0, UArg a1)
{
    // Initialize application
    ProjectZero_init();
    I2C_init();
    GPIO_init();
    init_mpu6060();
    init_BH1750();
    init_BME280();


    // Application main loop
    for(;; )
    {
        uint32_t events;

        // Waits for an event to be posted associated with the calling thread.
        // Note that an event associated with a thread is posted when a
        // message is queued to the message receive queue of the thread
        events = Event_pend(syncEvent, Event_Id_NONE, PZ_ALL_EVENTS,
                            ICALL_TIMEOUT_FOREVER);

        if(events)
        {
            ICall_EntityID dest;
            ICall_ServiceEnum src;
            ICall_HciExtEvt *pMsg = NULL;

            // Fetch any available messages that might have been sent from the stack
            if(ICall_fetchServiceMsg(&src, &dest,
                                     (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
            {
                uint8_t safeToDealloc = TRUE;

                if((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
                {
                    ICall_Stack_Event *pEvt = (ICall_Stack_Event *)pMsg;

                    // Check for BLE stack events first
                    if(pEvt->signature == 0xffff)
                    {
                        // Process stack events
                        ProjectZero_processStackEvent(pEvt->event_flag);
                    }
                    else
                    {
                        switch(pMsg->hdr.event)
                        {
                        case GAP_MSG_EVENT:
                            // Process GAP message
                            ProjectZero_processGapMessage((gapEventHdr_t*) pMsg);
                            break;

                        case GATT_MSG_EVENT:
                            // Process GATT message
                            safeToDealloc =
                                ProjectZero_processGATTMsg(
                                    (gattMsgEvent_t *)pMsg);
                            break;

                        case HCI_GAP_EVENT_EVENT:
                            ProjectZero_processHCIMsg(pMsg);
                            break;

                        case L2CAP_SIGNAL_EVENT:
                            // Process L2CAP free buffer notification
                            ProjectZero_processL2CAPMsg(
                                (l2capSignalEvent_t *)pMsg);
                            break;

                        default:
                            // do nothing
                            break;
                        }
                    }
                }

                if(pMsg && safeToDealloc)
                {
                    ICall_freeMsg(pMsg);
                }
            }

            // Process messages sent from another task or another context.
            while(!Queue_empty(appMsgQueueHandle))
            {
                pzMsg_t *pMsg = (pzMsg_t *)Util_dequeueMsg(appMsgQueueHandle);
                if(pMsg)
                {
                    // Process application-layer message probably sent from ourselves.
                    ProjectZero_processApplicationMessage(pMsg);
                    // Free the received message.
                    ICall_free(pMsg);
                }
            }
        }
    }
}

/*********************************************************************
 * @fn      ProjectZero_processL2CAPMsg
 *
 * @brief   Process L2CAP messages and events.
 *
 * @param   pMsg - L2CAP signal buffer from stack
 *
 * @return  None
 */
static void ProjectZero_processL2CAPMsg(l2capSignalEvent_t *pMsg)
{
    switch(pMsg->opcode)
    {
      case L2CAP_NUM_CTRL_DATA_PKT_EVT:
          break;
      default:
          break;
    }
}


/*********************************************************************
 * @fn      ProjectZero_processStackEvent
 *
 * @brief   Process stack event. The event flags received are user-selected
 *          via previous calls to stack APIs.
 *
 * @param   stack_event - mask of events received
 *
 * @return  none
 */
static void ProjectZero_processStackEvent(uint32_t stack_event)
{
    // Intentionally blank
}

/*********************************************************************
 * @fn      ProjectZero_processGATTMsg
 *
 * @brief   Process GATT messages and events.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t ProjectZero_processGATTMsg(gattMsgEvent_t *pMsg)
{
    if(pMsg->method == ATT_FLOW_CTRL_VIOLATED_EVENT)
    {
        // ATT request-response or indication-confirmation flow control is
        // violated. All subsequent ATT requests or indications will be dropped.
        // The app is informed in case it wants to drop the connection.

        // Display the opcode of the message that caused the violation.
        Log_error1("FC Violated: %d", pMsg->msg.flowCtrlEvt.opcode);
    }
    else if(pMsg->method == ATT_MTU_UPDATED_EVENT)
    {
        // MTU size updated
        Log_info1("MTU Size: %d", pMsg->msg.mtuEvt.MTU);
    }

    // Free message payload. Needed only for ATT Protocol messages
    GATT_bm_free(&pMsg->msg, pMsg->method);

    // It's safe to free the incoming message
    return(TRUE);
}

/*********************************************************************
 * @fn      ProjectZero_processApplicationMessage
 *
 * @brief   Handle application messages
 *
 *          These are messages not from the BLE stack, but from the
 *          application itself.
 *
 *          For example, in a Software Interrupt (Swi) it is not possible to
 *          call any BLE APIs, so instead the Swi function must send a message
 *          to the application Task for processing in Task context.
 *
 * @param   pMsg  Pointer to the message of type pzMsg_t.
 */
static void ProjectZero_processApplicationMessage(pzMsg_t *pMsg)
{
    // Cast to pzCharacteristicData_t* here since it's a common message pdu type.
    pzCharacteristicData_t *pCharData = (pzCharacteristicData_t *)pMsg->pData;

    switch(pMsg->event)
    {
      case HCI_BLE_HARDWARE_ERROR_EVENT_CODE:
          AssertHandler(HAL_ASSERT_CAUSE_HARDWARE_ERROR,0);
          break;


      case PZ_ADV_EVT:
          ProjectZero_processAdvEvent((pzGapAdvEventData_t*)(pMsg->pData));
          break;

      case PZ_SEND_PARAM_UPD_EVT:
      {
          // Send connection parameter update
          pzSendParamReq_t* req = (pzSendParamReq_t *)pMsg->pData;
          ProjectZero_sendParamUpdate(req->connHandle);
      }
      break;

      case PZ_START_ADV_EVT:
          if(linkDB_NumActive() < MAX_NUM_BLE_CONNS)
          {
              // Enable advertising if there is room for more connections
              GapAdv_enable(advHandleLegacy, GAP_ADV_ENABLE_OPTIONS_USE_MAX, 0);
          }
          break;

      case PZ_PAIRSTATE_EVT: /* Message about the pairing state */
          ProjectZero_processPairState((pzPairStateData_t*)(pMsg->pData));
          break;

      case PZ_PASSCODE_EVT: /* Message about pairing PIN request */
      {
          pzPasscodeReq_t *pReq = (pzPasscodeReq_t *)pMsg->pData;
          ProjectZero_processPasscode(pReq);
      }
      break;

      case PZ_CONN_EVT:
        ProjectZero_processConnEvt((Gap_ConnEventRpt_t *)(pMsg->pData));
        break;

      case APP_MSG_PERIODIC_TIMER:
          updateSensors();

      default:
        break;
    }

    if(pMsg->pData != NULL)
    {
        ICall_free(pMsg->pData);
    }
}

/*********************************************************************
 * @fn      ProjectZero_processGapMessage
 *
 * @brief   Process an incoming GAP event.
 *
 * @param   pMsg - message to process
 */
static void ProjectZero_processGapMessage(gapEventHdr_t *pMsg)
{
    switch(pMsg->opcode)
    {
    case GAP_DEVICE_INIT_DONE_EVENT:
    {
        bStatus_t status = FAILURE;

        gapDeviceInitDoneEvent_t *pPkt = (gapDeviceInitDoneEvent_t *)pMsg;

        if(pPkt->hdr.status == SUCCESS)
        {
            // Store the system ID
            uint8_t systemId[DEVINFO_SYSTEM_ID_LEN];

            // use 6 bytes of device address for 8 bytes of system ID value
            systemId[0] = pPkt->devAddr[0];
            systemId[1] = pPkt->devAddr[1];
            systemId[2] = pPkt->devAddr[2];

            // set middle bytes to zero
            systemId[4] = 0x00;
            systemId[3] = 0x00;

            // shift three bytes up
            systemId[7] = pPkt->devAddr[5];
            systemId[6] = pPkt->devAddr[4];
            systemId[5] = pPkt->devAddr[3];

            // Set Device Info Service Parameter
            DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN,
                                 systemId);

            // Display device address
            // Need static so string persists until printed in idle thread.
            static uint8_t addrStr[3 * B_ADDR_LEN + 1];
            util_arrtohex(pPkt->devAddr, B_ADDR_LEN, addrStr, sizeof addrStr,
                          UTIL_ARRTOHEX_REVERSE);
            Log_info1("GAP is started. Our address: " \
                      ANSI_COLOR(FG_GREEN) "%s" ANSI_COLOR(ATTR_RESET),
                      (uintptr_t)addrStr);

            // Setup and start Advertising
            // For more information, see the GAP section in the User's Guide:
            // http://software-dl.ti.com/lprf/ble5stack-latest/

            // Temporary memory for advertising parameters for set #1. These will be copied
            // by the GapAdv module
            GapAdv_params_t advParamLegacy = GAPADV_PARAMS_LEGACY_SCANN_CONN;

            // Create Advertisement set #1 and assign handle
            status = GapAdv_create(&ProjectZero_advCallback, &advParamLegacy,
                                   &advHandleLegacy);
            APP_ASSERT(status == SUCCESS);

//            Log_info1("Name in advertData array: " \
//                      ANSI_COLOR(FG_YELLOW) "%s" ANSI_COLOR(ATTR_RESET),
//                      (uintptr_t)util_getLocalNameStr(advertData,
//                                                      sizeof(advertData)));

            // Load advertising data for set #1 that is statically allocated by the app

            uint8_t *localName;
            localName=(uint8_t*)malloc(sizeof(name_board)+5);
            localName[0]=0x02;
            localName[1]=GAP_ADTYPE_FLAGS;
            localName[2]= DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
            localName[3]=sizeof(name_board)+1;
            localName[4]=GAP_ADTYPE_LOCAL_NAME_COMPLETE;

            int i=0;
            for(i=0;i<sizeof(name_board);i++){
                localName[5+i]=name_board[i];
            }

            status = GapAdv_loadByHandle(advHandleLegacy, GAP_ADV_DATA_TYPE_ADV,
                                         sizeof(name_board)+5, localName);
            APP_ASSERT(status == SUCCESS);

            // Load scan response data for set #1 that is statically allocated by the app
//            status =
//                GapAdv_loadByHandle(advHandleLegacy, GAP_ADV_DATA_TYPE_SCAN_RSP,
//                                    sizeof(scanRspData),
//                                    scanRspData);
//            APP_ASSERT(status == SUCCESS);

            // Set event mask for set #1
            status = GapAdv_setEventMask(advHandleLegacy,
                                         GAP_ADV_EVT_MASK_START_AFTER_ENABLE |
                                         GAP_ADV_EVT_MASK_END_AFTER_DISABLE |
                                         GAP_ADV_EVT_MASK_SET_TERMINATED);

            // Enable legacy advertising for set #1
            status =
                GapAdv_enable(advHandleLegacy, GAP_ADV_ENABLE_OPTIONS_USE_MAX,
                              0);
            APP_ASSERT(status == SUCCESS);
        }

        break;
    }

    case GAP_LINK_ESTABLISHED_EVENT:
    {
        gapEstLinkReqEvent_t *pPkt = (gapEstLinkReqEvent_t *)pMsg;

        // Display the amount of current connections
        Log_info2("Link establish event, status 0x%02x. Num Conns: %d",
                  pPkt->hdr.status,
                  linkDB_NumActive());

        if(pPkt->hdr.status == SUCCESS)
        {
            // Add connection to list
            ProjectZero_addConn(pPkt->connectionHandle);

            // Display the address of this connection
            static uint8_t addrStr[3 * B_ADDR_LEN + 1];
            util_arrtohex(pPkt->devAddr, B_ADDR_LEN, addrStr, sizeof addrStr,
                          UTIL_ARRTOHEX_REVERSE);
            Log_info1("Connected. Peer address: " \
                        ANSI_COLOR(FG_GREEN)"%s"ANSI_COLOR(ATTR_RESET),
                      (uintptr_t)addrStr);
        }

        if(linkDB_NumActive() < MAX_NUM_BLE_CONNS)
        {
            Log_info1("Continue to Advertise, %d possible connection remain", MAX_NUM_BLE_CONNS - linkDB_NumActive());
            // Start advertising since there is room for more connections
            GapAdv_enable(advHandleLegacy, GAP_ADV_ENABLE_OPTIONS_USE_MAX, 0);
        }
        else
        {
            Log_info1("Max Number of Connection reach: %d, Adv. will not be enable again", linkDB_NumActive());
        }
    }
    break;

    case GAP_LINK_TERMINATED_EVENT:
    {
        gapTerminateLinkEvent_t *pPkt = (gapTerminateLinkEvent_t *)pMsg;

        // Display the amount of current connections
        Log_info0("Device Disconnected!");
        Log_info1("Num Conns: %d", linkDB_NumActive());

        // Remove the connection from the list and disable RSSI if needed
        ProjectZero_removeConn(pPkt->connectionHandle);

        // GapAdv_enable will return success only if the maximum number of connections 
        // has been reached, and adv was not re-enable in GAP_LINK_ESTABLISHED_EVENT
        // switch case.
        // If less connection were in used, Advertisement will have been restart in 
        // the GAP_LINK_ESTABLISHED_EVENT switch case and calling GapAdv_enable will
        // just return an error.
        if ( GapAdv_enable(advHandleLegacy, GAP_ADV_ENABLE_OPTIONS_USE_MAX, 0) == SUCCESS)
        {
            Log_info1("Restart Advertising, %d possible connection remain", MAX_NUM_BLE_CONNS - linkDB_NumActive());
        }
    }
    break;

    case GAP_UPDATE_LINK_PARAM_REQ_EVENT:
        ProjectZero_handleUpdateLinkParamReq(
            (gapUpdateLinkParamReqEvent_t *)pMsg);
        break;

    case GAP_LINK_PARAM_UPDATE_EVENT:
        ProjectZero_handleUpdateLinkEvent((gapLinkUpdateEvent_t *)pMsg);
        break;

    case GAP_PAIRING_REQ_EVENT:
        // Disable advertising so that the peer device can be added to
        // the resolving list
        GapAdv_disable(advHandleLegacy);
        break;

    default:
        break;
    }
}

void ProjectZero_processHCIMsg(ICall_HciExtEvt *pEvt)
{
    ICall_Hdr *pMsg = (ICall_Hdr *)pEvt;

    // Process HCI message
    switch(pMsg->status)
    {
    case HCI_COMMAND_COMPLETE_EVENT_CODE:
        // Process HCI Command Complete Events here
        ProjectZero_processCmdCompleteEvt((hciEvt_CmdComplete_t *) pMsg);
        break;

    case HCI_BLE_HARDWARE_ERROR_EVENT_CODE:
        AssertHandler(HAL_ASSERT_CAUSE_HARDWARE_ERROR,0);
        break;

    // HCI Commands Events
    case HCI_COMMAND_STATUS_EVENT_CODE:
    {
        hciEvt_CommandStatus_t *pMyMsg = (hciEvt_CommandStatus_t *)pMsg;
        switch(pMyMsg->cmdOpcode)
        {
        case HCI_LE_SET_PHY:
        {
            if(pMyMsg->cmdStatus == HCI_ERROR_CODE_UNSUPPORTED_REMOTE_FEATURE)
            {
                Log_info0("PHY Change failure, peer does not support this");
            }
            else
            {
                Log_info1("PHY Update Status Event: 0x%x",
                          pMyMsg->cmdStatus);
            }

            ProjectZero_updatePHYStat(HCI_LE_SET_PHY, (uint8_t *)pMsg);
        }
        break;

        default:
            break;
        }
    }
    break;

    // LE Events
    case HCI_LE_EVENT_CODE:
    {
        hciEvt_BLEPhyUpdateComplete_t *pPUC =
            (hciEvt_BLEPhyUpdateComplete_t*) pMsg;

        // A Phy Update Has Completed or Failed
        if(pPUC->BLEEventCode == HCI_BLE_PHY_UPDATE_COMPLETE_EVENT)
        {
            if(pPUC->status != SUCCESS)
            {
                Log_info0("PHY Change failure");
            }
            else
            {
                // Only symmetrical PHY is supported.
                // rxPhy should be equal to txPhy.
                Log_info1("PHY Updated to %s",
                          (uintptr_t)((pPUC->rxPhy == PHY_UPDATE_COMPLETE_EVENT_1M) ? "1M" :
                                      (pPUC->rxPhy == PHY_UPDATE_COMPLETE_EVENT_2M) ? "2M" :
                                      (pPUC->rxPhy == PHY_UPDATE_COMPLETE_EVENT_CODED) ? "CODED" : "Unexpected PHY Value"));
            }

            ProjectZero_updatePHYStat(HCI_BLE_PHY_UPDATE_COMPLETE_EVENT,
                                      (uint8_t *)pMsg);
        }
    }
    break;

    default:
        break;
    }
}

/*********************************************************************
 * @fn      ProjectZero_processAdvEvent
 *
 * @brief   Process advertising event in app context
 *
 * @param   pEventData
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static void ProjectZero_processAdvEvent(pzGapAdvEventData_t *pEventData)
{
    switch(pEventData->event)
    {
    /* Sent on the first advertisement after a GapAdv_enable */
    case GAP_EVT_ADV_START_AFTER_ENABLE:
        Log_info1("Adv Set %d Enabled", *(uint8_t *)(pEventData->pBuf));
        break;

    /* Sent after advertising stops due to a GapAdv_disable */
    case GAP_EVT_ADV_END_AFTER_DISABLE:
        Log_info1("Adv Set %d Disabled", *(uint8_t *)(pEventData->pBuf));
        break;

    /* Sent at the beginning of each advertisement. (Note that this event
     * is not enabled by default, see GapAdv_setEventMask). */
    case GAP_EVT_ADV_START:
        break;

    /* Sent after each advertisement. (Note that this event is not enabled
     * by default, see GapAdv_setEventMask). */
    case GAP_EVT_ADV_END:
        break;

    /* Sent when an advertisement set is terminated due to a
     * connection establishment */
    case GAP_EVT_ADV_SET_TERMINATED:
    {
        GapAdv_setTerm_t *advSetTerm = (GapAdv_setTerm_t *)(pEventData->pBuf);

        Log_info2("Adv Set %d disabled after conn %d",
                  advSetTerm->handle, advSetTerm->connHandle);
    }
    break;

    /* Sent when a scan request is received. (Note that this event
     * is not enabled by default, see GapAdv_setEventMask). */
    case GAP_EVT_SCAN_REQ_RECEIVED:
        break;

    /* Sent when an operation could not complete because of a lack of memory.
       This message is not allocated on the heap and must not be freed */
    case GAP_EVT_INSUFFICIENT_MEMORY:
        break;

    default:
        break;
    }

  // All events have associated memory to free except the insufficient memory
  // event
  if (pEventData->event != GAP_EVT_INSUFFICIENT_MEMORY)
  {
    ICall_free(pEventData->pBuf);
  }
}

/*********************************************************************
 * @fn      ProjectZero_processPairState
 *
 * @brief   Process the new paring state.
 *
 * @param   pPairData - pointer to pair state data container
 */
static void ProjectZero_processPairState(pzPairStateData_t *pPairData)
{
    uint8_t state = pPairData->state;
    uint8_t status = pPairData->status;

    switch(state)
    {
    case GAPBOND_PAIRING_STATE_STARTED:
        Log_info0("Pairing started");
        break;

    case GAPBOND_PAIRING_STATE_COMPLETE:
        if(status == SUCCESS)
        {
            Log_info0("Pairing success");
        }
        else
        {
            Log_info1("Pairing fail: %d", status);
        }
        break;

    case GAPBOND_PAIRING_STATE_ENCRYPTED:
        if(status == SUCCESS)
        {
            Log_info0("Encryption success");
        }
        else
        {
            Log_info1("Encryption failed: %d", status);
        }
        break;

    case GAPBOND_PAIRING_STATE_BOND_SAVED:
        if(status == SUCCESS)
        {
            Log_info0("Bond save success");
        }
        else
        {
            Log_info1("Bond save failed: %d", status);
        }
        break;

    default:
        break;
    }
}

/*********************************************************************
 * @fn      ProjectZero_processPasscode
 *
 * @brief   Process the Passcode request.
 *
 * @param   pReq - pointer to passcode req
 */
static void ProjectZero_processPasscode(pzPasscodeReq_t *pReq)
{
    Log_info2("BondMgr Requested passcode. We are %s passcode %06d",
              (uintptr_t)(pReq->uiInputs ? "Sending" : "Displaying"),
              B_APP_DEFAULT_PASSCODE);

    // Send passcode response.
    GAPBondMgr_PasscodeRsp(pReq->connHandle, SUCCESS, B_APP_DEFAULT_PASSCODE);
}
/*********************************************************************
 * @fn      ProjectZero_processConnEvt
 *
 * @brief   Process connection event.
 *
 * @param pReport pointer to connection event report
 */
static void ProjectZero_processConnEvt(Gap_ConnEventRpt_t *pReport)
{
  Log_info1("Connection event done for connHandle: %d", pReport->handle);
}

/*********************************************************************
 * @fn      ProjectZero_processCmdCompleteEvt
 *
 * @brief   Process an incoming OSAL HCI Command Complete Event.
 *
 * @param   pMsg - message to process
 */
static void ProjectZero_processCmdCompleteEvt(hciEvt_CmdComplete_t *pMsg)
{
    uint8_t status = pMsg->pReturnParam[0];

    //Find which command this command complete is for
    switch(pMsg->cmdOpcode)
    {
    case HCI_READ_RSSI:
    {
            int8 rssi = (int8)pMsg->pReturnParam[3];
    
        // Display RSSI value, if RSSI is higher than threshold, change to faster PHY
        if(status == SUCCESS)
        {
            uint16_t handle = BUILD_UINT16(pMsg->pReturnParam[1],
                                           pMsg->pReturnParam[2]);

            Log_info2("RSSI:%d, connHandle %d",
                      (uint32_t)(rssi),
                      (uint32_t)handle);
        } // end of if (status == SUCCESS)
        break;
    }

    case HCI_LE_READ_PHY:
    {
        if(status == SUCCESS)
        {
            Log_info2("RXPh: %d, TXPh: %d",
                      pMsg->pReturnParam[3], pMsg->pReturnParam[4]);
        }
        break;
    }

    default:
        break;
    } // end of switch (pMsg->cmdOpcode)
}

/*********************************************************************
 * @fn      ProjectZero_handleUpdateLinkParamReq
 *
 * @brief   Receive and respond to a parameter update request sent by
 *          a peer device
 *
 * @param   pReq - pointer to stack request message
 */
static void ProjectZero_handleUpdateLinkParamReq(
    gapUpdateLinkParamReqEvent_t *pReq)
{
    gapUpdateLinkParamReqReply_t rsp;

    rsp.connectionHandle = pReq->req.connectionHandle;
    rsp.signalIdentifier = pReq->req.signalIdentifier;

    // Only accept connection intervals with slave latency of 0
    // This is just an example of how the application can send a response
    if(pReq->req.connLatency == 0)
    {
        rsp.intervalMin = pReq->req.intervalMin;
        rsp.intervalMax = pReq->req.intervalMax;
        rsp.connLatency = pReq->req.connLatency;
        rsp.connTimeout = pReq->req.connTimeout;
        rsp.accepted = TRUE;
    }
    else
    {
        rsp.accepted = FALSE;
    }

    // Send Reply
    VOID GAP_UpdateLinkParamReqReply(&rsp);
}

/*********************************************************************
 * @fn      ProjectZero_handleUpdateLinkEvent
 *
 * @brief   Receive and parse a parameter update that has occurred.
 *
 * @param   pEvt - pointer to stack event message
 */
static void ProjectZero_handleUpdateLinkEvent(gapLinkUpdateEvent_t *pEvt)
{
    // Get the address from the connection handle
    linkDBInfo_t linkInfo;
    linkDB_GetInfo(pEvt->connectionHandle, &linkInfo);

    static uint8_t addrStr[3 * B_ADDR_LEN + 1];
    util_arrtohex(linkInfo.addr, B_ADDR_LEN, addrStr, sizeof addrStr,
                  UTIL_ARRTOHEX_REVERSE);

    if(pEvt->status == SUCCESS)
    {
        uint8_t ConnIntervalFracture = 25*(pEvt->connInterval % 4);
        // Display the address of the connection update
        Log_info5(
            "Updated params for %s, interval: %d.%d ms, latency: %d, timeout: %d ms",
            (uintptr_t)addrStr,
            (uintptr_t)(pEvt->connInterval*CONN_INTERVAL_MS_CONVERSION),
            ConnIntervalFracture,
            pEvt->connLatency,
            pEvt->connTimeout*CONN_TIMEOUT_MS_CONVERSION);
    }
    else
    {
        // Display the address of the connection update failure
        Log_info2("Update Failed 0x%02x: %s", pEvt->opcode, (uintptr_t)addrStr);
    }

    // Check if there are any queued parameter updates
    pzConnHandleEntry_t *connHandleEntry = (pzConnHandleEntry_t *)List_get(
        &paramUpdateList);
    if(connHandleEntry != NULL)
    {
        // Attempt to send queued update now
        ProjectZero_sendParamUpdate(*(connHandleEntry->connHandle));

        // Free list element
        ICall_free(connHandleEntry->connHandle);
        ICall_free(connHandleEntry);
    }
}

/*********************************************************************
 * @fn      ProjectZero_addConn
 *
 * @brief   Add a device to the connected device list
 *
 * @param   connHandle - connection handle
 *
 * @return  bleMemAllocError if a param update event could not be sent. Else SUCCESS.
 */
static uint8_t ProjectZero_addConn(uint16_t connHandle)
{
    uint8_t i;
    uint8_t status = bleNoResources;

    // Try to find an available entry
    for(i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if(connList[i].connHandle == CONNHANDLE_INVALID)
        {
            // Found available entry to put a new connection info in
            connList[i].connHandle = connHandle;

            // Create a clock object and start
            connList[i].pUpdateClock
              = (Clock_Struct*) ICall_malloc(sizeof(Clock_Struct));

            if (connList[i].pUpdateClock)
            {
              Util_constructClock(connList[i].pUpdateClock,
                                  ProjectZero_paramUpdClockHandler,
                                  PZ_SEND_PARAM_UPDATE_DELAY, 0, true,
                                  (uintptr_t)connHandle);
            }

            // Set default PHY to 1M
            connList[i].currPhy = HCI_PHY_1_MBPS; // TODO: Is this true, neccessarily?

            break;
        }
    }

    return(status);
}

/*********************************************************************
 * @fn      ProjectZero_getConnIndex
 *
 * @brief   Find index in the connected device list by connHandle
 *
 * @param   connHandle - connection handle
 *
 * @return  the index of the entry that has the given connection handle.
 *          if there is no match, MAX_NUM_BLE_CONNS will be returned.
 */
static uint8_t ProjectZero_getConnIndex(uint16_t connHandle)
{
    uint8_t i;

    for(i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if(connList[i].connHandle == connHandle)
        {
            return(i);
        }
    }

    return(MAX_NUM_BLE_CONNS);
}

/*********************************************************************
 * @fn      ProjectZero_clearConnListEntry
 *
 * @brief   Clear the connection information structure held locally.
 *
 * @param   connHandle - connection handle
 *
 * @return  SUCCESS if connHandle found valid index or bleInvalidRange
 *          if index wasn't found. LINKDB_CONNHANDLE_ALL will always succeed.
 */
static uint8_t ProjectZero_clearConnListEntry(uint16_t connHandle)
{
    uint8_t i;
    // Set to invalid connection index initially
    uint8_t connIndex = MAX_NUM_BLE_CONNS;

    if(connHandle != CONNHANDLE_ALL)
    {
        // Get connection index from handle
        connIndex = ProjectZero_getConnIndex(connHandle);
        if(connIndex >= MAX_NUM_BLE_CONNS)
        {
            return(bleInvalidRange);
        }
    }

    // Clear specific handle or all handles
    for(i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if((connIndex == i) || (connHandle == CONNHANDLE_ALL))
        {
            connList[i].connHandle = CONNHANDLE_INVALID;
            connList[i].currPhy = 0;
            connList[i].phyCngRq = 0;
            connList[i].phyRqFailCnt = 0;
            connList[i].rqPhy = 0;
        }
    }

    return(SUCCESS);
}

/*********************************************************************
 * @fn      ProjectZero_removeConn
 *
 * @brief   Remove a device from the connected device list
 *
 * @param   connHandle - connection handle
 *
 * @return  index of the connected device list entry where the new connection
 *          info is removed from.
 *          if connHandle is not found, MAX_NUM_BLE_CONNS will be returned.
 */
static uint8_t ProjectZero_removeConn(uint16_t connHandle)
{
    uint8_t connIndex = ProjectZero_getConnIndex(connHandle);

    if(connIndex < MAX_NUM_BLE_CONNS)
    {
      Clock_Struct* pUpdateClock = connList[connIndex].pUpdateClock;

      if (pUpdateClock != NULL)
      {
        // Stop and destruct the RTOS clock if it's still alive
        if (Util_isActive(pUpdateClock))
        {
          Util_stopClock(pUpdateClock);
        }

        // Destruct the clock object
        Clock_destruct(pUpdateClock);
        // Free clock struct
        ICall_free(pUpdateClock);
      }
      // Clear Connection List Entry
      ProjectZero_clearConnListEntry(connHandle);
    }

    return connIndex;
}

/*********************************************************************
 * @fn      ProjectZero_sendParamUpdate
 *
 * @brief   Remove a device from the connected device list
 *
 * @param   connHandle - connection handle
 */
static void ProjectZero_sendParamUpdate(uint16_t connHandle)
{
    gapUpdateLinkParamReq_t req;
    uint8_t connIndex;

    req.connectionHandle = connHandle;
    req.connLatency = DEFAULT_DESIRED_SLAVE_LATENCY;
    req.connTimeout = DEFAULT_DESIRED_CONN_TIMEOUT;
    req.intervalMin = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    req.intervalMax = DEFAULT_DESIRED_MAX_CONN_INTERVAL;

    connIndex = ProjectZero_getConnIndex(connHandle);
    APP_ASSERT(connIndex < MAX_NUM_BLE_CONNS);

    // Deconstruct the clock object
    Clock_destruct(connList[connIndex].pUpdateClock);
    // Free clock struct
    ICall_free(connList[connIndex].pUpdateClock);
    connList[connIndex].pUpdateClock = NULL;

    // Send parameter update
    bStatus_t status = GAP_UpdateLinkParamReq(&req);

    // If there is an ongoing update, queue this for when the update completes
    if(status == bleAlreadyInRequestedMode)
    {
        pzConnHandleEntry_t *connHandleEntry =
            ICall_malloc(sizeof(pzConnHandleEntry_t));
        if(connHandleEntry)
        {
            connHandleEntry->connHandle = ICall_malloc(sizeof(uint16_t));

            if(connHandleEntry->connHandle)
            {
                *(connHandleEntry->connHandle) = connHandle;

                List_put(&paramUpdateList, (List_Elem *)&connHandleEntry);
            }
        }
    }
}

/*********************************************************************
 * @fn      ProjectZero_updatePHYStat
 *
 * @brief   Update the auto phy update state machine
 *
 * @param   eventCode - HCI LE Event code
 *          pMsg - message to process
 */
static void ProjectZero_updatePHYStat(uint16_t eventCode, uint8_t *pMsg)
{
    uint8_t connIndex;
    pzConnHandleEntry_t *connHandleEntry;

    switch(eventCode)
    {
    case HCI_LE_SET_PHY:
    {
        // Get connection handle from list
        connHandleEntry = (pzConnHandleEntry_t *)List_get(&setPhyCommStatList);

        if(connHandleEntry)
        {
            // Get index from connection handle
            connIndex = ProjectZero_getConnIndex(*(connHandleEntry->connHandle));
            APP_ASSERT(connIndex < MAX_NUM_BLE_CONNS);

            ICall_free(connHandleEntry->connHandle);
            ICall_free(connHandleEntry);

            hciEvt_CommandStatus_t *pMyMsg = (hciEvt_CommandStatus_t *)pMsg;

            if(pMyMsg->cmdStatus == HCI_ERROR_CODE_UNSUPPORTED_REMOTE_FEATURE)
            {
                // Update the phy change request status for active RSSI tracking connection
                connList[connIndex].phyCngRq = FALSE;
                connList[connIndex].phyRqFailCnt++;
            }
        }
        break;
    }

    // LE Event - a Phy update has completed or failed
    case HCI_BLE_PHY_UPDATE_COMPLETE_EVENT:
    {
        hciEvt_BLEPhyUpdateComplete_t *pPUC =
            (hciEvt_BLEPhyUpdateComplete_t*) pMsg;

        if(pPUC)
        {
            // Get index from connection handle
            uint8_t index = ProjectZero_getConnIndex(pPUC->connHandle);
            APP_ASSERT(index < MAX_NUM_BLE_CONNS);

            // Update the phychange request status for active RSSI tracking connection
            connList[index].phyCngRq = FALSE;

            if(pPUC->status == SUCCESS)
            {
                connList[index].currPhy = pPUC->rxPhy;
            }
            if(pPUC->rxPhy != connList[index].rqPhy)
            {
                connList[index].phyRqFailCnt++;
            }
            else
            {
                // Reset the request phy counter and requested phy
                connList[index].phyRqFailCnt = 0;
                connList[index].rqPhy = 0;
            }
        }

        break;
    }

    default:
        break;
    } // end of switch (eventCode)
}

/*
 * @brief   Handle a debounced button press or release in Task context.
 *          Invoked by the taskFxn based on a message received from a callback.
 *
 * @see     buttonDebounceSwiFxn
 * @see     buttonCallbackFxn
 *
 * @param   pState  pointer to pzButtonState_t message sent from debounce Swi.
 *
 * @return  None.
 */


/*
 * @brief   Handle a write request sent from a peer device.
 *
 *          Invoked by the Task based on a message received from a callback.
 *
 *          When we get here, the request has already been accepted by the
 *          service and is valid from a BLE protocol perspective as well as
 *          having the correct length as defined in the service implementation.
 *
 * @param   pCharData  pointer to malloc'd char write data
 *
 * @return  None.
 */


/*
 * @brief   Handle a CCCD (configuration change) write received from a peer
 *          device. This tells us whether the peer device wants us to send
 *          Notifications or Indications.
 *
 * @param   pCharData  pointer to malloc'd char write data
 *
 * @return  None.
 */
void ProjectZero_ButtonService_CfgChangeHandler(
    pzCharacteristicData_t *pCharData)
{
    // Cast received data to uint16, as that's the format for CCCD writes.
    uint16_t configValue = *(uint16_t *)pCharData->data;
    char *configValString;

    // Determine what to tell the user
    switch(configValue)
    {
    case GATT_CFG_NO_OPERATION:
        configValString = "Noti/Ind disabled";
        break;
    case GATT_CLIENT_CFG_NOTIFY:
        configValString = "Notifications enabled";
        break;
    case GATT_CLIENT_CFG_INDICATE:
        configValString = "Indications enabled";
        break;
    default:
        configValString = "Unsupported operation";
    }


}

/*
 * @brief   Handle a write request sent from a peer device.
 *
 *          Invoked by the Task based on a message received from a callback.
 *
 *          When we get here, the request has already been accepted by the
 *          service and is valid from a BLE protocol perspective as well as
 *          having the correct length as defined in the service implementation.
 *
 * @param   pCharData  pointer to malloc'd char write data
 *
 * @return  None.
 */

/*
 * @brief   Handle a CCCD (configuration change) write received from a peer
 *          device. This tells us whether the peer device wants us to send
 *          Notifications or Indications.
 *
 * @param   pCharData  pointer to malloc'd char write data
 *
 * @return  None.
 */
void ProjectZero_DataService_CfgChangeHandler(pzCharacteristicData_t *pCharData)
{
    // Cast received data to uint16, as that's the format for CCCD writes.
    uint16_t configValue = *(uint16_t *)pCharData->data;
    char *configValString;

    // Determine what to tell the user
    switch(configValue)
    {
    case GATT_CFG_NO_OPERATION:
        configValString = "Noti/Ind disabled";
        break;
    case GATT_CLIENT_CFG_NOTIFY:
        configValString = "Notifications enabled";
        break;
    case GATT_CLIENT_CFG_INDICATE:
        configValString = "Indications enabled";
        break;
    default:
        configValString = "Unsupported operation";
    }


}

/*
 * @brief  Convenience function for updating characteristic data via pzCharacteristicData_t
 *         structured message.
 *
 * @note   Must run in Task context in case BLE Stack APIs are invoked.
 *
 * @param  *pCharData  Pointer to struct with value to update.
 */


/******************************************************************************
 *****************************************************************************
 *
 *  Handlers of direct system callbacks.
 *
 *  Typically enqueue the information or request as a message for the
 *  application Task for handling.
 *
 ****************************************************************************
 *****************************************************************************/

/*
 *  Callbacks from the Stack Task context (GAP or Service changes)
 *****************************************************************************/

/*********************************************************************
 * @fn      ProjectZero_advCallback
 *
 * @brief   GapAdv module callback
 *
 * @param   pMsg - message to process
 *          pBuf - data potentially accompanying event
 *          arg - not used
 */
static void ProjectZero_advCallback(uint32_t event, void *pBuf, uintptr_t arg)
{
    pzGapAdvEventData_t *eventData = ICall_malloc(sizeof(pzGapAdvEventData_t));

    if(eventData != NULL)
    {
        eventData->event = event;
        eventData->pBuf = pBuf;

        if(ProjectZero_enqueueMsg(PZ_ADV_EVT, eventData) != SUCCESS)
        {
          ICall_free(eventData);
        }
    }
}

/*********************************************************************
 * @fn      ProjectZero_pairStateCb
 *
 * @brief   Pairing state callback.
 *
 * @param   connHandle - connection handle
 *          state - pair state
 *          status - pair status
 */
static void ProjectZero_pairStateCb(uint16_t connHandle, uint8_t state,
                                    uint8_t status)
{
    pzPairStateData_t *pairState =
        (pzPairStateData_t *)ICall_malloc(sizeof(pzPairStateData_t));

    if(pairState != NULL)
    {
        pairState->state = state;
        pairState->connHandle = connHandle;
        pairState->status = status;

        if(ProjectZero_enqueueMsg(PZ_PAIRSTATE_EVT, pairState) != SUCCESS)
        {
          ICall_free(pairState);
        }
    }
}

/*********************************************************************
 * @fn      ProjectZero_passcodeCb
 *
 * @brief   Passcode callback.
 *
 * @param   pDeviceAddr - not used
 *          connHandle - connection handle
 *          uiInpuits - if TRUE, the local device should accept a passcode input
 *          uiOutputs - if TRUE, the local device should display the passcode
 *          numComparison - the code that should be displayed for numeric
 *          comparison pairing. If this is zero, then passcode pairing is occurring.
 */
static void ProjectZero_passcodeCb(uint8_t *pDeviceAddr,
                                   uint16_t connHandle,
                                   uint8_t uiInputs,
                                   uint8_t uiOutputs,
                                   uint32_t numComparison)
{
    pzPasscodeReq_t *req =
        (pzPasscodeReq_t *)ICall_malloc(sizeof(pzPasscodeReq_t));
    if(req != NULL)
    {
        req->connHandle = connHandle;
        req->uiInputs = uiInputs;
        req->uiOutputs = uiOutputs;
        req->numComparison = numComparison;

        if(ProjectZero_enqueueMsg(PZ_PASSCODE_EVT, req) != SUCCESS)
        {
          ICall_free(req);
        }
    }
    ;
}



/*********************************************************************
 * @fn      ProjectZero_paramUpdClockHandler
 *
 * @brief   Handler function for clock timeouts.
 *
 * @param   arg - app message pointer
 */
static void ProjectZero_paramUpdClockHandler(UArg arg)
{
    pzSendParamReq_t *req =
        (pzSendParamReq_t *)ICall_malloc(sizeof(pzSendParamReq_t));
    if(req)
    {
        req->connHandle = (uint16_t)arg;
        if(ProjectZero_enqueueMsg(PZ_SEND_PARAM_UPD_EVT, req) != SUCCESS)
        {
          ICall_free(req);
        }
    }
}

/*********************************************************************
 * @fn     buttonDebounceSwiFxn
 *
 * @brief  Callback from Clock module on timeout
 *
 *         Determines new state after debouncing
 *
 * @param  buttonId    The pin being debounced
 */

/*********************************************************************
 * @fn     buttonCallbackFxn
 *
 * @brief  Callback from PIN driver on interrupt
 *
 *         Sets in motion the debouncing.
 *
 * @param  handle    The PIN_Handle instance this is about
 * @param  pinId     The pin that generated the interrupt
 */


/******************************************************************************
 *****************************************************************************
 *
 *  Utility functions
 *
 ****************************************************************************
 *****************************************************************************/

/*********************************************************************
 * @fn     ProjectZero_enqueueMsg
 *
 * @brief  Utility function that sends the event and data to the application.
 *         Handled in the task loop.
 *
 * @param  event    Event type
 * @param  pData    Pointer to message data
 */
static status_t ProjectZero_enqueueMsg(uint8_t event, void *pData)
{
    uint8_t success;
    pzMsg_t *pMsg = ICall_malloc(sizeof(pzMsg_t));

    if(pMsg)
    {
        pMsg->event = event;
        pMsg->pData = pData;

        success = Util_enqueueMsg(appMsgQueueHandle, syncEvent, (uint8_t *)pMsg);
        return (success) ? SUCCESS : FAILURE;
    }

    return(bleMemAllocError);
}

/*********************************************************************
 * @fn     util_arrtohex
 *
 * @brief   Convert {0x01, 0x02} to "01:02"
 *
 * @param   src - source byte-array
 * @param   src_len - length of array
 * @param   dst - destination string-array
 * @param   dst_len - length of array
 *
 * @return  array as string
 */
char * util_arrtohex(uint8_t const *src, uint8_t src_len,
                     uint8_t *dst, uint8_t dst_len, uint8_t reverse)
{
    char hex[] = "0123456789ABCDEF";
    uint8_t *pStr = dst;
    uint8_t avail = dst_len - 1;
    int8_t inc = 1;
    if(reverse)
    {
        src = src + src_len - 1;
        inc = -1;
    }

    memset(dst, 0, avail);

    while(src_len && avail > 3)
    {
        if(avail < dst_len - 1)
        {
            *pStr++ = ':';
            avail -= 1;
        }

        *pStr++ = hex[*src >> 4];
        *pStr++ = hex[*src & 0x0F];
        src += inc;
        avail -= 2;
        src_len--;
    }

    if(src_len && avail)
    {
        *pStr++ = ':'; // Indicate not all data fit on line.
    }
    return((char *)dst);
}

/*********************************************************************
 * @fn     util_getLocalNameStr
 *
 * @brief   Extract the LOCALNAME from Scan/AdvData
 *
 * @param   data - Pointer to the advertisement or scan response data
 * @param   len  - Length of advertisment or scan repsonse data
 *
 * @return  Pointer to null-terminated string with the adv local name.
 */
static char * util_getLocalNameStr(const uint8_t *data, uint8_t len)
{
    uint8_t nuggetLen = 0;
    uint8_t nuggetType = 0;
    uint8_t advIdx = 0;

    static char localNameStr[32] = { 0 };
    memset(localNameStr, 0, sizeof(localNameStr));

    for(advIdx = 0; advIdx < len; )
    {
        nuggetLen = data[advIdx++];
        nuggetType = data[advIdx];
        if((nuggetType == GAP_ADTYPE_LOCAL_NAME_COMPLETE ||
            nuggetType == GAP_ADTYPE_LOCAL_NAME_SHORT) )
        {
            uint8_t len_temp = nuggetLen < (sizeof(localNameStr)-1)? (nuggetLen - 1):(sizeof(localNameStr)-2);
            // Only copy the first 31 characters, if name bigger than 31.
            memcpy(localNameStr, &data[advIdx + 1], len_temp);
            break;
        }
        else
        {
            advIdx += nuggetLen;
        }
    }

    return(localNameStr);
}

/*********************************************************************
*********************************************************************/
