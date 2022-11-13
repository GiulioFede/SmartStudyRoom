
/*
	Inseriamo le librerie necessarie
*/
#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"

#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "node-id.h"

#include "os/dev/leds.h"

#include "lib/random.h"
#include <math.h>


#define LOG_MODULE "mqtt-client"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif


/* Indico l'indirizzo IP del broker. */
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"

static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Indico la porta a cui il Broker ascolta
#define DEFAULT_BROKER_PORT         1883

// il mio client voglio che pubblichi ogni 30 minuti TODO: CAMBIARE
#define DEFAULT_PUBLISH_INTERVAL    (10 * CLOCK_SECOND)


//Definisco gli stati in cui il mio client può essere
static uint8_t state;

#define STATE_INIT    	      0
#define STATE_NET_OK   	      1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_SUBSCRIBED      4  //subcribed to topic "actuator_temperature"
#define STATE_SUBSCRIBED_SYNCH 6 //subscribed to topic "time_synch"
#define STATE_DISCONNECTED    5

//creo il mio processo chiamandolo "mqtt_client_temperature" e lo faccio partire subito all'accensione del sensore
PROCESS_NAME(mqtt_client_temperature);
PROCESS(mqtt_client_temperature, "MQTT Client Temperature");
AUTOSTART_PROCESSES(&mqtt_client_temperature);


//definisco alcuni parametri per i segmenti TCP
#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN   64

#define BUFFER_SIZE 64

static char client_id[BUFFER_SIZE];
//semplicemente salverò dentro una stringa che indicherà il topic. Nel nostro caso il topic è sempre "temperature", ossia pubblicheremo messaggi sempre con topic temperature
static char pub_topic[BUFFER_SIZE];
//salverò qui dentro il "subscription topic", ossia il nome del topic a cui voglio sottoscrivermi che nel nostro caso sarà sempre "actuator_temperature_ID_BUILDING_ID_ROOM"
static char sub_topic[BUFFER_SIZE];
//salverò il topic a cui iscrivermi per la sincronizzazione
static char sub_topic_synch[BUFFER_SIZE];


//Quando dovrò pubblicare ogni 10 minuti dei messaggi (col topic "temperature") userò come contenuto del messaggio (come a simulare un campionamento) il valore della variabile "temperature".
static float temperature = 24.0; //temperatura iniziale in C°

//define the range in which the temperature can vary (in C°)
#define MIN_TEMPERATURE 18.0
#define MAX_TEMPERATURE 30.0

// Devo fare un check dello stato del sensore ogni 500ms. Pongo STATE_MACHINE_PERIOD=500ms.
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
// Creo un timer (periodic_timer) che faccio partire e ripartire ogni 500ms per fare quanto detto appena sopra.
static struct etimer periodic_timer;

#define APP_BUFFER_SIZE 512
//userò tale variabile per inserire il valore di temperature, ossia il contenuto del nuovo messaggio
static char app_buffer[APP_BUFFER_SIZE];

//definisco struttura che userò per pubblicare messaggi al Broker oppure che estrarrò (dai suoi campi) per capire cosa il Broker mi ha inviato
static struct mqtt_message *msg_ptr = 0;

//contiene quanto utile per connettermi al Broker MQTT
static struct mqtt_connection conn;

//ad ogni operazione l'MQTT ritornerà uno stato dell'operazione. Lo salveremo qui.
static mqtt_status_t status;

//E' un array di lunghezza 64 che conterrà l'indirizzo del Broker
static char broker_address[CONFIG_IP_ADDR_STR_LEN];

//define threshold to light up the GREEN (<) and RED (> =) LEDs. The first indicates cool temperature, the second hot.
#define LED_TEMPERATURE_THRESHOLD 23

//function to change LED color 
static void change_LED_Band(){

	//if current temperature is less the LED_TEMPERATURE_THRESHOLD, change to GREEN
	if ( temperature < LED_TEMPERATURE_THRESHOLD){
		leds_set(LEDS_GREEN);
	}else {
		leds_set(LEDS_RED);
	}

}

