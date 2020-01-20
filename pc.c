#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <signal.h>
#include <sys/signal.h>

#define idVendorArduino 0x4444
#define idProductArduino 0x5555
#define MAX_EP_IN   16
#define MAX_EP_OUT  16
#define MAX_DATA     1

int stop = 0;

uint8_t ep_in[MAX_EP_IN];
uint8_t ep_out[MAX_EP_OUT];


/*==============================Libération du handle===============================*/
void libere(libusb_device_handle **handle){
   struct libusb_device *device ;
   device = libusb_get_device(*handle);
   struct libusb_config_descriptor *desc;
   int status = libusb_get_active_config_descriptor(device, &desc);
   if(status!=0){perror("libusb_claim_interface"); exit(-1);}

   ssize_t i=0;
   for (i=0; i<desc->bNumInterfaces; i++){
    struct libusb_interface interf=(desc->interface[i]);
      struct libusb_interface_descriptor idesc = (interf.altsetting[0]);
      int interfnum = idesc.bInterfaceNumber;
      int status=libusb_release_interface(*handle,interfnum);
      if(status!=0){ perror("libusb_claim_interface"); exit(-1); }
      printf("interface n° %d relâchée \n", interfnum);
  }
   libusb_close(*handle);
}


/*============================Pré-traitement : énumération des endpoints=========================*/

void enumeration(libusb_context **context, libusb_device_handle **handle){

    libusb_device **list;
    ssize_t count=libusb_get_device_list(*context,&list);
    if(count<0) {perror("libusb_get_device_list"); exit(-1);}

    ssize_t i=0;
    for(i=0;i<count;i++){
      libusb_device *device=list[i];
      struct libusb_device_descriptor desc;
      int status=libusb_get_device_descriptor(device,&desc);
      if(status!=0) continue;

      //bonne récupération "poignee"
      if(desc.idVendor==idVendorArduino && desc.idProduct == idProductArduino){
            printf("device trouvé\n");
	 uint8_t bus=libusb_get_bus_number(device);
         uint8_t address=libusb_get_device_address(device);
         printf("Device Found @ (Bus:Address) %d:%d\n",bus,address);
         printf("Vendor ID 0x0%x\n",desc.idVendor);
         printf("Product ID 0x0%x\n",desc.idProduct);

	int status=libusb_open(device,handle);
	if(status!=0){ perror("libusb_open error"); exit(-1);}
      }
    }
    libusb_free_device_list(list,1);
}


/*============================Pré-traitement : configuration du handle=========================*/

void config(libusb_device_handle **handle){

    struct libusb_device *device = libusb_get_device(*handle);
    struct libusb_config_descriptor *desc;
    int config_index=0;
    int status = libusb_get_config_descriptor(device,config_index, &desc);
    int i=0;

    int nombre_interf = desc->bNumInterfaces;
    // printf("test\n");

    printf("Nombre d'interfaces : %d\n",nombre_interf);

    for(i=0; i<nombre_interf; i++){
        struct libusb_interface_descriptor idesc=(desc->interface[i].altsetting[0]);
        int interfnum = idesc.bInterfaceNumber;
        if(libusb_kernel_driver_active(*handle,interfnum)){
            status=libusb_detach_kernel_driver(*handle,interfnum);
            printf("interface détachée : %d\n", interfnum);
            if(status!=0){ perror("libusb_detach_kernel_driver"); exit(-1); }
        }
    }

    int configuration = desc->bConfigurationValue;
    printf("Numéro de configuration %d\n", configuration);
    status=libusb_set_configuration(*handle,configuration);
    if(status!=0){ perror("libusb_set_configuration"); exit(-1); }

    for (i=0; i<nombre_interf; i++){
        int i_in=0,i_out=0;
        struct libusb_interface_descriptor idesc=(desc->interface[i].altsetting[0]);
        int interfnum = idesc.bInterfaceNumber; //on récup le nm d'interface
        int status=libusb_claim_interface(*handle,interfnum);
        if(status!=0){ perror("libusb_claim_interface"); exit(-1); }
        printf("Interface n° %d réclamée\n", interfnum);

       //récupération des adresses des endpoints
        int k=0;
        for (k=0; k<idesc.bNumEndpoints; k++){ //on parcourt tous les endpoints du descritpor
            struct libusb_endpoint_descriptor endpointdesc = idesc.endpoint[k];

            int attribut = endpointdesc.bmAttributes;

            if ((attribut & 0b00000011)==LIBUSB_TRANSFER_TYPE_INTERRUPT){
                if ((endpointdesc.bEndpointAddress&0x80)==0x80){ //Si le bit 7 est à 1, c'est le endpoint d'entrée sinon c'est le endpoint de sortie
                    ep_in[i_in] = endpointdesc.bEndpointAddress;
                    i_in++;}
                else{
                    ep_out[i_out] = endpointdesc.bEndpointAddress;
                    i_out++;}
                printf("Adresse des endpoints de sortie: %x \n",ep_out[i_out-1]);
                printf("Adresse des endpoints d'entrée: %x \n",ep_in[i_in-1]);
            }
        }
        
    }
}



