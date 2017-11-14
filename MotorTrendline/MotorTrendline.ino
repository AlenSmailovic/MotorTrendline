#define MenuButton     2
#define IncreaseButton 3 
#define DecreaseButton 4

#define EnablePin   5
#define ControlPin1 6
#define ControlPin2 7

#define MIN_VOLTAGE 28    // 3.3V - 0%
#define MAX_VOLTAGE 255   // 12V  - 100%
#define MIN_MAX_DISTANCE (MAX_VOLTAGE - MIN_VOLTAGE)

#define MIN_SECONDS  3
#define MAX_SECONDS  15
#define FULL_SECONDS 20

int IncreaseSeconds = 0;
int DecreaseSeconds = 0;

// Secu Adrian
void ReadMemory() {
  // your code goes here
}

// Secu Adrian
void WriteMemory() {
  // your code goes here
}

// Bularca Luciana
bool Menu() {
  bool bFlag;
  
  // your code goes here
  
  return bFlag;
}

// Alen Smailovic
void FanController() {
  // your code goes here
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
