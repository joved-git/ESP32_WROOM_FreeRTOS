// basic_tasks.ino 
// MIT License (see file LICENSE)

// Change the following if you want to use
// different GPIO pins for the three LEDs

//            USB
//            ___
//       --- [___]---
//     --            --
//     --            --
//     --            --
// G15 --            --
//     --            -- G13
//     --            --
//     --            -- G12
//     --            --
//     --    TOP     --
//     --            --
//     --            --
//     --            --
//     --            --
//     --            --
//     --            --
//     --            --
//     --            --
//     --            --
//     --            --
//       ------------


#define LED1  12  // GPIO 12 - red
#define LED2  13  // GPIO 13 - yellow
#define LED3  15  // GPIO 15 - green

struct s_led {
  byte          gpio;	    // LED GPIO number
  byte          state;	  // LED state
  unsigned      napms;    // Delay to use (ms)
  TaskHandle_t  taskh;    // Task handle
  UBaseType_t   priority; // Task priority (0 = lowest)
};

static s_led leds[3] = {
  { LED1, 0, 500, 0, 2 },           // GPIO 12 - red
  { LED2, 0, 200, 0, 1 },           // GPIO 13 - yellow
  { LED3, 0, 750, 0, 1 }            // GPIO 15 - green
};

static void led_task_func(void *argp) {
  s_led *ledp = (s_led*)argp;
  unsigned stack_hwm = 0, temp;

  delay(1000);
  
  for (;;) 
  {
    digitalWrite(ledp->gpio,ledp->state ^= 1);
    temp = uxTaskGetStackHighWaterMark(nullptr);
    if ( !stack_hwm || temp < stack_hwm ) {
      stack_hwm = temp;
      printf("Task for gpio %d has stack hwm %u\n",
        ledp->gpio,stack_hwm);
    }
    delay(ledp->napms);
  }
}

void setup() {
  int app_cpu = 0;    // CPU number

  delay(500);       // Pause for serial setup

  app_cpu = xPortGetCoreID();
  printf("app_cpu is %d (%s core)\n",
    app_cpu,
    app_cpu > 0 ? "Dual" : "Single");

  printf("LEDs on gpios: ");
  for ( auto& led : leds ) {
    pinMode(led.gpio,OUTPUT);
    digitalWrite(led.gpio,LOW);
    xTaskCreatePinnedToCore(
      led_task_func,
      "led_task",
      2048,
      &led,
      led.priority,
      &led.taskh,
      app_cpu
    );
    printf("%d ",led.gpio);
  }
  putchar('\n');
}

void loop() {
  delay(1000);
}
