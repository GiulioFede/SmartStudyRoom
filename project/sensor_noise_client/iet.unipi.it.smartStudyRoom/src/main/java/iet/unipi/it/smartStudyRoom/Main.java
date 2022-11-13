package iet.unipi.it.smartStudyRoom;

import org.eclipse.paho.client.mqttv3.MqttException;

import iet.unipi.it.smartStudyRoom.noise.SubscriberNoise;
import iet.unipi.it.smartStudyRoom.seatMonitoring.SeatMonitoring;
import iet.unipi.it.smartStudyRoom.temperature.PublisherTemperature;
import iet.unipi.it.smartStudyRoom.temperature.SubscriberTemperature;



public class Main {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		try {
			
			//create database and tables
			DatabaseConnection.CreateDatabase();
			DatabaseConnection.CreateTemperatureTable();
			DatabaseConnection.CreateNoiseTable();
			
			//subscribe to the topics
			SubscriberTemperature st = new SubscriberTemperature();
			SubscriberNoise sn = new SubscriberNoise();
			
			PublisherTemperature pt = new PublisherTemperature();
			SeatMonitoring sm = new SeatMonitoring();
			
		}
		catch(MqttException em) {
			em.printStackTrace();
		}
		catch(Exception e) {
			e.printStackTrace();
		}

	}

}
