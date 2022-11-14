#include "contiki.h"
#include "mqtt.h"
#include <string.h>
#include <strings.h>
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include <stdio.h>
#include "os/sys/log.h"
#include <stdlib.h>
#include "node-id.h"
#include "lib/random.h"
#include <math.h>

#define LOG_MODULE "mqtt-client"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif

static struct mqtt_connection conn;

// MQTT broker address
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Default config values
#define DEFAULT_BROKER_PORT 1883
#define SEND_INTERVAL (15 * CLOCK_SECOND)
#define SAMPLING_INTERVAL (2 * CLOCK_SECOND)

/* Various states */
static uint8_t state;
#define STATE_INIT 0 // Initial state
#define STATE_NET_OK 1 // Network is initialized
#define STATE_CONNECTING 2 // Connecting to MQTT broker
#define STATE_CONNECTED 3 // Connection successful
#define STATE_SUBSCRIBED 4 // Topics of interest subscribed
#define STATE_DISCONNECTED 5 // Disconnected from MQTT broker


PROCESS(mqtt_noise_sensor, "Noise Sensor MQTT");
AUTOSTART_PROCESSES(&mqtt_noise_sensor);


/* Maximum TCP segment size and length of IPv6 addresses*/
#define MAX_TCP_SEGMENT_SIZE 32
#define CONFIG_IP_ADDR_STR_LEN 64

//static struct mqtt_message *msg_ptr = 0;

// Buffers for Client ID and Topics.
#define BUFFER_SIZE 64
static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];
// Application level buffer

static float value = 0.0;

#define APP_BUFFER_SIZE 512
static char app_buffer[APP_BUFFER_SIZE];


static struct mqtt_message *msg_ptr = 0;

// Periodic timer to check the state of the MQTT client
#define STATE_MACHINE_PERIODIC (CLOCK_SECOND >> 1)
#define ID_BUILDING 1
#define ID_ROOM 1
#define ID_TABLE 1

#define MIN_RANDOM_VALUE_NOISE 15.0
#define MAX_RANDOM_VALUE_NOISE 100.0


static struct etimer periodic_timer;
static struct etimer sampling_timer;
static struct etimer publish_timer;
static long time_synch = -1; //time in milliseconds

static void pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk, uint16_t chunk_len){
	printf("Pub Handler: topic=%s \n",topic);

	if(strcmp(topic,"time_synch")==0){
		printf("Receive Time from the client\n");
		printf("%s \n", chunk);
		
		//char *ptr;
		
		time_synch = clock_seconds();
			
	}
	return;
}

static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data){
	switch(event) {
		case MQTT_EVENT_CONNECTED: {
			// Connection completed
			printf("Connection operation completed\n");			
			
			state = STATE_CONNECTED;
			break;
		}
		case MQTT_EVENT_DISCONNECTED: {
			// Disconnection occurred
			printf("Disconnection\n");
			
			state = STATE_DISCONNECTED;
			process_poll(&mqtt_noise_sensor);
			break;
		}
		case MQTT_EVENT_PUBLISH: {
			// Notification on a subscribed topic received 
			// Data variable points to the MQTT message received
			msg_ptr = data;
			pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk, msg_ptr->payload_length);
			break;
		}
		case MQTT_EVENT_SUBACK: {
			// Subscription successful
			#if MQTT_311
    		mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

    		if(suback_event->success) {
      			printf("Application is subscribed to topic successfully\n");
    		} else {
      			printf("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
    		}
			#else
    		printf("Application is subscribed to topic successfully\n");
			#endif
    		break;
		}
		case MQTT_EVENT_UNSUBACK: {
			// Subscription canceled
			printf("Application is unsubscribed to topic successfully\n");
    		break; 
		}
		case MQTT_EVENT_PUBACK: {
			// Publishing completed
			printf("Publication successfully\n");
    		break;
		}
		default:
			printf("Somenthing was wrong\n");
			break;
	}
			
}

