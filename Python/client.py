# ------------------------------------------------------UDP------------------------------------------------------
# import socket
# import select

# # Sends a message to the server and receives the response.
# def send_and_receive(server_address, message):
#   bufferSize = 1024
#   timeout = 5  # Set a timeout value of 5 seconds
#   UDPClient = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Create a UDP socket
#   UDPClient.setblocking(0)  # Set socket to non-blocking mode
#   print('Sending message to server')
#   UDPClient.sendto(message.encode('utf-8'), server_address)

#   ready = select.select([UDPClient], [], [], timeout)  # Wait for socket to become readable
#   if ready[0]:  # If socket is readable
#     print('Receiving the response from the server')
#     data, address = UDPClient.recvfrom(bufferSize)
#     data = data.decode('utf-8')
#     return data, address
#   else:  # Timeout occurred
#     print('Timeout occurred. No response from the server.')
#     return None, None, None

# def main():
#   serverAddress = ('192.168.0.123', 7)
#   print("UDP Client started.")
#   while True:
#     msg = input('What do you want to send to server? ')
#     if msg != '':  # Validate the message
#       response, address  = send_and_receive(serverAddress, msg)  # Send the command to the server and receive the response
#       if response is not None:
#         print('Server IP address: {0}. Port: {1}'.format(address[0], address[1]))
#         print('Data from server:', response)
#     else:
#       print('Invalid or empty message.')

# if __name__ == "__main__":
#   main()




# ------------------------------------------------------TCP------------------------------------------------------
# import socket

# # Sends a message to the server and receives the response over TCP.
# def send_and_receive(server_address, message):
#   bufferSize = 1024
#   timeout = 5  # Set a timeout value of 5 seconds
#   TCPClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a TCP socket
#   TCPClient.settimeout(timeout)  # Set socket timeout
#   TCPClient.connect(server_address)
#   print('Connect successfully. Server IP address: {0}. Port: {1}'.format(server_address[0], server_address[1]))
  
#   TCPClient.sendall(message.encode('utf-8'))
#   print('Message is sent.')

#   try:
#     print('Receiving the response from the server')
#     data = TCPClient.recv(bufferSize)
#     data = data.decode('utf-8')
#     return data
#   except socket.timeout:
#     print('Timeout occurred/No response from the server.')
#     return None
#   finally:
#     TCPClient.close()

# def main():
#   serverAddress = ('192.168.0.123', 7)
#   print("TCP Client started.")
#   while True:
#     msg = input('\nWhat do you want to send to server? ')
#     if msg != '':  # Validate the message
#       response = send_and_receive(serverAddress, msg)  # Send the command to the server and receive the response
#       if response is not None:
#         print('Data received:', response) 
#     else:
#       print('Invalid or empty message.')

# if __name__ == "__main__":
#   main()


import socket
import threading

# Sends a message to the server and receives the response over TCP.
def send_and_receive(server_address, message):
    bufferSize = 1024
    timeout = 5  # Set a timeout value of 5 seconds
    TCPClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a TCP socket
    TCPClient.settimeout(timeout)  # Set socket timeout
    TCPClient.connect(server_address)
    print('Connect successfully. Server IP address: {0}. Port: {1}'.format(server_address[0], server_address[1]))

    TCPClient.sendall(message.encode('utf-8'))
    print('Message is sent.')

    try:
        print('Receiving the response from the server')
        data = TCPClient.recv(bufferSize)
        data = data.decode('utf-8')
        return data
    except socket.timeout:
        print('Timeout occurred/No response from the server.')
        return None
    finally:
        TCPClient.close()

def main():
    serverAddress = ('192.168.0.123', 7)
    # print("TCP Client started.")

    # msg = input('\nWhat do you want to send to server? ')
    # if msg != '':  # Validate the message
    #     response = send_and_receive(serverAddress, msg)  # Send the command to the server and receive the response
    #     if response is not None:
    #         print('Data received:', response)
    # else:
    #     print('Invalid or empty message.')



    while True:
        # Setup for listen mode
        bufferSize = 1024
        timeout = 5  # Set a timeout value of 5 seconds
        TCPClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a TCP socket
        TCPClient.settimeout(timeout)  # Set socket timeout
        TCPClient.connect(serverAddress)
        print('Connect successfully. Server IP address: {0}. Port: {1}'.format(serverAddress[0], serverAddress[1]))
        try:
            data = TCPClient.recv(bufferSize)
            data_size = (data[1] << 8) | data[0]
            data = data[2:2+data_size]
            try:
              data = data.decode('utf-8')
              print('Data received:', data)
            except UnicodeDecodeError:
              print('Failed to decode message.')
              print('Data received:', data)
            
        except socket.timeout:
            print('No response from the server.')

        finally:
          TCPClient.close()

    

if __name__ == "__main__":
    main()


# import socket
# import struct

# can_id = 0x741
# can_data = [1, 2, 3, 4, 5, 6, 7, 8]
# byte_array = bytearray(can_id.to_bytes(4, 'little') + bytes(can_data))

# # Sends a byte array to the server and receives the response over TCP.
# def send_and_receive(server_address, byte_array):
#   bufferSize = 1024
#   timeout = 5  # Set a timeout value of 5 seconds
#   TCPClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a TCP socket
#   TCPClient.settimeout(timeout)  # Set socket timeout
#   TCPClient.connect(server_address)
#   print('Connect successfully. Server IP address: {0}. Port: {1}'.format(server_address[0], server_address[1]))

#   TCPClient.sendall(byte_array)
#   print('Byte array is sent.')

#   try:
#     print('Receiving the response from the server')
#     data = TCPClient.recv(bufferSize)
#     data = data.decode('utf-8')
#     return data
#   except socket.timeout:
#     print('Timeout occurred/No response from the server.')
#     return None
#   finally:
#     TCPClient.close()

# def main():
#   serverAddress = ('192.168.0.123', 7)
#   print("TCP Client started.")
#   while True:
#     # msg = input('\nWhat do you want to send to server? ')
#     # if msg != '':  # Validate the message
#       # byte_array = bytes(msg, 'utf-8')  # Convert the message to a byte array
#       # byte_array = bytearray([0x41, 0x07, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08])
#       response = send_and_receive(serverAddress, byte_array)  # Send the byte array to the server and receive the response
#       if response is not None:
#         print('Data received:', response)
#     # else:
#     #   print('Invalid or empty message.')

# if __name__ == "__main__":
#   main()





# ------------------------------------------------------TFTP------------------------------------------------------
# from tftpy import TftpClient

# def send_file_via_tftp(server_ip, local_file_path, remote_file_name):
#     try:
#         # Open the local file in binary mode
#         with open(local_file_path, 'rb') as file:
#             # Create a TFTP client
#             client = TftpClient(host=server_ip, port=69)
#             client.options['blksize'] = 1024  # Set the block size to the desired value (adjust as needed)
#             client.options['timeout'] = 10  # Set the timeout value (adjust as needed)
#             client.options['tsize'] = len(file.read())  # Set the total file size

#             # Reset the file position to the beginning
#             file.seek(0)

#             # Upload the file
#             client.upload(remote_file_name, file)

#         print("File sent successfully!")
#     except Exception as e:
#         print(f"An error occurred while sending the file: {str(e)}")

# def main():
#     # Configuration
#     server_ip = "192.168.0.123"
#     local_file_path = "SMC_APP_Signature_Merged_6Ch_enc.bin"
#     remote_file_name = "file_received.bin"  # Specify the desired remote file name

#     # Send the file via TFTP
#     send_file_via_tftp(server_ip, local_file_path, remote_file_name)

# if __name__ == "__main__":
#     main()






