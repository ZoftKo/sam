#ifndef BBTX_TX_GUARD
#define BBTX_TX_GUARD

#include <stdint.h>

extern const uint8_t ONE_LIMIT;
extern const uint8_t ZERO_LIMIT;

enum MessageType {
    TXRQ = 0x01,
    ASCII = 0x2,
};

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

/**
 * Sets transmitter parameters and initializes it to a clear IDLE state. This means
 * it will be ready to admit frames for transmission after this call, and any previous
 * state and transmission data will be lost. Any ongoing transmission will stop after
 * this method is called.
 * */
void tx_setup(volatile uint8_t *port, uint8_t bit);

/**
 * Writes a single bit to the transmission channel while taking
 * into account line coding. This means that the payload bit
 * may not be transmitted if something like bit stuffing is
 * required.
 *
 * @param bit Bit to be transmitted
 * @return 0 if the desired bit was written, non-zero otherwise.
 * */
uint8_t tx_bit(uint8_t bit);

/**
 * Transmit a nibble (4 bits).
 * @param nibble The lower 4 bits will be transmitted.
 * @return 0 once the 4 bits have been transmitted. Non-zero otherwise.
 */
uint8_t tx_nibble(uint8_t nibble);

/**
 * Transmit a byte.
 * @param byte The byte to be transmitted
 * @return 0 once the byte has been transmitted. Non-zero otherwise.
 */
uint8_t tx_byte(uint8_t byte);

/**
 * Execute the transmitter's state machine. This is generally called
 * periodically after setting the frame to be transmitted with tx_frame.
 *
 * @return 0 if the machine has reached IDLE state after execution.
 * Non-zero otherwise.
 * */
uint8_t tx_next();

/**
 * Prepare the transmitter's state machine to transmit the given frame.
 * This will only take effect when the transmitter is in IDLE state.
 *
 * @param to 4 bit destination address
 * @param type First 4 bits of DINFO. States the type of payload.
 * @param payload Data to be transmitted, the frame's payload.
 * @param size Size of data to be transmitted. Last 12 bits of DINFO.
 * @return 0 if the frame has been accepted and is ready for transmission.
 * Non-zero otherwise.
 * */
uint8_t tx_frame(struct Frame frame);

#endif
