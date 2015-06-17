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
	gets broken-down date tiem and POSTs it 
	"""
	context = yield from Context.create_client_context()

#    yield from asyncio.sleep(2)

	now = datetime.datetime.now()
	y = now.year - 2000
	m = now.month
	d = now.day
	h = now.hour
	print("min= %d" % now.minute )
	print("sec= %d" % now.second )
	payload = b"y
	payload = b"The quick brown fox jumps over the lazy dog.\n" * 30
	request = Message(code=PUT, payload=payload)
	request.opt.uri_host = '127.0.0.1'
	request.opt.uri_path = ("other", "block")

	response = yield from context.request(request).response

	print('Result: %s\n%r'%(response.code, response.payload))

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
