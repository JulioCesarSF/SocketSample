
import requests
import json

# Server vars
serverAddress = "http://127.0.0.1"
serverPort = "1248"
serverFinalAddress = serverAddress + ":" + serverPort

print("Sending requests to server at: " + serverFinalAddress)

# EndPoints
endPoints = {
	"GET" : ["/",  "/json"],	
	"POST" : ["/",  "/process"]
}

# Send a GET request
def sendGet(endPoint) :
	print("Sending GET to : " + endPoint)
	response = requests.get(serverFinalAddress + endPoint)
	print("STATUS CODE:\r\n" + str(response.status_code))
	print("CONTENT:\r\n" + response.text)

# Send a POST request
def sendPost(endPoint) :
	print("> Sending POST to : " + endPoint)
	headers = { "Content-Type" : "application/json" }
	payload = { "name" : "Test" }
	response = requests.post(serverFinalAddress + endPoint, data = json.dumps(payload), headers = headers)
	print("STATUS CODE:\r\n" + str(response.status_code))
	print("CONTENT:\r\n" + response.text)	

if __name__ == "__main__" :

	for key, value in endPoints.items() :
		if key == "GET" :
			for endPoint in value :
				sendGet(endPoint)
				print("")
		if key == "POST" :
			for endPoint in value :
				sendPost(endPoint)
				print("")
