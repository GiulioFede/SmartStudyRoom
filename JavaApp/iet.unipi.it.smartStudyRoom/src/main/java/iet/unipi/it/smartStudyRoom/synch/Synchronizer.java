package iet.unipi.it.smartStudyRoom.synch;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;


public class Synchronizer {
	String topic = "time_synch";
	String broker = "tcp://127.0.0.1:1883";
	String clientId = "synch";
	MqttClient client;
	double collectTemperature = 0.0;
	int counterTemperature = 0;
	
	public Synchronizer(){
		try {
			client = new MqttClient(broker, clientId);
			client.connect();
			System.out.println("Synch: connection OK");
			
			startSynchronization();
			
		}catch(MqttException em) {
			em.printStackTrace();
		}
	}
	
	public void startSynchronization() {
		//just a symbolic message
		String content = "1";
		
		MqttMessage message = new MqttMessage(content.getBytes());
		System.out.println("Synch: Message Created");
		
		try {
			client.publish(topic, message);
			System.out.println("Publishing Synchronization...");
		} catch (MqttPersistenceException e) {
			e.printStackTrace();
		} catch (MqttException e) {
			e.printStackTrace();
		}
		
	}
	

}
