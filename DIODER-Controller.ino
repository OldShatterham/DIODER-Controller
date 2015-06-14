/**
 *  Code for the Arduino experiment boards to control a led strip like the 'DIODER' by IKEA; extra hardware is
 *   required (see http://makezine.com/projects/make-36-boards/android-arduino-led-strip-lights/)!
 *
 *  Copyright (kinda): © 2015 by BSM.
 */

///Set to true to get more information over serial
boolean debugMode = false;

String command = "";

static int RED_PIN = 3;    //Standard: 3
static int GREEN_PIN = 5;  //Standard: 5
static int BLUE_PIN = 6;   //Standard: 6
static int FADING_PIN_1 = 0;  //Standard: 0
static int FADING_PIN_2 = 1;  //Standard: 1

///Value at which full brightness is reached; Possible values reach from 0 to 255
static int full_brightness = 200;

///Describe current values of the ouputs; values reach from 0 to 255:
int red = 0;
int green = 0;
int blue = 0;

float brightness = 1.0f;

/**
 * Statuses (srsly, that's the plural of 'status', I looked it up!) of inputs, to be modified by listeners. If anything
 * changed, somethingChanged has to be set to true for this change to be processed!
 */
int fading_value_1 = 255;
int fading_value_2 = 255;
boolean somethingChanged = false;
long startTimestamp;
long currentTimestamp;

/**
 * Information for the execution of an self-changing effect
 * effect and effect_max should only be updated together.
 *     sEffect:       String of the effect function
 *     int sEffect_max:   max timestamp of the effect
 *     double sEffect_speed: speed in which the effect will be updated. 0.25f means one update every 40 ms; 2.0f means 2 updates
 *                           every 10 ms
 */
String sEf_func;
int sEf_max_ts;
double sEf_rel_speed;

int sEf_timestamp = 0;



void setup() {
  //Set up serial:
  Serial.begin(115200);  //Baud rate
  Serial.println("Program started");
  
  //Set up the system:
  somethingChanged = true;
  setColor(128, 128, 128);
  startTimestamp = millis();
  
  //Set effect to be executed:
  setSEffect("effect_gradient", 764, 0.25);
}

/**
 * Main function.
 * One execution every 10 ms.
 * Effects and listeners will be handled here
 */
void loop() {
  currentTimestamp = millis();
  
  ///Self-changing effect:
  //Reset timestamp if bigger than max timestamp:
  if(sEf_timestamp > sEf_max_ts) {
    sEf_timestamp = 0;
  }
  
  if((float)sEf_rel_speed < 1.0f) {
    int msPerExec = (int)(1 / sEf_rel_speed);
    if((currentTimestamp - startTimestamp) % msPerExec == 0) {
      if(sEf_func == "effect_gradient")
        effect_gradient(sEf_timestamp);
      else
        effect_sinus(sEf_timestamp);
        
      sEf_timestamp++;
    }
  } else {
    for(int i = 0; i < sEf_rel_speed; i++) {
      if(sEf_func == "effect_gradient")
        effect_gradient(sEf_timestamp);
      else
        effect_sinus(sEf_timestamp);
      
      sEf_timestamp++;
      if(sEf_timestamp > sEf_max_ts) {
        sEf_timestamp = 0;
      }
    }
  }
  
  //Dynamic effect:
  if(somethingChanged) {
    //Dynamic effect updates here
    //  Change effect speed dependent on fading_value_1:
    //sEf_rel_speed = fading_value_1 * 0.0392156863;
    //  Change brightness flag dependent on fading_value_2:
    //brightness = fading_value_2 / 255.0f;
    
    somethingChanged = false;
  }
  
  //Listeners:
  //  fading_value_1:
  int read_value = analogRead(FADING_PIN_1);
  if(abs((read_value / 4) - fading_value_1) > 2) {  //If change was big enough
    analogWrite(13, read_value);
    fading_value_1 = read_value / 4;
    if(debugMode) {
      Serial.print("[LISTENER] Fading value 1 changed to: ");
      Serial.println(fading_value_1);
    }
    somethingChanged = true;
  }
  //  fading_value_2:
  read_value = analogRead(FADING_PIN_2);
  if(abs((read_value / 4) - fading_value_2) > 2) {  //If change was big enough
    analogWrite(13, read_value);
    fading_value_2 = read_value / 4;
    if(debugMode) {
      Serial.print("[LISTENER] Fading value 2 changed to: ");
      Serial.println(fading_value_2);
    }
    somethingChanged = true;
  }
  
  //Meta stuff:
  checkSerial();
  delay(10);
}

