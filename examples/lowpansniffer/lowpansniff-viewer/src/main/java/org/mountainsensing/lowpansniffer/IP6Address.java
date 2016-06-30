/**
 * RPL Sniffing Control Application
 * Edward Crampin, University of Southampton, 2016
 * mountainsensing.org
 */
package org.mountainsensing.lowpansniffer;

import java.net.Inet6Address;
import java.net.UnknownHostException;

/**
 * A class to handle the formal outputting of IPv6 addresses.
 * @author Ed Crampin
 */
public class IP6Address {
    /**
     * A function that takes in an unformatted, 128 bit hex address (with no colon delimiters)
     * and parses it into it's shorthand IPv6 form.
     * @param addr  unformatted 128 bit IPv6 address
     * @return      shorthand version of 128 bit input address
     * @throws UnknownHostException if address cannot be parsed by Inet6Address
     * @see Inet6Address
     */
    public static String parse(String addr) throws UnknownHostException {
        String ret;
        if(!addr.contains(":")) {
            ret = "";
            for(int i = 0; i < addr.length(); i+=4) {
                ret += addr.substring(i, i + 4);
                ret += ":";
            }
            ret = ret.substring(0, ret.length() - 1);
        } else {
            ret = addr;
        }
        
        String inet6 = Inet6Address.getByName(ret).toString();
        String resultString = inet6.replaceAll("((?::0\\b){2,}):?(?!\\S*\\b\\1:0\\b)(\\S*)", "::$2");
        return resultString.substring(1);
    }
}
