#!/bin/bash

(cd rpl-border-router-improved/ && make TARGET=z1 border-router)
(cd rpl-border-router-improved/ && make TARGET=z1-feshie border-router)

(cd z1-sampler/ && make TARGET=z1 z1-sampler)
(cd z1-sampler/ && make TARGET=z1-feshie z1-sampler)

(cd http-proxy-node/ && make TARGET=z1 http-proxy)
(cd http-proxy-node/ && make TARGET=z1-feshie http-proxy)
