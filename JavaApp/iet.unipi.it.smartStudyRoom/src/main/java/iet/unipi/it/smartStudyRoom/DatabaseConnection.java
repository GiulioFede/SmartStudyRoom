package iet.unipi.it.smartStudyRoom;

import java.sql.*;
import java.util.*;

public class DatabaseConnection {
	private static String ipAddress = "localhost";
	private static String port = "3306";
	private static String user = "root";
	private static String password = "osboxes.org";
	private static String nameDB = "SmartStudyRoom";
	
	
	public static void CreateDatabase() {
		// SQL command to create a database in MySQL.
        String sql = "CREATE DATABASE IF NOT EXISTS "+nameDB;

        try {
        	Connection conn = DriverManager.getConnection("jdbc:mysql://"+ipAddress+":"+port,
        													user, password);
             PreparedStatement stmt = conn.prepareStatement(sql);
             stmt.execute();
             stmt.close();
             conn.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
	}
	
	public static void CreateTemperatureTable() {
		
		try{
			Connection conn = DriverManager.getConnection("jdbc:mysql://"+ipAddress+":"+port+"/"+nameDB, user, password);
		    Statement stmt = conn.createStatement();
		    String sql = "CREATE TABLE IF NOT EXISTS Temperature " +
	                   "(id int NOT NULL AUTO_INCREMENT , " +
	                   " timestamp TIMESTAMP NOT NULL, " +
	                   " idBuilding int NOT NULL, " + 
	                   " idRoom int NOT NULL, " + 
	                   " idDevice int NOT NULL, " +
	                   " temperature DOUBLE NOT NULL, " +
	                   " PRIMARY KEY (id))"; 
		    
		    stmt.executeUpdate(sql);
		    stmt.close();
		    conn.close();
		}catch(SQLException e) {
			e.printStackTrace();
		}
		
		
	}
	
	public static void CreateNoiseTable() {
			
			try{
				Connection conn = DriverManager.getConnection("jdbc:mysql://"+ipAddress+":"+port+"/"+nameDB, user, password);
			    Statement stmt = conn.createStatement();
			    String sql = "CREATE TABLE IF NOT EXISTS Noise " +
		                   "(id int NOT NULL AUTO_INCREMENT, " +
		                   " timestamp TIMESTAMP NOT NULL, " +
		                   " idBuilding int NOT NULL, " + 
		                   " idRoom int NOT NULL, " + 
		                   " idTable int NOT NULL, " +
		                   " noiseValue double NOT NULL, " +
		                   " noiseLevel VARCHAR(255) NOT NULL, "+
		                   " PRIMARY KEY (id))"; 
			    
			    stmt.executeUpdate(sql);
			    stmt.close();
			    conn.close();
			}catch(SQLException e) {
				e.printStackTrace();
			}
			
			
	}
	
	public static void addValueTemperature(Timestamp timestamp,int idBuilding, int idRoom, int idDevice, double temperature){
        try{
        	Connection conn = DriverManager.getConnection("jdbc:mysql://"+ipAddress+":"+port+"/"+nameDB, user, password);
            PreparedStatement ps = conn.prepareStatement("INSERT INTO Temperature(timestamp, idBuilding, idRoom, idDevice, temperature) VALUES(?, ?, ?, ?, ?)");
            
            ps.setTimestamp(1, timestamp);
            ps.setInt(2, idBuilding);
            ps.setInt(3, idRoom);
            ps.setInt(4, idDevice);
            ps.setDouble(5, temperature);

            System.out.println("rows effect: "+ ps.executeUpdate());
            ps.close();
            conn.close();
        }catch(SQLException e){
        	System.out.println(e.getMessage());
        }
	}

	public static void addValueNoise(Timestamp timestamp,int idBuilding, int idRoom, int idTable, double noiseValue, String noiseLevel){
        try{
        	Connection conn = DriverManager.getConnection("jdbc:mysql://"+ipAddress+":"+port+"/"+nameDB, user, password);
            PreparedStatement ps = conn.prepareStatement("INSERT INTO Noise(timestamp, idBuilding, idRoom, idTable, noiseValue, noiseLevel) VALUES(?, ?, ?, ?, ?, ?)");
            
            ps.setTimestamp(1, timestamp);
            ps.setInt(2, idBuilding);
            ps.setInt(3, idRoom);
            ps.setInt(4, idTable);
            ps.setDouble(5, noiseValue);
            ps.setString(6, noiseLevel);

            System.out.println("rows effect: "+ ps.executeUpdate());
            ps.close();
            conn.close();
        }catch(SQLException e){
        	System.out.println(e.getMessage());
        }
	}

}
