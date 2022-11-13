package iet.unipi.it.smartStudyRoom.temperature;

import java.sql.Timestamp;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.JSONException;
import org.json.JSONObject;

import iet.unipi.it.smartStudyRoom.DatabaseConnection;
import iet.unipi.it.smartStudyRoom.ui.MainFrame;


public class SubscriberTemperature implements MqttCallback{
	String topic = "temperature";
	String broker = "tcp://127.0.0.1:1883";
	String clientId = "monitoringTemperature";
	
	MainFrame my_window;
	long time_reference;
	
	public SubscriberTemperature(MainFrame my_window_,long time_reference_) throws MqttException{
		
		my_window = my_window_;
		time_reference = time_reference_;
		
		MqttClient client = new MqttClient(broker, clientId);
		client.setCallback(this);
		client.connect();
		System.out.println("Connect to the broker");
		
		client.subscribe(topic);
		System.out.println("Subscribe to the topic: "+topic);
	}
	
	public void connectionLost(Throwable cause) {
		// TODO Auto-generated method stub
		
	}

	public void messageArrived(String topic, MqttMessage message) throws Exception {
		
		try {
			
			// TODO Auto-generated method stub
			System.out.println(String.format("[%s] %s", topic, new String(message.getPayload())));
			
			JSONObject jsonTemperature = new JSONObject(new String(message.getPayload()));
			
			//inserire possibili controlli sui campi che arrivano dal device
			Timestamp timestampDato= new Timestamp(1000*(long)jsonTemperature.getLong("timestamp")+time_reference);
			long now = System.currentTimeMillis();
			Timestamp timestampJava= new Timestamp(now);
			System.out.println("SqlTimestampDato          : " + timestampDato);
			System.out.println("SqlTimestampJava          : " + timestampJava+"   valueTime: "+now);
			
			DatabaseConnection.addValueTemperature(timestampDato,
												  (int)jsonTemperature.getInt("idBuilding"),
												  (int)jsonTemperature.getInt("idRoom"),
												  (int)jsonTemperature.getInt("idDevice"),
												  (double)jsonTemperature.getDouble("temperature"));
			
			my_window.changeTemperatureValue(jsonTemperature.getDouble("temperature"));
			
		}catch(JSONException e) {
				System.out.println("Actuator sent a message:"+new String(message.getPayload()));
			}
	}

	public void deliveryComplete(IMqttDeliveryToken token) {
		// TODO Auto-generated method stub
		
	}

}
