/**
 * ____________________________________________________
 * Unit tests for tds login helper
 *
 * @file   ut_tds_login.cpp
 * @author Mustafa K. GILOR <mustafagilor@gmail.com>
 * @date   14.04.2022
 *
 * SPDX-License-Identifier:    MIT
 * ____________________________________________________
 */

#include <tdslite/detail/tdsl_login_context.hpp>
#include <tdslite/detail/tdsl_lang_code_id.hpp>
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
            buffer.insert(buffer.end(), data.begin(), data.end());
        }

        template <typename T>
        inline void do_write(tdsl::size_t offset, tdsl::span<T> data) noexcept {

            auto beg = std::next(buffer.begin(), offset);
            auto end = std::next(beg, data.size_bytes());
            if (beg >= buffer.end() || end > buffer.end()) {
                return;
            }

            std::copy(data.begin(), data.end(), beg);
        }

        /**
         * Get current write offset
         */
        inline tdsl::size_t do_get_write_offset() noexcept {
            return buffer.size();
        }

        inline void do_send(void) noexcept {}

        inline void do_receive_tds_pdu() {}

        inline void do_send_tds_pdu(tdsl::detail::e_tds_message_type) {}

        inline void set_tds_packet_size(tdsl::uint16_t) {}

        void register_packet_data_callback(
            tdsl::uint32_t (*)(void *, tdsl::detail::e_tds_message_type,
                               tdsl::binary_reader<tdsl::endian::little> &),
            void *) {}

        std::vector<uint8_t> buffer;
    };
} // namespace

// --------------------------------------------------------------------------------

/**
 * The type of the unit-under-test
 */
using uut_t     = tdsl::detail::login_context<mock_network_impl>;

using tds_ctx_t = uut_t::tds_context_type;

struct tdsl_login_ctx_ut_fixture : public ::testing::Test {

    virtual void TearDown() override {
        tdsl::util::hexdump(tds_ctx.buffer.data(), tds_ctx.buffer.size());
    }

    tds_ctx_t tds_ctx;

    uut_t login{tds_ctx};
};

// --------------------------------------------------------------------------------

TEST_F(tdsl_login_ctx_ut_fixture, encode_password) {

    char16_t buf [] = u"JaxView";
    EXPECT_NO_THROW(
        uut_t::encode_password(reinterpret_cast<tdsl::uint8_t *>(buf), sizeof(buf) - 2));

    constexpr tdsl::uint8_t expected_buf [] = {0x01, 0xa5, 0xb3, 0xa5, 0x22, 0xa5, 0xc0, 0xa5,
                                               0x33, 0xa5, 0xf3, 0xa5, 0xd2, 0xa5

    };

    EXPECT_EQ(std::memcmp(buf, expected_buf, sizeof(expected_buf)), 0);
}

// --------------------------------------------------------------------------------

TEST_F(tdsl_login_ctx_ut_fixture, test_01) {

    uut_t::login_parameters params;
    params.server_name = "localhost";
    params.db_name     = "test";
    params.user_name   = "sa";
    params.password    = "test";
    params.client_name = "unit test";
    login.do_login(params);
    // FIXME: No expected conditions?
}

// --------------------------------------------------------------------------------

TEST_F(tdsl_login_ctx_ut_fixture, test_jaxview) {
    uut_t::login_parameters params;
    params.server_name            = "192.168.2.38";
    params.db_name                = "JaxView";
    params.user_name              = "JaxView";
    params.password               = "JaxView";
    params.client_name            = "AL-DELL-02";
    params.app_name               = "jTDS";
    params.library_name           = "jTDS";
    params.client_pid             = 123;
    params.client_program_version = 7;
    params.packet_size            = 0;

    constexpr std::array<tdsl::uint8_t, 188> expected_packet_bytes{
        0xbc, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00,
        0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x00, 0x0a, 0x00, 0x6a, 0x00, 0x07, 0x00, 0x78,
        0x00, 0x07, 0x00, 0x86, 0x00, 0x04, 0x00, 0x8e, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xa6, 0x00, 0x04, 0x00, 0xae, 0x00, 0x00, 0x00, 0xae, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x41, 0x00, 0x4c, 0x00,
        0x2d, 0x00, 0x44, 0x00, 0x45, 0x00, 0x4c, 0x00, 0x4c, 0x00, 0x2d, 0x00, 0x30, 0x00, 0x32,
        0x00, 0x4a, 0x00, 0x61, 0x00, 0x78, 0x00, 0x56, 0x00, 0x69, 0x00, 0x65, 0x00, 0x77, 0x00,
        0x01, 0xa5, 0xb3, 0xa5, 0x22, 0xa5, 0xc0, 0xa5, 0x33, 0xa5, 0xf3, 0xa5, 0xd2, 0xa5, 0x6a,
        0x00, 0x54, 0x00, 0x44, 0x00, 0x53, 0x00, 0x31, 0x00, 0x39, 0x00, 0x32, 0x00, 0x2e, 0x00,
        0x31, 0x00, 0x36, 0x00, 0x38, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x33, 0x00, 0x38,
        0x00, 0x6a, 0x00, 0x54, 0x00, 0x44, 0x00, 0x53, 0x00, 0x4a, 0x00, 0x61, 0x00, 0x78, 0x00,
        0x56, 0x00, 0x69, 0x00, 0x65, 0x00, 0x77, 0x00};

    login.do_login(params);
    printf("sizeof char16_t %ld\n”", sizeof(char16_t));
    tdsl::util::hexdump(&expected_packet_bytes [0], sizeof(expected_packet_bytes));
    printf("\n %ld vs. %ld \n", sizeof(expected_packet_bytes), tds_ctx.buffer.size());

    ASSERT_EQ(sizeof(expected_packet_bytes), tds_ctx.buffer.size());
    ASSERT_EQ(sizeof(expected_packet_bytes), tds_ctx.buffer.size());
    ASSERT_THAT(tds_ctx.buffer, testing::ElementsAreArray(expected_packet_bytes));
}

