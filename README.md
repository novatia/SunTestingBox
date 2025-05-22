# SunTestingBox
A fine box, for a fine purpose. Generating fake fireball with arduino. Usefull if you are working on cameras detecting meteors.  

Spawn a web server. Before flashing, set MAC address and IP Address.
then you can call services:

* http://192.168.17.77/E1000 
set event lenght to 1000 [ms]

* http://192.168.17.77/I50000
set local time to 50000 [s]

* http://192.168.17.77/S1
set speed to 1  

* http://192.168.17.77/R
Generate a stationary single event

* http://192.168.17.77/D
Generate a fireball, need servos

* http://192.168.17.77/B
Set maximum event brightness  0->255

You can set those values also using serial port.
Note. In order to use Arduino ETH shields with modern switches, if something is not working as intended, try to force 100M Full duplex mode on the desired port.

#Fireball generator
A very simple program used to spawn different fireball settings.

Two different mode, brute force and bust. 
The first mode is usefull to find the correct event duration and brightness. 
The second one to try to generate an event.


In order to produce correctly linear events, need to use this object with servo.

https://www.thingiverse.com/thing:3170748