/**
 * This function parses any input via serial. Own behaviors have to be coded in this function.
 */
void checkSerial() {
  static int MAXLENGTH = 50;
  char recData[MAXLENGTH];
  
  //Clear recData[]:
  for(int i = 0; i < MAXLENGTH; i++)
    recData[i] = 0;
  
  //Get chars of recieved data:
  int index = 0;  
  while(Serial.available() > 0 && index < MAXLENGTH) {
    char inChar = Serial.read();
    recData[index] = inChar;
    index++;
        
    if(debugMode) {
      Serial.print("    Processed char '");
      Serial.print(inChar);
      Serial.print("', ASCII value: ");
      Serial.println((int)inChar);
    }
  }
  
  //Store chars in one String
  for(int i = 0; i < index; i++) {
    if(recData[i] != NULL) {
      char currentChar = recData[i];
      command += currentChar;
      
      if(debugMode) {
        Serial.print("Current value of 'command': ");
        Serial.println(command);
      }
    }
  }
  
  //If command ends with a new line character (ASCII value 13)
  int lastChar = (int)(command.charAt(command.length() - 1));
  if(lastChar == 10) {
    //Remove last character in command as it is hindering:
    command.remove(command.length() - 1);
    
    Serial.print("[INPUT] ");
    Serial.println(command);
    
    //Cycle thorugh all commands:
    String commands[] = {"help","debugMode","setColor([INT],[INT],[INT])","setSEffect([STRING],[INT],[DOUBLE])"};
    //  help
    if(command == "help") {
      for(int i = 0; i < (sizeof(commands) / sizeof(String)); i++) {
        Serial.print("    ");
        Serial.println(commands[i]);
      }
    }
    //  debugMode
    if(command == "debugMode") {
      if(debugMode) {
        debugMode = false;
        Serial.println("Disabled debug mode!");
      } else {
        debugMode = true;
        Serial.println("Enabled debug mode!");
      }
    }
    //  setColor(int,int,int)
    if(command.startsWith("setColor(") && command.endsWith(")")) {
      String rString = command.substring(9, command.indexOf(","));
      String gString = command.substring(command.indexOf(",")+1, command.indexOf(",", command.indexOf(",")+1));
      String bString = command.substring(command.indexOf(",", command.indexOf(",")+1)+1, command.indexOf(")"));
      
      //Cast to int:
      int rValue = (int)rString.toInt();
      int gValue = (int)gString.toInt();
      int bValue = (int)bString.toInt();
      
      Serial.print("Setting color to '");
      Serial.print(rValue);
      Serial.print("', '");
      Serial.print(gValue);
      Serial.print("', '");
      Serial.print(bValue);
      Serial.println("'!");
      
      setColor(rValue, gValue, bValue);
    }
    //  setSEffect(String,int,double)
    if(command.startsWith("setSEffect(") && command.endsWith(")")) {
      String func_String = command.substring(12, command.indexOf(',')-1);
      String max_ts_String = command.substring(command.indexOf(",")+1, command.indexOf(",", command.indexOf(",")+1));
      String rel_speed_String = command.substring(command.indexOf(",", command.indexOf(",")+1)+1, command.indexOf(")"));
      
      //Cast to respective types:
      String newFunc = func_String;
      int newMax_ts = (int)max_ts_String.toInt();
      
      char buffer[rel_speed_String.length()];
      rel_speed_String.toCharArray(buffer,rel_speed_String.length()+1);
      double newRel_speed = atof(buffer);
      
      Serial.print("Setting self-changing effect to '");
      Serial.print(newFunc);
      Serial.print("', with max value of '");
      Serial.print(newMax_ts);
      Serial.print("' and speed of ");
      Serial.print(newRel_speed);
      Serial.println("!");
      
      setSEffect(newFunc, newMax_ts, newRel_speed);
    }
    
    //Reset command:
    command = "";
  }
}

