# ESP32_SmartSocket
- This is a project making a smart socket using ESP32. ESP32 can create a webserver and can be accessed by a web browser.
- You can also use a internet-connected device to access the webserver from everywhere in the world.
- ESP32 can connect to the internet via Ngrok - tunnel local ports to public URLs.

# Preparation
- List of hardware required:
    + ESP32 (You can use any ESP32 or ESP8266 board, but I use the DevKit V1. If you use ESP8266, you need to change the library to ESP8266WebServer.)
    + 5VDC Relay
    + 3.3 to 5V logic level converter (I use TXS0108E, which is a 8-bit level converter, but i think a small mosfet will work as well.)
    + 1 breadboard
    + 5VDC Power Supply (I use the 5VDC 1A power supply)
    + 2 Switches (1 for Auto Mode, 1 for Manual Mode)
    + 1 220VAC Socket (I use the 4-pin socket)
    + 1 220VAC Fuse
    + 1 220VAC Plug
    + 220VAC Wires

- List of software required:
    + Arduino IDE or PlatformIO or any IDE that can be used to program ESP32.
    + CP2101 driver for Computer (This is a driver that allows you to communicate with ESP32 via USB.)


# How to use
- Download Ngrok at [this address](https://ngrok.com/)and open it. Choose your operating system.

- Roll down to below the dowload page and follow the instructions:
   + Download the Ngrok for your operating system.
   + Run the following command to add your authtoken to the default ngrok.yml configuration file.
       ''' ngrok config add-authtoken <YOUR AUTH TOKEN> '''
        Where <YOUR AUTH TOKEN> is the token you got from the ngrok website.
   + Deploy the static domain using this command:
       ''' ngrok http --domain=<your-domain> 80 '''
        Where <your-domain> is the domain you want to use. And 80 is the port number the ESP32 will be using.

- Copy the code in the [main file](/src/main.cpp)and paste it in your new Arduino IDE or PlatformIO's project, clone this repository or just download the zip file and extract it.

- Upload the code to your ESP32.

- Connect the wires as shown in the images below:
- ![ESP32_SmartSocket_ControlCircuit](/images/ESP32_SmartSocket_ControlCircuit.png)
- ![ESP32_SmartSocket_PowerCircuit](/images/ESP32_SmartSocket_PowerCircuit.png)

