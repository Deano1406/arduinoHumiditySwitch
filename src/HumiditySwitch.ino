/*
This code is for an Arduino Pro mini, a DTH11 temperature and humidity sensor and a 240v relay. This will will be connected to a bathroom ventilation fan, this will convert from a timed fan to a fan that runs from a humidity sensor in the bathroom. In the initial version the humidity trigger value is set in software, my thinking is I will change this to a potentiometer in a subsequent version and have a space on the board for the pot.
*/

#include <DHT.h>
#include <DHT_U.h>
#include "../../../../../../.platformio/packages/framework-arduinoavr/cores/arduino/HardwareSerial.h"

#define DHTTYPE           DHT11
#define DHTPIN            10         // Pin which is connected to the DHT sensor.
#define RELAYSWITCH       9         // Pin which is connected to the realy switch.
#define LEDPIN            13

DHT_Unified dht(DHTPIN, DHTTYPE);

const int blinkDuration = 500;
const int LEDInterval = 2500;
const int targetHumidity = 50; // 50% humidity is the trigger value for this version of code - this will be moved to a pot?


int sensorReadDelay = 10000;
int switchUpdateDelay = 30 * 1000;
int sensorHumidity = 60;

byte relaySwitchState = LOW;
byte relayPreviousSwitchState = LOW;
byte LEDState = LOW;
unsigned long currentMillis = 0;
unsigned long previousLEDMillis = 0;
unsigned long previousSensorReadMillis = 0;
unsigned long previousRelaySwitchMillis = 0;

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    pinMode(RELAYSWITCH, OUTPUT);
    updateLEDState();

    sensorHumidity = 0;

    Serial.println("------------------------------------");
    Serial.println("Humidity 240v Switch Setup");

    delay(10000);
    // Initialize device.
    dht.begin();

    // Print temperature sensor details.
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Temperature");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println(" *C");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println(" *C");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println(" *C");
    Serial.println("------------------------------------");
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Humidity");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println("%");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println("%");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println("%");
    Serial.println("------------------------------------");
    // Set delay between sensor readings based on sensor details.
    sensorReadDelay = sensor.min_delay / 1000;
    Serial.println("------------------------------------");

}

void loop() {
    // put your main code here, to run repeatedly:

    /*
    The code here isn't your usual Arduino style with delays. I have used the following guide for inspiration for moving away from Delays ->
    https://forum.arduino.cc/index.php?topic=503368.0
    */

    currentMillis = millis(); // gets the vurrent millisecond value
    updateLEDState(); // updates the LED state depending on the times and values above
    switchLED(); // Sets the LED to the LED State
    readHumidity(); // reads the current humidity
    updateRelayState(); // updates the relay switch state, based on the values above
    switchRelay(); // Sets the relay switch state

}


void switchFan() {
    if (relaySwitchState == LOW) {
        Serial.println("** Fan Off **");
        digitalWrite(RELAYSWITCH, HIGH);
        relaySwitchState = HIGH;
    } else {
        Serial.println("** Fam On **");
        digitalWrite(RELAYSWITCH, LOW);
        relaySwitchState = LOW;
    }
}

void readHumidity() {
    if (currentMillis - previousSensorReadMillis >= sensorReadDelay) {
        sensors_event_t event;
        dht.humidity().getEvent(&event);
        sensorHumidity = event.relative_humidity;
        Serial.println(sensorHumidity);
        previousSensorReadMillis += sensorReadDelay;
    }
}

void updateLEDState() {

    if (LEDState == LOW) {
        // if the Led is off, we must wait for the interval to expire before turning it on
        if (currentMillis - previousLEDMillis >= LEDInterval) {
            // time is up, so change the state to HIGH
            LEDState = HIGH;
            // and save the time when we made the change
            previousLEDMillis += LEDInterval;
            // NOTE: The previous line could alternatively be
            //              previousLEDMillis = currentMillis
            //        which is the style used in the BlinkWithoutDelay example sketch
            //        Adding on the interval is a better way to ensure that succesive periods are identical

        }
    } else {  // i.e. if LEDState is HIGH

        // if the Led is on, we must wait for the duration to expire before turning it off
        if (currentMillis - previousLEDMillis >= blinkDuration) {
            // time is up, so change the state to LOW
            LEDState = LOW;
            // and save the time when we made the change
            previousLEDMillis += blinkDuration;
        }
    }
}

void updateRelayState() {

    if (sensorHumidity >= targetHumidity) {
        relaySwitchState = HIGH;
    } else {
        relaySwitchState = LOW;
    }
}


void switchLED() {
    digitalWrite(LEDPIN, LEDState);
}

void switchRelay() {
    if (currentMillis - previousRelaySwitchMillis >= switchUpdateDelay ) {
        if (relaySwitchState != relayPreviousSwitchState) {
            Serial.println("Set relay pin to ");
            Serial.print(relaySwitchState);
            Serial.println(".");
            digitalWrite(RELAYSWITCH, relaySwitchState);
            relayPreviousSwitchState = relaySwitchState;
        }
        previousRelaySwitchMillis += switchUpdateDelay;
    }
}