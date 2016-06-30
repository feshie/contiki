/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

/**
 * Class to handle every node in a system, along with the NodeMapper view.
 *
 * @author Ed Crampin
 */
public class NodeHandler {

    private final NodeList nodes;
    private final NodeMapper nodeMapper;

    /**
     * Constructor, instantiates NodeList and takes input of NodeMapper view.
     *
     * @param nodeMapper the NodeMapper for which the NodeHandler should update
     */
    public NodeHandler(NodeMapper nodeMapper) {
        this.nodes = new NodeList();
        this.nodeMapper = nodeMapper;
    }

    /**
     * Registers a packet by extracting the source and destination addresses,
     * creating Nodes if they don't already exist and adding any edge/vertex to
     * the NodeMapper. Ignores destination IP if it is a multicast address
     *
     * @param p the Packet that is to be registered.
     */
    public void registerPacket(Packet p) {
        if (!p.multicast && !p.src().equals("") && !p.dst().equals("")) {

            String psrc = p.src();
            if (psrc.startsWith("aaaa")) {
                psrc = Node.getPrefix() + psrc.substring(4);
            }
            String pdst = p.dst();
            if (pdst.startsWith("aaaa")) {
                pdst = Node.getPrefix() + pdst.substring(4);
            }

            Node src = null;
            Node dst = null;

            if (!psrc.equals(Node.getPrefix() + "::1")) {
                if (nodes.contains(psrc)) {
                    src = nodes.get(psrc);
                } else {
                    src = new Node(psrc.substring(4));
                    nodes.add(src);
                }
                src.addPacket(p);
                nodeMapper.addVertex(src.getAddress());
            }
            if (!pdst.equals(Node.getPrefix() + "::1")) {
                if (nodes.contains(pdst)) {
                    dst = nodes.get(pdst);
                } else {
                    dst = new Node(pdst.substring(4));
                    nodes.add(dst);
                }
                dst.addPacket(p);
                nodeMapper.addVertex(dst.getAddress());
            }

            if (!pdst.equals(Node.getPrefix() + "::1") && !psrc.equals(Node.getPrefix() + "::1")) {
                nodeMapper.addEdge(src.getAddress(), dst.getAddress());
            }

        }
        if (p.multicast) {

            String psrc = p.src();
            if (psrc.startsWith("aaaa")) {
                psrc = Node.getPrefix() + psrc.substring(4);
            }
            Node src = null;
            if (!psrc.equals(Node.getPrefix() + "::1")) {
                if (nodes.contains(psrc)) {
                    src = nodes.get(psrc);
                } else {
                    src = new Node(psrc.substring(4));
                    nodes.add(src);
                }
                src.addPacket(p);
                nodeMapper.addVertex(src.getAddress());
            }
        }
    }

    /**
     * Returns the list of nodes that the NodeHandler holds.
     *
     * @return the NodeList
     */
    public NodeList getList() {
        return this.nodes;
    }

}
