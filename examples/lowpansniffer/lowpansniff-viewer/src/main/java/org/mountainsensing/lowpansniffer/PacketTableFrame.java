/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import javax.swing.ActionMap;
import javax.swing.JFrame;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.table.DefaultTableCellRenderer;

/**
 * The main table view for the application, displaying all packets detected by
 * the sniffer and interpreted by the application.
 *
 * @author Ed Crampin
 * @see JFrame
 */
public class PacketTableFrame extends javax.swing.JFrame {

    private ArrayList<Packet> packetList;
    private NodeList nodeList;

    /**
     * Creates new RPLMapperFrame
     *
     * @param packetList the list of packets that are displayed by the table
     */
    public PacketTableFrame(ArrayList<Packet> packetList) {

        this.packetList = packetList;

        initComponents();

        packetTable.getColumnModel().getColumn(0).setPreferredWidth(20);
        packetTable.getColumnModel().getColumn(1).setPreferredWidth(50);
        packetTable.getColumnModel().getColumn(2).setPreferredWidth(120);
        packetTable.getColumnModel().getColumn(3).setPreferredWidth(120);
        packetTable.getColumnModel().getColumn(4).setPreferredWidth(60);
        packetTable.getColumnModel().getColumn(5).setPreferredWidth(30);
        packetTable.getColumnModel().getColumn(6).setPreferredWidth(180);

        packetTable.setDefaultRenderer(Object.class, new DefaultTableCellRenderer() {
            @Override
            public Component getTableCellRendererComponent(JTable table,
                    Object value, boolean isSelected, boolean hasFocus, int row, int col) {

                super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, col);

                int packetrow = (int) table.getValueAt(row, 0) - 1;

                String protocol = packetList.get(packetrow).protocol();
                if (isSelected) {
                    setForeground(Color.WHITE);
                    setBackground(Color.BLACK);
                } else {
                    setForeground(Color.BLACK);
                    switch (protocol) {
                        default:
                            setBackground(Color.WHITE);
                            break;
                        case "ICMPv6":
                            setBackground(new Color(254, 237, 255));
                            break;
                        case "COAP":
                            setBackground(new Color(237, 247, 255));
                            break;
                        case "IEEE 802.15.4":
                            setBackground(new Color(238, 255, 237));
                            break;

                    }
                }
                setBorder(noFocusBorder);
                return this;
            }
        });
        packetTable.setDefaultRenderer(Number.class, packetTable.getDefaultRenderer(Object.class));
        packetTable.setDefaultRenderer(Double.class, packetTable.getDefaultRenderer(Object.class));
        packetTable.setShowGrid(false);
        packetTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        packetTable.setIntercellSpacing(new Dimension(0, 0));
        packetTable.getTableHeader().setReorderingAllowed(false);
        packetTable.addMouseListener(new MouseAdapter() {
            public void mousePressed(MouseEvent me) {
                JTable table = (JTable) me.getSource();
                Point p = me.getPoint();
                int row = table.rowAtPoint(p);
                int col = table.columnAtPoint(p);
                int packetrow = (int) table.getValueAt(row, 0) - 1;
                Packet packet = packetList.get(packetrow);
                packetSelected(packet);
            }
        });

        ActionMap am = packetTable.getActionMap();
        am.put("selectPreviousRow", new TableKeyAction("up", this));
        am.put("selectNextRow", new TableKeyAction("down", this));

        rawPacketField.setEditable(false);
        rawPacketText.setEditable(false);

    }

    /**
     * Sets the list of nodes in the application
     *
     * @param nl the NodeList
     */
    public void setNodeList(NodeList nl) {
        this.nodeList = nl;
    }

    /**
     * When a packet is selected in the table, update the text fields to display
     * information about the packet
     *
     * @param p the packet that was selected
     */
    public void packetSelected(Packet p) {
        rawPacketField.setText(p.hex(true));
        String packetInfo = "Protocol:\t\t" + p.protocol() + "\r\n";
        packetInfo += "Information:\t\t" + p.info() + "\r\n";
        packetInfo += "Source IP:\t\t" + p.src() + "\r\n";
        packetInfo += "Destination IP:\t\t" + p.dst() + "\r\n";
        packetInfo += "Destination PAN:\t" + p.dstPan + "\r\n";
        packetInfo += "Sequence Number:\t" + p.seqNo + "\r\n";
        packetInfo += "Hop Limit:\t\t" + p.hopLimit + "\r\n";
        //packetInfo += "Checksum:\t\t" + p.checksum();
        rawPacketText.setText(packetInfo);
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jScrollPane1 = new javax.swing.JScrollPane();
        packetTable = new javax.swing.JTable();
        packetDetailsPanel = new javax.swing.JPanel();
        jScrollPane2 = new javax.swing.JScrollPane();
        rawPacketField = new javax.swing.JTextArea();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane3 = new javax.swing.JScrollPane();
        rawPacketText = new javax.swing.JTextArea();
        jLabel2 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("LoWPAN Sniffer");

        jScrollPane1.setBackground(new java.awt.Color(254, 254, 254));

        packetTable.setAutoCreateRowSorter(true);
        packetTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {

            },
            new String [] {
                "No.", "Time", "Source", "Destination", "Protocol", "Length", "Info"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.Integer.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.Integer.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, false, false, false, false, false, false
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        jScrollPane1.setViewportView(packetTable);

        jScrollPane2.setAutoscrolls(true);

        rawPacketField.setColumns(20);
        rawPacketField.setFont(new java.awt.Font("Monospaced", 1, 12)); // NOI18N
        rawPacketField.setRows(5);
        jScrollPane2.setViewportView(rawPacketField);

        jLabel1.setText("Hex Packet Data");

        jScrollPane3.setAutoscrolls(true);

        rawPacketText.setColumns(20);
        rawPacketText.setFont(new java.awt.Font("Monospaced", 1, 12)); // NOI18N
        rawPacketText.setRows(5);
        jScrollPane3.setViewportView(rawPacketText);

        jLabel2.setText("Packet Information");

        javax.swing.GroupLayout packetDetailsPanelLayout = new javax.swing.GroupLayout(packetDetailsPanel);
        packetDetailsPanel.setLayout(packetDetailsPanelLayout);
        packetDetailsPanelLayout.setHorizontalGroup(
            packetDetailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, packetDetailsPanelLayout.createSequentialGroup()
                .addGroup(packetDetailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(packetDetailsPanelLayout.createSequentialGroup()
                        .addComponent(jLabel2)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addComponent(jScrollPane3, javax.swing.GroupLayout.DEFAULT_SIZE, 419, Short.MAX_VALUE))
                .addGap(18, 18, 18)
                .addGroup(packetDetailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel1)
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 369, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );
        packetDetailsPanelLayout.setVerticalGroup(
            packetDetailsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(packetDetailsPanelLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 153, Short.MAX_VALUE))
            .addGroup(packetDetailsPanelLayout.createSequentialGroup()
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane3))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(packetDetailsPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jScrollPane1))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 395, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(packetDetailsPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    /**
     * Returns the JTable
     *
     * @return the reference to the JTable
     */
    public JTable getTable() {
        return this.packetTable;
    }

    /**
     * Returns the packet list of the class.
     *
     * @return an ArrayList of Packets
     */
    public ArrayList<Packet> getPacketList() {
        return this.packetList;
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JPanel packetDetailsPanel;
    private javax.swing.JTable packetTable;
    private javax.swing.JTextArea rawPacketField;
    private javax.swing.JTextArea rawPacketText;
    // End of variables declaration//GEN-END:variables
}
