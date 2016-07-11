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
    private static final String COAP_PREFIX = "aaaa";

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
     * the NodeMapper. Ignores destination IP if it is a multicast address.
     *
     * @param p the Packet that is to be registered.
     */
    public void registerPacket(Packet p) {
        if (!p.multicast && !p.src().equals("") && !p.dst().equals("")) {

            String psrc = p.src();
            
            String pdst = p.dst();
            
            Node src = null;
            Node dst = null;

            if (!psrc.endsWith("::1") && !psrc.startsWith(COAP_PREFIX)) {
                if (nodes.contains(psrc)) {
                    src = nodes.get(psrc);
                } else {
                    src = new Node(psrc);
                    nodes.add(src);
                }
                src.addPacket(p);
                nodeMapper.addVertex(src.getRawAddress());
            }
            if (!pdst.endsWith("::1") && !pdst.endsWith("::1a") && !pdst.startsWith(COAP_PREFIX)) {
                if (nodes.contains(pdst)) {
                    dst = nodes.get(pdst);
                } else {
                    dst = new Node(pdst);
                    nodes.add(dst);
                }
                dst.addPacket(p);
                nodeMapper.addVertex(dst.getRawAddress());
            }

            if (!pdst.endsWith("::1") && !psrc.endsWith("::1") && 
                    !pdst.endsWith("::1a") && !pdst.startsWith(COAP_PREFIX)
                     && !psrc.startsWith(COAP_PREFIX)) {
                nodeMapper.addEdge(src.getRawAddress(), dst.getRawAddress());
            }

        }
        if (p.multicast) {

            String psrc = p.src();
            
            Node src = null;
            if (!psrc.endsWith("::1") && !psrc.startsWith(COAP_PREFIX)) {
                if (nodes.contains(psrc)) {
                    src = nodes.get(psrc);
                } else {
                    src = new Node(psrc);
                    nodes.add(src);
                }
                src.addPacket(p);
                nodeMapper.addVertex(src.getRawAddress());
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
