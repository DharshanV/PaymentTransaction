#include <Logger.h>
#include <catch2/catch_all.hpp>
#include <endian.h>
#include <iostream>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/fmt/ranges.h>

// https://increase.com/articles/iso-8583-the-language-of-credit-cards
// https://en.wikipedia.org/wiki/ISO_8583#Ver_2003

template <size_t N> using Bytes = std::array<uint8_t, N>;
template <size_t N> constexpr Bytes<N - 1> makeBytes(const char (&str)[N])
{
    Bytes<N - 1> result = {};
    for (size_t i = 0; i < N - 1; ++i) {
        result[i] = static_cast<uint8_t>(str[i]);
    }
    return result;
}

struct ISO8583 {
    static ISO8583 parse(const std::vector<unsigned char>& buffer);

    Bytes<4> mti = { 0 };               // 0:  Message Type Indicator
    uint64_t bitmap = 0;                //     Data Field Bitmap
    Bytes<16> pan = { 0 };              // 2:  Primary Account Number
    Bytes<6> processingCode = { 0 };    // 3:  Processing Code
    uint64_t amount = 0;                // 4:  Transaction Amount
    Bytes<10> transmissionDT = { 0 };   // 7:  Transmission Date/Time (MMDDhhmmss)
    Bytes<6> stan = { 0 };              // 11: System Trace Audit Number
    Bytes<6> localTime = { 0 };         // 12: Local Transaction Time (hhmmss)
    Bytes<4> localDate = { 0 };         // 13: Local Transaction Date (MMDD)
    Bytes<4> expirationDate = { 0 };    // 14: Expiration Date (YYMM)
    Bytes<3> posEntryMode = { 0 };      // 22: Point of Service Entry Mode
    Bytes<2> posCondCode = { 0 };       // 25: Point of Service Condition Code
    Bytes<12> rrn = { 0 };              // 37: Retrieval Reference Number
    Bytes<6> authId = { 0 };            // 38: Authorization ID Response
    Bytes<2> responseCode = { 0 };      // 39: Response Code
    Bytes<8> terminalId = { 0 };        // 41: Card Acceptor Terminal ID
    Bytes<15> merchantId = { 0 };       // 42: Card Acceptor ID Code
    Bytes<3> currencyCode = { 0 };      // 49: Currency Code (ISO 4217)
};

ISO8583 ISO8583::parse(const std::vector<unsigned char>& buffer) { return {}; }

template <> struct fmt::formatter<ISO8583> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext> auto format(const ISO8583& iso, FormatContext& ctx) const
    {
        std::stringstream ss;
        auto writeLine = [&ss](const std::string& str) { ss << '\t' << str << '\n'; };

        ss << "\nISO8583 Data:\n";
        writeLine(fmt::format("MTI:                    {}", iso.mti));
        writeLine(fmt::format("2  PAN:                 {}", iso.pan));
        writeLine(fmt::format("3  Processing Code:     {}", iso.processingCode));
        writeLine(fmt::format("4  Amount:              {}", iso.amount));
        writeLine(fmt::format("7  Transmission DT:     {}", iso.transmissionDT));
        writeLine(fmt::format("11 STAN:                {}", iso.stan));
        writeLine(fmt::format("12 Local Time:          {}", iso.localTime));
        writeLine(fmt::format("13 Local Date:          {}", iso.localDate));
        writeLine(fmt::format("14 Expiration Date:     {}", iso.expirationDate));
        writeLine(fmt::format("22 POS Entry Mode:      {}", iso.posEntryMode));
        writeLine(fmt::format("25 POS Condition Code:  {}", iso.posCondCode));
        writeLine(fmt::format("37 RRN:                 {}", iso.rrn));
        writeLine(fmt::format("38 Auth ID:             {}", iso.authId));
        writeLine(fmt::format("39 Response Code:       {}", iso.responseCode));
        writeLine(fmt::format("41 Terminal ID:         {}", iso.terminalId));
        writeLine(fmt::format("42 Merchant ID:         {}", iso.merchantId));
        writeLine(fmt::format("49 Currency Code:       {}", iso.currencyCode));
        return fmt::format_to(ctx.out(), ss.str());
    }
};

// get/set is 1-indexed and Big-Endian (Network byte order)
void setBit(uint64_t& bitmap, int bit)
{
    assert(bit >= 1 && bit <= 64);
    bitmap |= ((uint64_t)1 << (64 - bit));
}

bool getBit(const uint64_t bitmap, int bit)
{
    assert(bit >= 1 && bit <= 64);
    return (bitmap >> (64 - bit)) & 1;
}

void printBits(const uint64_t bitmap)
{
    for (int i = 1; i <= 64; ++i) {
        std::cout << getBit(bitmap, i);
        if (i % 8 == 0) {
            std::cout << '\n';
        }
    }
    std::cout << '\n';
}

