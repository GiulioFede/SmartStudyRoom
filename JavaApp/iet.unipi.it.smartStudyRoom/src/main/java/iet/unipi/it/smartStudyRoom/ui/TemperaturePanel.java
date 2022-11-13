package iet.unipi.it.smartStudyRoom.ui;

import java.awt.Color;
import java.awt.event.*;
import java.net.URL;
import java.awt.Dimension;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;

import org.eclipse.paho.client.mqttv3.MqttException;

import iet.unipi.it.smartStudyRoom.temperature.PublisherTemperature;

public class TemperaturePanel extends JPanel {
	
	JLabel temperature_image_label;
	JLabel title;
	JLabel temperature_label;
	float value = 0;
	
	JButton suggestion1;
	JButton suggestion2;
	JButton suggestion3;
	JLabel setl;
	
	int left_value, center_value,right_value;
	
	PublisherTemperature pt;

	TemperaturePanel(PublisherTemperature pt_){
		
		pt = pt_;
		
		Dimension dimension = getPreferredSize();
		dimension.width = 500;
		dimension.height = 550;

		setBackground(Color.white);
		
		setPreferredSize(dimension);

		setLayout(null);
	
		ImageIcon temperature_image = new ImageIcon(getClass().getResource("images/temperature/yellow.png"));

		setBorder(BorderFactory.createMatteBorder(0, 0, 1, 0, Color.BLACK));

		title= new JLabel("Temperature");
		title.setBounds(0, 0, 400, 400);
		title.setFont (title.getFont ().deriveFont (40.0f));

		temperature_image_label = new JLabel(temperature_image);
		temperature_image_label.setBounds(0, 0, 200, 400);

		temperature_label= new JLabel("-C°");
		temperature_label.setBounds(0, 0, 200, 400);
		temperature_label.setFont (temperature_label.getFont ().deriveFont (40.0f));
		
		//3 scelte
		suggestion1= new JButton("...");
		suggestion1.setFocusPainted(false);
		suggestion1.setBounds(0, 0, 100, 100);
		suggestion1.setFont (suggestion1.getFont ().deriveFont (20.0f));
		suggestion1.setOpaque(true);
		suggestion1.setBackground(new Color(255,255,255));

		suggestion2= new JButton("...");
		suggestion2.setFocusPainted(false);
		suggestion2.setBounds(0, 0, 100, 100);
		suggestion2.setFont (suggestion2.getFont ().deriveFont (20.0f));
		suggestion2.setOpaque(true);
		suggestion2.setBackground(new Color(255,255,255));
		
		suggestion3= new JButton("...");
		suggestion3.setFocusPainted(false);
		suggestion3.setBounds(0, 0, 100, 100);
		suggestion3.setFont (suggestion3.getFont ().deriveFont (20.0f));
		suggestion3.setOpaque(true);
		suggestion3.setBackground(new Color(255,255,255));
		
		setl= new JLabel("Set your preferred temperature",SwingConstants.CENTER);
		setl.setBounds(0, 0, 500, 100);
		setl.setFont (title.getFont ().deriveFont (17.0f));
		setl.setForeground(new Color(0,154,255));

		add(setl);
		add(temperature_image_label);
		add(temperature_label);
		add(title);
		add(suggestion1);
		add(suggestion2);
		add(suggestion3);


		temperature_image_label.setLocation(100, 40);
		temperature_label.setLocation(300, 40);	
		suggestion1.setLocation(70, 420);
		suggestion2.setLocation(200, 420);
		suggestion3.setLocation(330, 420);	
		title.setLocation(140, -150);
		setl.setLocation(0,350);
		
	}
	
