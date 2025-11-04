#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
//#include <Servo.h>

// Pin definitions
#define LIGHT_PIN A0
#define ONE_WIRE_BUS 13
const int ledPin = 8;
const int buttonPin = 2;

// EEPROM addresses
#define MAGIC_ADDR      0
#define MAGIC_VALUE     0x42
#define HIGH_TEMP_ADDR  1
#define HIGH_LIGHT_ADDR 5
#define LOW_TEMP_ADDR   9
#define LOW_LIGHT_ADDR  13
#define BUTTON_ADDR     17

// Hardware objects
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//Servo servo;

// Record tracking variables
float highestTemp = -100.0;
float lowestTemp = 1000.0;   
float highestLight = 0.0;
float lowestLight = 1000.0; 
int counter = 0;

// Current sensor readings
volatile float currentTemp = 0.0;
volatile float currentLight = 0.0;

// Display state tracking
float tempLast = -100.0;
float lightLast = -1.0;
int lastCounter = -1;

// Event flags
volatile bool buttonPressed = false;
volatile bool sensorReadFlag = false;
volatile bool displayUpdateFlag = false;

// Timer counters
volatile uint16_t sensorTimerCount = 0;
volatile uint16_t displayTimerCount = 0;
const uint16_t SENSOR_INTERVAL = 1000;   // Read sensors every 1000ms
const uint16_t DISPLAY_INTERVAL = 500;   // Update display every 500ms

// Button debouncing
volatile unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 100;


void buttonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonTime > debounceDelay) {
    buttonPressed = true;
    lastButtonTime = currentTime;
  }
}

void timer1_compa_handler() {
  sensorTimerCount++;
  displayTimerCount++;
  
  if (sensorTimerCount >= SENSOR_INTERVAL) {
    sensorReadFlag = true;
    sensorTimerCount = 0;
  }
  
  if (displayTimerCount >= DISPLAY_INTERVAL) {
    displayUpdateFlag = true;
    displayTimerCount = 0;
  }
}

// Timer1 ISR - connects hardware timer to handler
ISR(TIMER1_COMPA_vect) {
  timer1_compa_handler();
}

void setup() {
  
  //servo.attach(10);
  //servo.write(0);
  sensors.begin();
  lcd.begin(16, 2);
  
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.clear();
  lcd.print("Initializing...");
  
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
  setupTimer1();
  
  loadEEPROMData();
  
  delay(1000);
  lcd.clear();
  
  readSensors();
  updateDisplay();
}


void loop() {

  if (buttonPressed) {
    handleButtonPress();
    buttonPressed = false;
  }
  
  if (sensorReadFlag) {
    readSensors();
    checkAndUpdateRecords();
    controlLED();
    sensorReadFlag = false;
  }
  
  if (displayUpdateFlag) {
    updateDisplay();
    displayUpdateFlag = false;
  }
}

void readSensors() {
  currentLight = analogRead(LIGHT_PIN);
  
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0); 
 }

void checkAndUpdateRecords() {
  if (currentTemp > highestTemp) {
    highestTemp = currentTemp;
    EEPROM.put(HIGH_TEMP_ADDR, highestTemp);
  }
  
  if (currentTemp < lowestTemp) {
    lowestTemp = currentTemp;
    EEPROM.put(LOW_TEMP_ADDR, lowestTemp);
  }
  
  if (currentLight > highestLight) {
    highestLight = currentLight;
    EEPROM.put(HIGH_LIGHT_ADDR, highestLight);
  }
  
  if (currentLight < lowestLight) {
    lowestLight = currentLight;
    EEPROM.put(LOW_LIGHT_ADDR, lowestLight);
  }
}


void controlLED() {
  // Turn on LED when light level is under 300
  if (currentLight < 400) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void handleButtonPress() {
  counter++;
  if (counter > 5) { 
    counter = 0;
  }
  lastCounter = -1;  // Force display update
  displayUpdateFlag = true;
  EEPROM.put(BUTTON_ADDR, counter);
}

void updateDisplay() {
  bool needsUpdate = (lastCounter != counter);
  
  switch (counter) {
    case 0: 
      if (needsUpdate || tempLast != currentTemp) {
        tempLast = currentTemp;
        printData("TEMPERATURE:", " C", currentTemp, lcd);
        lastCounter = counter;
      }
      break;
      
    case 1: 
      if (needsUpdate || lightLast != currentLight) {
        lightLast = currentLight;
        printData("LIGHT LVL:", "", currentLight, lcd);
        lastCounter = counter;
      }
      break;
      
    case 2: 
      printData("HIGHEST TEMP:", " C", highestTemp, lcd);
      lastCounter = counter;
      break;
      
    case 3:
      printData("LOWEST TEMP:", " C", lowestTemp, lcd);
      lastCounter = counter;
      break;
      
    case 4: 
      printData("HIGHEST LIGHT:", "", highestLight, lcd);
      lastCounter = counter;
      break;
      
    case 5: 
      printData("LOWEST LIGHT:", "", lowestLight, lcd);
      lastCounter = counter;
      break;
  }
}

void printData(String text, String text2, float value, LiquidCrystal &lcd) {
  lcd.clear();
  lcd.print(text);
  lcd.setCursor(0, 1);
  lcd.print(value, 1);  // Show 1 decimal place
  lcd.print(text2);
}

void loadEEPROMData() {
  if (EEPROM.read(MAGIC_ADDR) == MAGIC_VALUE) {
    // Valid data exists, load it
    EEPROM.get(HIGH_TEMP_ADDR, highestTemp);
    EEPROM.get(LOW_TEMP_ADDR, lowestTemp);
    EEPROM.get(HIGH_LIGHT_ADDR, highestLight);
    EEPROM.get(LOW_LIGHT_ADDR, lowestLight);
    EEPROM.get(BUTTON_ADDR, counter);
  } else {
    // First run, initialize EEPROM with magic value
    EEPROM.write(MAGIC_ADDR, MAGIC_VALUE);
    EEPROM.put(HIGH_TEMP_ADDR, highestTemp);
    EEPROM.put(LOW_TEMP_ADDR, lowestTemp);
    EEPROM.put(HIGH_LIGHT_ADDR, highestLight);
    EEPROM.put(LOW_LIGHT_ADDR, lowestLight);
    EEPROM.put(BUTTON_ADDR, counter);
  }
}

void setupTimer1() {
  noInterrupts();
  
  // Clear Timer1 control registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Set compare match register for 1ms (1000 Hz)
  // Formula: (16MHz / (prescaler * frequency)) - 1
  // (16000000 / (64 * 1000)) - 1 = 249
  OCR1A = 249;
  
  // Configure Timer1
  TCCR1B |= (1 << WGM12);               // CTC mode
  TCCR1B |= (1 << CS11) | (1 << CS10);  // 64 prescaler
  TIMSK1 |= (1 << OCIE1A);              // Enable compare interrupt
  
  interrupts();
}