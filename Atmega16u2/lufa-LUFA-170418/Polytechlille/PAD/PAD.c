/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  OBinou (obconseil [at] gmail [dot] com)
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the atmega16u2 program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "PAD.h"
#include <LUFA/Drivers/Peripheral/Serial.h>



//Programme principal.
//On configure le périphérique puis on entre dans la boucle qui va vérifier périodiquement si des données doivent être émises ou reçues
int main(void){
    SetupHardware();
  GlobalInterruptEnable();
  for (;;) {
        USB_USBTask();
        PAD_Task();
        EndpointOut();
        EndpointIn();
    }
}


void EndpointIn(void){
           //Sélection des Endpoint d'entrée
    static int manette = 0;
    Endpoint_SelectEndpoint(ENDPOINT_IN_EPADDR0);
        
    if(Serial_IsCharReceived()){
        manette = Serial_ReceiveByte();
        uint8_t boutons = manette & 0x1e;
        if(Endpoint_IsINReady()){
            if (Endpoint_IsReadWriteAllowed()){
                Endpoint_Write_8(boutons);    //On envoie les données sur le endpoint
                          //On vide le endpoint
            }
            Endpoint_ClearIN();  
        }
           
    }
    
    Endpoint_SelectEndpoint(ENDPOINT_IN_EPADDR1);
    if(Serial_IsCharReceived()){
        manette = Serial_ReceiveByte();
        uint8_t joystick = manette & 0x01; 
        if(Endpoint_IsINReady()){
            if (Endpoint_IsReadWriteAllowed()){
                Endpoint_Write_8(joystick);    //On envoie les données sur le endpoint
                            //On vide le endpoint
            }
            Endpoint_ClearIN();     
        }
            
    }
}


void EndpointOut(void){
        //On sélectionne les Endpoints de sortie pour envoyer des données
    
    Endpoint_SelectEndpoint(ENDPOINT_OUT_EPADDR0); //Pour les LED
    if (Endpoint_IsOUTReceived()){
        if (Endpoint_IsReadWriteAllowed()){
                //On lit les données des boutons et on envoi
            uint8_t Donnees_Led = Endpoint_Read_8();
	    //Donnees_Led = Donnees_Led<<1; On décale de 1 vers la gauche
            Serial_SendByte(Donnees_Led);
               
        }
		/* Handshake the out Endpoint - clear endpoint and ready for next report */
        Endpoint_ClearOUT();   
    } 
}

void SetupHardware(void)
{
  	USB_Init(); //initialisation des leds
//Serial_Init(9600, 0);
#if (ARCH == ARCH_AVR8)
  /* Disable watchdog if enabled by bootloader/fuses */
  	MCUSR &= ~(1 << WDRF);
  	wdt_disable();

  /* Disable clock division */
  	clock_prescale_set(clock_div_1);
#endif

  /* Hardware Initialization */
  
	USB_Init();
	Serial_Init(9600, 0);
}

//Fonction appellée pour créer les endpoints
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;
  /* Setup HID Report Endpoints */
  ConfigSuccess &= Endpoint_ConfigureEndpoint(ENDPOINT_IN_EPADDR0, EP_TYPE_INTERRUPT, EPSIZE, 1);
  ConfigSuccess &= Endpoint_ConfigureEndpoint(ENDPOINT_OUT_EPADDR0, EP_TYPE_INTERRUPT, EPSIZE, 1);
  ConfigSuccess &= Endpoint_ConfigureEndpoint(ENDPOINT_IN_EPADDR1, EP_TYPE_INTERRUPT, EPSIZE, 1);

}

void PAD_Task(void)
{
  /* Device must be connected and configured for the task to run */
  if (USB_DeviceState != DEVICE_STATE_Configured) return;
}
