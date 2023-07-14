import subprocess

def ping_test(hostname):
  """
  Perform a ping test on the specified hostname or IP address.
  """
  # Use the 'ping' command with the specified count of 4 packets
  ping_process = subprocess.Popen(['ping', '-n', '4', hostname], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

  # Wait for the ping process to finish and get the output
  ping_output, ping_error = ping_process.communicate()

  # Decode the output from bytes to string
  ping_output = ping_output.decode('utf-8')

  # Display the ping output
  print("Ping Test Results:")
  print("------------------")
  print(ping_output)

  # Check if the ping was successful or not
  if ping_process.returncode == 0:
    print(f"\nPing test to {hostname} was successful!")
  else:
    print(f"\nPing test to {hostname} failed!")

# Ask the user to enter the hostname or IP address to ping
hostname = input("Enter the hostname or IP address to ping: ")

def main():
  # Call the ping_test function with the provided hostname
  ping_test(hostname)

if __name__ == "__main__":
  main()

