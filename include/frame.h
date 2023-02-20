#ifndef SAM_FRAME_H
#define SAM_FRAME_H

#define ZERO_LIMIT 4
#define ONE_LIMIT 7

enum MessageType {
    TXRQ = 0x01,
    ASCII = 0x2,
};

enum HeaderType { TO, FROM, TYPE, SIZE, PAYLOAD, CRC };

struct AddressHeader {
    uint8_t to   : 4;
    uint8_t from : 4;
};

struct DInfoHeader {
    enum MessageType type : 4;
    uint16_t size         : 12;
};

struct Frame {
    struct AddressHeader address;
    struct DInfoHeader dinfo;
    uint8_t *payload;
    uint16_t crc;
};

#endif
