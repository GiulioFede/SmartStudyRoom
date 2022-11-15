package iet.unipi.it.smartStudyRoom.ui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.Border;
import javax.swing.border.LineBorder;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;


public class SeatPanel extends JPanel {
	
	JLabel label_table_image;
	JLabel title;
	
	SeatPanel(){

		
		Dimension dimension = getPreferredSize();
		dimension.width = 800;
		dimension.height = 800;
		
		setBackground(Color.white);
		
		setPreferredSize(dimension);

		setLayout(null);

		

		title= new JLabel("Building: 1, Room: 1");
		title.setBounds(0, 0, 500, 400);
		title.setFont (title.getFont ().deriveFont (40.0f));

		setBorder(BorderFactory.createMatteBorder(0, 0, 0, 1, Color.BLACK));

		ImageIcon table_image = new ImageIcon(getClass().getResource("images/table/5.png"));
		label_table_image = new JLabel(table_image);
		label_table_image.setBounds(0, 0, 500, 500);

		add(label_table_image);
		add(title);

		label_table_image.setLocation(130, 50);
		title.setLocation(190, 500);

	}

	public void changeState(int[] state){

		System.out.println("ciao");
			  
		remove(label_table_image);
		
		String type_configuration = "5";
		
		if( state[0]==1 && state[1]== 1 && state[2]== 1 && state[3]== 1)
			type_configuration = "5";
		else if( state[0]==0 && state[1]==1 && state[2]==1 && state[3]==1)
			type_configuration = "8";
		else if( state[0]==1 && state[1]==0 && state[2]==1 && state[3]==1)
			type_configuration = "4";
		else if( state[0]==1 && state[1]==1 && state[2]==0 && state[3]==1)
			type_configuration = "17";
		else if( state[0]==0 && state[1]==0 && state[2]==1 && state[3]==1)
			type_configuration = "15";
		else if( state[0]==0 && state[1]==1 && state[2]==0 && state[3]==1)
			type_configuration = "14";
		else if( state[0]==1 && state[1]==0 && state[2]==0 && state[3]==1)
			type_configuration = "3";
		else if( state[0]==0 && state[1]==0 && state[2]==0 && state[3]==1)
			type_configuration = "6";
		else
			type_configuration = "5";
		
		
		ImageIcon table_image = new ImageIcon(getClass().getResource("images/table/"+type_configuration+".png"));
		label_table_image = new JLabel(table_image);
		label_table_image.setBounds(0, 0, 500, 500);
		
		add(label_table_image);
		label_table_image.setLocation(130, 50);
		revalidate();
		repaint();

		
	}


}
