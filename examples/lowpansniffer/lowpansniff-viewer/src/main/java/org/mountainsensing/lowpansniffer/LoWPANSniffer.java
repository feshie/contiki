/**
 * RPL Sniffing Control Application
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.awt.Dimension;
import java.awt.Toolkit;
import java.util.ArrayList;
import java.util.Enumeration;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import jssc.*;

/**
 * LoWPANSniffer is an application that, when paired with a sniffer using a slip-bridge,
 takes input through a serial port receiving raw packet data. This packet data provides
 * network data of a RPL-based network, presenting all packets seen in table form and
 * mapping out the nodes data flow on a JGraphT graph.
 * @author Ed Crampin
 */
public class LoWPANSniffer {
    
    private static SerialPort serialPort;   
    
    /**
     * Main method for the application, initialising the JFrame in which a user
     * selects the serial port they wish to use.
     * @param args 
     */
    public static void main(String args[]) {
        
        String port;
        
        if(args.length > 0) {
            port = args[0];
            LoWPANSniffer.init(port);
        } else {
            SerialHandler serialHandler = new SerialHandler();
            SerialChooser sc = new SerialChooser(serialHandler.getPortNames());
            sc.setVisible(true);
            sc.setLocationRelativeTo(null);
        }
    }
    
    /**
     * Once the port has been chosen, the init method sets up the Serial Listener
     * and all other UI for the application.
     * @param port the serial port to listen to packets through
     */
    public static void init(String port) {
        
        SerialHandler serialHandler = new SerialHandler();
        
        ArrayList<Packet> packetList = new ArrayList<Packet>();
        
        PacketTableFrame mainFrame = new PacketTableFrame(packetList);
        mainFrame.setVisible(true);
        
        PacketTableHandler packetTable = new PacketTableHandler(mainFrame.getTable());
        
        
        NodeMapper applet = new NodeMapper();
        applet.init();

        JFrame frame = new JFrame();
        frame.getContentPane().add(applet);
        JPanel pane = (JPanel) frame.getContentPane();
        pane.setBorder(new EmptyBorder(10, 10, 10, 10));
        frame.setTitle("Feshie Mapper - Node Map");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setPreferredSize(new Dimension(500, 500));
        frame.pack();
        frame.setLocation(mainFrame.getWidth() + mainFrame.getX(), mainFrame.getY() + (mainFrame.getHeight() / 2) - (frame.getHeight() / 2));
        frame.setVisible(true);
        
        NodeHandler nodeHandler = new NodeHandler(applet);
        mainFrame.setNodeList(nodeHandler.getList());
        SerialPort sp;
        
        try {
            sp = serialHandler.openSerialPort(port, 115200);
            sp.addEventListener(new SerialPacketListener(sp, packetTable, packetList, nodeHandler));
        } catch (SerialPortException ex) {
            ex.printStackTrace();
            System.exit(0);
        }
        
        
        
    }
}
