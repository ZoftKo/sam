#include "gtest/gtest.h"

extern "C" {
#include "rx.h"
#include "tx.h"
}

/**
 * Test Suite to run all tests  for the RX library.
 * NOTE: Bits are transmitted from MSB to LSB. Since right shifting is used on tests and it starts
 * at 0, most TX data is flipped, meaning the MSB is at the right most position. For example:
 * 0xFE is actually 0x7F when you flip it.
 */
class TestBasebandRx : public testing::Test {
   protected:
    uint8_t port = 0;
    const uint8_t pin = 0;
    Frame frame = {};

    void SetUp() override { rx_setup(&port, pin, &frame, nullptr); }
};

TEST_F(TestBasebandRx, RxBitZeroRunCorrectStuffing) {
    uint8_t tx_data = 0xA1;
    uint8_t decoded_data = 0x41;

    for (int counter = 0; counter < 8; counter++) {
        port = tx_data >> counter;
        rx_bit();
    }

    ASSERT_EQ(decoded_data, rx_buffer());
}

TEST_F(TestBasebandRx, RxBitOneRunCorrectStuffing) {
    uint16_t tx_data = 0x17F;
    uint8_t decoded_data = 0xFF;

    for (int counter = 0; counter < 9; counter++) {
        port = tx_data >> counter;
        rx_bit();
    }

    ASSERT_EQ(decoded_data, rx_buffer());
}

TEST_F(TestBasebandRx, RxByteZeroRunCorrectStuffing) {
    uint16_t tx_data = 0x1E1;
    uint8_t decoded_data = 0x87;
    uint8_t counter = 0;

    do {
        port = tx_data >> counter;
        counter++;
    } while (rx_byte() != 0);

    ASSERT_EQ(decoded_data, rx_buffer());
}

TEST_F(TestBasebandRx, RxByteOneRunCorrectStuffing) {
    uint16_t tx_data = 0x7F;
    uint8_t decoded_data = 0xFE;
    uint8_t counter = 0;

    do {
        port = tx_data >> counter;
        counter++;
    } while (rx_byte() != 0);

    ASSERT_EQ(decoded_data, rx_buffer());
}

TEST_F(TestBasebandRx, TxAndRxFrame) {
    uint8_t payload[5] = {'H', 'e', 'l', 'l', 'o'};
    uint8_t rx_payload[5] = {0};
    frame.payload = rx_payload;

    Frame tx_data = {
        .address = {.to = 0x07, .from = 0x01},
        .dinfo = {.type = ASCII, .size = 0x02},
        .payload = payload,
        .crc = 0x5555};
    tx_setup(&port, 0, nullptr);
    tx_frame(&tx_data);

    uint8_t working;
    do {
        working = tx_next();
        rx_next();
    } while (working != 0);

    ASSERT_EQ(END, rx_state());
    ASSERT_EQ(tx_data.address.to, frame.address.to);
    ASSERT_EQ(tx_data.address.from, frame.address.from);
    ASSERT_EQ(tx_data.dinfo.type, frame.dinfo.type);
    ASSERT_EQ(tx_data.dinfo.size, frame.dinfo.size);

    for (int counter = 0; counter < 2; counter++) {
        ASSERT_EQ(payload[counter], rx_payload[counter]);
    }
    ASSERT_EQ(tx_data.crc, frame.crc);
}
