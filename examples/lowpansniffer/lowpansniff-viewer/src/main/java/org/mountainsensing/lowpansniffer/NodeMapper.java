/**
 * RPL Sniffing Control Application
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import com.mxgraph.layout.mxCircleLayout;
import com.mxgraph.swing.mxGraphComponent;
import com.mxgraph.util.mxConstants;
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JApplet;
import org.jgrapht.ListenableGraph;
import org.jgrapht.ext.JGraphXAdapter;
import org.jgrapht.graph.DefaultEdge;
import org.jgrapht.graph.ListenableDirectedGraph;

/**
 * A class to map out graphically the data flow between Nodes using JGraphT and
 * JGraphX.
 * @author Ed Crampin
 */
public class NodeMapper extends JApplet
{
    private static final Dimension DEFAULT_SIZE = new Dimension(500, 500);

    private JGraphXAdapter<String, DefaultEdge> jgxAdapter;
    private ListenableGraph<String, DefaultEdge> g;
    
    /**
     * Initialises the NodeMapper's UI constraints.
     */
    @Override
    public void init() {
        g = new ListenableDirectedGraph<>(DefaultEdge.class);

        jgxAdapter = new JGraphXAdapter<>(g);

        getContentPane().add(new mxGraphComponent(jgxAdapter));
        resize(DEFAULT_SIZE);
        
        jgxAdapter.setCellsEditable(false);
        jgxAdapter.setAllowDanglingEdges(false);
        jgxAdapter.setAllowLoops(false);
        jgxAdapter.setCellsDeletable(false);
        jgxAdapter.setCellsCloneable(false);
        jgxAdapter.setCellsDisconnectable(false);
        jgxAdapter.setDropEnabled(false);
        jgxAdapter.setSplitEnabled(false);
        jgxAdapter.setCellsBendable(false);
        jgxAdapter.setCellsResizable(false);
        jgxAdapter.setCellsMovable(true);
        jgxAdapter.setCellsSelectable(true);
        jgxAdapter.getStylesheet().getDefaultEdgeStyle().put(mxConstants.STYLE_NOLABEL, "1");
                
        this.resetLayout();


    }
    
    /**
     * Resets the layout of the Nodes, organising them in a circular layout.
     * @see mxCircleLayout
     */
    public void resetLayout() {
       
        mxCircleLayout layout = new mxCircleLayout(jgxAdapter);
        layout.execute(jgxAdapter.getDefaultParent());
    }
    
    /**
     * Adds a vertex to the graph for the specified address.
     * @param addr the address of the vertex to be added
     */
    public void addVertex(String addr) {
        if(!g.containsVertex(addr)) {
            g.addVertex(addr);
            this.resetLayout();
        }
    }
    
    /**
     * Adds a directed edge between two nodes in the graph.
     * @param src the source address
     * @param dst the destination address
     */
    public void addEdge(String src, String dst) {
        if(!g.containsEdge(src, dst)) {
            g.addEdge(src, dst);
        }
    }
}