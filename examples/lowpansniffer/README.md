# LoWPANSniffer

LoWPANSniffer is a sniffer application to fully map and analyse a network of 6LoWPAN devices using RPL and CoAP.

### Features

* Sniffer packet table displaying all RPL and CoAP packets obtained by the sniffer node
* Directed graph displaying network traffic between nodes

### Build & Run
To build, upload and run the tool, run the following.
```
$ make TARGET=<target> MOTES=<serial port> lowpan-sniffer.upload view
```

### Implementation
The sniffer works by using a fake RDC driver that routes all received packets, including those not intended for it, through the *sniffer_callback()* function. This function then dumps the raw packet out, allowing the Java application to fetch the received packets.

The sniffer node itself does not act as an intermediary device within the network. If no packets are being received, ensure that the sniffer node is on the same RF channel and PAN as the network that is being sniffed.

[Ed Crampin](http://edcrampin.co.uk) // [Feshie Project Repo](http://github.com/feshie) // [University of Southampton](http://soton.ac.uk)
