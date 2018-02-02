# Genericor Generic Packet Format

## Summary
GGPF is a simple message format for exchanging short, arbitrary
commands and responses between sensors and a data collector over an XBee radio link. The maximum payload size is 64 bytes (to fit inside an unfragmented XBee S2C frame along with 16 bytes of header data)

## Message Format

### Message Header

- 2 bytes: GGPF Identifier
  - Always 0x7A69
  - Messages without this header are discarded
- 1 byte: GGPF version
  - Currently 0x01 (version 1)
- 4 bytes - Station ID
  - This is the low 4 bytes of the XBee MAC
  - For collector -> sensor traffic, this is the destination
  - For sensor -> collector traffic, this is the source
- 4 bytes - Time
  - 32-bit UNIX timestamp
- 1 byte: Message type
  - 0x00 - NOOP
  - 0x01 - Response - contains a response to a command
  - 0x02 - Command - contains a command
  - 0x03 - Event - contains a sensor event
  - 0x04 - Log - contains operational data sent to controller
- 1 byte: Request ID
  - For Response (0x01) messages, the Request ID of the Command message that this message is in response to
  - For Command messages, an arbitrary number to identify this Request
  - For Event and Log messages, this byte is set to 0x00
- 1 byte: Message Subtype
  - Identifies the specific command, response, event, or log type.
- 1 byte: Data length
  - Identifies the number of bytes remaining in the message.
  - Valid range: 0-64
- 0-64 bytes - Message data (string)
- 1 byte: Terminator
  - 0x00

### Message Types
#### 0x01 responses
- 0x01 Pong
  - Response to 0x01 ping message
  - Data: same string as ping message
- 0x03 Reply reader frequency
  - Response to 0x03 get reader frequency
  - Data: 5 character MOF output
    - nnn.n

#### 0x02 commands
- 0x01 Ping
  - Request a response. Include data in responses
  - Data: up to 64 characters of string data
- 0x03 Get reader frequency (blocking)
  - Execute MOF command on microchip reader
  - Data: NULL

#### 0x03 events
- 0x01 Tag Scan
  - The scanner has read a microchip
  - Data: 16 character FDX-B read
    - &lt;3 digit manufacturer&gt;\_&lt;12 digit tag id&gt;

#### 0x04 logging
- 0x01 Error Message
  - Indicates an abnormal condition likely to prevent normal operation of the device
- 0x02 Warning Message
  - Indicates an abnormal condition that may cause unexpected behaviour, but is not necessarily an Error
- 0x03 Informational Message
  - Normal messages that are part of the regular operation of the device
- 0x04 Debug Message
  - Detailed troubleshooting messages, normally used during development only.
