#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include <esp_adc_cal.h>

//Definicje pinów zgodnie ze schematem
#define CLK_BMS 18
#define DT_BMS 19
#define CLK_VOLT 32
#define DT_VOLT 33
#define CLK_TIME 13
#define DT_TIME 15
#define BUTTON_PIN 26 // Pin przycisku enkodera
#define ENCODER_BUTTON_PIN 14 // Pin przycisku enkodera
#define ANALOG_OUT 25
#define VP_PIN 36 // VP (ADC0)
#define VN_PIN 39 // VN (ADC3)
#define DIGITAL_OUT 2 // wy dioda
#define DIGITAL_BMS 5 // BMS dioda
#define DIGITAL_EV 17 //EV dioda

// Zmienne dla enkoderów
volatile int counter_bms = 0;
volatile int counter_volt = 0;
volatile int index_time = 0;

float bms_value = 0.0;
float volt_value = 0.0;

int values_time[] = {5, 15, 30, 45, 60};

int lastStateCLK_BMS = LOW;
int lastStateCLK_VOLT = LOW;
int lastStateCLK_TIME = LOW;


// Zmienne obsługi przycisku
bool buttonState = HIGH; // Aktualny stan przycisku
bool lastButtonState = HIGH; // Poprzedni stan przycisku
unsigned long lastDebounceTime = 0; // Czas ostatniego odczytu przycisku
const unsigned long debounceDelay = 50; // Opóźnienie dla eliminacji drgań styków

// Zmienne obsługi przycisku
bool buttonState1 = HIGH; // Aktualny stan przycisku
bool lastButtonState1 = HIGH; // Poprzedni stan przycisku
unsigned long lastDebounceTime1 = 0; // Czas ostatniego odczytu przycisku
const unsigned long debounceDelay1 = 50; // Opóźnienie dla eliminacji drgań styków


// OLED
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// RTC
RTC_DS1307 rtc; // Obiekt RTC

// Tryb pracy
enum Mode { AUTOMATIC, MANUAL };
Mode currentMode = AUTOMATIC; // Domyślny tryb

// Tryby źródeł wartości
enum ValueSource { BMS_INPUT, BMS_ENCODER };
ValueSource currentValueSource = BMS_INPUT; // Domyślny tryb


// Kalibracja ADC
const float VREFS[] = {0.0, 1.1, 2.2, 3.3}; // Napięcia referencyjne (V)
const uint32_t ADC_POINTS[] = {0, 1370, 2740, 4095}; // Wartości ADC dla kalibracji

// Eksponencjalne wygładzanie
float filteredVP = 0.0;
float filteredVN = 0.0;
const float ALPHA = 0.1; // Współczynnik wygładzania

float targetValvePosition = 8.0; // Celowa wartość otwarcia zaworu (80%)
float outputVoltage = 10.0;      // Początkowa wartość wyjściowa (maksymalna)
unsigned long lastAdjustmentTime = 0;

static unsigned long previousMillis = 0; // Czas ostatniego wykonania kodu opóźnienia
const unsigned long interval = 100;      // Interwał czasowy (100 ms)

// Korekta offsetu dla dolnych, średnich i górnych wartości
float applyOffsetCorrection(float voltage) {

  if (voltage > 0.0 && voltage <= 2.7) {
    return voltage + 0.1; // Korekta dla dolnego zakresu

  } else if (voltage > 2.7 && voltage <= 3.0) {
    return voltage - 0.05; // Korekta dla górnego zakresu
  }
  else if (voltage > 3.0 && voltage <= 3.05) {
    return voltage - 0.1; // Korekta dla górnego zakresu
  }
   else if (voltage > 3.05 && voltage <= 3.1) {
    return voltage - 0.2; // Korekta dla górnego zakresu
  }
   else if (voltage > 3.1 && voltage <= 3.14){
    return voltage - 0.3; // Korekta dla górnego zakresu
  }
  return voltage; // Bez korekcji poza zakresem
}

// Funkcja interpolacji kawałkowej
float interpolateVoltage(uint32_t adc_reading) {
  for (int i = 0; i < 3; i++) {
    if (adc_reading >= ADC_POINTS[i] && adc_reading <= ADC_POINTS[i + 1]) {
      // Oblicz współczynnik skalowania w bieżącym zakresie
      float slope = (VREFS[i + 1] - VREFS[i]) / (ADC_POINTS[i + 1] - ADC_POINTS[i]);
      return VREFS[i] + slope * (adc_reading - ADC_POINTS[i]);
    }
  }
  // Zabezpieczenie przed wartościami poza zakresem
  return adc_reading < ADC_POINTS[0] ? VREFS[0] : VREFS[3];
}

