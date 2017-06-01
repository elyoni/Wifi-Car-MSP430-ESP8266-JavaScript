# Wifi-controlled Mini Car (with ESP8266 and MSP430)

I used the MSP430 as the main processor and combined it with the ESP8266 for
wifi connectivity.

The ESP8266 works as WebSocket server and as a buffer for communicating with a
website that acts as the remote control.

## Getting Started

- To run your car you need to build the following circuit
![fritzing circute - copy_schem](https://cloud.githubusercontent.com/assets/12208012/26436965/f5ddb5f2-4122-11e7-86fa-a0ab34036ffb.png)
- Flash the file main-MSP430.c into your MSP.
- Flash the file Serial-With-Websocket.ino into your ESP8266
- Run the website from the `Control Site` Folder

Now that you're done, you can start having fun with your car.

## Known issues

### Using the ESP8266 as both WebSocket(WS) server and Access Point(AP)

When using the ESP as AP and WS it sometimes overload the ESP and causes it
crash and reboot it self.

To fix that I set my phone as AP and made the ESP to connect to it. The only
problem with that is there's no guarantee that the ESP's ip address won't
change.

### Only the last user that connected to the server can control the car

Because I didn't want to manage the users that connected and disconnected I
decided that only the last user that connected to the WebSocket server can
control the car.

The thought behind it was that in every given time you can have only one driver.
