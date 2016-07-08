/**
 * 6LoWPAN Sniffer
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

/**
 * A class to handle all packets received by the sniffer
 *
 * @author Ed Crampin
 */
public class PacketHandler {

    /**
     * A (currently messy, very hacky) implementation of how to decipher the raw
     * bytes of a packet that is received by the sniffer.
     *
     * @param packetBytes the bytes of the received packet
     * @return a Packet object of the interpreted packet
     * @throws UnsupportedOperationException if packet is corrupt/not supported
     * @throws StringIndexOutOfBoundsException when the packet is corrupt, this
     * can be thrown
     */
    public Packet parsePacket(byte[] packetBytes) throws UnsupportedOperationException, StringIndexOutOfBoundsException {

        Packet finalPacket = new Packet(packetBytes);

        String packetHex = "";
        for (int i = 0; i < packetBytes.length; i++) {
            if (i == 0 || i == packetBytes.length - 1) {
                continue;
            }
            packetHex += String.format("%02x", packetBytes[i]);
        }

        packetHex = packetHex.replace(" ", "");
        int packetLength = packetBytes.length;

        //TODO: Improve this
        if (packetHex.contains("c0c00200")) {
            //Corrupt packet
            return null;
        }

        if (packetHex.startsWith("0200")) {
            finalPacket.type = Packet.TYPE_IEEE802154;
            finalPacket.subtype = Packet.ACK;
            finalPacket.src = "";
            finalPacket.dst = "";
            //System.out.println("ACK");
            //ACK, ignore.
            return finalPacket;
        }

        finalPacket.type = Packet.TYPE_IEEE802154;
        finalPacket.subtype = Packet.IEEE_RESERVED;

        int index = 0;

        /**
         * ******* START 802.15.4 HEADER *******
         */
        String frameControlField = hexToBin(packetHex.substring(index + 2, index + 4)) + hexToBin(packetHex.substring(index, index + 2));

        // 3 = 64 bit, 2 = 16 bit
        int srcAddressingMode = binToDec(frameControlField.substring(0, 2));
        int dstAddressingMode = binToDec(frameControlField.substring(4, 6));

        // frameType 1 = data
        int frameType = binToDec(frameControlField.substring(13, 16));

        index += 4;

        finalPacket.seqNo = hexToDec(packetHex.substring(index, index + 2));

        index += 2;

        finalPacket.dstPan = packetHex.substring(index + 2, index + 4) + packetHex.substring(index, index + 2);

        index += 4;

        String dstIP = "", srcIP = "";

        if (dstAddressingMode == 2) {
            dstIP = packetHex.substring(index + 2, index + 4) + packetHex.substring(index, index + 2);
            index += 4;
        } else if (dstAddressingMode == 3) {
            dstIP = packetHex.substring(index + 14, index + 16) + packetHex.substring(index + 12, index + 14)
                    + packetHex.substring(index + 10, index + 12) + packetHex.substring(index + 8, index + 10)
                    + packetHex.substring(index + 6, index + 8) + packetHex.substring(index + 4, index + 6)
                    + packetHex.substring(index + 2, index + 4) + packetHex.substring(index, index + 2);
            index += 16;
        }

        if (srcAddressingMode == 2) {
            srcIP = packetHex.substring(index + 2, index + 4) + packetHex.substring(index, index + 2);
            index += 4;
        } else if (srcAddressingMode == 3) {
            srcIP = packetHex.substring(index + 14, index + 16) + packetHex.substring(index + 12, index + 14)
                    + packetHex.substring(index + 10, index + 12) + packetHex.substring(index + 8, index + 10)
                    + packetHex.substring(index + 6, index + 8) + packetHex.substring(index + 4, index + 6)
                    + packetHex.substring(index + 2, index + 4) + packetHex.substring(index, index + 2);
            index += 16;
        }

        finalPacket.srcMac = srcIP;
        finalPacket.dstMac = dstIP;

        /**
         * ******* END 802.15.4 HEADER *******
         */
        /**
         * ******* START 6LoWPAN HEADER *******
         */
        int hopLimit;
        int nextHeaderType;

        String finalSrcAddr = "";
        String finalDstAddr = "";

        String lowpanHeader = packetHex.substring(index, index + 2);

        boolean IPHC = !lowpanHeader.equals("41");
        if (IPHC) {
            lowpanHeader = packetHex.substring(index, index + 4);
            lowpanHeader = hexToBin(lowpanHeader);
            if (binToDec(lowpanHeader.substring(0, 3)) != 3) {
                throw new UnsupportedOperationException("Not yet implemented.");
            }
            int trafficClassFlow = binToDec(lowpanHeader.substring(3, 5));
            int nextHeader = binToDec(lowpanHeader.substring(5, 6));
            /*
            * https://tools.ietf.org/html/rfc6282#section-3
            * HOP LIMIT
            * 0 = inline
            * 1 = 1
            * 2 = 64
            * 3 = 255
             */
            hopLimit = binToDec(lowpanHeader.substring(6, 8));
            switch (hopLimit) {
                case 0:
                default:
                    hopLimit = 0;
                    break;
                case 1:
                    hopLimit = 1;
                    break;
                case 2:
                    hopLimit = 64;
                    break;
                case 3:
                    hopLimit = 255;
                    break;
            }
            finalPacket.hopLimit = hopLimit;
            int contextIdentifierExtension = binToDec(lowpanHeader.substring(8, 9));
            int srcAddrCompression = binToDec(lowpanHeader.substring(9, 10));
            /*
            * SAM
            * 0 = 128 bits, full address inline
            * 1 = 64 bits, first 64 bits of addr is link local prefix padded with 0s
            * 2 = 16 bits, first 64 bits of addr is llp^, next 64 is 0000:00ff:fe00:XXXX
            * 3 = 0 bits, first 64 bits of addr is llp^ next 64 computed from encapsulating hdr
             */
            int srcAddrMode = binToDec(lowpanHeader.substring(10, 12));
            /*
            * MC
            * 0 = dst is not multicast addr
            * 1 = dst is a multicast addr
             */
            boolean dstMulticast = binToDec(lowpanHeader.substring(12, 13)) == 1;
            int dstAddrCompression = binToDec(lowpanHeader.substring(13, 14));

            if (dstAddrCompression == 1 || srcAddrCompression == 1) {
                throw new UnsupportedOperationException("Not yet implemented.");
            }

            /*
            * https://tools.ietf.org/html/rfc6282#section-3 p9
            *
            * MC = 0, DAC = 0, matched SAC = 0 above
            *
            * MC = 1, DAC = 0
            * 0 = 128 bits, full address inline
            * 1 = 48 bits, ffXX::00XX:XXXX:XXXX
            * 2 = 32 bits, ffXX::00XX:XXXX
            * 3 = 8 bits, ff02::00XX
            *
             */
            int dstAddrMode = binToDec(lowpanHeader.substring(14, 16));

            index += 4;

            if (trafficClassFlow == 1) {
                index += 6;
            }

            // If not 58 (ICMPv6), throw error?
            nextHeaderType = hexToDec(packetHex.substring(index, index + 2));
            // If 00, next header is Hop by Hop options
            if (nextHeaderType == 0) {
                index += 2;
                finalPacket.type = Packet.TYPE_COAP;
                if (hopLimit == 0) {
                    finalPacket.hopLimit = hexToDec(packetHex.substring(index, index + 2));
                    index += 2;
                }
                // TODO implement non-inline src/dst
                if (dstAddrMode == 0 && srcAddrMode == 0) {
                    finalPacket.src = packetHex.substring(index, index + 32);
                    index += 32;
                    finalPacket.dst = packetHex.substring(index, index + 32);
                    index += 32;
                }
                //HOP BY HOP Options
                int hbhNextHeader = hexToDec(packetHex.substring(index, index + 2));

                if (hbhNextHeader != 17) {
                    throw new UnsupportedOperationException();
                }

                int hbhNextLength = hexToDec(packetHex.substring(index, index + 2));

                index += 2;
                //Skip RPL Option part
                index += 14;

                //UDP Header
                int udpSrcPort = hexToDec(packetHex.substring(index, index + 4));
                index += 4;
                int udpDstPort = hexToDec(packetHex.substring(index, index + 4));
                index += 4;
                int udpLength = hexToDec(packetHex.substring(index, index + 4));
                index += 4;
                int udpChecksum = hexToDec(packetHex.substring(index, index + 4));
                index += 4;

                //COAP Packet
                String coapOptions = hexToBin(packetHex.substring(index, index + 2));
                index += 2;

                int coapCode = hexToDec(packetHex.substring(index, index + 2));
                index += 2;
                int coapMsgId = hexToDec(packetHex.substring(index, index + 4));
                index += 4;
                String coapToken = packetHex.substring(index, index + 8);
                index += 8;
                if (coapCode == 69) {
                    finalPacket.subtype = Packet.COAP_CONTENT;
                } else if (coapCode == 1) {
                    finalPacket.subtype = Packet.COAP_GET;
                }

                return finalPacket;
            } else if (nextHeaderType != 58) {
                throw new UnsupportedOperationException();
            } else {
                finalPacket.type = Packet.TYPE_ICMP;
            }

            index += 2;

            switch (srcAddrMode) {
                case 0:
                    finalSrcAddr = packetHex.substring(index, index + 32);
                    index += 32;
                    break;
                case 1:
                    finalSrcAddr = "fe80000000000000" + packetHex.substring(index, index + 16);
                    index += 16;
                    break;
                case 2:
                    finalSrcAddr = "fe80000000000000000000fffe00" + packetHex.substring(index, index + 4);
                    index += 4;
                    break;
                case 3:
                default:
                    String two = srcIP.substring(1, 2);
                    two = hexToBin(two);
                    String invertedSeven = two.substring(2, 3).equals("0") ? "1" : "0";
                    two = two.substring(0, 2) + invertedSeven + two.substring(3, 4);
                    two = Integer.toString(binToDec(two), 16);
                    finalSrcAddr = "fe80000000000000" + srcIP.substring(0, 1) + two + srcIP.substring(2);
                    break;
            }

            if (dstMulticast) {
                switch (dstAddrMode) {
                    case 0:
                        finalDstAddr = packetHex.substring(index, index + 32);
                        index += 32;
                        break;
                    case 1:
                        finalDstAddr = "ff" + packetHex.substring(index, index + 2);
                        index += 2;
                        finalDstAddr += "000000000000000000" + packetHex.substring(index, index + 2);
                        index += 2;
                        finalDstAddr += packetHex.substring(index, index + 8);
                        index += 8;
                        break;
                    case 2:
                        finalDstAddr = "ff" + packetHex.substring(index, index + 2);
                        index += 2;
                        finalDstAddr += "0000000000000000000000" + packetHex.substring(index, index + 2);
                        index += 2;
                        finalDstAddr += packetHex.substring(index, index + 4);
                        index += 4;
                        break;
                    case 3:
                    default:
                        finalDstAddr = "ff0200000000000000000000000000" + packetHex.substring(index, index + 2);
                        index += 2;
                        break;
                }
            } else {
                switch (dstAddrMode) {
                    case 0:
                        finalDstAddr = packetHex.substring(index, index + 32);
                        index += 32;
                        break;
                    case 1:
                        finalDstAddr = "fe80000000000000" + packetHex.substring(index, index + 16);
                        index += 16;
                        break;
                    case 2:
                        finalDstAddr = "fe80000000000000000000fffe00" + packetHex.substring(index, index + 4);
                        index += 4;
                        break;
                    case 3:
                    default:
                        String two = dstIP.substring(1, 2);
                        two = hexToBin(two);
                        String invertedSeven = two.substring(2, 3).equals("0") ? "1" : "0";
                        two = two.substring(0, 2) + invertedSeven + two.substring(3, 4);
                        two = Integer.toString(binToDec(two), 16);
                        finalDstAddr = "fe80000000000000" + dstIP.substring(0, 1) + two + dstIP.substring(2);
                        break;
                }
            }

            if (dstMulticast || finalDstAddr.startsWith("ff02")) {
                finalPacket.multicast = true;
            }

        } else {
            index += 2;

            //SKIP Version/Traffic class/Flow Label/Length
            index += 12;

            nextHeaderType = hexToDec(packetHex.substring(index, index + 2));
            if (nextHeaderType != 58) {
                throw new UnsupportedOperationException();
            }
            index += 2;

            hopLimit = hexToDec(packetHex.substring(index, index + 2));
            index += 2;

            finalSrcAddr = packetHex.substring(index, index + 32);
            index += 32;

            finalDstAddr = packetHex.substring(index, index + 32);
            index += 32;

            finalPacket.hopLimit = hopLimit;
        }

        finalPacket.src = finalSrcAddr;
        finalPacket.dst = finalDstAddr;

        //Check for ICMP?
        String checksumMsg = packetHex.substring(index, index + 4) + "0000" + packetHex.substring(index + 8);
        // No idea why we have to subtract 74 to get correct checksum? Doesn't work for some packets..
        String checkLength = Integer.toHexString(checksumMsg.length() - 74);
        while (checkLength.length() < 8) {
            checkLength = "0" + checkLength;
        }
        checksumMsg = finalSrcAddr + finalDstAddr + checkLength + "000000" + "3a" + checksumMsg;

        long checksumL = this.calculateChecksum(hexToByte(checksumMsg));

        String icmpPacket = packetHex.substring(index + 2);
        String rplType = icmpPacket.substring(0, 2);
        switch (rplType) {
            // DODAG Information Solicitation
            // Source is requesting DODAG Information Object from Target
            case "00":
                finalPacket.type = Packet.TYPE_ICMP;
                finalPacket.subtype = Packet.RPL_DODAG_SOLICIT;
                // CHECKSUM (4) | FLAGS (2) | RESERVED (2)
                break;
            // DODAG Information Object
            case "01":
                finalPacket.subtype = Packet.RPL_DODAG_INFO;
                // CHECKSUM (4) | RPLInstanceID (2) | VERSION (2) | Rank (4) |
                // FLAGS (2) | DATSN (2) | FLAGS (2) | RESERVED (2) | DODAG ID (32) |
                // DODAG CONFIG (32) | PREFIX INFO (64)
                String dodagId = icmpPacket.substring(20, 52);
                String dodagConfig = icmpPacket.substring(52, 84);
                String prefixInfo = icmpPacket.substring(84, 148);
                break;
            // Destination Advertisement Object
            case "02":
                finalPacket.subtype = Packet.RPL_DEST_ADVERT;
                // CHECKSUM (4) | RPLInstanceID (2) | FLAGS (2) | RESERVED (2) | 
                // DAO Sequence (2) | DODAG ID (32) | RPL TARGET (variable) |
                // TRANSMIT INFO (6)
                break;
        }
        finalPacket.checksum = icmpPacket.substring(2, 6);
        long checksum = (long) hexToDec(finalPacket.checksum);
        finalPacket.checksumConf = (checksum == checksumL);
        if(!finalPacket.checksumConf) {
            //System.out.println("CHECKSUM FAILED: " + checksum + " | " + checksumL);
        }

        return finalPacket;
    }

