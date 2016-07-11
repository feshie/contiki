/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import javax.swing.JTable;
import javax.swing.JViewport;

/**
 * Class to handle key actions on a JTable
 *
 * @author Ed Crampin
 */
public class TableKeyAction extends AbstractAction {

    private String cmd;
    private PacketTableFrame parent;
    private JTable table;

    /**
     * Constructs our TableKeyAction
     *
     * @param cmd the name of the command
     * @param parent the parent PacketTableFrame
     */
    public TableKeyAction(String cmd, PacketTableFrame parent) {
        this.cmd = cmd;
        this.parent = parent;
        this.table = parent.getTable();
    }

    /**
     * Called when key action occurs on the table
     *
     * @param e the ActionEvent
     */
    @Override
    public void actionPerformed(ActionEvent e) {
        if (table.getRowCount() > 0) {
            int index = table.getSelectedRow();
            if (this.cmd.equals("up") && index != 0) {
                index -= 1;
            } else if (this.cmd.equals("down") && index != table.getRowCount() - 1) {
                index += 1;
            }
            table.setRowSelectionInterval(index, index);
            int packetrow = (int) table.getValueAt(index, 0) - 1;
            Packet packet = parent.getPacketList().get(packetrow);
            parent.packetSelected(packet);

            checkVisibility(index, 0);
        }
    }

    /**
     * A method that checks whether a cell is visible, and if not scrolls it
     * into the viewport.
     *
     * @param rowIndex
     * @param vColIndex
     */
    public void checkVisibility(int rowIndex, int vColIndex) {
        if (!(table.getParent() instanceof JViewport)) {
            return;
        }
        JViewport viewport = (JViewport) table.getParent();
        Rectangle rect = table.getCellRect(rowIndex, vColIndex, true);
        Point pt = viewport.getViewPosition();
        rect.setLocation(rect.x - pt.x, rect.y - pt.y);
        if (!new Rectangle(viewport.getExtentSize()).contains(rect)) {
            viewport.scrollRectToVisible(rect);
        }
    }
}
