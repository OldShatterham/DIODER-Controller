# DIODER-Controller
Code for controlling the IKEA DIODER LED strip with an Arduino and some extra hardware
(see http://makezine.com/projects/make-36-boards/android-arduino-led-strip-lights/ on how to make that)  
  
This program is only meant for learning purposes, but you can use it anyway you want. I take no responsibility for any damage you do to your LED strip, Arduino board and/or other equipment.
This software is provided "as is" with no without any express or implied warranty of any kind.  
  
Use the *_PIN fields to set the output/input pins.
---
###Serial connection:
Baud rate is 115200, 8 data bits, 1 stop bit, no parity, no flow control; commands have to end with a carriage return character (ASCII code 13) to be processed.  
Available commands are:  
	* **help**									-	Prints out a list of all commands  
	* **debugMode**								-	Enables debug mode which provides dev info over serial  
	* **toggleSelfChanging**					-	Toggles the 'selfChangingEnabled' flag which, if disabled, pauses the current self changing effect (Also useful to stop debugMode spam)  
	* **toggleFading**							-	Toggles the code which updates the fading values  (This can also help to keep debugMode messages clean)  
	* **setColor([INT],[INT],[INT])**			-	Sets the pin output color (values reaching from 0 to 255)  
	* **setSEffect([STRING],[INT],[DOUBLE])**	-	Sets the current self-changing effect, max timestamp and speed  