	public void changeTemperatureValue(double new_value) {
		System.out.println("UPDATING UI: change temperature displayed.");
		  
		remove(temperature_label);
		remove(temperature_image_label);
		remove(suggestion1);
		remove(suggestion2);
		remove(suggestion3);
		
		String color_t = "yellow";
		
		if(new_value<=19) {
			color_t = "blue";
		}
		else if(new_value <=22) {
			color_t = "yellow";
		}
		else if(new_value<=26) {
			color_t = "green";
		}
		else {
			color_t = "red";
		}
		
		Triad []triad;
		
		//ritorna la terna di colori corretta per le alternative
		triad = getTriad(color_t);
		
		ImageIcon temperature_image = new ImageIcon(getClass().getResource("images/temperature/"+color_t+".png"));
		temperature_image_label = new JLabel(temperature_image);
		temperature_image_label.setBounds(0, 0, 200, 400);
		
		temperature_label= new JLabel(new_value+" C°");
		temperature_label.setBounds(0, 0, 200, 400);
		temperature_label.setFont (temperature_label.getFont().deriveFont (40.0f));
		
		left_value = (int)((new_value-2<18)?18:(new_value-2));
		center_value = (int)((new_value-1<18)?18:(new_value-1));
		right_value = (int)((new_value+1>30)?30:(new_value+1));
		
		//3 scelte
		suggestion1= new JButton(left_value+"C°");
		suggestion1.setFocusPainted(false);
		suggestion1.setBounds(0, 0, 100, 100);
		suggestion1.setFont (suggestion1.getFont ().deriveFont (20.0f));
		suggestion1.setOpaque(true);
		suggestion1.setBackground(new Color(triad[1].r,triad[1].g,triad[1].b));

		suggestion2= new JButton(center_value+"C°");
		suggestion2.setFocusPainted(false);
		suggestion2.setBounds(0, 0, 100, 100);
		suggestion2.setFont (suggestion2.getFont ().deriveFont (20.0f));
		suggestion2.setOpaque(true);
		suggestion2.setBackground(new Color(triad[0].r,triad[0].g,triad[0].b));
		
		suggestion3= new JButton(right_value+"C°");
		suggestion3.setFocusPainted(false);
		suggestion3.setBounds(0, 0, 100, 100);
		suggestion3.setFont (suggestion3.getFont ().deriveFont (20.0f));
		suggestion3.setOpaque(true);
		suggestion3.setBackground(new Color(triad[2].r,triad[2].g,triad[2].b));
		
		add(temperature_label);
		add(temperature_image_label);
		add(suggestion1);
		add(suggestion2);
		add(suggestion3);
		
		
		temperature_image_label.setLocation(100, 40);
		temperature_label.setLocation(300, 40);
		suggestion1.setLocation(70, 420);
		suggestion2.setLocation(200, 420);
		suggestion3.setLocation(330, 420);	
		
		suggestion1.addActionListener(new ActionListener(){  

			public void actionPerformed(ActionEvent e){  
						try {
							pt.collectValuationTemperature((double)left_value);
						} catch (MqttException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}  
				}); 
		
		suggestion2.addActionListener(new ActionListener(){  

			public void actionPerformed(ActionEvent e){  
						try {
							pt.collectValuationTemperature((double)center_value);
						} catch (MqttException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}  
				}); 
		
		suggestion3.addActionListener(new ActionListener(){  

			public void actionPerformed(ActionEvent e){  
						try {
							pt.collectValuationTemperature((double)right_value);
						} catch (MqttException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}  
				}); 
		
		revalidate();
		repaint();
	}
	
	class Triad {
		int r;
		int g;
		int b;
	};
	
	Triad[] getTriad(String color) {
		
		Triad t1 = new Triad();
		Triad t2 = new Triad();
		Triad t3 = new Triad();
		
		
		if(color == "blue") {
			t1.r = 0;
			t1.g = 60;
			t1.b = 255;
			
			t2.r = 77;
			t2.g = 113;
			t2.b = 255;
			
			t3.r = 0;
			t3.g = 43;
			t3.b = 230;
		}
		else if(color == "yellow") {
			t1.r = 255;
			t1.g = 230;
			t1.b = 0;
			
			t2.r = 255;
			t2.g = 230;
			t2.b = 81;
			
			t3.r = 212;
			t3.g = 191;
			t3.b = 0;
		}
		else if(color == "green") {
			t1.r = 50;
			t1.g = 255;
			t1.b = 0;
			
			t2.r = 11;
			t2.g = 255;
			t2.b = 82;
			
			t3.r = 36;
			t3.g = 215;
			t3.b = 0;
		}
		else {
			t1.r = 255;
			t1.g = 60;
			t1.b = 0;
			
			t2.r = 255;
			t2.g = 11;
			t2.b = 67;
			
			t3.r = 219;
			t3.g = 51;
			t3.b = 0;
		}
		
		Triad[] t = {t1,t2,t3};
		
		return t;
	}

}
