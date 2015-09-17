# MountainSensing

This folder contains the main applications developed as part of the MountainSensing project.
They have been developed to run on the Zolertia z1 platform,
but could be ported to other platforms.
External sensors (such as an RTC) may be required.

### Applications

#### `z1-coap`

`z1-coap` is the main application.
It periodically takes samples, and stores them in flash.
It has a CoAP interface to retrieve the samples,
and to configure the node.
`fetcher` can be used to retrieve samples and configure the node.

#### `z1-coap-serial-dumper`

The serial-dumper is a rescue application for `z1-coap`.
It dumps the samples and configuration files as saved by
`z1-coap` off the flash and to the serial link.
`fetcher` can decode the dump produced by it.

#### `z1-node-setup`

`z1-node-setup` prepares the node to run `z1-coap`.
This includes things like formatting the flash.

#### `z1-nothing`

This is an application that does nothing.
Useful for testing purposes,
or for a reliable router only node.

#### `sensors-test`

This folder contains a suite of applications used
for testing the sensors available on the z1.

#### `rpl-border-router`

This application in the RPL border router used.
It contains slight modifications from the default
one in `examples/`

#### `cfs-utils`

This folder contains a variety of applications
useful for working with Coffee.
Some can dump the contents of flash to serial,
others can list the files available,
or attempt to fill the flash.
