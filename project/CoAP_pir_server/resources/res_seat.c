
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "node-id.h"
#include <stdbool.h>

#define BOOL bool


//info building, study room and device
#define ID_BUILDING 1
#define ID_ROOM 1
#define ID_DEVICE node_id


//definisco a priori la GET callback
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

//definisco a priori la Observing Callback
static void res_event_handler(void);

/*
 	Definisco gli handler da chiamare per ogni tipo (GET, POST,...)	
 */
EVENT_RESOURCE(res_seat,
	 //core link format
         "title=\"State of seat:\";rt=\"Text\"; ct=50;", //ct=50 means application/json
         res_get_handler, //viene chiamata ogni volta che si fa la GET sulla risorsa
         NULL, 
         NULL, 
         NULL, 
	res_event_handler //viene chiamata ogni volta che nel CoAP server si chiama res_seat.trigger()
	);


//contain the state of the resource (true means that the seat is FREE)
static bool is_free = true;

//GET callback
static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    char json_message[200];

    sprintf(json_message, "{ \"idBuilding\": %d, \"idRoom\": %d,  \"idDevice\": %d,  \"state\": %s }", ID_BUILDING, ID_ROOM, ID_DEVICE, is_free?"1":"0");

    printf("Notify client about the new state of seat: %s\n", json_message);

    //copio stringa nel buffer
    memcpy(buffer, json_message, strlen(json_message)+1);

   int length = strlen(json_message)+1;

   //invio al client che ne ha fatto la richiesta la risorsa
   coap_set_header_max_age(response, -1);
   coap_set_header_content_format(response, TEXT_PLAIN);
   coap_set_header_etag(response, (uint8_t *)&length, 1);
   coap_set_payload(response, buffer, length);
}

//observing callback
static void res_event_handler(void) {

	//The server want to change/switch the state of the resource

	is_free = !is_free;

	printf("I am in the handler %s \n",  is_free ? "true" : "false");
	//notifico tutti i client che osservano la risorsa
	//NB: questa funzione chiamata la GET (ossia la res_get_handler)
	coap_notify_observers(&res_seat);

}
