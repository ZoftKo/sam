#include <cstdint>

#include "gtest/gtest.h"

extern "C" {
#include "bits.h"
}

class TestBits : public testing::Test {
   protected:
    uint8_t port = 0;

    void SetUp() override { port = 0; }
};

TEST_F(TestBits, TestReadBit) {
    port = 0x80;
    ASSERT_EQ(read_bit(&port, 7), 1);

    port = 0x40;
    ASSERT_EQ(read_bit(&port, 7), 0);

    port = 0x08;
    ASSERT_EQ(read_bit(&port, 3), 1);

    port = 0x01;
    ASSERT_EQ(read_bit(&port, 0), 1);

    port = 0x00;
    ASSERT_EQ(read_bit(&port, 0), 0);
}

TEST_F(TestBits, TestBitOn) {
    bit_on(&port, 0);
    ASSERT_EQ(0x01, port);

    bit_on(&port, 3);
    ASSERT_EQ(0x09, port);

    bit_on(&port, 7);
    ASSERT_EQ(0x89, port);
}

TEST_F(TestBits, TestBitOff) {
    port = 0xFF;

    bit_off(&port, 0);
    ASSERT_EQ(0xFE, port);

    bit_off(&port, 4);
    ASSERT_EQ(0xEE, port);

    bit_off(&port, 7);
    ASSERT_EQ(0x6E, port);
}

TEST_F(TestBits, TestSetHighNibble) {
    uint8_t value = 0xF5;

    set_high_nibble(&port, value, 0);
    ASSERT_EQ(0x0F, port);
    port = 0;

    set_high_nibble(&port, value, 2);
    ASSERT_EQ(0x3C, port);
    port = 0;

    set_high_nibble(&port, value, 4);
    ASSERT_EQ(0xF0, port);
}

TEST_F(TestBits, TestSetLowNibble) {
    uint8_t value = 0x73;

    set_low_nibble(&port, value, 0);
    ASSERT_EQ(0x03, port);
    port = 0;

    set_low_nibble(&port, value, 3);
    ASSERT_EQ(0x18, port);
    port = 0;

    set_low_nibble(&port, value, 4);
    ASSERT_EQ(0x30, port);
}
