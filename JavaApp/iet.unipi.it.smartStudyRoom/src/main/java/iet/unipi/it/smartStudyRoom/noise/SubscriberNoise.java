package iet.unipi.it.smartStudyRoom.noise;

import java.sql.Timestamp;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.JSONObject;

import iet.unipi.it.smartStudyRoom.DatabaseConnection;
import iet.unipi.it.smartStudyRoom.ui.MainFrame;

public class SubscriberNoise implements MqttCallback{
	final String highValue = "HIGH";
	final String mediumValue = "MEDIUM";
	final String lowValue = "LOW";
	final int tresholdLowMedium = 45;
	final int tresholdMediumHigh = 60;
	
	String topic = "noise";
	String broker = "tcp://127.0.0.1:1883";
	String clientId = "monitoringNoise";
	
	MainFrame my_window;
	long time_reference;
	
	public SubscriberNoise(MainFrame my_window_, long time_reference_) throws MqttException{
		
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
		// TODO Auto-generated method stub
		System.out.println(String.format("[%s] %s", topic, new String(message.getPayload())));
		JSONObject jsonNoise = new JSONObject(new String(message.getPayload()));
		
		double noiseValue = jsonNoise.getDouble("value");
		String noiseLevel;
		if(noiseValue < tresholdLowMedium)
			noiseLevel = new String(lowValue);
		else if (noiseValue >= tresholdLowMedium && noiseValue < tresholdMediumHigh)
			noiseLevel = new String(mediumValue);
		else
			noiseLevel = new String(highValue);
		
		Timestamp timestampDato= new Timestamp(1000*(long)jsonNoise.getLong("timestamp")+time_reference);
		long now = System.currentTimeMillis();
		Timestamp timestampJava= new Timestamp(now);
		System.out.println("SqlTimestampDato          : " + timestampDato);
		System.out.println("SqlTimestampJava          : " + timestampJava+"   valueTime: "+now);
		
		
		DatabaseConnection.addValueNoise(timestampDato,
									  	(int)jsonNoise.getInt("idBuilding"),
									  	(int)jsonNoise.getInt("idRoom"),
									  	(int)jsonNoise.getInt("idTable"),
									  	noiseValue, noiseLevel);
		
		my_window.updateLoudnessValue(noiseValue);
	}

	public void deliveryComplete(IMqttDeliveryToken token) {
		// TODO Auto-generated method stub
		
	}
}
