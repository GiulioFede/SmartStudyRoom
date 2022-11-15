package iet.unipi.it.smartStudyRoom.seatMonitoring;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.OptionSet;
import org.json.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;

import iet.unipi.it.smartStudyRoom.ui.MainFrame;

public class SeatMonitoring extends CoapClient {
	
	
	
	CoapClient client;
	private JSONObject seat;
	MainFrame my_window;
	int id;
	
	static int[] state = { 1,1,1,1 };//condivisa tra tutti
	
	public SeatMonitoring(MainFrame my_window_, String serverAddress, int id_){
		
		my_window = my_window_;
		id = id_;
		
		client = new CoapClient(serverAddress);
		
		
		CoapHandler ch = new CoapHandler() {
			 		public void onLoad(CoapResponse response) {
			 			String content = response.getResponseText();
			 			System.out.println(content+ "   len="+ content.length());
			 			seat = new JSONObject(content);
			 			state[id-4]=seat.getInt("state");
			 			my_window.changeStateTable(state);
			 		}
			 		
			 		public void onError() {
			 			System.err.println("-Failed--------");
			 		}
				};
		
		
	
		CoapObserveRelation relation = client.observe(ch);
		
	}
	
	public JSONObject returnInfoSeat() {
		return seat;
	}
	
}
