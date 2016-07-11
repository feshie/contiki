/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.util.ArrayList;
import java.util.Arrays;
import jssc.SerialPort;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;

/**
 * The listener used for our SerialPort, listening for SerialPortEvents
 *
 * @author Ed Crampin
 */
public class SerialPacketListener implements SerialPortEventListener {

    private final SerialPort serialPort;
    private final PacketTableHandler packetTable;
    private final NodeHandler nodeHandler;
    private final PacketHandler ph;
    private final ArrayList<Packet> packetList;
    private static final byte PACKET_DELIMITER = (byte) 0xC0;

    /**
     * Our constructor class, creates new SerialPacketListener
     *
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
        this.ph = new PacketHandler();
    }

    /**
     * The function called when a SerialEvent is triggered
     *
     * @param event the SerialPortEvent object
     * @see jssc.SerialPortEvent
     */
    @Override
    public void serialEvent(SerialPortEvent event) {
        try {
            byte[] output = serialPort.readBytes();
            ArrayList<byte[]> packets = splitPackets(output);
            for(byte[] packet : packets) {
                Packet p = ph.parsePacket(packet);
                if (p != null) {
                    packetTable.addPacket(p);
                    packetList.add(p);
                    nodeHandler.registerPacket(p);
                }
            }
        } catch (SerialPortException | UnsupportedOperationException | StringIndexOutOfBoundsException e) {
        }
    }
    
    /**
     * This function is used to separate packets based on the start/end delimiter
     * set by PACKET_DELIMITER.
     * 
     * @param bytes
     * @return array of packet bytes 
     */
    public ArrayList<byte[]> splitPackets(byte[] bytes) {
        
        ArrayList<byte[]> r = new ArrayList<>();
        int ptr = 0;
        for(int i = 0; i < bytes.length; i++) {
            if(bytes[i] == PACKET_DELIMITER && (i == bytes.length - 1 || bytes[i + 1] == PACKET_DELIMITER)) {
                r.add(Arrays.copyOfRange(bytes, ptr, i + 1));
                ptr = i + 1;
            }
        }
        return r;
    }
}