    /**
     * Converts a hex string to a binary string
     *
     * @param hex the hex to be converted
     * @return a binary string equivalent to the hex provided
     */
    public static String hexToBin(String hex) {
        int i = Integer.parseInt(hex, 16);
        String bin = Integer.toBinaryString(i);
        while (bin.length() < hex.length() * 4) {
            bin = "0" + bin;
        }
        return bin;
    }

    /**
     * Converts a binary string to a decimal number
     *
     * @param binary the binary to convert
     * @return the resulting decimal integer
     */
    public static int binToDec(String binary) {
        return Integer.parseInt(binary, 2);
    }

    /**
     * Converts a hexadecimal string to the equivalent decimal integer
     *
     * @param hex the hexadecimal to convert
     * @return the resulting decimal integer
     */
    public static int hexToDec(String hex) {
        return binToDec(hexToBin(hex));
    }

    /**
     * Converts a hexadecimal string into an array of bytes
     *
     * @param s the hexadecimal string to convert
     * @return the resulting byte array
     */
    public static byte[] hexToByte(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i + 1), 16));
        }
        return data;
    }

    /**
     * Calculate the Internet Checksum of a buffer (RFC 1071 -
     * http://www.faqs.org/rfcs/rfc1071.html) Algorithm is 1) apply a 16-bit 1's
     * complement sum over all octets (adjacent 8-bit pairs [A,B], final odd
     * length is [A,0]) 2) apply 1's complement to this final sum
     *
     * Notes: 1's complement is bitwise NOT of positive value. Ensure that any
     * carry bits are added back to avoid off-by-one errors
     * http://stackoverflow.com/questions/4113890/how-to-calculate-the-internet-checksum-from-a-byte-in-java
     *
     * @param buf The message
     * @return The checksum
     */
    public long calculateChecksum(byte[] buf) {
        int length = buf.length;
        int i = 0;

        long sum = 0;
        long data;

        // Handle all pairs
        while (length > 1) {
            // Corrected to include @Andy's edits and various comments on Stack Overflow
            data = (((buf[i] << 8) & 0xFF00) | ((buf[i + 1]) & 0xFF));
            sum += data;
            // 1's complement carry bit correction in 16-bits (detecting sign extension)
            if ((sum & 0xFFFF0000) > 0) {
                sum = sum & 0xFFFF;
                sum += 1;
            }

            i += 2;
            length -= 2;
        }

        // Handle remaining byte in odd length buffers
        if (length > 0) {
            // Corrected to include @Andy's edits and various comments on Stack Overflow
            sum += (buf[i] << 8 & 0xFF00);
            // 1's complement carry bit correction in 16-bits (detecting sign extension)
            if ((sum & 0xFFFF0000) > 0) {
                sum = sum & 0xFFFF;
                sum += 1;
            }
        }

        // Final 1's complement value correction to 16-bits
        sum = ~sum;
        sum = sum & 0xFFFF;
        return sum;

    }
}
