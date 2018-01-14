#define MenuButton     2
#define IncreaseButton 3 
#define DecreaseButton 4

#define EnablePin   5
#define ControlPin1 6
#define ControlPin2 7

#define NULL_VOLTAGE 0     // 0V   - 0%
#define MIN_VOLTAGE  28    // 3.3V - 0%
#define MAX_VOLTAGE  255   // 12V  - 100%
#define MIN_MAX_DISTANCE (MAX_VOLTAGE - MIN_VOLTAGE)

#define MIN_SECONDS  3
#define MAX_SECONDS  15
#define FULL_SECONDS 20

#define MENU_INPUT 10     // 10 -> 5sec
#define FAN_LOOP_DELAY 5  // 5  -> 5sec

#define DEBUG false

int IncreaseSeconds = 0;
int DecreaseSeconds = 0;

void analogWriteReg(int iValue) {
  if (EnablePin == 5) OCR0B = iValue;
}

// Secu Adrian
void ReadMemory() {
  // testam prin pooling bitul EEPE(daca o operatie de scriere are loc)
  while(EECR & 0X02) {}
  
  // setam adresa EEPROM la 0x00
  EEAR = 0x00;
  // setam EERE (read enable)
  EECR = EECR | 0x01;
  // variabila IncreaseSeconds primeste valoarea stocata la adresa 0x00
  IncreaseSeconds = EEDR;

  // setam adresa EEPROM la 0x01;
  EEAR = 0x01;
  // setam EERE (read enable)
  EECR = EECR | 0x01;
  //variabila DecreaseSeconds primeste valoarea stocata la adresa 0x01
  DecreaseSeconds = EEDR;
  // dezactivam read enable
  EECR = EECR & 0xFE;
}

// Secu Adrian
void WriteMemory() {
  // variabila primeste 0x00
  unsigned int iAddress = 0x00;  
  LOOP:
    // pas 1) testam prin pooling bitul EEPE(daca o operatie de scriere are loc)
    while(EECR & 0x02) {}
    // pas 2) testam daca bootloaderul se scrie in memoria flash 
    while(SPMCSR & 0x01) {}

    // pas 3) setam adresa la 0x00
    EEAR = iAddress;

    // daca adresa este 0x00
    if(iAddress == 0x00){
      // pas 4) datele ce vor fi inscrise in memorie sunt cele stocate in variabila IncreaseSeconds
      EEDR = IncreaseSeconds;
      // daca adresa este 0x01
    } else {
      // pas 4) datele ce vor fi inscrise in memorie sunt cele stocate in variabila DecreaseSeconds
      EEDR = DecreaseSeconds;
    } 
    // nu verificam intreruperile deoarece nu folosim in program

    // pas 5) setam bitul EEMPE, bitii 7:6 si EEPMx sunt 0 la reset asa ca nu e necesar sa ii stergem noi
    EECR = EECR | 0x04;
    // pas 6) setam bitul EEPE, start operatie de scriere
    EECR = EECR | 0x02;
    
    // daca adresa este 0x00
    if(iAddress == 0x00) {
      // incarcam in variabila iAddress valoarea 0x01 
      iAddress = 0x01;
      // repetam operatiile pentru noua adresa
      goto LOOP;
    }
}

// Bularca Luciana
bool Menu() {
  bool bFlag = false;

  int iStep = 0;
  while(iStep < MENU_INPUT) {
    int iMBPressed = digitalRead(MenuButton);
    if (iMBPressed == 0) {
      delay(1000);
      if (DEBUG) Serial.println("Menu button pressed.");
      // increase zone
      while(digitalRead(MenuButton)) {
        if(!digitalRead(IncreaseButton)) {
          delay(500);
          if (DEBUG) Serial.println("Increase button pressed for UP slope.");
          IncreaseSeconds++;
          if (IncreaseSeconds > MAX_SECONDS) IncreaseSeconds = MAX_SECONDS;
          bFlag = true;
        }
        if(!digitalRead(DecreaseButton)) {
          delay(500);
          if (DEBUG) Serial.println("Decrease button pressed for UP slope.");
          IncreaseSeconds--;
          if (IncreaseSeconds < MIN_SECONDS) IncreaseSeconds = MIN_SECONDS;
          bFlag = true;
        }
        delay(500);
      }
      delay(1000);
      // decrease zone
      while(digitalRead(MenuButton)) {
        if(!digitalRead(IncreaseButton)) {
          delay(500);
          if (DEBUG) Serial.println("Increase button pressed for DOWN slope.");
          DecreaseSeconds++;
          if (DecreaseSeconds > MAX_SECONDS) DecreaseSeconds = MAX_SECONDS;
          bFlag = true;
        }
        if(!digitalRead(DecreaseButton)) {
          delay(500);
          if (DEBUG) Serial.println("Decrease button pressed for DOWN slope.");
          DecreaseSeconds--;
          if (DecreaseSeconds < MIN_SECONDS) DecreaseSeconds = MIN_SECONDS;
          bFlag = true;
        }
        delay(500);
      }
      delay(1000);
      return bFlag;
    }
    delay(500);
    iStep++;
  }
  return bFlag;
}

