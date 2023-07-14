import socket

def start_tcp_server(host, port):
    # Create a TCP/IP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Bind the socket to the host and port
    server_socket.bind((host, port))
    
    # Listen for incoming connections
    server_socket.listen(1)
    print(f"Server is listening on {host}:{port}...")

    # Wait for a connection
    print("Waiting for a connection...")
    client_socket, client_address = server_socket.accept()
    print(f"Connected to client: {client_address}")
    
    try:
        while True:
            # Receive data from the client
            data = client_socket.recv(1024).decode().strip()
            if not data:
                break
            
            print(f"Received data from client: {data}")
            
    except socket.error as e:
        print(f"Error: {e}")
            
    finally:
        # Close the connection
        client_socket.close()

# Start the TCP server
host = '192.168.0.2'  # Change this to your desired host
port = 1234  # Change this to your desired port
start_tcp_server(host, port)
