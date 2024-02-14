/*
 * Edited by Andrea Celotto for SQE_RTSwithcBox project
 * Date: 14/02/2024
 * Version: 4.0
 *
 * Edited by Bernardo Galvano for INRiM_CSwitch project
 * Date: 13/12/2023
 * Version: 1.0
 */

// Each switch is associated to 2 control pins, one for each switch state (either 1 or 2), and two read-out pins, each one connected to a led
// To add a new switch called "X", change N_SWITCHES and add the line Switch("X", X_E1, X_E1, X_LED1, X_LED2, X_address) inside the definition of Switches. 
// Refer to the Switch class definition for the meanings of the variables
// Don't forget to flash the code to the board! (COM6)

#include <EEPROM.h>
#define N_SWITCHES 2
#define power_sens A0

class RTSwitch {
  public:
    String name; // The name of the switch, usually a letter
    int E1; // Pin activating state 1
    int E2; // Pin activating state 2
    int LED1; // Pin activating the LED corresponding to state 1
    int LED2; // Pin activating the LED corrsponding to state 2
    int address; // Address of the byte in the board internal memory where the switch state is stored
    RTSwitch(){ // Default constructor
      name = "";
      E1 = 0;
      E2 = 0;
      LED1 = 0;
      LED2 = 0;
      address = 0;
    }
    RTSwitch(String _name, int _E1, int _E2, int _LED1, int _LED2, int _address) { // Constructor
      name = _name;
      E1 = _E1;
      E2 = _E2;
      LED1 = _LED1;
      LED2 = _LED2;
      address = _address;
    }
};


RTSwitch RTSwitches[N_SWITCHES] = { // An array containing all the switches
  RTSwitch("A", 2, 3, 4, 5, 0), 
  RTSwitch("B", 8, 9, 10, 11, 1),
  };
String command = ""; // Variable to store che commands received through the serial channel


int getIdx(RTSwitch RTSwitches[N_SWITCHES], String name){ // A fucntion that returns the position of the switch with a given name inside an array of switches
  for (int i = 0; i < N_SWITCHES; i++) {
    if (RTSwitches[i].name == name){
      return i;
    }
  }
  return -1; // If this line is executed, it means that no switch has the wanted name. -1 is returned
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 bps
  Serial.setTimeout(1);  // Delay 

  pinMode(A0, INPUT);

  // Initializing output pins
  for (int i = 0; i < N_SWITCHES; i++) {
    // Setting the pins associated with each switch as output pins
    pinMode(RTSwitches[i].E1, OUTPUT);
    pinMode(RTSwitches[i].E2, OUTPUT);
    pinMode(RTSwitches[i].LED1, OUTPUT);
    pinMode(RTSwitches[i].LED2, OUTPUT);
    // Turning off any voltage on the control pins
    digitalWrite(RTSwitches[i].E1, LOW);
    digitalWrite(RTSwitches[i].E2, LOW);

    digitalWrite(RTSwitches[i].LED1, (EEPROM.read(RTSwitches[i].address) == 1) ? HIGH : LOW);
    digitalWrite(RTSwitches[i].LED2, (EEPROM.read(RTSwitches[i].address) == 2) ? HIGH : LOW);
  }

  return;
}

void loop() {
  // Wait for any character available on the serial channel
  while (!Serial.available()) {
    delay(10);
  }
  // Wait for the voltage to be supplied. It is not, turn off the leds
  while (digitalRead(power_sens) != HIGH) {
    delay(10);
    for (int i = 0; i < N_SWITCHES; i++) {
      digitalWrite(RTSwitches[i].LED1, LOW);
      digitalWrite(RTSwitches[i].LED2, LOW);
    }
  }
  // Read the available character and add it to the current command
  char receivedChar = Serial.read();
  command += receivedChar;
  // If the last received character is a newline ("\n"), process the command
  if (receivedChar == '\n') {
    // Remove any excess spaces
    command.trim();
    // Process the command
    if (command=="*IDN?"){
      Serial.println("SQE_RTSwitchBox");
    }
    else if (command == "*CLS"){
      Serial.flush();
    }
    else if (command == "*RST"){
      for (int i = 0; i < N_SWITCHES; i++) {
        EEPROM.update(RTSwitches[i].address, 0); // Flush the internal memory of the board
      }
      setup(); // This will turn off any pin
    }
    else if (command.endsWith("?")) {
      processQuery(command);
    }
    else {
      processWrite(command);
    }
    command = "";
  }
}

void processQuery(String cmd){
  String switchName = String(cmd[0]);
  int idx = getIdx(RTSwitches, switchName); // Gets the position of the switch with a particular name inside the array
  Serial.println(EEPROM.read(RTSwitches[idx].address)); // The switch state saved in the board memory is returned of the serial channel
  return;
}

void processWrite(String cmd) {
  String switchName = String(cmd[0]);
  int idx = getIdx(RTSwitches, switchName); // Gets the position of the switch with a particular name inside the array
  if (cmd.endsWith("1")) {
    // Sending a 15 ms pulse on the right pin
    digitalWrite(RTSwitches[idx].E1, HIGH);
    delay(15);
    digitalWrite(RTSwitches[idx].E1, LOW);
    // Saving the switch state in the board memory and turning on the right led
    EEPROM.update(RTSwitches[idx].address, 1);
    digitalWrite(RTSwitches[idx].LED1, HIGH);
    digitalWrite(RTSwitches[idx].LED2, LOW);

  }
  else if (cmd.endsWith("2")) {
    // Sending a 15 ms pulse on the right pin
    digitalWrite(RTSwitches[idx].E2, HIGH);
    delay(15);
    digitalWrite(RTSwitches[idx].E2, LOW);
    // Saving the switch state in the board memory and turning on the right led
    EEPROM.update(RTSwitches[idx].address, 2);
    digitalWrite(RTSwitches[idx].LED2, HIGH);
    digitalWrite(RTSwitches[idx].LED1, LOW);
  }
  return;
}
