
#include "contiki.h"

//librerie routing RPL
#include "net/routing/routing.h"
#include "net/netstack.h"

#include "net/ipv6/uip-debug.h"

//librerie UDP
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

//definisco la porta su cui il server riceve i pacchetti
#define UDP_PORT_SERVER 8765
//definisco la porta su cui i client inviano i pacchetti
#define UDP_PORT_CLIENT 5678

//definisco la connessione udp
static struct simple_udp_connection connection;

PROCESS(server, "Server");
AUTOSTART_PROCESSES(&server);



//callback da chiamare ogni volta che il Server riceve un pacchetto
static void callback_ricezione_pacchetto(struct simple_udp_connection *c,
					 const uip_ipaddr_t *indirizzo_mittente,
					 uint16_t porta_mittente,
					 const uip_ipaddr_t *indirizzo_ricevitore,
					 uint16_t porta_ricevitore,
					 const uint8_t *dati_ricevuti,
					 uint16_t lunghezza_dati_ricevuti ){

	printf("Dati ricevuti da \n");
	uip_debug_ipaddr_print(indirizzo_mittente);
	printf("sulla porta %d dalla porta %d \n", porta_ricevitore, porta_mittente);
}


PROCESS_THREAD(server, ev, data)
{ 
  
  PROCESS_BEGIN();

  //Indico che tale nodo è il root. Questo implicherà l'invio dei DAG message per costruire il DAG
  NETSTACK_ROUTING.root_start();

  #if BORDER_ROUTER_CONF_WEBSERVER
    PROCESS_NAME(webserver_nogui_process);
    process_start(&webserver_nogui_process, NULL);
  #endif

  //avvio il sottoprocesso figlio che sarà sempre attivo in attesa di ricevere pacchetti
  simple_udp_register(&connection, UDP_PORT_SERVER, NULL, UDP_PORT_CLIENT, callback_ricezione_pacchetto);

  //adesso posso anche morire, tanto la funzione sopra ha creato un processo figlio sempre attivo
  PROCESS_END();
}



















