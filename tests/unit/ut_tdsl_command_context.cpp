/**
 * _________________________________________________
 * Unit tests for tdslite command context
 *
 * @file   ut_tdsl_command_context.cpp
 * @author Mustafa K. GILOR <mustafagilor@gmail.com>
 * @date   05.10.2022
 *
 * SPDX-License-Identifier:    MIT
 * _________________________________________________
 */

#include <tdslite/detail/tdsl_command_context.hpp>
#include <tdslite/util/tdsl_hex_dump.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstring>
#include <array>

namespace {

    struct mock_network_impl {

        template <typename T>
        inline void do_write(tdsl::span<T> data) noexcept {
            send_buffer.insert(send_buffer.end(), data.begin(), data.end());
        }

        template <typename T>
        inline void do_write(tdsl::uint32_t offset, tdsl::span<T> data) noexcept {

            auto beg = std::next(send_buffer.begin(), offset);
            auto end = std::next(beg, data.size_bytes());
            if (beg >= send_buffer.end() || end > send_buffer.end()) {
                return;
            }

            std::copy(data.begin(), data.end(), beg);
        }

        inline void do_send(void) noexcept {}

        inline void do_receive_tds_pdu() {}

        void register_packet_data_callback(void *, tdsl::uint32_t (*)(void *, tdsl::detail::e_tds_message_type,
                                                                      tdsl::binary_reader<tdsl::endian::little> &)) {}

        std::vector<uint8_t> send_buffer;
    };
} // namespace

// --------------------------------------------------------------------------------

/**
 * The type of the unit-under-test
 */
using uut_t     = tdsl::detail::command_context<mock_network_impl>;

using tds_ctx_t = uut_t::tds_context_type;

struct tdsl_command_ctx_ut_fixture : public ::testing::Test {

    virtual void TearDown() override {
        tdsl::util::hexdump(tds_ctx.send_buffer.data(), tds_ctx.send_buffer.size());
    }

    tds_ctx_t tds_ctx;

    uut_t command{tds_ctx};
};

// --------------------------------------------------------------------------------

TEST_F(tdsl_command_ctx_ut_fixture, test_01) {
    command.execute_query(tdsl::string_view{"SELECT * FROM FOO;"});

    constexpr std::array<tdsl::uint8_t, 44> expected_packet_bytes{
        0x01, 0x01, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x53, 0x00, 0x45, 0x00, 0x4c, 0x00, 0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x20, 0x00,
        0x2a, 0x00, 0x20, 0x00, 0x46, 0x00, 0x52, 0x00, 0x4f, 0x00, 0x4d, 0x00, 0x20, 0x00, 0x46, 0x00, 0x4f, 0x00, 0x4f, 0x00, 0x3b, 0x00};

    ASSERT_EQ(sizeof(expected_packet_bytes), tds_ctx.send_buffer.size());
    ASSERT_EQ(sizeof(expected_packet_bytes), tds_ctx.send_buffer.size());
    ASSERT_THAT(tds_ctx.send_buffer, testing::ElementsAreArray(expected_packet_bytes));
}