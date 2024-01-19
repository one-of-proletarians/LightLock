#ifndef _MAIN_H_
#define _MAIN_H_

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <timers.h>
// #include <EEPROM.h>

#define EX_ENABLE() attachInterrupt(0, readISR, CHANGE)
#define EX_DISABLE() detachInterrupt(0)

#define MAX_LEN 40
#define ACCEPT_DIFF 16
#define ATTEMPS_COUNT 3

#define RESET_TIMER pdMS_TO_TICKS(2000)
#define RELAY_TIMER pdMS_TO_TICKS(20000)
#define POWER_TIMER pdMS_TO_TICKS(60000)

#define SENSOR_PIN 2
#define ERROR_PIN 3
#define RESET_PIN 4
#define RELAY_PIN 5
#define STATUS_PIN 6
#define POWER_PIN 7

#define openLock() digitalWrite(RELAY_PIN, HIGH)
#define setReset() digitalWrite(RESET_PIN, LOW)
#define setStatus(x) digitalWrite(STATUS_PIN, x)
#define powerOFF() digitalWrite(POWER_PIN, LOW)

volatile uint16_t password[MAX_LEN];
volatile uint8_t count = 0;

static BaseType_t resetCount = 0;

TaskHandle_t xComparisonHandle;
TaskHandle_t xNewPassHandle;

TaskHandle_t *pxHandle = &xComparisonHandle;

TimerHandle_t xTimerReset;
TimerHandle_t xTimerPower;

void vTaskNewPassword(void *);
void vTaskComparison(void *);

void vTimerReset(TimerHandle_t);
void vTimerPower(TimerHandle_t);

void readISR(void);
void newPassButtonISR(void);

#endif