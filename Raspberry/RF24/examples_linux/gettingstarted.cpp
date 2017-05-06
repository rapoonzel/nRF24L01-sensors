/*
2015-04-06 : Johan Boeckx - Arduino/RPi(2) nRF24L01+ : Raspberry Pi (2) code
  Tested on Arduino UNO R3 and Raspberry Pi B Rev. 2.0 and Raspberry Pi 2 B

 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

 03/17/2013 : Charles-Henri Hallard (http://hallard.me)
              Modified to use with Arduipi board http://hallard.me/arduipi
                          Changed to use modified bcm2835 and RF24 library
TMRh20 2014 - Updated to work with optimized RF24 Arduino library

*/

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <RF24/RF24.h>

using namespace std;
//
// Hardware configuration
// Configure the appropriate pins for your connections

/****************** Raspberry Pi ***********************/

// Radio CE Pin, CSN Pin, SPI Speed
// See http://www.airspayce.com/mikem/bcm2835/group__constants.html#ga63c029bd6500167152db4e57736d0939 and the related enumerations for pin information.

// Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 4Mhz
//RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_4MHZ);

// NEW: Setup for RPi B+
//RF24 radio(RPI_BPLUS_GPIO_J8_15,RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);

// Setup for GPIO 15 CE and CE0 CSN with SPI Speed @ 8Mhz
//RF24 radio(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);

// RPi generic:
RF24 radio(22,0);

/*** RPi Alternate ***/
//Note: Specify SPI BUS 0 or 1 instead of CS pin number.
// See http://tmrh20.github.io/RF24/RPi.html for more information on usage

//RPi Alternate, with MRAA
//RF24 radio(15,0);

//RPi Alternate, with SPIDEV - Note: Edit RF24/arch/BBB/spi.cpp and  set 'this->device = "/dev/spidev0.0";;' or as listed in /dev
//RF24 radio(22,0);


/****************** Linux (BBB,x86,etc) ***********************/

// See http://tmrh20.github.io/RF24/pages.html for more information on usage
// See http://iotdk.intel.com/docs/master/mraa/ for more information on MRAA
// See https://www.kernel.org/doc/Documentation/spi/spidev for more information on SPIDEV

// Setup for ARM(Linux) devices like BBB using spidev (default is "/dev/spidev1.0" )
//RF24 radio(115,0);

//BBB Alternate, with mraa
// CE pin = (Header P9, Pin 13) = 59 = 13 + 46
//Note: Specify SPI BUS 0 or 1 instead of CS pin number.
//RF24 radio(59,0);

/********** User Config *********/
// Assign a unique identifier for this node, 0 or 1
// 0 Rx / 1 Tx
bool radioNumber = 0;
unsigned long timeoutPeriod = 3000;     // Set a user-defined timeout period. With auto-retransmit set to (15,15) retransmission will take up to 60ms and as little as 7.5ms with it set to (1,15).

/********************************/

// Radio pipe addresses for the 2 nodes to communicate.
// const uint8_t pipes[][6] = {"1Node","2Node"};
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };   // Radio pipe addresses for the 2 nodes to communicate.
char data[32] = {"_A message from RPi w/ NRF24L+!"};            //Data buffer

void showData(void)
{
      printf("Data: ");
      for(int i=0; i<32; i++){
         if(isprint(data[i]))
           printf("%c", data[i]);
         else
           printf(".");
      }
      printf("\n\r");
}

int main(int argc, char** argv){

  const int role_rx=0, role_tx=1;
  int role=role_rx;
/********* Role chooser ***********/

  printf("\n ************ Role Setup ***********\n");
  string input = "";
  char myChar = {0};

  cout << "Choose a role: Enter 0 for Rx, 1 for Tx (CTRL+C to exit) \n>";
  getline(cin,input);

  if(input.length() == 1) {
    myChar = input[0];
    if(myChar == '0'){
        cout << "Role: Rx " << endl << endl;
    }else{  cout << "Role: Tx " << endl << endl;
        role = role_tx;
    }
  }
  switch(role) {
      case role_rx :
        radioNumber=0;
        break;

      case role_tx :
        radioNumber=1;
        break;
  }

/***********************************/
  // Setup and configure rf radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  // Set the channel
  radio.setChannel(1);
  // Set the data rate
  //radio.setDataRate(RF24_2MBPS);
  radio.setDataRate(RF24_250KBPS);
  //radio.setPALevel(RF24_PA_MAX);
  radio.setPALevel(RF24_PA_MIN);

    if ( !radioNumber )    {
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
        memset(&data,'\0',sizeof(data));
        radio.startListening();
    } else {
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(1,pipes[0]);
        radio.stopListening();
    }
    // Dump the configuration of the rf unit for debugging
    radio.printDetails();
    printf("Start loop:\n");
    // forever loop
    while (1)
    {
        if (radioNumber) {
            if (radio.writeBlocking(&data,sizeof(data),timeoutPeriod)) {
                printf(".");
            }
            else {
                printf("?");
            }
            fflush(stdout);
            //printf("\n");
        }
        else {
        //
        // Receive each packet, dump it
        //
            if(radio.available()){
                // Read any available payloads for analysis
                radio.read(&data,32);
                // Dump the printable data of the payload
                showData();
                fflush(stdout);
            }
        }
        delay(5);
    } // forever loop

  return 0;
}
