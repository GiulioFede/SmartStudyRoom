package iet.unipi.it.smartStudyRoom.ui;

import java.awt.Color;
import java.awt.Dimension;

import javax.swing.BoxLayout;
import javax.swing.JPanel;

import iet.unipi.it.smartStudyRoom.temperature.PublisherTemperature;

public class RightSidePanel extends JPanel {
	
	private TemperaturePanel temperaturePanel;
	private LoudnessPanel loudnessPanel;
	PublisherTemperature pt;
	
	RightSidePanel(PublisherTemperature pt_){
		
		pt = pt_;
		
		Dimension dimension = getPreferredSize();
		dimension.width = 500;
		dimension.height = 800;
		
		setPreferredSize(dimension);
		
		//setto sfondo
		setBackground(Color.orange);
		
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		
		temperaturePanel = new TemperaturePanel(pt);
		loudnessPanel = new LoudnessPanel();
		
		add(temperaturePanel);
		add(loudnessPanel);
		
		
		
	}

	public void changeTemperatureValue(double new_value) {
		temperaturePanel.changeTemperatureValue(new_value);
	}
	
	public void updateLoudnessValue(double new_value) {
		loudnessPanel.updateLoudnessValue(new_value);
	}
}
