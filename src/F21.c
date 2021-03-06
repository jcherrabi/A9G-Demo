/* ------------------------------------------------------------------------- */
/* --------------------------------- F21.c --------------------------------- */
/* ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "api_os.h"
#include "api_event.h"
#include "api_debug.h"
#include "api_info.h"

#include "api_hal_pm.h"

#include "../include/F21.h"

#if defined(ENABLE_GPIO_TASK)
#include "../include/gpio_task.h"
#endif /*   ENABLE_GPIO_TASK   */

#if defined(ENABLE_UART_TASK)
#include "../include/uart_task.h"
#endif /*  ENABLE_UART_TASK    */

#if defined(ENABLE_GPS_TASK)
#include "../include/gps_task.h"
#endif /*  ENABLE_GPS_TASK */

#if defined(ENABLE_SMS_TASK)
#include"../include/sms_task.h"
#endif /*  ENABLE_SMS_TASK    */

#if defined(ENABLE_CALL_TASK)
#include "../include/call_task.h"
#endif /*  ENABLE_CALL_TASK    */

#if defined(ENABLE_GPRS_TASK)
#include "../include/gprs_task.h"
#endif /*  ENABLE_GPRS_TASK    */

#if defined(ENABLE_MQTT_TASK)
#include "../include/mqtt_task.h"
#endif  /*  ENABLE_MQTT_TASK    */

/* ------------------------------------------------------------------------- */
/* --------------------- Global Variables & Definitions -------------------- */
/* ------------------------------------------------------------------------- */

/* ---------------------------- Main task handle --------------------------- */

static HANDLE mainTaskHandle;

/* ---------------------------- GPIO task handle --------------------------- */
#if defined(ENABLE_GPIO_TASK)
extern HANDLE gpioTaskHandle;
#endif /*  ENABLE_GPIO_TASK    */

/* ---------------------------- UART task handle --------------------------- */
#if defined(ENABLE_UART_TASK)
extern HANDLE uartTaskHandle;
#endif /*  ENABLE_UART_TASK    */

/* ---------------------------- GPS task handle --------------------------- */
#if defined(ENABLE_GPS_TASK)
extern HANDLE gpsTaskHandle;
#endif /*   ENABLE_GPS_TASK     */

/* ---------------------------- CALL task handle -------------------------- */
#if defined(ENABLE_CALL_TASK)
extern HANDLE callTaskHandle;
#endif /*  ENABLE_CALL_TASK    */

/* ---------------------------- SMS task handle ---------------------------- */
#if defined(ENABLE_SMS_TASK)
extern HANDLE smsTaskHandle;
#endif /*  ENABLE_SMS_TASK  */

/* ---------------------------- GPRS task handle --------------------------- */
#if defined(ENABLE_GPRS_TASK)
extern HANDLE gprsTaskHandle;
extern HANDLE cellInfoTaskHandle;
extern HANDLE locationUpdateTaskHandle;
#endif /*  ENABLE_GPRS_TASK    */

/* ---------------------------- MQTT task handle --------------------------- */
#if defined(ENABLE_MQTT_TASK)
static HANDLE mqttTaskHandle;
extern HANDLE uartTaskHandle;
#endif /*  ENABLE_MQTT_TASK    */

/* ----------------------------- Trace Indices ----------------------------- */

#define SIGNAL_INFO_TRACE_INDEX     15
#define BATTERY_INFO_TRACE_INDEX    16
#define SYS_INFO_TRACE_INDEX        16

/* ----------------------------- System Globals ---------------------------- */

bool bNetworkRegisteration;
static HANDLE mNetworkState;
uint8_t DeviceIMEI [20] = {0};

/* ------------------------------------------------------------------------- */
/* ---------------------------- API Definitions ---------------------------- */
/* ------------------------------------------------------------------------- */

void F21_Main(void *pData)
{
    mNetworkState = OS_CreateMutex();
    mainTaskHandle = OS_CreateTask(F21MainTask, NULL, NULL, MAIN_TASK_STACK_SIZE, MAIN_TASK_PRIORITY, 0, 0, MAIN_TASK_NAME);
    OS_SetUserMainHandle(&mainTaskHandle);
}

/* ------------------------------------------------------------------------- */

