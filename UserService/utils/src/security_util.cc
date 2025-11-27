// Copyright (c) 2025 seaStarLxy.
// Licensed under the MIT License.

#include "utils/include/security_util.h"
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>

using namespace user_service::util;

std::string SecurityUtil::GenerateSalt() {
    static thread_local CryptoPP::AutoSeededRandomPool prng;

    unsigned char salt[16];
    prng.GenerateBlock(salt, sizeof(salt));

    std::string salt_hex;
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(salt_hex));
    encoder.Put(salt, sizeof(salt));
    encoder.MessageEnd();

    return salt_hex;
}

std::string SecurityUtil::HashPassword(const std::string& raw_password, const std::string& salt) {
    CryptoPP::SHA256 hash;
    std::string digest;
    const std::string input = raw_password + salt;

    CryptoPP::StringSource s(input, true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(digest)
            )
        )
    );
    return digest;
}

bool SecurityUtil::VerifyPassword(const std::string& raw_password, const std::string& salt, const std::string& stored_hash) {
    const std::string calculated_hash = HashPassword(raw_password, salt);

    // 长度不一致直接失败（VerifyBufsEqual 要求长度一致）
    if (calculated_hash.size() != stored_hash.size()) {
        return false;
    }

    // 使用常数时间比较，防止计时攻击
    return CryptoPP::VerifyBufsEqual(
        reinterpret_cast<const unsigned char*>(calculated_hash.data()),
        reinterpret_cast<const unsigned char*>(stored_hash.data()),
        stored_hash.size()
    );
}