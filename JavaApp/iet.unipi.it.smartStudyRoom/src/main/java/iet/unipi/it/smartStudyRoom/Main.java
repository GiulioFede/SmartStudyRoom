package iet.unipi.it.smartStudyRoom;

import org.eclipse.paho.client.mqttv3.MqttException;

import iet.unipi.it.smartStudyRoom.noise.SubscriberNoise;
import iet.unipi.it.smartStudyRoom.seatMonitoring.SeatMonitoring;
import iet.unipi.it.smartStudyRoom.temperature.PublisherTemperature;
import iet.unipi.it.smartStudyRoom.temperature.SubscriberTemperature;


import iet.unipi.it.smartStudyRoom.ui.MainFrame;
import iet.unipi.it.smartStudyRoom.synch.Synchronizer;

public class Main {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		String pir1addr = "coap://[fd00::212:4b00:f24:4]:5683/seat";
		String pir2addr = "coap://[fd00::f6ce:3636:5325:98f8]:5683/seat";
		String pir3addr = "coap://[fd00::f6ce:36ed:babb:5620]:5683/seat";

		
		//get reference time for synchronization
		long time_reference = System.currentTimeMillis();
		
		try {
			
			PublisherTemperature pt = new PublisherTemperature();
			MainFrame mainWindow = new MainFrame(pt);
			
			//create database and tables
			DatabaseConnection.CreateDatabase();
			DatabaseConnection.CreateTemperatureTable();
			DatabaseConnection.CreateNoiseTable();
			
			//subscribe to the topics
			SubscriberTemperature st = new SubscriberTemperature(mainWindow,time_reference);
			SubscriberNoise sn = new SubscriberNoise(mainWindow, time_reference);
			
			//send command to all the sensors (just temperature and noise) to synchronize with a common reference time
			Synchronizer synchronizer = new Synchronizer();
			
			
			//SeatMonitoring sm = new SeatMonitoring(mainWindow,"coap://[fd00::204:4:4:4]:5683/seat",4);
			SeatMonitoring sm = new SeatMonitoring(mainWindow,pir1addr,4);
			
			//SeatMonitoring sm2 = new SeatMonitoring(mainWindow,"coap://[fd00::205:5:5:5]:5683/seat",5);
			SeatMonitoring sm2 = new SeatMonitoring(mainWindow,pir2addr,5);
			
			//SeatMonitoring sm3 = new SeatMonitoring(mainWindow,"coap://[fd00::206:6:6:6]:5683/seat",6);
			SeatMonitoring sm3 = new SeatMonitoring(mainWindow,pir3addr,6);
			
		}
		catch(MqttException em) {
			em.printStackTrace();
		}
		catch(Exception e) {
			e.printStackTrace();
		}

	}

}
