/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.text.DecimalFormat;
import javax.swing.JTable;
import javax.swing.table.DefaultTableModel;

/**
 * Handles the JTable used to display packets
 *
 * @author Ed Crampin
 */
public class PacketTableHandler {

    private final JTable packetTable;
    private final long startTime;

    /**
     * Constructor for PacketTableHandler
     *
     * @param packetTable the table we should use to output packets
     */
    public PacketTableHandler(JTable packetTable) {
        this.packetTable = packetTable;
        this.startTime = System.nanoTime();
    }

    /**
     * Adds a Packet object to the JTable
     *
     * @param packet the Packet that should be added to the table
     */
    public void addPacket(Packet packet) {
        DecimalFormat df = new DecimalFormat("#.###");
        double timeRunning = (System.nanoTime() - startTime) / 1000000000.0;
        Object[] tableData = {packetTable.getModel().getRowCount() + 1, df.format(timeRunning), packet.src(), packet.dst(), packet.protocol(), packet.length(), packet.info()};
        //packetTable.getModel().setValueAt(object, row, col);
        ((DefaultTableModel) packetTable.getModel()).addRow(tableData);
    }

    /**
     * Initial implementation of addPacket when the Packet model did not exist.
     *
     * @param src
     * @param dst
     * @param protocol
     * @param length
     * @param info
     * @deprecated
     * @see #addPacket(org.mountainsensing.rplmapper.Packet)
     */
    public void addPacket(String src, String dst, String protocol, int length, String info) {
        DecimalFormat df = new DecimalFormat("#.###");
        double timeRunning = (System.nanoTime() - startTime) / 1000000000.0;
        Object[] tableData = {packetTable.getModel().getRowCount() + 1, df.format(timeRunning), src, dst, protocol, length, info};
        //packetTable.getModel().setValueAt(object, row, col);
        ((DefaultTableModel) packetTable.getModel()).addRow(tableData);
    }

}
