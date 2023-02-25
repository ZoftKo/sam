# Communication Protocol
This document briefly outlines the communication protocol used to connect devices with the
central unit.

## Line Coding
RZ is used and bit stuffing is used for transparency. After any
sequence of four zeros (0000) a 1 must be inserted (00001).

Bit stuffing is also used to prevent a continuous run of 1s from
being interpreted as a frame start/end. A sequence of more than 7 continuous 1s needs to have a
zero added to it. For example, if you wanted to transmit 0xFF the encoded bits would be 0xFE1.

## Frames
A total of 6 bytes are used for housekeeping per frame. Each frame can contain (theoretically) up to 4096 bits of
payload (4k bytes). The format is the following:

| Start Flag    | To     | From   | Type   | Size        | Payload | CRC     |
|---------------|--------|--------|--------|-------------|---------|---------|
 | 1 byte (0xFF) | 4 bits | 4 bits | 4 bits | 12 bits (N) | N bytes | 2 bytes |

## Message types
The following message types are reserved and should be interpreted in the same way by all implementations. The
rest can be implementation dependent.

| Bits | Type  | Meaning                                                                                      |
|------|-------|----------------------------------------------------------------------------------------------|
| 0x00 | NULL  | Has no meaning and should not be used                                                        |
| 0x01 | TXRQ  | Request for a bus slave to transmit. Payload is implementation dependent and it may be empty |
| 0x02 | ASCII | Interpreted as ASCII text                                                                    |

## Communication Scheme
The network is a bus consisting of slaves and a controlling unit (CU). Communications can only be started by the CU,
and all slaves transmissions must be as a response to a CU TXRQ Frame.

### Addressing
The Controlling Unit always has the address 0x1. Address 0x0 is reserved and should not be used by any network node.
Slaves can have addresses 0x2 to 0xF. This means that in total, there may be up to 7 nodes in a network. One CU and
six slaves.