// --------------------------------------------------------------------------------

TEST_F(tdsl_login_ctx_ut_fixture, test_mdac) {

    uut_t::login_parameters params;
    params.server_name            = "217.77.3.25";
    params.user_name              = "testuser";
    params.password               = "host";
    params.client_name            = "LIV-VXP-007";
    params.app_name               = "Microsoft Data Access Components";
    params.library_name           = "ODBC";
    params.client_pid             = 1712;
    params.client_program_version = 0x07000000;
    params.packet_size            = 0;
    params.collation = static_cast<tdsl::uint32_t>(tdsl::detail::e_ms_lang_code_id::en_gb_english);
    params.client_id [0] = 0x00;
    params.client_id [1] = 0x0c;
    params.client_id [2] = 0x29;
    params.client_id [3] = 0x4c;
    params.client_id [4] = 0x84;
    params.client_id [5] = 0x92;

    constexpr std::array<tdsl::uint8_t, 226> expected_packet_bytes{
        0xe2, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x07, 0xb0, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x09, 0x08, 0x00, 0x00, 0x56, 0x00, 0x0b, 0x00, 0x6c, 0x00, 0x08, 0x00, 0x7c,
        0x00, 0x04, 0x00, 0x84, 0x00, 0x20, 0x00, 0xc4, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xda, 0x00, 0x04, 0x00, 0xe2, 0x00, 0x00, 0x00, 0xe2, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x29,
        0x4c, 0x84, 0x92, 0x00, 0x00, 0x00, 0x00, 0xe2, 0x00, 0x00, 0x00, 0x4c, 0x00, 0x49, 0x00,
        0x56, 0x00, 0x2d, 0x00, 0x56, 0x00, 0x58, 0x00, 0x50, 0x00, 0x2d, 0x00, 0x30, 0x00, 0x30,
        0x00, 0x37, 0x00, 0x74, 0x00, 0x65, 0x00, 0x73, 0x00, 0x74, 0x00, 0x75, 0x00, 0x73, 0x00,
        0x65, 0x00, 0x72, 0x00, 0x23, 0xa5, 0x53, 0xa5, 0x92, 0xa5, 0xe2, 0xa5, 0x4d, 0x00, 0x69,
        0x00, 0x63, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00, 0x74, 0x00,
        0x20, 0x00, 0x44, 0x00, 0x61, 0x00, 0x74, 0x00, 0x61, 0x00, 0x20, 0x00, 0x41, 0x00, 0x63,
        0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 0x73, 0x00, 0x20, 0x00, 0x43, 0x00, 0x6f, 0x00,
        0x6d, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x73,
        0x00, 0x32, 0x00, 0x31, 0x00, 0x37, 0x00, 0x2e, 0x00, 0x37, 0x00, 0x37, 0x00, 0x2e, 0x00,
        0x33, 0x00, 0x2e, 0x00, 0x32, 0x00, 0x35, 0x00, 0x4f, 0x00, 0x44, 0x00, 0x42, 0x00, 0x43,
        0x00};

    login.do_login(params);
    printf("sizeof char16_t %ld\n”", sizeof(char16_t));
    tdsl::util::hexdump(&expected_packet_bytes [0], sizeof(expected_packet_bytes));
    printf("\n %ld vs. %ld \n", sizeof(expected_packet_bytes), tds_ctx.buffer.size());

    ASSERT_EQ(sizeof(expected_packet_bytes), tds_ctx.buffer.size());
    ASSERT_EQ(sizeof(expected_packet_bytes), tds_ctx.buffer.size());
    ASSERT_THAT(tds_ctx.buffer, testing::ElementsAreArray(expected_packet_bytes));
}