/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed-drivers/mbed.h"
#include "ble/BLE.h"
#include "EddystoneService.h"

#include "PersistentStorageHelper/ConfigParamsPersistence.h"

EddystoneService *eddyServicePtr;

/* Duration after power-on that config service is available. */
static const int CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS = 30;

/* Default UID frame data */
static const UIDNamespaceID_t uidNamespaceID = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
static const UIDInstanceID_t  uidInstanceID  = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

/* Default version in TLM frame */
static const uint8_t tlmVersion = 0x00;

/* Values for ADV packets related to firmware levels, calibrated based on measured values at 1m */
static const PowerLevels_t defaultAdvPowerLevels = {-47, -33, -21, -13};
/* Values for radio power levels, provided by manufacturer. */
static const PowerLevels_t radioPowerLevels      = {-30, -16, -4, 4};

#define REDLED_OFF YOTTA_CFG_PLATFORM_REDLED_OFF
#define LED_OFF YOTTA_CFG_PLATFORM_LED_OFF

DigitalOut led(YOTTA_CFG_PLATFORM_LED, LED_OFF);
DigitalOut redled(YOTTA_CFG_PLATFORM_REDLED, REDLED_OFF);
InterruptIn button(YOTTA_CFG_PLATFORM_RESET_BUTTON);

static int beaconIsOn = 1;
static minar::callback_handle_t handle = 0;

static void red_off(void) {
    redled = REDLED_OFF;
}

static void led_off(void) {
    led = LED_OFF;
}

/**
 * Callback triggered upon a disconnection event.
 */
static void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *cbParams)
{
    (void) cbParams;
    BLE::Instance().gap().startAdvertising();
    led_off();
}

/**
 * Callback triggered some time after application started to switch to beacon mode.
 */
static void timeout(void)
{
    Gap::GapState_t state;
    state = BLE::Instance().gap().getState();
    if (!state.connected) { /* don't switch if we're in a connected state. */
        eddyServicePtr->startBeaconService();
        EddystoneService::EddystoneParams_t params;
        eddyServicePtr->getEddystoneParams(params);
        saveEddystoneServiceConfigParams(&params);
    } else {
        handle = minar::Scheduler::postCallback(timeout)
                 .delay(minar::milliseconds(CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS * 1000))
                 .getHandle();
    }
}

/**
 * Blink the LED to show that we're in config mode
 */
static int stillBlinking = 0;
static const int BLINKY_ON = 200;
static const int BLINKY_OFF = 600;

// Could be better way to do this. Recursively calls itself to toggle the LED
// global stillBlinking is the flag to stop, set in config_LED_off
static void blinky(void)
{
    // Creating a blink effect (more off than on)
    if (stillBlinking) {
        if (led) {
            minar::Scheduler::postCallback(blinky).delay(minar::milliseconds(BLINKY_OFF));
        }
        else {
            minar::Scheduler::postCallback(blinky).delay(minar::milliseconds(BLINKY_ON));
        }
        led = !led;
    }
}

// Stops the blinking
static void config_LED_off(void) {
    led = LED_OFF;
    stillBlinking = 0;
}

// Starts the blinking
static void config_LED_on(void) {
    stillBlinking = 1;
    led = !LED_OFF;
    minar::Scheduler::postCallback(blinky).delay(minar::milliseconds(BLINKY_ON));
    minar::Scheduler::postCallback(config_LED_off).delay(minar::milliseconds(CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS * 1000));
}


/**
 * Callback used to handle button presses from thread mode (not IRQ)
 */
static void button_task(void) {
    /* Once we get here, we don't want the old timeout callback to fire */
    minar::Scheduler::cancelCallback(handle);

    if (beaconIsOn) {
        eddyServicePtr->stopCurrentService();
        beaconIsOn = 0;

        /* Turn the red LED on and schedule a task to turn it off in 1s */
        redled = !REDLED_OFF;
        config_LED_off();   // just in case it's still running...
        minar::Scheduler::postCallback(red_off).delay(minar::milliseconds(1000));
    } else {
        /* We always startup in config mode */
        eddyServicePtr->startConfigService();
        beaconIsOn = 1;
        handle = minar::Scheduler::postCallback(timeout)
                 .delay(minar::milliseconds(CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS * 1000))
                 .getHandle();

        /* Turn on the main LED and schedule a task to turn it off after 1s timeout */
        config_LED_on();
    }
}

/**
 * Raw IRQ handler for the reset button. We don't want to actually do any work here.
 * Instead, we queue work to happen later using minar, by posting a callback.
 * This has the added avantage of serialising actions, so if the button press happens
 * during the config->beacon mode transition timeout, the button_task won't happen
 * until the previous task has finished.
 *
 * If your buttons aren't debounced, you should do this in software, or button_task
 * might get queued multiple times.
 */
static void reset_rise(void)
{
    minar::Scheduler::postCallback(button_task);
}

static void onBleInitError(BLE::InitializationCompleteCallbackContext* initContext)
{
    /* Initialization error handling goes here... */
    (void) initContext;
}

static void initializeEddystoneToDefaults(BLE &ble)
{
    /* Set everything to defaults */
    eddyServicePtr = new EddystoneService(ble, defaultAdvPowerLevels, radioPowerLevels);

    /* Set default URL, UID and TLM frame data if not initialized through the config service */
    const char* url = YOTTA_CFG_EDDYSTONE_DEFAULT_URL;
    eddyServicePtr->setURLData(url);
    eddyServicePtr->setUIDData(uidNamespaceID, uidInstanceID);
    eddyServicePtr->setTLMData(tlmVersion);
}

static void bleInitComplete(BLE::InitializationCompleteCallbackContext* initContext)
{
    BLE         &ble  = initContext->ble;
    ble_error_t error = initContext->error;

    if (error != BLE_ERROR_NONE) {
        onBleInitError(initContext);
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);

    EddystoneService::EddystoneParams_t params;
    if (loadEddystoneServiceConfigParams(&params)) {
        eddyServicePtr = new EddystoneService(ble, params, radioPowerLevels);
    } else {
        initializeEddystoneToDefaults(ble);
    }

    /* Start Eddystone in config mode */
    config_LED_on();
    eddyServicePtr->startConfigService();
    handle = minar::Scheduler::postCallback(timeout)
             .delay(minar::milliseconds(CONFIG_ADVERTISEMENT_TIMEOUT_SECONDS * 1000))
             .getHandle();
}

void app_start(int, char *[])
{
    /* Tell standard C library to not allocate large buffers for these streams */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    setbuf(stdin, NULL);

    button.rise(&reset_rise);

    BLE &ble = BLE::Instance();
    ble.init(bleInitComplete);
}
