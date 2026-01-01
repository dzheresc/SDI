# UDP Receiver

A simple Windows console application that listens on UDP port 5555 and prints all received data to the console.

## Features

- Listens on UDP port 5555 on all network interfaces
- Displays sender IP address and port for each received packet
- Shows received data as text (if printable) or hexadecimal format
- Real-time packet reception and display
- Error handling for network operations

## Requirements

- Windows 10 or later
- Visual Studio 2022 (or compatible IDE)
- C++17 compatible compiler

## Building the Project

1. Open `udp_recv.sln` in Visual Studio 2022
2. Select your desired configuration (Debug or Release) and platform (x64)
3. Build the solution:
   - Press `F7`, or
   - Go to `Build` → `Build Solution`, or
   - Use `Ctrl+Shift+B`

The executable will be created in:
- Debug: `x64\Debug\udp_recv.exe`
- Release: `x64\Release\udp_recv.exe`

## Usage

1. Run the executable (`udp_recv.exe`)
2. The program will start listening on UDP port 5555
3. Send UDP packets to port 5555 on the machine running the program
4. Received data will be displayed in the console with sender information
5. Press `Ctrl+C` to stop the program

### Example Output

```
UDP listener started on port 5555
Waiting for data... (Press Ctrl+C to exit)

Received 12 bytes from 192.168.1.100:54321
Data: Hello World!

Received 8 bytes from 10.0.0.5:12345
Data: [Hex: 48 65 6C 6C 6F 21 0A 00 ]
```

## Testing

You can test the UDP receiver using various tools:

### Using PowerShell
```powershell
$udpClient = New-Object System.Net.Sockets.UdpClient
$endpoint = New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Parse("127.0.0.1"), 5555)
$bytes = [System.Text.Encoding]::ASCII.GetBytes("Test message")
$udpClient.Send($bytes, $bytes.Length, $endpoint)
$udpClient.Close()
```

### Using netcat (if installed)
```bash
echo "Hello from netcat" | nc -u 127.0.0.1 5555
```

### Using Python
```python
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hello from Python", ("127.0.0.1", 5555))
sock.close()
```

## Project Structure

```
udp_recv/
├── udp_recv.sln          # Visual Studio solution file
├── udp_recv.vcxproj      # Project configuration
├── udp_recv.vcxproj.filters  # Solution Explorer organization
├── main.cpp              # Main source code
├── main.h                # Header file
└── README.md             # This file
```

## Technical Details

- **Language**: C++
- **Standard**: C++17
- **Networking**: Winsock2 (ws2_32.lib)
- **Socket Type**: UDP (SOCK_DGRAM)
- **Port**: 5555 (configurable in `main.cpp`)
- **Buffer Size**: 4096 bytes

## Configuration

To change the listening port, modify the `UDP_PORT` constant in `main.cpp`:

```cpp
const int UDP_PORT = 5555;  // Change to desired port
```

## Troubleshooting

### Port Already in Use
If you see a bind error, another application may be using port 5555. Either:
- Stop the other application, or
- Change the port number in `main.cpp`

### Firewall Issues
Windows Firewall may block incoming UDP packets. You may need to:
- Allow the program through Windows Firewall, or
- Disable the firewall temporarily for testing

### Administrator Privileges
Some ports may require administrator privileges. If needed, run the executable as Administrator.

## License

This is a simple utility project. Use as needed.