// Alen Smailovic
void FanController() {
  // rotate fan in one direction
  digitalWrite(ControlPin1, HIGH);
  digitalWrite(ControlPin2, LOW);

  // set minimum voltage on fan
  int iVoltage = MIN_VOLTAGE;
  // increase voltage at each step
  for(int iStep = 0; iStep < IncreaseSeconds; ++iStep) {
    // set scalar on enable pin
    analogWriteReg(iVoltage);
    delay(1000);
    // compute voltage for next step
    iVoltage = iVoltage + (int)(MIN_MAX_DISTANCE / IncreaseSeconds);
    // make sure don't pass maximum limit
    if (iVoltage > MAX_VOLTAGE) iVoltage = MAX_VOLTAGE;
  }

  // set maximum voltage on fan for [FULL_SECONDS] sec
  iVoltage = MAX_VOLTAGE;
  analogWriteReg(iVoltage);
  delay(FULL_SECONDS * 1000);

  // decrease voltage at each step
  for(int iStep = 0; iStep < DecreaseSeconds; ++iStep) {
    // set scalar on enable pin
    analogWriteReg(iVoltage);
    delay(1000);
    // compute voltage for next step
    iVoltage = iVoltage - (int)(MIN_MAX_DISTANCE / DecreaseSeconds);
    // make sure don't pass minimum limit
    if (iVoltage < MIN_VOLTAGE) iVoltage = MIN_VOLTAGE;
  }

  // stop the fan
  iVoltage = NULL_VOLTAGE;
  analogWriteReg(iVoltage);
}

// setup code - to run once
void setup() {
  Serial.begin(9600);
  
  // declare buttons MENU/+/- as INPUT
  pinMode(MenuButton, INPUT);
  pinMode(IncreaseButton, INPUT);
  pinMode(DecreaseButton, INPUT);

  // declare DC motor connectors as OUTPUT
  DDRD = B00100000;
  TCCR0A = B00100001;
  TCCR0B = B00000011;
  
  pinMode(ControlPin1, OUTPUT);
  pinMode(ControlPin2, OUTPUT);

  if(DEBUG) Serial.println("Read EEPROM...");
  
  // read seconds from EEPROM and save into IncreaseSeconds/DecreaseSeconds
  ReadMemory();

  if(DEBUG) Serial.print("Increase = ");
  if(DEBUG) Serial.println(IncreaseSeconds);
  if(DEBUG) Serial.print("Decrease = ");
  if(DEBUG) Serial.println(DecreaseSeconds);
  
  // menu section; return true if user change IncreaseSeconds/DecreaseSeconds
  bool bVariablesChange = Menu();

  if(DEBUG) Serial.println("Exit from Menu..");
  if(DEBUG) Serial.print("Increase = ");
  if(DEBUG) Serial.println(IncreaseSeconds);
  if(DEBUG) Serial.print("Decrease = ");
  if(DEBUG) Serial.println(DecreaseSeconds);
  
  if ( bVariablesChange ) {
    // update seconds into EEPROM according with IncreaseSeconds/DecreaseSeconds
    if(DEBUG) Serial.println("Update EEPROM..");
    WriteMemory();
  }
}

// main code - to run repeatedly
void loop() {
  if(DEBUG) Serial.println("Fan started..");
  
  // delay of 5 seconds between each loop
  delay(FAN_LOOP_DELAY * 1000);

  // controll the fan speed according with IncreaseSeconds/DecreaseSeconds
  FanController();
}