void F21MainTask(void *pData)
{
    API_Event_t *Local_sEvent = NULL;

#if defined(ENABLE_GPIO_TASK)
    gpioTaskHandle = OS_CreateTask(GPIO_Task, NULL, NULL, GPIO_TASK_STACK_SIZE, GPIO_TASK_PRIORITY, 0, 0, GPIO_TASK_NAME);
#endif /*   ENABLE_GPIO_TASK  */

#if defined(ENABLE_UART_TASK)
    uartTaskHandle = OS_CreateTask(UART_Task, NULL, NULL, UART_TASK_STACK_SIZE, UART_TASK_PRIORITY, 0, 0, UART_TASK_NAME);
#endif /*   ENABLE_UART_TASK    */

#if defined(ENABLE_GPS_TASK)
    gpsTaskHandle = OS_CreateTask(GPS_Task, NULL, NULL, GPS_TASK_STACK_SIZE, GPS_TASK_PRIORITY, 0, 0, GPS_TASK_NAME);
#endif /*   ENABLE_UART_TASK    */

#if defined(ENABLE_SMS_TASK)
    smsTaskHandle = OS_CreateTask(SMS_Task, NULL, NULL, SMS_TASK_STACK_SIZE, SMS_TASK_PRIORITY, 0, 0, SMS_TASK_NAME);
#endif /*   ENABLE_SMS_TASK     */

#if defined(ENABLE_CALL_TASK)
    callTaskHandle = OS_CreateTask(CALL_Task, NULL, NULL, CALL_TASK_STACK_SIZE, CALL_TASK_PRIORITY, 0, 0, CALL_TASK_NAME);
#endif /*  ENABLE_CALL_TASK    */

#if defined(ENABLE_GPRS_TASK)
    gprsTaskHandle = OS_CreateTask(GPRS_Task, NULL, NULL, GPRS_TASK_STACK_SIZE, GPRS_TASK_PRIORITY, 0, 0, GPRS_TASK_NAME);
    cellInfoTaskHandle = OS_CreateTask(GPRS_GetCellInfoTask, NULL, NULL, CELL_INFO_TASK_STACK_SIZE, CELL_INFO_TASK_PRIORITY, 0, 0, CELL_INFO_TASK_NAME);
    locationUpdateTaskHandle = OS_CreateTask(GPRS_LocationUpdateTask, NULL, NULL, LOCATION_UPDATE_TASK_STACK_SIZE, LOCATION_UPDATE_TASK_PRIORITY, 0, 0, LOCATION_UPDATE_TASK_NAME);
#endif /*  ENABLE_GPRS_TASK   */

#if defined(ENABLE_MQTT_TASK)
    mqttTaskHandle = OS_CreateTask(MQTT_Task, NULL, NULL, MQTT_TASK_STACK_SIZE, MQTT_TASK_PRIORITY, 0, 0, MQTT_TASK_NAME);
    uartTaskHandle = OS_CreateTask(UART_Task, NULL, NULL, UART_TASK_STACK_SIZE, UART_TASK_PRIORITY, 0, 0, UART_TASK_NAME);
#endif  /*  ENABLE_MQTT_TASK    */

    while (1)
    {
        if (OS_WaitEvent(mainTaskHandle, (void **)&Local_sEvent, OS_TIME_OUT_WAIT_FOREVER))
        {
            EventDispatch(Local_sEvent);
            OS_Free(Local_sEvent->pParam1);
            OS_Free(Local_sEvent->pParam2);
            OS_Free(Local_sEvent);
        }
    }
}

/* ------------------------------------------------------------------------- */