std::vector<unsigned char> buildISOBuffer()
{
    std::vector<unsigned char> isoBuffer;
    isoBuffer.reserve(512);

    auto append = [&isoBuffer](const std::string& asciiStr) {
        isoBuffer.insert(isoBuffer.end(), asciiStr.begin(), asciiStr.end());
    };

    auto appendBitmap = [&isoBuffer](const std::vector<int>& bits) {
        uint64_t bitmap = 0;
        for (int bit : bits) {
            setBit(bitmap, bit);
        }
        bitmap = htobe64(bitmap);

        std::array<unsigned char, 8> bitmapBytes = { 0 };
        std::memcpy(bitmapBytes.data(), &bitmap, 8);
        isoBuffer.insert(isoBuffer.end(), bitmapBytes.begin(), bitmapBytes.end());
    };

    append("0100");                             // MTI: Authorization Request
    appendBitmap({ 2, 3, 4, 7, 11, 12, 13, 14, 22, 25, 37, 38, 39, 41, 42, 49 });
    append("164200123456789012");               // 2:  PAN (length=16, 16-digit card)
    append("000000");                           // 3:  Processing Code (purchase)
    append("000000002500");                     // 4:  Amount ($25.00)
    append("0425143022");                       // 7:  Transmission Date/Time (MMDDhhmmss)
    append("000001");                           // 11: STAN
    append("143022");                           // 12: Local Time (hhmmss)
    append("0425");                             // 13: Local Date (MMDD)
    append("2612");                             // 14: Expiration Date (YYMM)
    append("051");                              // 22: POS Entry Mode (chip)
    append("00");                               // 25: POS Condition Code (normal)
    append("000000000001");                     // 37: Retrieval Reference Number
    append("ABC123");                           // 38: Authorization ID
    append("00");                               // 39: Response Code (approved)
    append("TERM0001");                         // 41: Terminal ID
    append("MERCHANT000001\0");                 // 42: Merchant ID (15 chars)
    append("840");                              // 49: Currency Code (USD)

    return isoBuffer;
}

TEST_CASE("ISO-8583 Parser", "[iso-8583]")
{
    const std::vector<unsigned char> isoBuffer = buildISOBuffer();
    pay::Logger::LOG()->debug("ISO Buffer:{:aX}", spdlog::to_hex(isoBuffer));

    const ISO8583 iso8583 = ISO8583::parse(isoBuffer);
    pay::Logger::LOG()->debug("{}", iso8583);

    // MTI
    REQUIRE(iso8583.mti == makeBytes("0100"));

    // Bitmap: expected bits set
    REQUIRE_FALSE(getBit(iso8583.bitmap, 1));  // no secondary bitmap
    REQUIRE(getBit(iso8583.bitmap, 2));        // PAN
    REQUIRE(getBit(iso8583.bitmap, 3));        // Processing Code
    REQUIRE(getBit(iso8583.bitmap, 4));        // Amount
    REQUIRE(getBit(iso8583.bitmap, 7));        // Transmission DT
    REQUIRE(getBit(iso8583.bitmap, 11));       // STAN
    REQUIRE(getBit(iso8583.bitmap, 12));       // Local Time
    REQUIRE(getBit(iso8583.bitmap, 13));       // Local Date
    REQUIRE(getBit(iso8583.bitmap, 14));       // Expiration Date
    REQUIRE(getBit(iso8583.bitmap, 22));       // POS Entry Mode
    REQUIRE(getBit(iso8583.bitmap, 25));       // POS Condition Code
    REQUIRE(getBit(iso8583.bitmap, 37));       // RRN
    REQUIRE(getBit(iso8583.bitmap, 38));       // Auth ID
    REQUIRE(getBit(iso8583.bitmap, 39));       // Response Code
    REQUIRE(getBit(iso8583.bitmap, 41));       // Terminal ID
    REQUIRE(getBit(iso8583.bitmap, 42));       // Merchant ID
    REQUIRE(getBit(iso8583.bitmap, 49));       // Currency Code

    // Field 2: PAN
    REQUIRE(iso8583.pan == makeBytes("4200123456789012"));

    // Field 3: Processing Code
    REQUIRE(iso8583.processingCode == makeBytes("000000"));

    // Field 4: Amount ($25.00 = 2500 cents)
    REQUIRE(iso8583.amount == 2500);

    // Field 7: Transmission Date/Time
    REQUIRE(iso8583.transmissionDT == makeBytes("0425143022"));

    // Field 11: STAN
    REQUIRE(iso8583.stan == makeBytes("000001"));

    // Field 12: Local Time
    REQUIRE(iso8583.localTime == makeBytes("143022"));

    // Field 13: Local Date
    REQUIRE(iso8583.localDate == makeBytes("0425"));

    // Field 14: Expiration Date (YYMM)
    REQUIRE(iso8583.expirationDate == makeBytes("2612"));

    // Field 22: POS Entry Mode
    REQUIRE(iso8583.posEntryMode == makeBytes("051"));

    // Field 25: POS Condition Code
    REQUIRE(iso8583.posCondCode == makeBytes("00"));

    // Field 37: Retrieval Reference Number
    REQUIRE(iso8583.rrn == makeBytes("000000000001"));

    // Field 38: Authorization ID
    REQUIRE(iso8583.authId == makeBytes("ABC123"));

    // Field 39: Response Code (approved)
    REQUIRE(iso8583.responseCode == makeBytes("00"));

    // Field 41: Terminal ID
    REQUIRE(iso8583.terminalId == makeBytes("TERM0001"));

    // Field 42: Merchant ID
    REQUIRE(iso8583.merchantId == makeBytes("MERCHANT000001\0"));

    // Field 49: Currency Code (USD)
    REQUIRE(iso8583.currencyCode == makeBytes("840"));
}