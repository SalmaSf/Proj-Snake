import socket
import time
import struct

server_ip = "127.0.0.1"
server_port = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(2)  # wait max 2 seconds for a reply

mouse_data = [
    (100, 100),
    (200, 150),
    (300, 200),
    (400, 250)
]

for coord in mouse_data:
    x, y = coord
    # Send as raw bytes (int x, int y) using struct
    sock.sendto(struct.pack('ii', x, y), (server_ip, server_port))
    print(f"Sent: ({x}, {y})")

    try:
        data, _ = sock.recvfrom(1024)
        server_x, server_y = struct.unpack('ii', data[:8])
        print(f"Received from server: x={server_x}, y={server_y}")
    except socket.timeout:
        print("No response from server.")
    
    time.sleep(1)

sock.close()
