#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <random>

uint32_t get_client_capabilities() {
    uint32_t client_capabilities = 0;

    // Enable common flags based on client requirements
    client_capabilities |= 0x00000001;  // CLIENT_LONG_PASSWORD
    client_capabilities |= 0x00000008;  // CLIENT_CONNECT_WITH_DB
    client_capabilities |= 0x00000200;  // CLIENT_PROTOCOL_41 (supports protocol version 4.1+)
    client_capabilities |= 0x00000800;  // CLIENT_SSL (if you want to use SSL)
    client_capabilities |= 0x00002000;  // CLIENT_TRANSACTIONS (if you need transactions)

    return client_capabilities;
}

// Function to hash the password with MySQL's native password method (using SHA1)
std::string hash_mysql_native_password(const std::string& password, const std::vector<uint8_t>& salt) {
    // Step 1: Combine password with salt
    std::string salted_password = password + std::string(salt.begin(), salt.end());

    // Step 2: Hash the salted password using SHA1
    unsigned char hash1[SHA_DIGEST_LENGTH];  // SHA1 produces 20-byte hashes
    SHA1(reinterpret_cast<const unsigned char*>(salted_password.c_str()), salted_password.length(), hash1);

    // Step 3: Perform a second SHA1 hash on the result
    unsigned char final_hash[SHA_DIGEST_LENGTH];
    SHA1(hash1, SHA_DIGEST_LENGTH, final_hash);

    // Step 4: Return the hashed result as a hex string
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)final_hash[i];
    }
    return ss.str();
}

// Function to craft the login packet
std::vector<uint8_t> craft_login_request_packet(
    const std::string& username,
    const std::string& password,
    const std::vector<uint8_t>& salt,
    const std::string& database,
    uint32_t client_capabilities,
    uint8_t charset
) {

    std::vector<uint8_t> packet;

    // 1. Username (null-terminated string)
    packet.insert(packet.end(), username.begin(), username.end());
    packet.push_back(0x00); // Null terminator for username

    // 2. Auth Response (Hashed Password)
    std::string hashed_password = hash_mysql_native_password(password, salt);
    packet.insert(packet.end(), hashed_password.begin(), hashed_password.end());

    // 3. Desired Database (null-terminated string)
    packet.insert(packet.end(), database.begin(), database.end());
    packet.push_back(0x00); // Null terminator for database

    // 4. Client Capabilities (4 bytes, little-endian)
    uint32_t capabilities = htonl(client_capabilities);
    packet.push_back((capabilities >> 0) & 0xFF);
    packet.push_back((capabilities >> 8) & 0xFF);
    packet.push_back((capabilities >> 16) & 0xFF);
    packet.push_back((capabilities >> 24) & 0xFF);

    // 5. Charset (1 byte)
    packet.push_back(charset);

    // 6. Filler bytes (1 byte) for MySQL Protocol
    packet.push_back(0x00); // filler

    // The first 4 bytes will be the length of the packet (excluding these first 4 bytes)
    uint32_t packet_length = htonl(packet.size() - 4);
    packet.insert(packet.begin(), (packet_length >> 0) & 0xFF);
    packet.insert(packet.begin() + 1, (packet_length >> 8) & 0xFF);
    packet.insert(packet.begin() + 2, (packet_length >> 16) & 0xFF);
    packet.insert(packet.begin() + 3, (packet_length >> 24) & 0xFF);

    return packet;
}

struct MYSQLCON {
    uint8_t protocol_version;
    std::string server_version;
    uint32_t connection_id;
    std::vector<uint8_t> salt1;  // 8 bytes
    std::vector<uint8_t> salt2;  // variable length, typically 12
    uint16_t capability_flags_lower;
    uint16_t capability_flags_upper;
    uint32_t capability_flags;
    uint8_t charset;
    uint16_t status_flags;
    uint8_t auth_plugin_data_len;
    std::string auth_plugin_name;

    std::vector<uint8_t> get_salt() {
        std::vector<uint8_t> salt1(8); // 8 bytes for the first salt part
        std::vector<uint8_t> salt2(12); // 12 bytes for the second salt part
    
        // Use random number generation to populate salt1 and salt2
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 255);
    
        // Populate salt1 (8 bytes)
        for (int i = 0; i < 8; ++i) {
            salt1[i] = static_cast<uint8_t>(dist(gen));
        }
    
        // Populate salt2 (12 bytes, can be adjusted if needed)
        for (int i = 0; i < 12; ++i) {
            salt2[i] = static_cast<uint8_t>(dist(gen));
        }
    
        // Combine salt1 and salt2 into one vector
        std::vector<uint8_t> full_salt;
        full_salt.insert(full_salt.end(), salt1.begin(), salt1.end());
        full_salt.insert(full_salt.end(), salt2.begin(), salt2.end());
    
