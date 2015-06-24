#!/usr/bin/env python3
# POST the current time to a node

import logging
import asyncio
import datetime

from aiocoap import *

logging.basicConfig(level=logging.INFO)

@asyncio.coroutine
def main():
    """
    gets broken-down date-time and POSTs it to set the node rtc
    """
    context = yield from Context.create_client_context()

#    yield from asyncio.sleep(2)

    now = datetime.datetime.now()
    y = now.year
    m = now.month
    d = now.day
    h = now.hour
    mi = now.minute
    se = now.second

    payload = b"y"
    #payload = b"The quick brown fox jumps over the lazy dog.\n" * 30
# not sure if we need a payload!
    request = Message(code=POST, payload=payload)
    request.opt.uri_host = 'aaaa::c30c:0:0:1058'
    URI = "/date?y=%d&mo=%d&d=%d&h=%d&mi=%d&se=%d" % (y,m,d,h,mi,se)
    print("uri: %s\n" % URI)
    request.opt.uri_path = (URI, "block")

    response = yield from context.request(request).response

    print('Result: %s\n%r'%(response.code, response.payload))

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
