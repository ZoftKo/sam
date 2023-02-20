#include "gtest/gtest.h"

extern "C" {
#include "tx.h"
}

class TestBasebandTx : public testing::Test {
   protected:
    uint8_t port = 0x00;
    const uint8_t pin = 0;

    void SetUp() override { tx_setup(&port, pin, nullptr); }

    /**
     * Transmit the specified data and store the transmitted bytes. This means transmitted bytes
     * may contain extra bits as the result of bit stuffing.
     * @param buffer Transmitted bytes will be stored in this buffer. Bytes are grouped from left to
     * right (MSB to LSB), and bits shifted from right to left as they are received.
     * @param data Bytes to be transmitted
     * @param data_size Amount of bytes to be transmitted
     * @param tx_func Function that will be used to transmit data
     */
    void runTxAndStore(
        uint8_t *buffer, uint8_t *data, uint8_t data_size, uint8_t (*tx_func)(uint8_t)
    ) {
        uint8_t tx_state, tx_end, counter, curr_block, curr_buffer, tx_global_end;
        tx_end = counter = curr_block = curr_buffer = 0;

        tx_state = 1;
        while (tx_state != 0) {
            for (counter = 0; counter < 8; counter++) {
                tx_end = tx_func(data[curr_block]);
                tx_global_end = (tx_end == 0) && (curr_block == (data_size - 1));
                if ((counter < 7) && !tx_global_end) {
                    port = port << 1;
                }
                if (tx_end == 0) {
                    curr_block++;
                    if (curr_block == data_size) {
                        tx_state = 0;
                        break;
                    }
                }
            }
            buffer[curr_buffer] = port;
            curr_buffer++;
            port = 0;
        }
    }
};

TEST_F(TestBasebandTx, RegisterTxFrame) {
    Frame frame{};
    ASSERT_EQ(tx_frame(&frame), 0);
}

TEST_F(TestBasebandTx, RegisterTxFrameBusy) {
    Frame frame{};
    ASSERT_EQ(tx_frame(&frame), 0);
    ASSERT_EQ(tx_frame(&frame), 1);
}

TEST_F(TestBasebandTx, TxBitZeroRun) {
    uint8_t retval;
    for (uint8_t counter = 0; counter < ZERO_LIMIT; counter++) {
        retval = tx_bit(0);
        port = port << 1;
    }
    ASSERT_EQ(0, retval);
    ASSERT_EQ(1, tx_bit(0));
    ASSERT_EQ(0x01, port);

    port = port << 1;
    ASSERT_EQ(0, tx_bit(0));
    ASSERT_EQ(0x02, port);
}

TEST_F(TestBasebandTx, TxBitOneRun) {
    uint8_t retval;
    for (uint8_t counter = 0; counter < ONE_LIMIT; counter++) {
        retval = tx_bit(1);
        port = port << 1;
    }
    ASSERT_EQ(0, retval);
    ASSERT_EQ(0xFE, port);
    ASSERT_EQ(1, tx_bit(1));
    ASSERT_EQ(0xFE, port);

    ASSERT_EQ(0, tx_bit(1));
    ASSERT_EQ(0xFF, port);
}

TEST_F(TestBasebandTx, TxNibble) {
    while (tx_nibble(0x03) != 0) {
        port = port << 1;
    }
    ASSERT_EQ(port, 0x03);
}

TEST_F(TestBasebandTx, TxNibblesWithZeroRun) {
    uint8_t buffer[2] = {0};
    uint8_t nibbles[2] = {0x06, 0x01};
    uint8_t expected_buffer[2] = {0x61, 0x01};

    this->runTxAndStore(buffer, nibbles, 2, &tx_nibble);
    for (uint8_t counter = 0; counter < 2; counter++) {
        ASSERT_EQ(expected_buffer[counter], buffer[counter]);
    }
}

TEST_F(TestBasebandTx, TxNibblesWithOneRun) {
    uint8_t buffer[2] = {0};
    uint8_t nibbles[3] = {0x05, 0x0F, 0x0F};
    uint8_t expected_buffer[2] = {0x5F, 0x1B};

    this->runTxAndStore(buffer, nibbles, 3, &tx_nibble);
    for (uint8_t counter = 0; counter < 2; counter++) {
        ASSERT_EQ(expected_buffer[counter], buffer[counter]);
    }
}

TEST_F(TestBasebandTx, TxBytesWithOneRun) {
    uint8_t buffer[3] = {0};
    uint8_t bytes[2] = {0xF5, 0xFD};
    uint8_t expected_buffer[3] = {0xF5, 0xFC, 0x01};

    this->runTxAndStore(buffer, bytes, 2, &tx_byte);
    for (uint8_t counter = 0; counter < 3; counter++) {
        ASSERT_EQ(expected_buffer[counter], buffer[counter]);
    }
}

TEST_F(TestBasebandTx, TxBytesWithZeroRun) {
    uint8_t buffer[3] = {0};
    uint8_t bytes[2] = {0x8C, 0x1A};
    uint8_t expected_buffer[3] = {0x8C, 0x2D, 0x00};

    this->runTxAndStore(buffer, bytes, 2, &tx_byte);
    for (uint8_t counter = 0; counter < 3; counter++) {
        ASSERT_EQ(expected_buffer[counter], buffer[counter]);
    }
}

TEST_F(TestBasebandTx, TxFrameTXRQ) {
    uint8_t payload[1] = {0xAA};
    Frame frame = {
        .address = {.to = 0x07, .from = 0x01},
        .dinfo = {.type = TXRQ, .size = 0x01},
        .payload = payload,
        .crc = 0x5555};
    ASSERT_EQ(0, tx_frame(&frame));

    uint8_t tx_state, end, buffer_idx;
    uint8_t buffer[8] = {0};
    uint8_t expected_bytes[] = {0xFF, 0x71, 0x10, 0x84, 0x6A, 0x95, 0x55, 0x01};

    buffer_idx = 0;
    tx_state = 1;

    while (tx_state != 0) {
        for (uint8_t counter = 0; counter < 8; counter++) {
            end = tx_next();
            if ((counter < 7) && (end != 0)) {
                port = port << 1;
            }
            if (end == 0) {
                tx_state = 0;
                break;
            }
        }
        buffer[buffer_idx] = port;
        buffer_idx++;
        port = 0;
    }

    for (uint8_t counter = 0; counter < 8; counter++) {
        ASSERT_EQ(expected_bytes[counter], buffer[counter]);
    }
}
