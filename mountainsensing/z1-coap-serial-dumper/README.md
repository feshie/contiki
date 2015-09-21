# Serial-Dumper

Application to extract Samples saved in the flash by z1-coap, and dump them over the serial link
as hex encoded delimited protocol-buffers.

The dumped serial output can be parsed by the fetcher, using either `decode-sample -s` or `decode-config -s`
to decode the configuration and the samples respectively.

## Using with older deployments

In order to use `serial-dumper` to recover nodes that were running an older version of `z1-coap`,
it is necessary to use the coffee-cfs configuration that was in use at the time.

This can be achieved by running, on `mountain-sensing-master`:

    git checkout $TAG platform/z1-feshie/cfs-coffee-arch.h

For example, for the June 2015 deployment:

    git checkout 2015-07-deployment platform/z1-feshie/cfs-coffee-arch.h

