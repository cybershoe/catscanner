# Genericor Generic Message Format

## Summary
GGMF is a simple message format for exchanging short, arbitrary
commands and responses between sensors and a data collector over an XBee radio link. The maximum message size is 64 bytes (to fit inside an unfragmented XBee S2C frame)

## Message Format

### Message Header

- 2 bytes: GGNF Identifier
  - Always 0x7A69
  - Messages without this header are discarded
- 1 byte: GGNF version
  - Currently 0x01 (version 1)
- 8 bytes - Station ID
  - This is the low 8 bytes of the XBee MAC
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
- 0-64 bytes - Message data
- 1 byte: Terminator
  - 0x00
