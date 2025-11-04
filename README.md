# GreenhouseManagmentV2

The Arduino Uno R3 microcontroller is connected to multiple sensors, an LED light and a servo:
DS1820 TO92 digital temperature sensor

Photoresistor
LED light
LCD display
Button
There is an LCD display that shows the current value of the measurements. The values can be cycled through by pressing the button. The button cycles between the current temperature, current light level, highest and lowest temperature and then highest and lowest light.

Pin definitions:
 Temperature Sensor (DS18B20) -  Digital 13 
 Light Sensor - Analog A0 
 LED Output  - Digital 8 
 Button Input - Digital 2
 LCD RS - Digital 12 
 LCD Enable - Digital 11 
 LCD D4 - Digital 5 
 LCD D5 - Digital 4 
 LCD D6 - Digital 3 
 LCD D7 - Digital 6 


EEPROM layout

Address
Size
Name
Explanation
0
1 byte
Magic_Value
Indicates valid EEPROM data
1-4
4 bytes
highestTemp
Highest recorded temperature
5-8
4 bytes
highestLight
Highest recorded light level
9-12
4 bytes
lowestTemp
Lowest recorded temperature
13-16
4 bytes
lowestLight
Lowest recorded light level
17-20
4 bytes
counter
Last display mode


Interrupt instructions:

buttonISR():
- Debounces button input (50ms threshold)
- Sets `buttonPressed` flag for main loop processing
- Uses `millis()` for software debouncing

2. Function: `ISR(TIMER1_COMPA_vect)`
- Increments counter variables
- Sets `sensorReadFlag` every 1000ms
- Sets `displayUpdateFlag` every 500ms

What works
	The system successfully measures the values from the sensors and is able to write them to memory to survive a power reset. The LCD screen switches values by button press.

	
Future improvements

Assuming that the light sensor would be in the same room as the plants, there needs to be a system that would make sure that the LED lights would not trigger the light sensor and create an ON/OFF loop.
A motor or servo could be connected to activate whenever the temperature goes over a certain value.
