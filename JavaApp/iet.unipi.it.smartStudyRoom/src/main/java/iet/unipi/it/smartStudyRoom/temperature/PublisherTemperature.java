package iet.unipi.it.smartStudyRoom.temperature;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;


public class PublisherTemperature {
	final String idBuilding = "1";
	final String idRoom = "1";
	String topic = "temperature_actuator"+"_"+idBuilding+"_"+idRoom;
	//String content = "Message From Client";
	String broker = "tcp://127.0.0.1:1883";
	String clientId = "temperatureActuator";
	MqttClient client;
	double collectTemperature = 0.0;
	int counterTemperature = 0;
	
	public PublisherTemperature(){
		try {
			client = new MqttClient(broker, clientId);
			client.connect();
			System.out.println("Connection OK");
			
		}catch(MqttException em) {
			em.printStackTrace();
		}
	}
	
	public void publishMessage(String content) throws MqttException{
		MqttMessage message = new MqttMessage(content.getBytes());
		System.out.println("Message Created");
		
		client.publish(topic, message);
		System.out.println("Published the message");
		
	}
	
	public void collectValuationTemperature(double temperature) throws MqttException{
		
		System.out.println("Nuovo suggerimento memorizzato.");
		
		counterTemperature++;
		collectTemperature += temperature;
		
		if(counterTemperature >= 10) {
			String meanTemperature = Double.toString((double)(collectTemperature/counterTemperature));
			
			publishMessage(meanTemperature);
			
			counterTemperature = 0;
			collectTemperature = 0.0;
			
			System.out.println("Nuova temperatura "+(double)(collectTemperature/counterTemperature)+" C° inviata.");
		}
		
		
	}

}
