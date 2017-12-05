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

int IncreaseSeconds = 0;
int DecreaseSeconds = 0;

// Secu Adrian
void ReadMemory() {
  while(EECR & 0X02) {
    // NOP
  }
  EEAR = 0x00;
  EECR = EECR | 0x01;
  IncreaseSeconds = EEDR;

  EEAR = 0x01;
  DecreaseSeconds = EEDR;
}

// Secu Adrian
void WriteMemory() {
  unsigned int address = 0x00; 
  bool flag = false;
  LOOP:
    while(EECR & 0x02){
      //nop
    }
    while(SPMCSR & 0x01){
      //nop
    }
        
    EEAR = address;
    
    if(address== 0x00){
      EEDR = IncreaseSeconds;
    } else{
      EEDR = DecreaseSeconds;
    } 
 
    EECR = EECR | 0x04;
    EECR = EECR | 0x02;
    
    if(flag == false){
      address = 0x01;
      flag = true;
      goto LOOP;
    }
}

// Bularca Luciana
bool Menu() {
  bool bFlag = false;

  int iStep = 0;
  while(iStep < 10) {
    int iMBPressed = digitalRead(MenuButton);
    if (iMBPressed == 1) {
      // increase zone
      while(!digitalRead(MenuButton)) {
        if(digitalRead(IncreaseButton)) {
          IncreaseSeconds++;
          if (IncreaseSeconds > MAX_SECONDS) IncreaseSeconds = MAX_SECONDS;
          bFlag = true;
        }
        if(digitalRead(DecreaseButton)) {
          IncreaseSeconds--;
          if (IncreaseSeconds < MIN_SECONDS) IncreaseSeconds = MIN_SECONDS;
          bFlag = true;
        }
        delay(500);
      }
      // decrease zone
      while(!digitalRead(MenuButton)) {
        if(digitalRead(IncreaseButton)) {
          DecreaseSeconds++;
          if (DecreaseSeconds > MAX_SECONDS) DecreaseSeconds = MAX_SECONDS;
          bFlag = true;
        }
        if(digitalRead(DecreaseButton)) {
          DecreaseSeconds--;
          if (DecreaseSeconds < MIN_SECONDS) DecreaseSeconds = MIN_SECONDS;
          bFlag = true;
        }
        delay(500);
      }
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
    analogWrite(EnablePin, iVoltage);
    delay(1000);
    // compute voltage for next step
    iVoltage = iVoltage + (int)(MIN_MAX_DISTANCE / IncreaseSeconds);
    // make sure don't pass maximum limit
    if (iVoltage > MAX_VOLTAGE) iVoltage = MAX_VOLTAGE;
  }

  // set maximum voltage on fan for [FULL_SECONDS] sec
  iVoltage = MAX_VOLTAGE;
  analogWrite(EnablePin, iVoltage);
  delay(FULL_SECONDS * 1000);

  // decrease voltage at each step
  for(int iStep = 0; iStep < DecreaseSeconds; ++iStep) {
    // set scalar on enable pin
    analogWrite(EnablePin, iVoltage);
    delay(1000);
    // compute voltage for next step
    iVoltage = iVoltage - (int)(MIN_MAX_DISTANCE / DecreaseSeconds);
    // make sure don't pass minimum limit
    if (iVoltage < MIN_VOLTAGE) iVoltage = MIN_VOLTAGE;
  }

  // stop the fan
  iVoltage = NULL_VOLTAGE;
  analogWrite(EnablePin, iVoltage);
}

// setup code - to run once
void setup() {
  
  // declare buttons MENU/+/- as INPUT
  pinMode(MenuButton, INPUT);
  pinMode(IncreaseButton, INPUT);
  pinMode(DecreaseButton, INPUT);

  // declare DC motor connectors as OUTPUT
  pinMode(EnablePin, OUTPUT);
  pinMode(ControlPin1, OUTPUT);
  pinMode(ControlPin2, OUTPUT);

  // read seconds from EEPROM and save into IncreaseSeconds/DecreaseSeconds
  ReadMemory();

  // menu section; return true if user change IncreaseSeconds/DecreaseSeconds
  bool bVariablesChange = Menu();

  if ( bVariablesChange ) {
    // update seconds into EEPROM according with IncreaseSeconds/DecreaseSeconds
    WriteMemory();
  }
}

// main code - to run repeatedly
void loop() {
  
  // delay of 5 seconds between each loop
  delay(5000);

  // controll the fan speed according with IncreaseSeconds/DecreaseSeconds
  FanController();
}