static bool have_connectivity(void){
	if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL) {
		return false;
	}
	return true;
}

static const char* convert_float_to_string(float value){
	char *string_float = malloc(10);
	gcvt(value, 5, string_float);

	return string_float;
}

static float generate_random_noise_value(void){
	//srand(time(NULL));
	float random_value = (float)((random_rand())%((int)MAX_RANDOM_VALUE_NOISE));//( (float)(rand()) )/RAND_MAX; 
	//random_value*=(MAX_RANDOM_VALUE_NOISE - MIN_RANDOM_VALUE_NOISE); 
	random_value+= MIN_RANDOM_VALUE_NOISE;

	return random_value;
}

mqtt_status_t status;
static float mean_noise;
static uint16_t num_sampling;
char broker_address[CONFIG_IP_ADDR_STR_LEN];

PROCESS_THREAD(mqtt_noise_sensor, ev, data){

	PROCESS_BEGIN();
	printf("MQTT Client Process\n");
	// Initialize the ClientID as MAC address of the node
	snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
			linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
			linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
			linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

	// Broker registration
	mqtt_register(&conn, &mqtt_noise_sensor, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);

	
	mean_noise = 0;
	num_sampling = 0;
	state=STATE_INIT;
	etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
	etimer_set(&sampling_timer, SAMPLING_INTERVAL);
	etimer_set(&publish_timer, SEND_INTERVAL);
	while(1){
		PROCESS_YIELD();		

		if((ev == PROCESS_EVENT_TIMER && etimer_expired(&periodic_timer)) || ev == PROCESS_EVENT_POLL){
			if(state==STATE_INIT){
				if(have_connectivity()==true) 
					state = STATE_NET_OK;	
			}

			if(state == STATE_NET_OK){
				// Connect to MQTT server
				printf("Connecting!\n");
				memcpy(broker_address, broker_ip, strlen(broker_ip));
				mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT, (SEND_INTERVAL) / CLOCK_SECOND,
							MQTT_CLEAN_SESSION_ON);
				state = STATE_CONNECTING;
			}
			
			if(state==STATE_CONNECTED){
				// Subscribe to a topic
				strcpy(sub_topic,"time_synch");
				status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);
				printf("Subscribing to time_synch topic!\n");
				if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
					LOG_ERR("Tried to subscribe but command queue was full!\n");
					PROCESS_EXIT();
				}
				state = STATE_SUBSCRIBED;
			}

			if(state == STATE_SUBSCRIBED && time_synch >= 0){
				if(ev == PROCESS_EVENT_TIMER && etimer_expired(&sampling_timer)){ 
					mean_noise += generate_random_noise_value();
					num_sampling ++;
		
					printf("noise: %s, sampling: %d\n", convert_float_to_string(mean_noise), num_sampling);
					etimer_set(&sampling_timer, SAMPLING_INTERVAL);
				}
				
				// Publish something
				if(ev == PROCESS_EVENT_TIMER && etimer_expired(&publish_timer)){
					value = (float)(mean_noise/num_sampling);
					
					sprintf(pub_topic, "%s", "noise");

					double i,f;
					f = modf((double)value, &i);

					sprintf(app_buffer, "{\"timestamp\": %ld,\"idBuilding\": %d, \"idRoom\": %d, \"idTable\": %d, \"value\": %d.%d}", clock_seconds()-time_synch, ID_BUILDING, ID_ROOM, node_id, (int)i, (int)(f*100)/*convert_float_to_string(value)*/);
					
					mqtt_publish(&conn, NULL, pub_topic, (uint8_t*) app_buffer, strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
					num_sampling = 0;
					mean_noise = 0;
					etimer_set(&publish_timer, SEND_INTERVAL);
				}
				
			}

			if(state == STATE_DISCONNECTED){
				printf("Disconnected form MQTT broker, try to reconnecting\n");	
		   // Recover from error
                state=STATE_INIT;
			}
			etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);

		}
				
	}
	PROCESS_END();
	
}