        return full_salt;  // Return combined salt (salt1 + salt2)
    }

    void show() {
        std::cout << "🔢 Protocol Version: " << static_cast<int>(protocol_version) << "\n";
        std::cout << "🧠 Server Version: " << server_version << "\n";
        std::cout << "🆔 Connection ID: " << connection_id << "\n";

        std::cout << "🧂 Salt Part 1: ";
        for (uint8_t b : salt1)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
        std::cout << std::dec << "\n";

        std::cout << "🧂 Salt Part 2: ";
        for (uint8_t b : salt2)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
        std::cout << std::dec << "\n";

        std::cout << "🔧 Capability Flags (Lower): 0x" << std::hex << capability_flags_lower << "\n";
        std::cout << "🔧 Capability Flags (Upper): 0x" << capability_flags_upper << "\n";
        std::cout << "🛡️ Capability Flags (Combined): 0x" << capability_flags << std::dec << "\n";

        std::cout << "🌐 Charset: " << static_cast<int>(charset) << "\n";
        std::cout << "📶 Status Flags: 0x" << std::hex << status_flags << std::dec << "\n";
        std::cout << "🔢 Auth Plugin Data Length: " << static_cast<int>(auth_plugin_data_len) << "\n";
        std::cout << "🔐 Auth Plugin Name: " << auth_plugin_name << "\n";
    };

    
};

MYSQLCON parse_mysql_handshake_packet(const char* buffer, ssize_t bytes_received) {
    MYSQLCON result{};
    if (bytes_received < 4) {
        throw std::runtime_error("Packet too small");
    }

    int i = 4; // skip 3-byte length + 1-byte sequence ID

    result.protocol_version = static_cast<uint8_t>(buffer[i++]);

    while (buffer[i] != '\0') {
        result.server_version += buffer[i++];
    }
    i++; // null terminator

    std::memcpy(&result.connection_id, &buffer[i], 4);
    i += 4;

    result.salt1.insert(result.salt1.end(), buffer + i, buffer + i + 8);
    i += 8;

    i++; // filler byte

    std::memcpy(&result.capability_flags_lower, &buffer[i], 2);
    i += 2;

    result.charset = static_cast<uint8_t>(buffer[i++]);

    std::memcpy(&result.status_flags, &buffer[i], 2);
    i += 2;

    std::memcpy(&result.capability_flags_upper, &buffer[i], 2);
    i += 2;

    result.capability_flags = (result.capability_flags_upper << 16) | result.capability_flags_lower;

    result.auth_plugin_data_len = static_cast<uint8_t>(buffer[i++]);

    i += 10; // reserved bytes

    int salt2_len = std::max(13, (int)(result.auth_plugin_data_len - 8));
    for (int j = 0; j < salt2_len && i < bytes_received; ++j) {
        if (buffer[i] == '\0') break;
        result.salt2.push_back(static_cast<uint8_t>(buffer[i++]));
    }

    if (buffer[i] == '\0') i++;

    while (i < bytes_received && buffer[i] != '\0') {
        result.auth_plugin_name += buffer[i++];
    }
    result.show();
    return result;
}

int connect_mysql() {
    std::string username = "testuser";
    std::string password = "testpass";
    std::string database = "testdb";
    const char* mysql_host = "127.0.0.1";
    const int mysql_port = 3307;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in server_addr{};
    MYSQLCON conn;
    std::vector<uint8_t> login_packet;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(mysql_port);
    inet_pton(AF_INET, mysql_host, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    std::cout << "✅ Connected to MySQL server on port " << mysql_port << std::endl;

    char buffer[512] = {0};
    ssize_t bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        MYSQLCON conn = parse_mysql_handshake_packet(buffer, bytes_received);
        std::vector<uint8_t> login_packet = craft_login_request_packet("username", "password", conn.get_salt(), "mydb", get_client_capabilities(), 255);

        // Send login packet
        ssize_t bytes_sent = send(sock, login_packet.data(), login_packet.size(), 0);
        if (bytes_sent < 0) {
            perror("Failed to send login packet");
            close(sock);
            return 1;
        }
        std::cout << "📤 Sent login packet (" << login_packet.size() << " bytes)" << std::endl;

        // Receive response from MySQL server
        bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::cout << "📥 Received response (" << bytes_received << " bytes)" << std::endl;
            // Parse and output the response (for example, a successful authentication response or error message)
            std::cout << "Response: ";
            for (int i = 0; i < bytes_received; i++) {
                std::cout << std::hex << (int)(unsigned char)buffer[i] << " ";
            }
            std::cout << std::dec << std::endl;
        } else {
            std::cerr << "Failed to receive response from MySQL server" << std::endl;
        }
    }

    close(sock);
    return 0;
};