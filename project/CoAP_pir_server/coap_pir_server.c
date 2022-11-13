
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"

//librerie routing RPL
#include "net/routing/routing.h"
#include "net/netstack.h"

#include "lib/random.h"

#include "net/ipv6/uip-debug.h"

#include "sys/log.h"
#include "node-id.h"

#include "os/dev/button-hal.h"
#include <stdbool.h>

#define BOOL bool


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern coap_resource_t res_seat;


//timer for chronometer
static struct etimer chronometer;
#define CHRONOMETER_MAXIMUM_TIME CLOCK_SECOND*30  //TODO: CAMBIARE A 30 MINUTI

static struct etimer chronometer2;
#define CHRONOMETER_MAXIMUM_TIME2 CLOCK_SECOND*30  //TODO: CAMBIARE A  5 MINUTI

//timer for PIR (for simulation)
static struct etimer timer_pir;
#define PIR_MAXIMUM_WAIT CLOCK_SECOND*8  //TODO: CAMBIARE

//keep track of movements (simulated by timer_pir) during the chronometer time
static int total_of_movements = 0;
//thresholds to say that a seat is occupied or free
#define OCCUPIED_THRESHOLD 7 //se si rilevano almeno OCCUPIED_THRESHOLD movimenti quando il posto è libero, allora lo possiamo considerare occupato.
#define FREE_THRESHOLD 10 //se lo studente, che occupa il posto, si muove almeno di FREE_THRESHOLD volte in CHRONOMETER_MAXIMUM_TIME secondi, allora resettiamo il cronometro dato che è ancora seduto 


//keep track of the status of the seat
static bool is_seat_free = true;

PROCESS(Coap_Pir_server, "CoAP Pir Server");
AUTOSTART_PROCESSES(&Coap_Pir_server);

PROCESS_THREAD(Coap_Pir_server, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  LOG_INFO("Starting CoAP PIR Server\n");

  LOG_INFO("Registering 'seat' resource\n");
  //registro la risorsa
  coap_activate_resource(&res_seat, "seat");

  //activate chronometer for the first time. 
  etimer_set(&chronometer, CHRONOMETER_MAXIMUM_TIME);
 
  //activate pir timer (for simulation) for the first time. Random between 1 second and PIR_MAXIMUM_WAIT
  etimer_set(&timer_pir, (random_rand() % PIR_MAXIMUM_WAIT) );

  while(1){

	//wait for event...
	PROCESS_WAIT_EVENT();

	//if the chronometer is terminated...or if the button is pressed by the user...
	if( (ev == PROCESS_EVENT_TIMER && data == &chronometer) || (ev == button_hal_press_event) ){

		LOG_INFO("The chronometer is terminated or the button was pressed. Seat release in progress ...\n");

		//set to free the resource if and only if the resource is occupied
		if(is_seat_free == false){

			is_seat_free = true;
		
			//switch the state of the resource and advice the cloud app client
			res_seat.trigger();

  			LOG_INFO("Consider seat FREE %d \n", is_seat_free);

			//now i activate a timer in which if in a short window of time the number of movements are not enought, i will still consider the seat free
			//activate chronometer2  
  			etimer_set(&chronometer2, CHRONOMETER_MAXIMUM_TIME2);
		}

		//reset total of movements
		total_of_movements = 0;		

	}

	if(ev == PROCESS_EVENT_TIMER && data == &chronometer2){

		//if the seat was not already occupied: can appean that, before the termination of this chronometer, the total_of_movements is enought to get the seat
		if (is_seat_free == true){

			LOG_INFO("The chronometer2 is terminated. The seat remains free ...\n");

			total_of_movements = 0;

			etimer_restart(&chronometer2);
		}
		
	}


	//if the pir detected movement ..
	if(ev == PROCESS_EVENT_TIMER && data == &timer_pir){

  		LOG_INFO("New PIR movement detected!\n");

		//if the seat is free
		if(is_seat_free == true){
			
			//increment number of total movements
			total_of_movements++;

  			LOG_INFO("Increment movements...%d \n", total_of_movements);

			//if is greater than a threshold...
			if ( total_of_movements > OCCUPIED_THRESHOLD ){

				//consider the seat occupied
				total_of_movements = 0;
				is_seat_free = false;

  				LOG_INFO("Consider the seat OCCUPIED %d \n", is_seat_free);

				//restart chronometer
				etimer_restart(&chronometer);

				//switch the state of the resource and advice the cloud app client
				res_seat.trigger();
				
			}
		}
		//if the seat is occupied
		else {
			//increment number of total movements
			total_of_movements++;

  			LOG_INFO("Increment movements...%d \n", total_of_movements);

			//if is greater than a threshold...
			if ( total_of_movements > FREE_THRESHOLD ){

  				LOG_INFO("Consider the seat STILL OCCUPIED %d. Reset chronometer. \n", is_seat_free);

				//consider the seat still occupied
				total_of_movements = 0;

				//restart chronometer
				etimer_restart(&chronometer);
				
			}
		}

	  //activate pir timer (for simulation) again. Random between 1 second and PIR_MAXIMUM_WAIT
	  etimer_set(&timer_pir, (random_rand() % PIR_MAXIMUM_WAIT) );
	}

		
  }
  
  PROCESS_END();
}