//info building, study room and device
#define ID_BUILDING 1
#define ID_ROOM 1
#define ID_DEVICE node_id

static char topic_of_interest[50]; //will contain the topic for actuator
static char topic_for_synchronization[50]; //will contain the topic for actuator

static long base_time = -1; //s --> will contain the time, respect to the boot, to use as a reference time


//to avoid to store all the sampling value (even if they are just 6), to make the system scalable we will use the incremental mean.

//define how many temperature values to collect before calculating the mean ( sampling every 5 minutes and wanting to send the mean after 30 min, I will have a window of 6 values.)
#define TEMPERATURE_BUFFER_LENGTH 6
//keep track of the total number of values until now collected respect the last update
static unsigned short int number_of_collected_samples = 0;

//timer for sampling temperature TODO: CAMBIARE  (DOVREI CAMPIONARE OGNI 5 MINUTI INVECE CHE 5 SECONDI)
#define SAMPLING_TIMER_PERIOD (CLOCK_SECOND * 5)
static struct etimer sampling_timer;

//utility function --> from float to string
static const char* from_float_to_string(unsigned short int c, double f){

	double i,fr; //integer and fractionar part

	fr = modf((double)f,&i);

    	char *buf = malloc(c+1); //+1 to count also the dot point
  
    	snprintf(buf, c+1, "%d.%d", (int)i, (int)(fr*100));

	return buf;
}

//publish new temperature
static void notify_new_temperature(){

            //indico che il topic del messaggio è "temperature"
	    sprintf(pub_topic, "%s", "temperature");
	    
	    //prepare json message
	    char json_message[200];

	    //compute delta time from time_base
	    unsigned long t = clock_seconds() - base_time;

            sprintf(json_message, "{ \"idBuilding\": %d, \"idRoom\": %d,  \"idDevice\": %d,  \"temperature\": %s,  \"timestamp\": %ld }", ID_BUILDING, ID_ROOM, ID_DEVICE, from_float_to_string(6,(double)temperature), t);

	    printf("Notify client about the new temperature: %s\n", json_message);

	    sprintf(app_buffer, "%s", json_message);

	    //reset
	    number_of_collected_samples = 0;
	   
	    //pubblico	
	    mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
            strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
	
}


//function to compute the new temperature, that is the mean of the values sampled up to now
static void update_mean_temperature(float last_sample){

	printf("Compute new mean temperature\n");

	//incremental mean 
	temperature = temperature + (float)((float) 1/(TEMPERATURE_BUFFER_LENGTH) ) * (float)(last_sample - temperature);

}



//simulate new temperature
static void simulate_new_temperature_sampling(){

	 //keep track only of the last sample
	 float last_sample = -1.0;

	 //to add or to remove from the current temperature (between 0 and 1)
    	 float v = (float)(random_rand() % 10 ) / 10;

	 //generate a choice, to add or to remove
	 unsigned short int choice = ( unsigned short int) random_rand() & 1;

	if( choice == 0) 
		last_sample = temperature + v;
	else
		last_sample = temperature - v;

	
	if (last_sample < MIN_TEMPERATURE)
		last_sample = MIN_TEMPERATURE;
	if (last_sample > MAX_TEMPERATURE)
		last_sample = MAX_TEMPERATURE;

	
	printf("New temperature sampled: %s\n", from_float_to_string(4,(double)last_sample));

	number_of_collected_samples = number_of_collected_samples + 1;

	if(number_of_collected_samples > TEMPERATURE_BUFFER_LENGTH ){

		//reset
		number_of_collected_samples = 0;
	}

	//update temperature
	update_mean_temperature(last_sample);
	
}





