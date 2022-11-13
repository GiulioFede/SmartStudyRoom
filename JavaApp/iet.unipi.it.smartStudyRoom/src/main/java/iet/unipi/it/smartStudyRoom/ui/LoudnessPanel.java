package iet.unipi.it.smartStudyRoom.ui;

import java.awt.Color;
import java.awt.Dimension;

import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JPanel;

public class LoudnessPanel extends JPanel {
	
	JLabel loudness_label;
	JLabel title;

	LoudnessPanel(){
		
		Dimension dimension = getPreferredSize();
		dimension.width = 500;
		dimension.height = 250;

		setBackground(Color.white);
		
		setPreferredSize(dimension);

		setLayout(null);

		title= new JLabel("Loudness");
		title.setBounds(0, 0, 400, 400);
		title.setFont (title.getFont ().deriveFont (40.0f));

		loudness_label= new JLabel("-db");
		loudness_label.setBounds(0, 0, 200, 200);
		loudness_label.setFont (loudness_label.getFont ().deriveFont (30.0f));

		add(loudness_label);
		add(title);

		loudness_label.setLocation(180, 60);	
		title.setLocation(150, -150);		
	}
	
	public void updateLoudnessValue(double new_value) {
		System.out.println("UPDATING UI: change loudness displayed.");
		  
		remove(loudness_label);
		
		loudness_label= new JLabel(new_value+" db");
		loudness_label.setBounds(0, 0, 200, 200);
		loudness_label.setFont (loudness_label.getFont ().deriveFont (30.0f));

		add(loudness_label);
		loudness_label.setLocation(180, 60);	
		
		revalidate();
		repaint();
	}

}