void EventDispatch(API_Event_t *pEvent)
{
    switch (pEvent->id)
    {
    case API_EVENT_ID_POWER_ON:
        Trace(SYS_INFO_TRACE_INDEX, "[POWER ON EVENT] Pudding powered on...");
        break;

    case API_EVENT_ID_SYSTEM_READY:
        Trace(SYS_INFO_TRACE_INDEX, "[SYSTEM READY EVENT] Pudding system is ready...");
        if(INFO_GetIMEI(DeviceIMEI))
        {
            Trace(SYS_INFO_TRACE_INDEX, "[SYS INFO EVENT] Device IMEI: %s", DeviceIMEI);
        }
        break;

    case API_EVENT_ID_NO_SIMCARD:
        Trace(SYS_INFO_TRACE_INDEX, "[NO SIM CARD EVENT] No sim card detected!");
        break;

    case API_EVENT_ID_SIMCARD_DROP:
        Trace(SYS_INFO_TRACE_INDEX, "[SIM CARD DROP EVENT] Sim card drop!");
        break;

    case API_EVENT_ID_SIGNAL_QUALITY:
        Trace(SIGNAL_INFO_TRACE_INDEX, "[SIGNAL QUALITY EVENT] Signal Quality: %d", pEvent->param1);
        Trace(SIGNAL_INFO_TRACE_INDEX, "[SIGNAL QUALITY EVENT] RSSI: %d", pEvent->param1 * 2 - 113);
        Trace(SIGNAL_INFO_TRACE_INDEX, "[SIGNAL QUALITY EVENT] RX Qual: %d", pEvent->param2);
        break;

    case API_EVENT_ID_NETWORK_GOT_TIME:
    {
        RTC_Time_t *Local_psCurrentTime = (RTC_Time_t *)pEvent->pParam1;
        Trace(SYS_INFO_TRACE_INDEX, "[NETWORK TIME EVENT] Netowrk Date: %02d/%02d/%04d Time : %02d:%02d:%02d", Local_psCurrentTime->day, Local_psCurrentTime->month, Local_psCurrentTime->year, Local_psCurrentTime->hour, Local_psCurrentTime->minute, Local_psCurrentTime->second);
    }
    break;

    case API_EVENT_ID_NETWORK_CELL_INFO:
    {
        uint8_t Local_u8Number = pEvent->param1;
        Network_Location_t *Local_psnetworkLocation = (Network_Location_t *)pEvent->pParam1;
        Trace(SYS_INFO_TRACE_INDEX, "[NETWORK CELL INFO EVENT] Current cell number: %d, Neighbor cell number: %d", Local_u8Number, Local_u8Number - 1);

        for (uint32_t i = 0; i < Local_u8Number; i++)
        {
            Trace(SYS_INFO_TRACE_INDEX, "[NETWORK CELL INFO EVENT] Cell %d info:", i);
            Trace(SYS_INFO_TRACE_INDEX, "[NETWORK CELL INFO EVENT] CellId: %d, CellLac: %d, CellBsic: %d", Local_psnetworkLocation->sCellID, Local_psnetworkLocation->sLac, Local_psnetworkLocation->iBsic);
            Trace(SYS_INFO_TRACE_INDEX, "[NETWORK CELL INFO EVENT] CellRxLevel: %d, CellRxLevelSub: %d, CellARFCN: %d", Local_psnetworkLocation->iRxLev, Local_psnetworkLocation->iRxLevSub, Local_psnetworkLocation->nArfcn);
        }
    }
    break;

    case API_EVENT_ID_NETWORK_REGISTERED_HOME:
    case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
        Trace(SYS_INFO_TRACE_INDEX, "[NETWORK REGISTATION EVENT] Network registration complete.");
        OS_LockMutex(mNetworkState);
        bNetworkRegisteration = true;
        OS_UnlockMutex(mNetworkState);

#if defined(ENABLE_GPRS_TASK)
        GPRS_NetworkRegistered_EventHandler(pEvent);
#endif  /*  ENABLE_GPRS_TASK    */

#if defined(ENABLE_MQTT_TASK)
    MQTT_NetworkRegistered_EventHandler(pEvent);
#endif  /*  ENABLE_MQTT_TASK    */

        break;
    
    case API_EVENT_ID_NETWORK_DEREGISTER:
        Trace(SYS_INFO_TRACE_INDEX, "[NETWORK DE_REGISTRATION EVENT] Network deregistered!");
        OS_LockMutex(mNetworkState);
        bNetworkRegisteration = false;
        OS_UnlockMutex(mNetworkState);
        Trace(SYS_INFO_TRACE_INDEX, "[NETWORK DE_REGISTRATION EVENT] Restarting after 5 seconds...");
        OS_Sleep(5000);
        PM_Restart();
        break;

#if defined(ENABLE_UART_TASK) || defined(ENABLE_SMS_TASK) || defined(ENABLE_CALL_TASK) || defined(ENABLE_GPRS_TASK) || defined (ENABLE_MQTT_TASK)
    case API_EVENT_ID_UART_RECEIVED:

#if defined(ENABLE_SMS_TASK)

        SMS_UART_RX_EventHandler(pEvent);
        break;

#elif defined(ENABLE_CALL_TASK)
        CALL_UART_RX_EventHandler(pEvent);
        break;

#elif defined(ENABLE_GPRS_TASK)
        GPRS_UART_RX_EventHandler(pEvent);
        break;

#elif defined(ENABLE_MQTT_TASK)
        UART_RX_EventHandler(pEvent);
        break;

#else /* UART_TASK  */

#if defined(ENABLE_UART_EVENTS)

        UART_RX_EventHandler(pEvent);
        break;

#endif /*  ENABLE_UART_EVENTS  */

#endif /*   ENABLE_SMS_TASK    */

#endif /*  ENABLE_UART_TASK || ENABLE_SMS_TASK || ENABLE_CALL_TASK || ENABLE_GPRS_TASK || ENABLE_MQTT_TASK  */

#if defined(ENABLE_GPS_TASK)

    case API_EVENT_ID_GPS_UART_RECEIVED:
        GPS_UART_RX_EventHandler(pEvent);
        break;

#endif  /*  ENABLE_GPRS_TASK  */

#if defined(ENABLE_SMS_TASK)

    case API_EVENT_ID_SMS_RECEIVED:
        SMS_Received_EventHandler(pEvent);
        break;

    case API_EVENT_ID_SMS_SENT:
        Trace(SMS_TRACE_INDEX, "[SMS SENT EVENT] SMS Sent...");
        break;

#endif /*  ENABLE_SMS_TASK    */

#if defined(ENABLE_CALL_TASK)

    case API_EVENT_ID_CALL_DIAL:
        CALL_Dial_EventHandler(pEvent);
        break;

    case API_EVENT_ID_CALL_DTMF:
        CALL_DTMF_EventHandler(pEvent);
        break;

    case API_EVENT_ID_CALL_ANSWER:
        CALL_Answer_EventHandler(pEvent);
        break;

    case API_EVENT_ID_CALL_HANGUP:
        CALL_HangUp_EventHandler(pEvent);
        break;

    case API_EVENT_ID_CALL_INCOMING:
        CALL_Incoming_EventHandler(pEvent);
        break;

#endif /*  ENABLE_CALL_TASK    */

#if defined(ENABLE_GPRS_TASK)

    case API_EVENT_ID_GPS_UART_RECEIVED:
        GPRS_GPS_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_DETACHED:
        GPRS_NetworkDeAttached_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ATTACH_FAILED:
        GPRS_NetworkAttachFailed_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ATTACHED:
        GPRS_NetworkAttached_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_DEACTIVED:
        GPRS_NetworkDeActivated_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ACTIVATE_FAILED:
        GPRS_NetworkActivationFailed_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ACTIVATED:
        GPRS_NetworkActivated_EventHandler(pEvent);
        break;
    
    case API_EVENT_ID_SOCKET_CONNECTED:
        GPRS_SocketConnected_EventHandler(pEvent);
        break;

    case API_EVENT_ID_SOCKET_CLOSED:
        GPRS_SocketClosed_EventHandler(pEvent);
        break;

    case API_EVENT_ID_SOCKET_SENT:
        GPRS_SokcetSent_EventHandler(pEvent);
        break;

    case API_EVENT_ID_SOCKET_RECEIVED:
        GPRS_SocektReceived_EventHandler(pEvent);
        break;

    case API_EVENT_ID_SOCKET_ERROR:
        GPRS_SocketError_EventHandler(pEvent);
        break;

#endif /*  ENABLE_GPRS_TASK    */

#if defined(ENABLE_MQTT_TASK)

    case API_EVENT_ID_NETWORK_DETACHED:
        MQTT_NetworkDeAttached_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ATTACH_FAILED:
        MQTT_NetworkAttachFailed_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ATTACHED:
        MQTT_NetworkAttached_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_DEACTIVED:
        MQTT_NetworkDeActivated_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ACTIVATE_FAILED:
        MQTT_NetworkActivationFailed_EventHandler(pEvent);
        break;

    case API_EVENT_ID_NETWORK_ACTIVATED:
        MQTT_NetworkActivated_EventHandler(pEvent);
        break;

#endif  /*  ENABLE_MQTT_TASK    */


    case API_EVENT_ID_POWER_INFO:
        Trace(BATTERY_INFO_TRACE_INDEX, "[POWER INFO EVENT] Battery charger state: %d", ((pEvent->param1 >> 16) & 0xFFFFu));
        Trace(BATTERY_INFO_TRACE_INDEX, "[POWER INFO EVENT] Battery charge level: %d %%", (pEvent->param1 & 0xFFFFu));
        Trace(BATTERY_INFO_TRACE_INDEX, "[POWER INFO EVENT] Battery state: %d", ((pEvent->param2 >> 16) & 0xFFFFu));
        Trace(BATTERY_INFO_TRACE_INDEX, "[POWER INFO EVENT] Battery voltage: %d mv", (pEvent->param2 & 0xFFFFu));
        break;

    default:
        Trace(SYS_INFO_TRACE_INDEX, "handling event id [ %d ]", pEvent->id);
        break;
    }
}


/* ------------------------------------------------------------------------- */
/* ------------------------------ End Of File ------------------------------ */
/* ------------------------------------------------------------------------- */