float applyExponentialSmoothing(float currentValue, float previousValue, float alpha) {
  return alpha * currentValue + (1 - alpha) * previousValue;
}

// Funkcja zmiany trybu
void changeMode() {
  currentMode = (currentMode == AUTOMATIC) ? MANUAL : AUTOMATIC;
}


//Funkcja obsługi enkodera BMS
void IRAM_ATTR handleEncoderBMS() {
  int currentStateCLK = digitalRead(CLK_BMS);
  if (currentStateCLK != lastStateCLK_BMS && currentStateCLK == HIGH) {
    if (digitalRead(DT_BMS) != currentStateCLK) {
      if (counter_bms < 100) counter_bms++;
    } else {
      if (counter_bms > 0) counter_bms--;
    }
    bms_value = counter_bms * 0.1;
  }
  lastStateCLK_BMS = currentStateCLK;
}

//Funkcja obsługi enkodera VOLT
void IRAM_ATTR handleEncoderVOLT() {
  int currentStateCLK = digitalRead(CLK_VOLT);
  if (currentStateCLK != lastStateCLK_VOLT && currentStateCLK == HIGH) {
    if (digitalRead(DT_VOLT) != currentStateCLK) {
      if (counter_volt < 100) counter_volt++;
    } else {
      if (counter_volt > 0) counter_volt--;
    }
    volt_value = counter_volt * 0.1;
  }
  lastStateCLK_VOLT = currentStateCLK;
}

//Funkcja obsługi enkodera TIME
void IRAM_ATTR handleEncoderTIME() {
  int currentStateCLK = digitalRead(CLK_TIME);
  if (currentStateCLK != lastStateCLK_TIME && currentStateCLK == HIGH) {
    if (digitalRead(DT_TIME) != currentStateCLK) {
      if (index_time < (sizeof(values_time) / sizeof(values_time[0]) - 1)) {
        index_time++;
      }
    } else {
      if (index_time > 0) {
        index_time--;
      }
    }
  }
  lastStateCLK_TIME = currentStateCLK;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  u8g2.begin();

  if (!rtc.begin()) {
    Serial.println("RTC nie znaleziono!");
    while (1);
  }

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Ustawienie czasu na podstawie kompilacji
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Start programu!");
  u8g2.sendBuffer();
  delay(2000);

  pinMode(CLK_BMS, INPUT_PULLUP);
  pinMode(DT_BMS, INPUT_PULLUP);
  pinMode(CLK_VOLT, INPUT_PULLUP);
  pinMode(DT_VOLT, INPUT_PULLUP);
  pinMode(CLK_TIME, INPUT_PULLUP);
  pinMode(DT_TIME, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ANALOG_OUT, OUTPUT);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP); // Konfiguracja przycisku enkodera
  pinMode(DIGITAL_OUT, OUTPUT);
  pinMode(DIGITAL_BMS, OUTPUT);
  pinMode(DIGITAL_EV, OUTPUT);


  lastStateCLK_BMS = digitalRead(CLK_BMS);
  lastStateCLK_VOLT = digitalRead(CLK_VOLT);
  lastStateCLK_TIME = digitalRead(CLK_TIME);

  attachInterrupt(digitalPinToInterrupt(CLK_BMS), handleEncoderBMS, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CLK_VOLT), handleEncoderVOLT, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CLK_TIME), handleEncoderTIME, CHANGE);
}

