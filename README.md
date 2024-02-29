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