//una volta sottoscritto al topic "actuator_temperature_ID_BUILDING_ID_ROOM" verrà chiamata questa callback ogni volta che un nuovo messaggio è stato pubblicato
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{

 

  if(strcmp(topic, topic_of_interest) == 0) {
	 
  	float new_temperature = atof((char*)chunk);

	//if new setting is in a correct range of [18, 30] C°, accept the new temperature
	if ( new_temperature >= MIN_TEMPERATURE && new_temperature <= MAX_TEMPERATURE ) {

		temperature = new_temperature;

		change_LED_Band();

		printf("New command received (accepted): set the temperature to %s C °\n", from_float_to_string(4,(double)new_temperature));
	}else
		printf("New command received (rejected): impossible to set the temperature to %s C ° because it's outside the allowed range. \n", from_float_to_string(4,(double)new_temperature));
  }

  //if the topic is for synchronizazion
  if(strcmp(topic, topic_for_synchronization) == 0) {

	base_time = clock_seconds(); //reset reference time


	printf("Time synchronized to a base time of %lu \n", base_time);
  }

    return;
}


//funzione che viene chiamata ogni volta che succede qualcosa con l'MQTT server 
// es. disconnessione, errore, pubblicazione riuscita, sottoscrizione riusciuta, ricezione mess
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {

  //nel caso si connette all'MQTT server
  case MQTT_EVENT_CONNECTED: {
    printf("Application has a MQTT connection\n");

    state = STATE_CONNECTED;
    break;
  }

  //nel caso si improvvisa disconnessione dall'MQTT server
  case MQTT_EVENT_DISCONNECTED: {
    printf("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    //riavvio il processo iniziale del sensore cosi da ritentare la connessione
    process_poll(&mqtt_client_temperature);
    break;
  }

  //nel caso in cui il Broker ci notifica di un nuovo messaggio circa un topic a cui ci siamo iscritti
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    //semplicemente stampa il contenuto di tale messaggio con dettagli come topic, lunghezza... (oppure nel caso della sincronizzazione, inizializza il base time).
    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic),
                msg_ptr->payload_chunk, msg_ptr->payload_length);

    break;
  }

  //nel caso in cui il Broker ci notifica di esserci sottoscritti a un topic con successo o meno
  case MQTT_EVENT_SUBACK: {
#if MQTT_311
    mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

    //sottoscrizione avvenuta con successo
    if(suback_event->success) {
      printf("Application is subscribed to topic successfully\n");
    //sottoscrizione fallita
    } else {
      printf("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
    }
#else
    //sottoscrizione avvenuta con successo
    printf("Application is subscribed to topic successfully\n");
#endif
    break;
  }

  //il Broker ci notifica di aver cancellato la nostra sottoscrizione al topic con successo
  case MQTT_EVENT_UNSUBACK: {
    printf("Application is unsubscribed to topic successfully\n");
    break;
  }

  //il Broker ci notifica di aver pubblicato il messaggio riguardante un topic con successo
  case MQTT_EVENT_PUBACK: {
    printf("Publishing complete.\n");
    break;
  }
  default:
    printf("Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}

//funzione che testa se sono connesso 
static bool
have_connectivity(void)
{
  //ottengo l'indirizzo IP globale del nodo. Se non esiste allora non esiste connessione
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL ||
     uip_ds6_defrt_choose() == NULL) {
    printf("Non c'è alcuna connessione\n");
    return false;
  }
  
  printf("Sei connesso\n");
  return true;
}




//INIZIO DEL PROCESSO PRINCIPALE
PROCESS_THREAD(mqtt_client_temperature, ev, data)
{

  //avvio il processo
  PROCESS_BEGIN();
  

 //Inizializzo il client_id sfruttando l'unicita (nella rete) del MAC del device
  snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  // Inizializzo l'MQTT Engine					 
  mqtt_register(&conn, &mqtt_client_temperature, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);

  //quando il sensore si accende, inizializzo lo stato pari a STATE_INIT
  state=STATE_INIT;
				    
  // Imposto un timer, che parte subito, per controllare ogni 500ms lo stato del sensore
  etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);

  // Imposto un timer, che invece simula il campionamento della temperatura (che qui simulo)
  etimer_set(&sampling_timer, SAMPLING_TIMER_PERIOD);

  printf("MQTT Server for monitoring temperature activated. \n");

  //set the topic of interest for actuator
  sprintf(topic_of_interest, "temperature_actuator_%d_%d", ID_BUILDING, ID_ROOM);

  //set the topic of interest for actuator
  sprintf(topic_for_synchronization, "time_synch");

  /* Main loop */
  while(1) {

    PROCESS_YIELD();

    //se l'evento che mi ha svegliato è il classico controllo dello stato ogni 500ms
    //oppure è un evento di polling (fatto quando l'MQTT server mi invia una disconnessione...
    if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || 
	      ev == PROCESS_EVENT_POLL){
			  
		  //se il mio stato è quello iniziale			  
		  if(state==STATE_INIT){
		         //tento intanto di capire se ho una connessione di rete
			 if(have_connectivity()==true)  
				 state = STATE_NET_OK;
		  } 

		  //se possiedo una connessione di rete...
		  if(state == STATE_NET_OK){
			  // Tento di connettermi all'MQTT server...
			  printf("Connecting to MQTT server!\n");
			  
			  //inizializzo l'indirizzo del broker con quello definito (broker_ip)
			  memcpy(broker_address, broker_ip, strlen(broker_ip));
			  
			  //avvio la connessione all'MQTT server
			  //Quando sarà connesso, verrà chiamata la mqtt_event callback che setterà lo stato a STATE_CONNECTED
			  mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT,
						   DEFAULT_PUBLISH_INTERVAL, //TODO: CAMBIA!
						   MQTT_CLEAN_SESSION_ON);
			  
			  //passo a uno stato di "Tentata connessione all'MQTT server..."
			  state = STATE_CONNECTING;
		  }

		  //Se sono attualmente connesso all'MQTT server...
		  if(state==STATE_CONNECTED){
		  
			  // Mi sottoscrivo al topic actuator
			  strcpy(sub_topic, topic_of_interest);

			  status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);

			  printf("Subscription in progress...\n");

			  //se il Broker mi ha inviato uno stato di errore... 
			  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
				LOG_ERR("Tried to subscribe but command queue was full!\n");
				PROCESS_EXIT();
			  }
			  
			  printf("Subscription to %s topic was successful.\n", topic_of_interest);
			  //indico di essere passato a uno stato di Sottoscrizione al topic
			  state = STATE_SUBSCRIBED;
		  }

		//se sono iscritto al topic "actuator_temperature"....
		if(state == STATE_SUBSCRIBED){
		    //allora posso passare a sottoscrivermi al topic di sincronizzazione

	   	  // Mi sottoscrivo al topic "time_synch"
		  strcpy(sub_topic_synch, topic_for_synchronization);

		  status = mqtt_subscribe(&conn, NULL, sub_topic_synch, MQTT_QOS_LEVEL_0);

		  printf("Subscription for synchronization in progress...\n");

		  //se il Broker mi ha inviato uno stato di errore... 
		  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
			LOG_ERR("Tried to subscribe but command queue was full!\n");
			//PROCESS_EXIT();
		  }else {
		  
			  printf("Subscription to %s topic was successful.\n", topic_for_synchronization);
			  //indico di essere passato a uno stato di Sottoscrizione al topic
			  state = STATE_SUBSCRIBED_SYNCH;
		  }
		}

		//se sono iscritto al topic "time_synch"....
		if(state == STATE_SUBSCRIBED_SYNCH){
		    //allora posso passare alla pubblicazione dei miei messaggi

		   //se il numero di campioni collezionati è sufficiente, notificare il client (faccio un check ulteriore, per sicurezza, sul base time)
		   if (number_of_collected_samples == TEMPERATURE_BUFFER_LENGTH && base_time!=-1)
		   	notify_new_temperature();
		

		//se invece il mio stato è disconnesso (dal Broker)...
		} else if ( state == STATE_DISCONNECTED ){
		   LOG_ERR("Disconnected form MQTT broker\n");	
		   // Recover from error
                   state=STATE_INIT;
		}
		

	        //resetto il timer per avere un successivo controllo dello stato dopo 500ms
		etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
      
    }

    //se il timer del sampling è scaduto, campionare
    if(etimer_expired(&sampling_timer)){

	simulate_new_temperature_sampling();

	change_LED_Band();

	etimer_reset(&sampling_timer);
    }

  }

	

  PROCESS_END();
}



