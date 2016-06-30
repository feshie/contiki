/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.net.UnknownHostException;

/**
 * An implementation of a Packet model to hold IEEE 802.15.4 based packets.
 *
 * @author Ed Crampin
 */
public class Packet {

    /**
     * Denotes when a packet is ICMP.
     */
    public static final int TYPE_ICMP = 0;
    /**
     * Denotes when a packet is CoAP.
     */
    public static final int TYPE_COAP = 1;
    /**
     * Denotes when a packet is IEEE 802.15.4.
     */
    public static final int TYPE_IEEE802154 = 2;

    /**
     * Denotes when the subtype when an RPL packet is DODAG Info.
     */
    public static final int RPL_DODAG_INFO = 3;
    /**
     * Denotes when the subtype when an RPL packet is DODAG Solicitation.
     */
    public static final int RPL_DODAG_SOLICIT = 4;
    /**
     * Denotes when the subtype when an RPL packet is Destination Advertisement.
     */
    public static final int RPL_DEST_ADVERT = 5;

    /**
     * Denotes when a packet subtype is an IEEE 802.15.4 reserved packet.
     */
    public static final int IEEE_RESERVED = 6;

    /**
     * Denotes when a packet is an ACK.
     */
    public static final int ACK = 7;

    /**
     * Denotes when a CoAP Packet is a GET request.
     */
    public static final int COAP_GET = 8;
    
    /**
     * Denotes when a CoAP Packet contains content.
     */
    public static final int COAP_CONTENT = 9;
    
    /**
     * Raw bytes of the packet.
     */
    public byte[] raw;

    /**
     * Source IPv6 string of the packet.
     */
    public String src;
    /**
     * Destination IPv6 string of the packet.
     */
    public String dst;
    /**
     * Source MAC IPv6 string of the packet.
     */
    public String srcMac;
    /**
     * Destination MAC IPv6 string of the packet.
     */
    public String dstMac;

    /**
     * The type of the packet.
     */
    public int type;
    /**
     * The subtype of the packet.
     */
    public int subtype;
    private int length;

    /**
     * The sequence number of the packet.
     */
    public int seqNo;
    /**
     * The destination PAN address.
     */
    public String dstPan;
    /**
     * The hop limit of the packet.
     */
    public int hopLimit;

    /**
     * Boolean of whether the packet is multicast.
     */
    public boolean multicast;
    /**
     * Whether or not the checksum has been confirmed.
     */
    public boolean checksumConf;
    /**
     * The checksum provided in the packet.
     */
    public String checksum;

    private long created;

    /**
     * Constructor for packet model, sets default values for the Packet.
     *
     * @param data the byte data of the packet
     */
    public Packet(byte[] data) {
        this.raw = data;
        this.length = data.length;
        this.hopLimit = -1;
        this.dstPan = "";
        this.seqNo = -1;
        this.subtype = -1;
        this.type = -1;
        this.dstMac = "";
        this.dst = "";
        this.srcMac = "";
        this.src = "";
        this.multicast = false;
        this.checksumConf = true;
        this.checksum = "";
        this.created = System.currentTimeMillis();
    }

    /**
     * Return checksum of the packet and whether it's correct.
     *
     * @return formatted string confirming the validity of the checksum
     */
    public String checksum() {
        if (checksum.equals("")) {
            return this.checksum;
        } else {
            return "0x" + this.checksum + ((this.checksumConf) ? " (correct)" : " (INVALID)");
        }
    }

    /**
     * Returns packet length.
     *
     * @return the length of the packet
     */
    public int length() {
        return this.length;
    }

    /**
     * Attempts to return formatted String of the source address.
     *
     * @return formatted string of source address, or raw source address
     */
    public String src() {
        try {
            return IP6Address.parse(src);
        } catch (UnknownHostException | StringIndexOutOfBoundsException e) {
            return src;
        }
    }

    /**
     * Attempts to return formatted String of the destination address.
     *
     * @return formatted string of destination address, or raw destination
     * address
     */
    public String dst() {
        try {
            return IP6Address.parse(dst);
        } catch (UnknownHostException | StringIndexOutOfBoundsException e) {
            return dst;
        }
    }

    /**
     * Returns the protocol of the packet.
     *
     * @return formatted string of the packet's protocol
     */
    public String protocol() {
        switch (this.type) {
            case TYPE_ICMP:
                return "ICMPv6";
            case TYPE_COAP:
                return "COAP";
            case TYPE_IEEE802154:
                return "IEEE 802.15.4";
            default:
                return "Unknown";
        }
    }

    /**
     * Returns formatted string with info about the packet. RPL Packets include
     * the type of RPL packet they are.
     *
     * @return formatted string of packet information
     */
    public String info() {
        if (this.type == TYPE_COAP) {
            switch(this.subtype) {
                case COAP_GET:
                    return "Constrained Application Protocol (GET)";
                case COAP_CONTENT:
                    return "Constrained Application Protocol (CONTENT)";
                default:
                    return "Constrained Application Protocol";
            }
        }
        switch (this.subtype) {
            case RPL_DODAG_INFO:
                return "RPL Control (DODAG Info Object)";
            case RPL_DODAG_SOLICIT:
                return "RPL Control (DODAG Info Solicitation)";
            case RPL_DEST_ADVERT:
                return "RPL Control (Destination Advertisement)";
            case ACK:
                return "ACK";
            default:
            case IEEE_RESERVED:
                return "Reserved";
        }
    }

    /**
     * Returns the hex of the packet with the option to format the output
     * string.
     *
     * @param formatted boolean of whether the output should be formatted
     * @return string of the raw hex of the packet
     */
    public String hex(boolean formatted) {
        String hex = "";
        for (int i = 0; i < this.raw.length; i++) {

            if (i == 0 || i == this.raw.length - 1) {
                continue;
            }

            hex += String.format("%02x", this.raw[i]);

            if (formatted) {
                hex += " ";
                if (i % 16 == 8) {
                    hex += " ";
                } else if (i % 16 == 0) {
                    hex += "\r\n";
                }
            }

        }
        return hex;
    }

    /**
     * The creation date of the packet.
     *
     * @return the system's current time in milliseconds when the packet was
     * instantiated
     */
    public long getCreated() {
        return this.created;
    }
}
