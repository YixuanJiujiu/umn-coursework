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
OK = 'HTTP/1.1 200 OK{}{}'.format(CRLF, CRLF, CRLF)
NOTFOUND = 'HTTP/1.1 404 Not Found{}{}'.format(CRLF, CRLF, CRLF)
FORBIDDEN = 'HTTP/1.1 403 Forbidden{}{}'.format(CRLF, CRLF, CRLF)
NOTALLOWED = 'HTTP/1.1 405 Method Not Allowed{}{}'.format(CRLF, CRLF)
NOTACCEPTABLE = 'HTTP/1.1 406 Not Acceptable{}{}'.format(CRLF, CRLF)

# supported resource types
HTML = 'text/html'
JPEG = 'image/jpeg'
GIF = 'image/gif'
PDF = 'application/pdf'
DOC = 'application/msword'

# request types
GET = 'GET'
HEAD = 'HEAD'

FILE_CALENDAR = 'calendar.html'
FILE_PRIVATE = 'private.html'
FILE_403 = '403.html'
FILE_404 = '404.html'

def get_extension(resource_path):
	return resource_path.split('.', 1)[1]

def accept_check(req, resource_path):
	resource_t = get_extension(resource_path)
	for item in req.split(CRLF):			
		if "Accept:" in item:
#			print('debug - accept header: ' + item)
			if any(['html' in item, 'jpeg' in item, 'gif' in item, 'pdf' in item, 'doc' in item]):
				return True
			else:
				return False
	return False

def construct_response(resource_path, status):
	response = ''
	body = ''
	if (status == 200):
		response += OK
		body += open(resource_path, 'r').read()
	elif (status == 403):
		response += FORBIDDEN
		body += open(FILE_403, 'r').read()
	elif (status == 404):
		response += NOTFOUND
		body += open(FILE_404, 'r').read()
	elif (status == 405):
		response += NOTALLOWED
		body += open(FILE_PRIVATE, 'r').read()
	elif (status == 406):
		response += NOTACCEPTABLE
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
#	print('debug - body: ' + body)
	response += CRLF
	response += body
	return response 

def process_request(req):
	req_t = req.split(' ', 1)[0]
	if (req_t == GET or req_t == HEAD):
		resource_path = req.split(' ', 2)[1]
		if (resource_path.find('%') == -1):
			resource_path = resource_path[1:]	# remove leading '/' or it will look for resource in root of  file system
			if (os.path.exists(resource_path)):
				statinfo = os.stat(resource_path)
				if (bool(statinfo.st_mode & stat.S_IROTH)):
#					print('debug - requested resource path: ' + resource_path)
					if (accept_check(req, resource_path)):
						response = construct_response(resource_path, 200)
#						print('response')
						return response	# 200
					else:
						print('406')
						response = construct_response(resource_path, 406)
						return response
				else: 
					response = construct_response(resource_path, 403)
					return response	# 403
			else:
				response = construct_response(resource_path, 404)
				return response		# 404
		else:
			print('invalid header characters')
			return NOTALLOWED
	else:
		response = construct_response(resource_path, 404)
		return response	# 404

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
		#while True:
		(client, address) = self.sock.accept()
		th = Thread(target=client_talk, args=(client, address))
		th.start()

def parse_args():
	parser = ArgumentParser()
	parser.add_argument('--host', type=str, default='localhost',
		      help='specify a host to operate on (default: localhost)')
	parser.add_argument('-p', '--port', type=int, default=9001,
		      help='specify a port to operate on (default: 9001)')
	args = parser.parse_args()
	return (args.host, args.port)


if __name__ == '__main__':
	(host, port) = parse_args()
	EchoServer(host, port)

