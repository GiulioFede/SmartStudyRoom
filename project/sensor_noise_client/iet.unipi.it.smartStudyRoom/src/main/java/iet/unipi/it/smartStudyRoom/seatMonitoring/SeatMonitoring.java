package iet.unipi.it.smartStudyRoom.seatMonitoring;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.OptionSet;
import org.json.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;

public class SeatMonitoring extends CoapClient {
	
	
	
	CoapClient client = new CoapClient("coap://[fd00::204:4:4:4]:5683/seat");
	private JSONObject seat;
	public SeatMonitoring(){
		
		CoapHandler ch = new CoapHandler() {
			 		public void onLoad(CoapResponse response) {
			 			String content = response.getResponseText();
			 			///try {
							//seat = (JSONObject)JSONValue.parseWithException(content);
			 				seat = new JSONObject(content);
						//} catch (ParseException e) {
							//e.printStackTrace();
						//}
			 			System.out.println("*******************seat info: "+ seat.getInt("state"));
			 			//System.out.println(content);
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
