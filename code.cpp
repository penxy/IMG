#include <iostream>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>

void encrypt(const std::string& plaintext, const std::string& key, std::string& ciphertext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "EVP_CIPHER_CTX_new failed" << std::endl;
        return;
    }

    // 初始化IV
    unsigned char iv[EVP_CIPHER_iv_length(EVP_aes_128_cbc())];
    RAND_bytes(iv, sizeof(iv)); // 随机生成IV

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), iv)) {
        std::cerr << "EVP_EncryptInit_ex failed" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    int block_size = EVP_CIPHER_block_size(EVP_aes_128_cbc());
    std::string encrypted;
    encrypted.resize(plaintext.size() + block_size);

    int len = 0;
    int plaintext_len = 0;
    if (1 != EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&encrypted[0]), &len, reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size())) {
        std::cerr << "EVP_EncryptUpdate failed" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    plaintext_len += len;

    if (1 != EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&encrypted[0]) + len, &len)) {
        std::cerr << "EVP_EncryptFinal_ex failed" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    plaintext_len += len;

    encrypted.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);

    // 将IV附加到密文前面
    ciphertext = std::string(reinterpret_cast<char*>(iv), sizeof(iv)) + encrypted;
}

void decrypt(const std::string& ciphertext, const std::string& key, std::string& plaintext) {
    int iv_length = EVP_CIPHER_iv_length(EVP_aes_128_cbc());
    if (ciphertext.size() < iv_length) {
        std::cerr << "Ciphertext is too short to contain IV" << std::endl;
        return;
    }

    unsigned char iv[iv_length];
    std::copy(ciphertext.begin(), ciphertext.begin() + iv_length, iv);

    std::string encrypted_data = ciphertext.substr(iv_length);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "EVP_CIPHER_CTX_new failed" << std::endl;
        return;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key.data()), iv)) {
        std::cerr << "EVP_DecryptInit_ex failed" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    std::string decrypted;
    decrypted.resize(encrypted_data.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));

    int len = 0;
    int plaintext_len = 0;
    if (1 != EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(&decrypted[0]), &len, reinterpret_cast<const unsigned char*>(encrypted_data.data()), encrypted_data.size())) {
        std::cerr << "EVP_DecryptUpdate failed" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    plaintext_len += len;

    if (1 != EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&decrypted[0]) + len, &len)) {
        std::cerr << "EVP_DecryptFinal_ex failed" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    plaintext_len += len;

    decrypted.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);

    plaintext = decrypted;
}

int main() {
    std::string plaintext = "Hello, World!";
    std::string key = "0123456789abcdef"; // AES-128 key，必须是16字节
    std::string ciphertext;

    // 加密
    encrypt(plaintext, key, ciphertext);
    std::cout << "Original: " << plaintext << std::endl;
    std::cout << "Encrypted: ";
    for (char c : ciphertext) {
        printf("%02x", static_cast<unsigned char>(c));
    }
    std::cout << std::endl;

    // 解密
    std::string decrypted_text;
    decrypt(ciphertext, key, decrypted_text);
    std::cout << "Decrypted: " << decrypted_text << std::endl;

    return 0;
}