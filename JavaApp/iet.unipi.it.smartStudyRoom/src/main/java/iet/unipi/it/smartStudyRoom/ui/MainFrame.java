package iet.unipi.it.smartStudyRoom.ui;

import java.awt.BorderLayout;

import javax.swing.BorderFactory;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.WindowConstants;

import iet.unipi.it.smartStudyRoom.temperature.PublisherTemperature;

import java.awt.Dimension;

public class MainFrame extends JFrame {
	
	private SeatPanel seatPanel;
	private RightSidePanel rightSidePanel;
	PublisherTemperature pt;

	public MainFrame(PublisherTemperature pt_) {
	
		super("Smart Study Room Dashboard");
		pt = pt_;
		
		//quando clicco sulla X, termina l'intero processo
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		//imposto dimensioni finestra
		setSize(1300,800);
		
		//impedisco il resize della finestra
		setResizable(false);
		
		//la pongo al centro
		setLocationRelativeTo(null);
		
		setLayout(new BorderLayout());
		
		rightSidePanel = new RightSidePanel(pt);
		
		seatPanel = new SeatPanel();
		//aggiungo primo pannello --> per mostrare il tavolo
		add(seatPanel, BorderLayout.LINE_START);
		
		//aggiungo secondo pannello --> per mostrare la temperatura
		add(rightSidePanel, BorderLayout.CENTER);
		
		setVisible(true);

	}
	
	public void changeStateTable(int[] state) {
		seatPanel.changeState(state);
	}
	
	public void changeTemperatureValue(double new_value) {
		rightSidePanel.changeTemperatureValue(new_value);
	}
	
	public void updateLoudnessValue(double new_value) {
		rightSidePanel.updateLoudnessValue(new_value);
	}
}