void loop() {
    static unsigned long lastAdjustmentTime = 0; // Ostatni czas regulacji
    static bool firstRun = true; // Flaga dla pierwszego uruchomienia
    static bool lastButtonState = HIGH; // Poprzedni stan przycisku enkodera
    bool currentButtonState = digitalRead(ENCODER_BUTTON_PIN); // Odczyt stanu przycisku enkodera

    // Obsługa przycisku enkodera do zmiany trybu
    if (currentButtonState == LOW && lastButtonState == HIGH) { // Wciśnięcie przycisku
        delay(debounceDelay); // Eliminacja drgań styków
        if (digitalRead(ENCODER_BUTTON_PIN) == LOW) { // Potwierdzenie stanu
            currentValueSource = static_cast<ValueSource>((currentValueSource + 1) % 2); // Przełączenie trybu
        }
    }
    lastButtonState = currentButtonState;

    // Obsługa przycisku z eliminacją drgań styków
    bool reading = digitalRead(BUTTON_PIN);
    if (reading != lastButtonState1) {
        lastDebounceTime1 = millis();
    }
    if ((millis() - lastDebounceTime1) > debounceDelay1) {
        if (reading != buttonState1) {
            buttonState1 = reading;
            if (buttonState1 == LOW) { // Aktywacja na zboczu opadającym
                changeMode();
            }
        }
    }
    lastButtonState1 = reading;

    // Początkowy stan (100% mocy pompy)
    if (firstRun) {
        outputVoltage = 10.0; // Ustawienie na maksymalną moc
        firstRun = false;
    }

    // Odczyt wartości VP i VN
    uint32_t adcVP = analogRead(VP_PIN);
    uint32_t adcVN = analogRead(VN_PIN);

    // Interpolacja i korekta napięć
    float vpVoltage = applyOffsetCorrection(interpolateVoltage(adcVP));
    float vnVoltage = applyOffsetCorrection(interpolateVoltage(adcVN));

    // Wygładzanie sygnałów
    filteredVP = applyExponentialSmoothing(vpVoltage, filteredVP, ALPHA);
    filteredVN = applyExponentialSmoothing(vnVoltage, filteredVN, ALPHA);

    // Skalowanie napięć do 0-10 V
    float scaledVP = filteredVP * (10.0 / 3.3);
    float scaledVN = filteredVN * (10.0 / 3.3);

    // Ustawienie czasu regulacji na podstawie enkodera
    int adjustmentInterval = values_time[index_time];

    // Logika regulacji w trybie automatycznym
    if (currentMode == AUTOMATIC) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastAdjustmentTime >= adjustmentInterval * 1000) { // Sprawdzenie interwału czasu
                // Priorytet sygnału BMS
                if (scaledVN >= 0.1 && currentValueSource == BMS_INPUT) {
                  outputVoltage = scaledVN; // Korzystaj z wartości BMS_INPUT, jeśli VN > 0
              } else if (bms_value >= 0.1 && currentValueSource == BMS_ENCODER) {
                  outputVoltage = bms_value; // W przeciwnym razie użyj enkodera  
                }
             else {
      
                 if (scaledVP < outputVoltage) { // Jeśli położenie zaworu mniejsze niż zadane
                    outputVoltage = max(outputVoltage - 0.1, 0.0); // Zmniejsz napięcie
                } else if (scaledVP > outputVoltage) { // Jeśli położenie zaworu większe niż zadane
                    outputVoltage = min(outputVoltage + 0.1, 10.0); // Zwiększ napięcie
                }
                else if(scaledVP == outputVoltage){
                  outputVoltage = scaledVP;
                }
            }
            lastAdjustmentTime = currentMillis; // Aktualizacja czasu regulacji
        
        }
        // Aktualizacja PWM w trybie automatycznym
        int pwmValue1 = (outputVoltage / 10.0) * 255;
        analogWrite(ANALOG_OUT, pwmValue1);
    }

    if (currentMode == MANUAL) {
        // Wyjście PWM w trybie ręcznym
        int pwmValue = (volt_value / 10.0) * 255;
        analogWrite(ANALOG_OUT, pwmValue);
    }

    // Obsługa diod
    digitalWrite(DIGITAL_OUT, (outputVoltage > 0.1) ? HIGH : LOW); // Sygnalizacja wyjścia
    digitalWrite(DIGITAL_BMS, (scaledVN > 0.1) ? HIGH : LOW);      // Sygnalizacja BMS
    digitalWrite(DIGITAL_EV, (scaledVP > 0.1) ? HIGH : LOW);       // Sygnalizacja EV

    // Wyświetlanie na OLED
    DateTime now = rtc.now();
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.printf("Czas: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
    u8g2.setCursor(0, 20);
    u8g2.printf("Tryb: %s", currentMode == AUTOMATIC ? "AUTO" : "MANUAL");
    u8g2.setCursor(0, 30);
    u8g2.printf("EV: %.1fV BMS: %.1fV", scaledVP, scaledVN);
// Wyświetlanie wartości wyjściowej w zależności od trybu
if (currentMode == AUTOMATIC) {
    u8g2.setCursor(0, 40);
    u8g2.printf("Output: %.1fV", outputVoltage); // Wyjście w trybie automatycznym
} else if (currentMode == MANUAL) {
    u8g2.setCursor(0, 40);
    u8g2.printf("Output: %.1fV", volt_value); // Wyjście w trybie manualnym
}
    u8g2.setCursor(0, 50);
    u8g2.printf("BMS_IN: %s", currentValueSource == BMS_INPUT ? "BMS_INPUT" : "BMS_ENC");
    u8g2.setCursor(0, 60);
    u8g2.printf("N: T=%ds B=%.1fV V=%.1fV", adjustmentInterval, bms_value, volt_value);
    u8g2.sendBuffer();


// Obsługa opóźnienia 100 ms bez blokowania
unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

}

}


