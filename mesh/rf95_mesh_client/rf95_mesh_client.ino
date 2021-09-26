// rf95_mesh_client.cpp
// -*- mode: C++ -*-
// Example application showing how to create a simple addressed, routed reliable messaging client
// with the RHMesh class.
// It is designed to work with the other examples rf95_mesh_server*
// Hint: you can simulate other network topologies by setting the 
// RH_TEST_NETWORK define in RHRouter.h

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
//
// Requires Pigpio GPIO library. Install by downloading and compiling from
// http://abyz.me.uk/rpi/pigpio/, or install via command line with 
// "sudo apt install pigpio". To use, run "make" at the command line in 
// the folder where this source code resides. Then execute application with
// sudo ./rf95_mesh_client.
// Tested on Raspberry Pi Zero and Zero W with LoRaWan/TTN RPI Zero Shield 
// by ElectronicTricks. Although this application builds and executes on
// Raspberry Pi 3, there seems to be missed messages and hangs.
// Strategically adding delays does seem to help in some cases.

//(9/20/2019)   Contributed by Brody M. Based off rf22_mesh_client.pde.
//              Raspberry Pi mods influenced by nrf24 example by Mike Poublon,
//              and Charles-Henri Hallard (https://github.com/hallard/RadioHead)

// #include <pigpio.h>
// #include <stdio.h>
// #include <signal.h>
// #include <unistd.h>


#include <SPI.h>

#include <RHMesh.h>
#include <RH_RF95.h>

#define RH_MESH_MAX_MESSAGE_LEN 50

//Function Definitions
void sig_handler(int sig);

//Pin Definitions
// #define RFM95_CS_PIN 8
// #define RFM95_IRQ_PIN 25
// #define RFM95_LED 4

// In this small artifical network of 4 nodes,
#define CLIENT_ADDRESS 1
#define SERVER1_ADDRESS 2
#define SERVER2_ADDRESS 3
#define SERVER3_ADDRESS 4

//RFM95 Configuration
#define RFM95_FREQUENCY  915.00
#define RFM95_TXPOWER 14

// Singleton instance of the radio driver
// RH_RF95 rf95(RFM95_CS_PIN, RFM95_IRQ_PIN);
// RH_RF95 rf95(5, 4);  // automato
RH_RF95 rf95(PIN_LORA_CS, PIN_LORA_IRQ); // Slave select, interrupt pin for Automato Sensor Module.


// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(rf95, CLIENT_ADDRESS);

//Flag for Ctrl-C
int flag = 0;


void setup() 
{
  pinMode(PIN_LORA_RST, INPUT); // Let the pin float.

  // Disable other SPI devices.
  pinMode(PIN_LCD_CS, OUTPUT);
  digitalWrite(PIN_LCD_CS, HIGH);
  pinMode(PIN_TCH_CS, OUTPUT);
  digitalWrite(PIN_TCH_CS, HIGH);
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  
  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available
  Serial.println("Initializing LoRa"); 
  if (!rf95.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  rf95.setFrequency(915.0);

  // The default transmitter power is 13dBm, using PA_BOOST.
  // You can set transmitter powers from 5 to 23 dBm:
  //  driver.setTxPower(23);


  if (!manager.init())
  {
    Serial.println( "\n\nMesh Manager Failed to initialize.\n\n" );
    // return 1;
  }

  /* Begin Manager/Driver settings code */
  Serial.println("RFM 95 Settings:");

  Serial.print("Frequency="); 
  Serial.print(RFM95_FREQUENCY);
  Serial.println("MHz\n");

  Serial.print("Power= ");
  Serial.println(RFM95_TXPOWER);

  Serial.print("Client(This) Address= ");
  Serial.println(CLIENT_ADDRESS);
  
  Serial.print("Server Address 1= ");
  Serial.println( SERVER1_ADDRESS);

  Serial.print("Server Address 2= ");
  Serial.println( SERVER2_ADDRESS);

  Serial.print("Server Address 3= ");
  Serial.println( SERVER3_ADDRESS);

  Serial.println("Route: Client->Server 3 is automatic in MESH.\n");

  rf95.setTxPower(RFM95_TXPOWER, false);
  rf95.setFrequency(RFM95_FREQUENCY);
  /* End Manager/Driver settings code */

  Serial.println( "\nRPI rf95_mesh_client startup OK.\n" );

}

  uint8_t data[] = "Hello World!";
  // Dont put this on the stack:
  uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];


void loop ()
{
  // if (gpioInitialise()<0)
  // {
  //   printf( "\n\nRPI rf95_mesh_client startup Failed.\n" );
  //   return 1;
  // }

  // gpioSetSignalFunc(2, sig_handler); //2 is SIGINT. Ctrl+C will cause signal.


// #ifdef RFM95_LED
//   gpioSetMode(RFM95_LED, PI_OUTPUT);
//   Serial.println("\nINFO: LED on GPIO %d\n", (uint8_t) RFM95_LED);
//   gpioWrite(RFM95_LED, PI_ON);
//   gpioDelay(500000);
//   gpioWrite(RFM95_LED, PI_OFF);
// #endif


  // while(!flag)
  // {

    Serial.println("Sending to manager_mesh_server1");
// #ifdef RFM95_LED
//     gpioWrite(RFM95_LED, PI_ON);
// #endif

    // Send a message to a rf95_mesh_server
    // A route to the destination will be automatically discovered.
    if (manager.sendtoWait(data, sizeof(data), SERVER1_ADDRESS) == RH_ROUTER_ERROR_NONE)
    {
      // It has been reliably delivered to the next node.
      // Now wait for a reply from the ultimate server
      uint8_t len = sizeof(buf);
      uint8_t from;
      if (manager.recvfromAckTimeout(buf, &len, 3000, &from))
      {
        Serial.print("got reply from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println((char*)buf);
      }
      else
      {
        Serial.println("No reply, is rf95_mesh_server1, rf95_mesh_server2 and rf95_mesh_server3 running?");
      }
    }
    else
      Serial.println("sendtoWait failed. Are the intermediate mesh servers running?");
// #ifdef RFM95_LED
//     gpioWrite(RFM95_LED, PI_OFF);
// #endif
    // gpioDelay(400000);
    delay(400);
 // }

//  Serial.println( "\nrf95_mesh_client Tester Ending\n" );
//  gpioTerminate();
//  return 0;
}

void sig_handler(int sig)
{
  flag=1;
}
