/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "test.h"

#include "lwipthread.h"

#include "web/web.h"
#include "mqtt/mqtt.h"

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);

static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palClearPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(500);
  }
}

static THD_WORKING_AREA(wa_lwip_thread, LWIP_THREAD_STACK_SIZE);
static msg_t lwip_thread(void *arg) {

  (void)arg;
  chRegSetThreadName("lwip_thread");
  while (TRUE) {
    palClearPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(100);
    palSetPad(GPIOD, GPIOD_LED4);
    chThdSleepMilliseconds(100);
  }
}
/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 6 using the driver default configuration.
   */
  sdStart(&SD6, NULL);
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(8));
  palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(8));

  /*
   * Set Pad Mode to PUSHPULL on D0
   */
  palSetPadMode(GPIOD, GPIOD_PIN0, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOD, GPIOD_PIN0);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Creates the LWIP threads (it changes priority internally).
   */
  chThdCreateStatic(wa_lwip_thread, LWIP_THREAD_STACK_SIZE, NORMALPRIO + 1,
                    lwip_thread, NULL);

  /*
   * Creates the HTTP thread (it changes priority internally).
   */
  chThdCreateStatic(wa_http_server, sizeof(wa_http_server), NORMALPRIO + 1,
                    http_server, NULL);

  /*
   * Creates the MQTT Client thread (it changes priority internally).
   */
  chThdCreateStatic(wa_mqtt_client, sizeof(wa_mqtt_client), NORMALPRIO + 1,
                    mqtt_client, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (TRUE) {
    if (palReadPad(GPIOA, GPIOA_BUTTON))
      TestThread(&SD6);
    chThdSleepMilliseconds(500);
  }
}
