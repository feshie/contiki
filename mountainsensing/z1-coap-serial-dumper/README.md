# Serial-Dumper

Application to extract Samples saved in the flash by z1-coap, and dump them over the serial link
as hex encoded delimited protocol-buffers.

The dumped serial output can be parsed by the fetcher, using either `decode-sample -s` or `decode-config -s`
to decode the configuration and the samples respectively.
