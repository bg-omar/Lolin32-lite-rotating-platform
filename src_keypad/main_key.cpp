
#include "Keypad.hpp"
#include <stdio.h>
#include <Arduino.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {  //key code
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {35, 32, 33, 23 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26,27, 14, 12 }; //connect to the column pinouts of the keypad
//create Keypad object
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char key = 0;

__attribute__((unused)) void setup(){
    printf("Program is starting ... \n");

	keypad.setDebounceTime(50);

}

void loop(){
        key = keypad.getKey();  //get the state of keys
        if (key){       //if a key is pressed, print out its key code
            printf("You Pressed key :  %c \n",key);
        }

}