/*======================Envoi et réception clavier PC et manette===================*/

void lecture_clavier(libusb_device_handle *handle){
    char caractere;
    int status;
    unsigned char data[MAX_DATA];    //buffer pour envoyer ou recevoir des données
	int length= sizeof(data);        //taille mémoire maximale pour envoyer ou recevoir. Ici 1 octet suffit
    int timeout=10;                  //timeout en ms
    int bytes_out;    

    while (1){
        caractere = getchar();
        data[0]=caractere;
        status=libusb_interrupt_transfer(handle,ep_out[0],data,length,&bytes_out,timeout);
        if(status!=0){ perror("libusb_interrupt_transfer");}
        else{printf("caractère reçu : %c\n",data[0]);}
        if(caractere=='z'){
            printf("sortie du clavier\n");
            break;
        }
    }
}


void manette(libusb_device_handle *handle){
    unsigned char data2;
    unsigned char data[MAX_DATA];    //buffer pour envoyer ou recevoir des données
	int length= sizeof(data2);       //taille mémoire maximale pour envoyer ou recevoir. Ici 1 octet suffit
    int taille = sizeof(data);
    int timeout=100;
    int status, bytes_in, bytes_out;    	 //timeout en ms
 
    while(1){
        data2 = 0;
        status=libusb_interrupt_transfer(handle,ep_in[0],&data2,length,&bytes_in,timeout);
        if(status!=0){ perror("libusb_interrupt_transfer");}
            else {
                if(data2==14){
                    printf("bouton reçu : gauche\n");
                    data[0]='A'; //on allume la LED 1
                    status=libusb_interrupt_transfer(handle,ep_out[0],data,length,&bytes_out,timeout);
                    if(status!=0){ perror("libusb_interrupt_transfer");}
                }
                if(data2==22){
                    printf("bouton reçu : bas \n");
                    data[0]='a'; //on éteind la LED
                    status=libusb_interrupt_transfer(handle,ep_out[0],data,length,&bytes_out,timeout);
                    if(status!=0){ perror("libusb_interrupt_transfer");}
                }
                if(data2==26){printf("bouton reçu : haut\n");}
                if(data2==28){printf("bouton reçu : droit\n");}
                if(data2==30){printf("joystick\n");
                    break;
                }
            }
    }
}


/*==============================Main===============================*/


int main(){
    libusb_context *context;
    libusb_device_handle *handle=NULL;
    int status=libusb_init(&context);
    if(status!=0) {perror("libusb_init"); exit(-1);}
    int choix = 0;
    unsigned char data2;
    unsigned char data[MAX_DATA];    //buffer pour envoyer ou recevoir des données
    int length= sizeof(data2);       //taille mémoire maximale pour envoyer ou recevoir. Ici 1 octet suffit
    int taille = sizeof(data);
    int timeout=100;
    int bytes_in,bytes_out;    	 //timeout en ms

    //Enumeration peripheriques USB
    enumeration(&context, &handle);
    printf("test\n");
    config(&handle);

    //Traitement des données envoyées et reçues via les endpoints d'entrées/sorties
    while(stop!=1){
        printf("clavier (1) ou boutons (2) ou éteindre (3)\n");
        scanf("%d", &choix);
        switch (choix){
            case 1: 
                printf("saisissez 'z' pour revenir au menu\n");
                lecture_clavier(handle);
                break;
            case 2:
                printf("appuyez sur le joystick pour revenir au menu\n");
                manette(handle);
                break;
            case 3 :
                stop=1;
            default :
                printf("le choix n'est pas proposé\n");
                choix = 0;
        }
    }
    
    //Libération de l'appareil 
    libere(&handle);
    libusb_exit(context);
}

