import socket	# for sockets
import stat	# for file permission checking
import sys	# for getting overriding port number from program args
import os	# for checking if file exists
import datetime	# for constructing Date response header

from threading import Thread		# for data processing
from argparse import ArgumentParser	# for...

# utility
BUFSIZE = 4096
CRLF = '\r\n'

# status lines
OK = 'HTTP/1.1 200 OK{}'.format(CRLF)
NOTFOUND = 'HTTP/1.1 404 Not Found{}'.format(CRLF)
FORBIDDEN = 'HTTP/1.1 403 Forbidden{}'.format(CRLF)
NOTALLOWED = 'HTTP/1.1 405 Method Not Allowed{}'.format(CRLF)
NOTACCEPTABLE = 'HTTP/1.1 406 Not Acceptable{}'.format(CRLF)
REDIRECT = 'HTTP/1.1 301 Moved Permanently{}'.format(CRLF)

# supported resource types
HTML = 'text/html'
JPEG = 'image/jpeg'
GIF = 'image/gif'
PDF = 'application/pdf'
DOC = 'application/msword'
PPTX = 'application/vnd.openxmlformats-officedocument.presentationml.presentation'

# request types
GET = 'GET'
HEAD = 'HEAD'

FILE_CALENDAR = 'calendar.html'
FILE_PRIVATE = 'private.html'	
FILE_403 = '403.html'
FILE_404 = '404.html'
UMN_EDU = 'https://www.cs.umn.edu'

# this function return the extension of a resource
def get_extension(resource_path):
	return resource_path.split('.', 1)[1]

def accept_check(req, resource_path):
	resource_t = get_extension(resource_path)
	for item in req.split(CRLF):			
		if "Accept:" in item:
			if any([HTML in item, JPEG in item, GIF in item, PDF in item, DOC in item, PPTX in item]):
				return True
			else:
				return False
	return False

# this function constructs a response to return to the client
def construct_response(resource_path, status, req_t):
	# begin contructing response status lines & response body
	response = ''
	body = ''
	if (status == 200):
		response += OK
		if (req_t == HEAD):	# return status line only
			return response
		body += open(resource_path, 'r').read()	# construct body
	elif (status == 403):
		response += FORBIDDEN
		if (req_t == HEAD):
			return response
		body += open(FILE_403, 'r').read()
	elif (status == 404):
		response += NOTFOUND
		if (req_t == HEAD):
			return response
		body += open(FILE_404, 'r').read()
	elif (status == 405):
		response += NOTALLOWED
		if (req_t == HEAD):
			return response
		body += '405: NOT ALLOWED\n' + 'The request method must be GET or HEAD.\n'
	elif (status == 406):
		response += NOTACCEPTABLE
		if (req_t == HEAD):
			return response
		body += '406: NOT ACCEPTABLE\n' + 'Accept header content not acceptable.\n'

	# add response headers
	if ((req_t == GET) and (os.path.exists(resource_path))):
		response += ('Date: ' + str(datetime.date) + CRLF)
		response += ('Last-Modified: ' + str(os.path.getmtime(resource_path)) + CRLF)
		response += ('Content-Length: ' + str(os.path.getsize(resource_path)) + CRLF)
		resource_t = get_extension(resource_path)
		if (resource_t == 'html'):
			response += ('Content-Type: ' + HTML + CRLF)
		elif (resource_t == 'jpeg'):
			response += ('Content-Type: ' + JPEG + CRLF)
		elif (resource_t == 'gif'):
			response += ('Content-Type: ' + GIF + CRLF)
		elif (resource_t == 'pdf'):
			response += ('Content-Type: ' + PDF + CRLF)
		elif (resource_t == 'doc'):
			response += ('Content-Type: ' + DOC + CRLF)
		elif (resource_t == 'pptx'):
			response += ('Content-Type: ' + PPTX + CRLF)

	response += CRLF
	response += body
	return response 

# this function constructs a response for a redirection
def construct_redirect():
	response = ''
	response += REDIRECT
	response += ('Location: ' + UMN_EDU + CRLF)
	return response

def process_request(req):
	print('***** LOGGING REQUEST *****\n' + req)
	req_t = req.split(' ', 1)[0]
	resource_path = req.split(' ', 2)[1]	# get requested resource path from request
	if ((req_t == GET) or (req_t == HEAD)):
		# remove leading '/' if it exists or it will look for resource in root of file system
		if (resource_path[0] == '/'):
			resource_path = resource_path[1:]			
		if (resource_path == 'csumn'):
			response = construct_redirect()
			return response
		else:	# then redirection response not necessary
			if (os.path.exists(resource_path)):
				statinfo = os.stat(resource_path)	# get permission info
				if (bool(statinfo.st_mode & stat.S_IROTH)):
					if (accept_check(req, resource_path)):	# check header content
						response = construct_response(resource_path, 200, req_t)
						return response	
					else:	# content header not acceptable
						response = construct_response(resource_path, 406, req_t)
						return response
				else: 	# inappropriate permissions for accessing requested resource
					response = construct_response(resource_path, 403, req_t)
					return response	
			else:	# requested resource does not exist
				response = construct_response(resource_path, 404, req_t)
				return response		
	else:	# method not allowed
		response = construct_response(resource_path, 405, req_t)
		return response	

def client_talk(client_sock, client_addr):
	print('talking to {}'.format(client_addr))
	data = client_sock.recv(BUFSIZE)

	req = data.decode('utf-8')
	response = process_request(req)
	client_sock.send(bytes(response, 'utf-8'))

	client_sock.shutdown(1)
	client_sock.close()
	print('connection closed.')

class EchoServer:
	def __init__(self, host, port):
		print('listening on port {}'.format(port))
		self.host = host
		self.port = port
		self.setup_socket()
		self.accept()
		self.sock.shutdown(socket.SHUT_RDWR)
		self.sock.close()

	def setup_socket(self):
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.bind((self.host, self.port))
		self.sock.listen(128)

	def accept(self):
		while True:
			(client, address) = self.sock.accept()
			th = Thread(target=client_talk, args=(client, address))
			th.start()

# This function parses the command line arguments for a port number that overrides 9001.
def parse_args():

	if (len(sys.argv) > 1):
		port = int(sys.argv[1])
		return ('localhost', port)
	else:
		return ('localhost', 9001)

if __name__ == '__main__':
	(host, port) = parse_args()
	EchoServer(host, port)

