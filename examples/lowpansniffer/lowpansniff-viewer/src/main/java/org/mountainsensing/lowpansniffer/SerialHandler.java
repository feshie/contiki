/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortList;

/**
 * Class to handle the serial port that is used to obtain packets.
 *
 * @author Ed Crampin
 */
public class SerialHandler {

    private SerialPort serialPort;

    /**
     * Returns the names of all serial ports.
     *
     * @return string array of all available port names
     */
    public String[] getPortNames() {
        String[] portNames = SerialPortList.getPortNames();
        return portNames;
    }

    /**
     * Opens the serial port specified with baudrate specified
     *
     * @param name the name of the port to open
     * @param bauds the baudrate of which to use
     * @return a SerialPort object of the opened SerialPort
     * @see jssc.SerialPort
     */
    public SerialPort openSerialPort(String name, int bauds) {

        serialPort = new SerialPort(name);
        try {
            serialPort.openPort();
            serialPort.setParams(bauds, 8, 1, 0);
            //Preparing a mask. In a mask, we need to specify the types of events that we want to track.
            //Well, for example, we need to know what came some data, thus in the mask must have the
            //following value: MASK_RXCHAR. If we, for example, still need to know about changes in states 
            //of lines CTS and DSR, the mask has to look like this: SerialPort.MASK_RXCHAR + SerialPort.MASK_CTS + SerialPort.MASK_DSR
            int mask = SerialPort.MASK_RXCHAR;
            //Set the prepared mask
            serialPort.setEventsMask(mask);

            return serialPort;
        } catch (SerialPortException ex) {
            ex.printStackTrace();
            return null;
        }
    }
}