/**
 * Used to show that an error has ocurred. Calling this function will print an error message over serial and blink the onboard
 *  led 5 times.
 *     String message: Additional information that will appear in the error message if this parameter is not ""
 */
void signalError(String message) {
  //Print error message to Serial:
  Serial.print("[ERROR] An error occured");
  if(message != "") {
    Serial.print(" (");
    Serial.print(message);
    Serial.print(")");
  }
  Serial.println("!");
  
  //Blink LED 13 (onboard) 5 times
  for(int i = 0; i < 5; i++) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
  }
}

/**
 * Sets the current color to be output at the set pins in rgb form.
 *     int r: 'Strength' of the red   channel; value must be between 0 and 255, anything else will produce an error!
 *     int g: 'Strength' of the green channel; value must be between 0 and 255, anything else will produce an error!
 *     int b: 'Strength' of the blue  channel; value must be between 0 and 255, anything else will produce an error!
 */
void setColor(int r, int g, int b) {
  if((r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255)) {  //Check if new values are valid
    //Update variables:
    red = (int)(r * brightness);
    green = (int)(g * brightness);
    blue = (int)(b * brightness);
    
    //Set pins:
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
    
    if(debugMode) {
      Serial.print("Set color to ");
      Serial.print(red);
      Serial.print(", ");
      Serial.print(green);
      Serial.print(", ");
      Serial.print(blue);
      Serial.println("!");
    }
  } else
    signalError("setColor parameters are not between 0 and 255");
}

/**
 * This function sets the current self-changing effect.
 *     String func: name of the function of the effect
 *     int max_ts: max amount of timestamps after which the given interval parameter should revert to 0; See effect
 *                       documentation for this value
 *     double rel_speed: speed in which the effect will be updated. 0.25f means one update every 40 ms; 2.0f means 2 updates
 *                          every 10 ms
 */
void setSEffect(String func, int max_ts, double rel_speed) {
  Serial.println(" Trying to change self-changing effect");
  sEf_func = func;
  sEf_max_ts = max_ts;
  sEf_rel_speed = rel_speed;
  
  sEf_timestamp = 0;
  
  Serial.println("Changed self-changing effect");
}



/**=================================
 * =============EFFECTS=============
 * =================================
 * Self-changing effects are functions that do something dependent on a timestamp parameter that will get larger with every
 *  execution.
 * Each self-changing effect should have a maximum timestamp. After the execution with this parameter the timestamp values will
 *  start counting up from 0 again.
 * Other parameters are optional
 * All effects should be of the 'void' type.
 *     int interval: Current timestamp
 * 
 * Each function starting with 'effect_' should represent an effect.
 */
 
 /**
  * This effect will cycle through all colours by in- and decreasing each channel time-shifted.
  * There are 3 cycles: Red to Green, Green to Blue, and Blue to Red.
  * Each cycle lasts 255 intervals.
  * Each cycle starts with the first color being fully lit (255) and the other one being fully dark (0) and ends with the first
  *  one being almost fully dark (1) and the other one almost fully lit (254).
  * int cycleInterval must be bigger (or equal to) 0 and smaller (or equal to) 254
  * 
  * MAX TIMESTAMP: 764
  */
 void effect_gradient(int interval) {
   if(interval < 255)  //Cycle 1: Red to Green
     setColor(255 - interval, interval, 0);
   else {
     if(interval < 510) {  //Cycle 2: Green to Blue
       int cycleInterval = interval - 255;
       setColor(0, 255 - cycleInterval, cycleInterval);
     } else {
       if(interval < 765) {  //Cycle 2: Blue to Red
         int cycleInterval = interval - 510;
         setColor(cycleInterval, 0, 255 - cycleInterval);
       }
     }
   }
 } 
 
 /**
  * This effect will turn the brightness of all channels up and down dynamically like a sinus curve
  * 
  * MAX TIMESTAMP: 1000
  */
 void effect_sinus(int interval) {
   int sinus_value = (int)(full_brightness * (0.5 + sin((2.0 * 3.14159 * (float)interval) / 1000.0) / 2.0));
   
   if(debugMode) {
     Serial.print("Calculated value for breathing effect: ");
     Serial.println(sinus_value);
   }
   setColor(sinus_value, sinus_value, sinus_value);
 }
