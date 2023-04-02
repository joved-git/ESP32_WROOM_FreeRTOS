// debounce.ino 
// MIT License (see file LICENSE)

// LED is active high
#define GPIO_LED    12 
#define GPIO_BUTTON 25
#define LED_ON      0
#define LED_OFF     1

static QueueHandle_t queue;

//
// Button Debouncing task:
//
static void debounce_task(void *argp) 
{
  uint32_t level, state = 0, last = 0xFFFFFFFF;
  uint32_t mask = 0x7FFFFFFF;
  bool event;
  
  for (;;) {
    level = !!digitalRead(GPIO_BUTTON);
    
    state = (state << 1) | level;

    if ( (state & mask) == mask || (state & mask) == 0 )          // Check if the state changes without bouncing effect.
    {
      if ( level != last ) 
      {
        event = !!level;

        if ( xQueueSendToBack(queue, &event,1) == pdPASS )
          last = level;
      }
    }
    
    taskYIELD();
  }
}

//
// LED queue receiving task
//
static void led_task(void *argp) 
{
  BaseType_t s;
  bool event=false;
  bool led = true;          // Set here the default value of the LED. Here true means the the LED will be OFF at startup.
  
  for (;;) {
    s = xQueueReceive(
      queue,
      &event,
      portMAX_DELAY
    );

    assert(s == pdPASS);

    if ( event ) 
    {
      // Button press:
      // Toggle LED
      led ^= true;
      digitalWrite(GPIO_LED,led);
    }
  }
}

//
// Initialization:
//
void setup() 
{
  int app_cpu = xPortGetCoreID();
  TaskHandle_t h;
  BaseType_t rc;

  delay(2000);          // Allow USB to connect
  queue = xQueueCreate(40, sizeof(bool));
  assert(queue);

  pinMode(GPIO_LED,OUTPUT);
  pinMode(GPIO_BUTTON,INPUT_PULLUP);

  // Light LED initially
  digitalWrite(GPIO_LED, LED_ON);
  vTaskDelay(pdMS_TO_TICKS(300));
  digitalWrite(GPIO_LED, LED_OFF);
  vTaskDelay(pdMS_TO_TICKS(300));
  digitalWrite(GPIO_LED, LED_ON);
  vTaskDelay(pdMS_TO_TICKS(300));

  rc = xTaskCreatePinnedToCore(
    debounce_task,
    "debounce",
    2048,     // Stack size
    nullptr,  // No args
    1,        // Priority
    &h,       // Task handle
    app_cpu   // CPU
  );
  
  assert(rc == pdPASS);
  assert(h);

  rc = xTaskCreatePinnedToCore(
    led_task,
    "led",
    2048,     // Stack size
    nullptr,  // Not used
    1,        // Priority
    &h,       // Task handle
    app_cpu   // CPU
  );

  assert(rc == pdPASS);
  assert(h);
}

// Not used:
void loop() 
{
  vTaskDelete(nullptr);
}
