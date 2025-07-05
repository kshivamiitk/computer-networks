#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>    // Provides struct ip (BSD style IP header)
#include <netinet/tcp.h>   // Provides struct tcphdr (BSD style TCP header)

#define SERVER_PORT 12345  // Server’s listening port

// Function to send a SYN-ACK packet to the client.
// It builds an IP header and a TCP header, with:
//   - Server’s fixed sequence number set to 400
//   - Acknowledgment set to (client’s seq + 1)
void send_syn_ack(int sock, struct sockaddr_in *client_addr, uint32_t client_seq) {
    // Buffer to hold IP header + TCP header.
    char packet[sizeof(struct ip) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    // Pointers to the IP and TCP headers.
    struct ip *ip_hdr = (struct ip *) packet;
    struct tcphdr *tcp_hdr = (struct tcphdr *) (packet + sizeof(struct ip));

    // --- Build the IP header --- //
    ip_hdr->ip_hl = 5;                       // Header length in 32-bit words (5*4=20 bytes)
    ip_hdr->ip_v = 4;                        // IPv4
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_len = htons(sizeof(packet));  // Total packet length
    ip_hdr->ip_id = htons(54321);            // Identification
    ip_hdr->ip_off = 0;
    ip_hdr->ip_ttl = 64;                     // Time to live
    ip_hdr->ip_p = IPPROTO_TCP;              // Protocol type: TCP
    // Set source IP as the server’s IP (here, loopback "127.0.0.1")
    ip_hdr->ip_src.s_addr = inet_addr("127.0.0.1");
    // Set destination IP as the client’s IP.
    ip_hdr->ip_dst.s_addr = client_addr->sin_addr.s_addr;
    ip_hdr->ip_sum = 0;  // Leaving 0 allows many OSes to recalc the checksum.

    // --- Build the TCP header --- //
    tcp_hdr->th_sport = htons(SERVER_PORT);         // Server’s source port
    tcp_hdr->th_dport = client_addr->sin_port;        // Client’s source port becomes destination
    uint32_t server_seq = 400;                        // Fixed server initial sequence number
    tcp_hdr->th_seq = htonl(server_seq);
    tcp_hdr->th_ack = htonl(client_seq + 1);          // Acknowledge client's SYN by incrementing its sequence
    tcp_hdr->th_off = 5;                              // Data offset (5*4=20 bytes)
    tcp_hdr->th_flags = TH_SYN | TH_ACK;              // Set SYN and ACK flags
    tcp_hdr->th_win = htons(8192);                    // Window size
    tcp_hdr->th_sum = 0;                              // Checksum (left 0; many OSes recalc it)
    tcp_hdr->th_urp = 0;

    // --- Send the SYN-ACK packet --- //
    if (sendto(sock, packet, sizeof(packet), 0,
               (struct sockaddr *)client_addr, sizeof(*client_addr)) < 0) {
        perror("sendto() failed");
    } else {
        std::cout << "[+] Sent SYN-ACK to " << inet_ntoa(client_addr->sin_addr) << std::endl;
    }
}

int main() {
    std::cout << "[+] Server listening on port " << SERVER_PORT << "..." << std::endl;

    // Create a raw socket to capture TCP packets.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed (ensure you are running with sudo)");
        exit(EXIT_FAILURE);
    }

    // Enable inclusion of the IP header in the packets we build.
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char buffer[65536];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    bool handshake_complete = false;
    uint32_t client_seq = 0;

    while (!handshake_complete) {
        memset(buffer, 0, sizeof(buffer));
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&source_addr, &addr_len);
        if (data_size < 0) {
            perror("recvfrom() failed");
            continue;
        }

        // Parse the IP header.
        struct ip *ip_hdr = (struct ip *) buffer;
        int ip_header_length = ip_hdr->ip_hl * 4;

        // Parse the TCP header (which follows immediately after the IP header).
        struct tcphdr *tcp_hdr = (struct tcphdr *)(buffer + ip_header_length);

        // Process only packets destined to our server port.
        if (ntohs(tcp_hdr->th_dport) != SERVER_PORT)
            continue;

        // --- Check for the initial SYN packet (SYN flag set; ACK flag not set) --- //
        if ((tcp_hdr->th_flags & TH_SYN) && !(tcp_hdr->th_flags & TH_ACK)) {
            client_seq = ntohl(tcp_hdr->th_seq);
            std::cout << "[+] Received SYN from " << inet_ntoa(source_addr.sin_addr)
                      << " with seq=" << client_seq << std::endl;
            send_syn_ack(sock, &source_addr, client_seq);
        }

        // --- Check for the final ACK (ACK flag set and no SYN flag) --- //
        if ((tcp_hdr->th_flags & TH_ACK) && !(tcp_hdr->th_flags & TH_SYN)) {
            // The client should acknowledge our server sequence by having th_ack equal to (server_seq + 1)
            // In our SYN-ACK, we set server_seq = 400, so expect th_ack to be 401.
            if (ntohl(tcp_hdr->th_ack) == 401) {
                std::cout << "[+] Received final ACK from " << inet_ntoa(source_addr.sin_addr)
                          << ", handshake complete." << std::endl;
                handshake_complete = true;
                break;
            }
        }
    }

    close(sock);
    return 0;
}
