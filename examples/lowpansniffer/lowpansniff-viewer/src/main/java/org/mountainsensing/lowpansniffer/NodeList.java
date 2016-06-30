/**
 * RPL Sniffing Control Application
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.util.ArrayList;
/**
 * An extension of ArrayList designed to hold nodes
 * @author Ed Crampin
 * @see ArrayList
 */
public class NodeList extends ArrayList<Node> {
    
    /**
     * An extension of ArrayList.contains but checks if any Node in the list has
     * the address specified.
     * @param addr the address we want to see if the NodeList contains
     * @return boolean of whether the list contains the address
     */
    public boolean contains(String addr) {
        return this.stream().anyMatch((node) -> (node.getAddress().equals(addr)));
    }
    
    /**
     * An extension of ArrayList.get but gets a Node based on the address
     * provided.
     * @param addr the address of the node we wish to retrieve
     * @return the node if it exists in the NodeList, null otherwise
     */
    public Node get(String addr) {
        for(Node node : this) {
            if(node.getAddress().equals(addr)) {
                return node;
            }
        }
        
        return null;
    }
}
