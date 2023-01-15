import requests

class server_client(object):
	def __init__(self):
		pass

	def default_get(self):
		url = "http://127.0.0.1:8/"
		print (requests.get(url).content)
	def dummy_post(self, data):
		url = "http://127.0.0.1:8/dummy"
		print (requests.post(url, json = data).content)
