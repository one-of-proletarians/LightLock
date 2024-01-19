#include <main.h>

void setup()
{
  uint16_t currentPassword[MAX_LEN];

  Serial.begin(9600);

  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT); // Выключить питание при подаче 1

  digitalWrite(STATUS_PIN, LOW);
  digitalWrite(ERROR_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(POWER_PIN, HIGH);

  delay(1000);

  if (digitalRead(SENSOR_PIN) == HIGH)
  {
    if (resetCount++ < 3)
      xTimerStart(xTimerReset, 0);
    else
      powerOFF();
  }
  else
  {
    xTimerCreate("reset_t", RESET_TIMER, 0, nullptr, vTimerReset);
    xTimerCreate("power_t", POWER_TIMER, 0, nullptr, vTimerPower);

    xTaskCreate(vTaskComparison, "COMP", configMINIMAL_STACK_SIZE, currentPassword, 0, &xComparisonHandle);
    xTaskCreate(vTaskNewPassword, "NEW", configMINIMAL_STACK_SIZE, nullptr, 0, &xNewPassHandle);

    vTaskStartScheduler();

    xTimerStart(xTimerPower, 0);

    setStatus(HIGH);
  }
}

// ************************
void loop() {}
// ************************

void readISR(void) // CHANGE MODE
{
  password[count++] = millis();
  if (count == MAX_LEN)
  {
    vTaskNotifyGiveFromISR(*pxHandle, nullptr);
  }
}

void newPassButtonISR(void)
{
  count = 0;
  pxHandle = &xNewPassHandle;
  xTimerChangePeriodFromISR(xTimerPower, pdMS_TO_TICKS(20000), 0);
  EX_ENABLE();
}

void vTimerReset(TimerHandle_t xTimer)
{
  setReset();
}

void vTimerPower(TimerHandle_t xTimer)
{
  powerOFF();
}

void vTaskComparison(void *pvParam)
{
  uint16_t *currentPass = (uint16_t *)pvParam;

  uint8_t errorsCount = 0;
  uint16_t duration = 0, prevTime = 0;
  int16_t difference = 0;
  bool isError = false;

  for (;;)
  {
    ulTaskNotifyTake(0, portMAX_DELAY);
    xTimerStop(xTimerPower, 0);
    EX_DISABLE();

    for (uint8_t i = 0; i < MAX_LEN; i++)
    {
      duration = currentPass[i] - prevTime;
      prevTime = currentPass[i];

      difference = password[i] - duration;

      if (abs(difference) > ACCEPT_DIFF)
      {
        if (errorsCount++ < ATTEMPS_COUNT)
        {
          digitalWrite(ERROR_PIN, HIGH);
          xTimerStart(xTimerReset, 0);
          isError = true;
          break;
        }
        else
        {
          powerOFF();
        }
      }
    }

    if (isError == false)
    {
      openLock();
      attachInterrupt(1, newPassButtonISR, FALLING);
      xTimerChangePeriod(xTimerPower, RELAY_TIMER, 0);
    }
  }
}

void vTaskNewPassword(void *pvParam)
{
  for (;;)
  {
    ulTaskNotifyTake(0, portMAX_DELAY);
  }
}

void vApplicationIdleHook()
{
  BaseType_t i = 0;
  for (;;)
  {
    i++;
  }
}
