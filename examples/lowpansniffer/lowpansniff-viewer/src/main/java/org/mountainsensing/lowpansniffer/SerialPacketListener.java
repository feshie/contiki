/**
 * RPL Sniffing Control Application
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.util.ArrayList;
import jssc.SerialPort;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;

/**
 * The listener used for our SerialPort, listening for SerialPortEvents
 * @author Ed Crampin
 */
public class SerialPacketListener implements SerialPortEventListener {
 
    private final SerialPort serialPort;
    private final PacketTableHandler packetTable;
    private final NodeHandler nodeHandler;
    
    private final ArrayList<Packet> packetList;
    
    /**
     * Our constructor class, creates new SerialPacketListener
     * @param sp
     * @param pt
     * @param pl
     * @param nh 
     */
    public SerialPacketListener(SerialPort sp, PacketTableHandler pt, ArrayList<Packet> pl, NodeHandler nh) {
        super();
        this.packetList = pl;
        this.serialPort = sp;
        this.packetTable = pt;
        this.nodeHandler = nh;
    }
    
    /**
     * The function called when a SerialEvent is triggered
     * @param event the SerialPortEvent object
     * @see jssc.SerialPortEvent
     */
    @Override
    public void serialEvent(SerialPortEvent event) {
        try {
            byte[] output = serialPort.readBytes();
            PacketHandler ph = new PacketHandler();
            Packet p = ph.parsePacket(output);
            if(p != null && p.checksumConf) {
                packetTable.addPacket(p);
                packetList.add(p);
                nodeHandler.registerPacket(p);
            }
        } catch (SerialPortException | UnsupportedOperationException | StringIndexOutOfBoundsException e) {
        }
    }
    
    
}
