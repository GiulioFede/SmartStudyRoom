package iet.unipi.it.subscriber;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class MyClient implements MqttCallback{
	String topic = "alert";
	String broker = "tcp://127.0.0.1:1883";
	String clientId = "Java App";
	
	public MyClient() throws MqttException{
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
		// TODO Auto-generated method stub
		System.out.println(String.format("[%s] %s", topic, new String(message.getPayload())));
		
	}

	public void deliveryComplete(IMqttDeliveryToken token) {
		// TODO Auto-generated method stub
		
	}
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try {
			MyClient c = new MyClient();
		}catch(MqttException em) {
			em.printStackTrace();
		}
	}


}
