//#define ENABLE_SD     // Need to (un)comment headers manually
#define ENABLE_DISPLAY  // Need to (un)comment headers manually

void setup();

#include <Arduino.h>
#include <SPI.h>
#if defined(ENABLE_SD)
//#include <SD.h>
#endif
#if defined(ENABLE_DISPLAY)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_DC 11    // DC
#define OLED_CS 12    // CS
#define OLED_CLK 10   // D0
#define OLED_MOSI 9   // D1
#define OLED_RESET 13 // RST
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#endif

#define THERMOMETER_PROBE_PIN 3
#define FAN_PIN 7

#define STEINHART_0 5.36924e-4
#define STEINHART_1 1.91396e-4
#define STEINHART_2 6.60399e-8
#define STEINHART_3 9.87e+3

#define ADCmax 1023.0f
//#define minTemp 210.0f
//#define maxTemp 220.0f
#define minTemp 310.0f
#define maxTemp 320.0f


float computeTemp(int probeVal)
{
  // Voltage divider to find resistance in the thermistor.
  float Vin = 5.0f;
  float R1 = STEINHART_3;
  float Vout = ((Vin * (float)probeVal) / 1023.0f);
  float R2 = (R1 * Vout) / (Vin - Vout);   // Resistance in the thermistor.

  // Compute degrees Kelvin.
  float R = log(R2);
  float tempK = 1.0f / ((STEINHART_2 * R * R + STEINHART_1) * R + STEINHART_0);

  // Return temperature in fahrenheit.
  return max((((tempK - 273.15f) * 9.0f) / 5.0f) + 32.0f, 0.0f);
}


void setFan(boolean on)
{
  if (on)
  {
    digitalWrite(FAN_PIN, LOW);
  }
  else
  {
    digitalWrite(FAN_PIN, HIGH);
  }
}

#ifdef ENABLE_SD
File tempLogFile;
#endif

boolean initSD() {
#ifdef ENABLE_SD
  Serial.println("Initializing SD card...");
  
  pinMode(4, OUTPUT);
  if (false == SD.begin(4)) {
    Serial.println("SD init fail");
    return false;
  }
  
  char filename[13];
  for (uint16_t index = 1; index < 10000; index++) {
    sprintf(filename, "QLOG%04d.csv", index);
    if (false == SD.exists(filename))
    {
      break;
    }
  }

  tempLogFile = SD.open(filename, FILE_WRITE);

  if (!tempLogFile) {
    Serial.print("Error opening: ");
    Serial.println(filename);
    return false;
  }
  
  Serial.print("Successfully opened file on SD card: ");
  Serial.println(filename);
  #endif
  return true;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

#ifdef ENABLE_DISPLAY
  display.begin();
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setTextSize(5);
#endif

  initSD();

  pinMode(FAN_PIN, OUTPUT);

  Serial.println("Running...");
}


void loop() {
  // put your main code here, to run repeatedly:

  int16_t probeVal = analogRead(THERMOMETER_PROBE_PIN);    // read the input pin

  float temp = computeTemp(probeVal);

  Serial.println(temp);

  if (temp > maxTemp)
  {
    setFan(false);
  }
  else if (temp < minTemp)
  {
    setFan(true);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  int roundTemp = int(temp + 0.5);
  if (roundTemp > 999) {
    display.print("HIGH");
  }
  else {
    if (roundTemp < 100) {
      display.print(" ");
    }
    display.print(roundTemp);
    display.print((char)247);
  }
  display.display();
  
#ifdef ENABLE_SD
  if (tempLogFile)
  {
    tempLogFile.print(millis() / 1000);
    tempLogFile.print(", ");
    tempLogFile.println(temp);
    tempLogFile.flush();
  }
#endif
  delay(1000);
}
