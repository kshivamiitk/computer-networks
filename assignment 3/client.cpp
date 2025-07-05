#include<iostream>
#include<cstring>
#include<cstdlib>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<unistd.h>

using namespace std;

#define SERVER_PORT 12345 // this is taken from server.cpp
#define SERVER_IP "127.0.0.1" // the server IP address (localhost)
#define CLIENT_PORT 54321


// The function implemented to print the TCP flags for the debugging purposes
void print_output(struct tcphdr *tcp){
    cout << "[+] TCP Flags: "
              << " SYN: " << (int)tcp->syn
              << " ACK: " << (int)tcp->ack
              << " FIN: " << (int)tcp->fin
              << " RST: " << (int)tcp->rst
              << " PSH: " << (int)tcp->psh
              << " SEQ: " << ntohl(tcp->seq) << endl;
}

// Function to send the packet (either SYN or final ACK) using a raw socket;
// this function builds the IP and TCP headers for the packet
void send_packet(int sock , char * src_ip , char * dest_ip , uint16_t src_port , uint16_t dst_port , uint32_t seq , uint32_t ack_seq , bool syn , book ack){

    // The packet is made up of an IP header followed by a TCP header
    char packet[sizeof(struct iphdr ) + sizeof(struct tcphdr)];
    memset(packet , 0 , sizeof(packet));


    // pointers to the IP and TCP header portions within packet
    struct iphdr *ip = (struct iphdr *) packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));


    /*-----------filling the ip header------*/
    ip->ihl = 5;                                // This is the internet header length (5 * 32 bit words)
    ip->version = 4;                            // THis is the IPv4
    ip->tos = 0;                                // The type of the service
    ip->tot_len = htons(sizeof(packet));//      // htons translates the short integer from host byte order to network byte order
    ip->id = htons(54321);                      // identification flag
    ip->frag_off = 0;                           // no fragmentation
    ip->ttl = 64;                               // Time to live
    ip->protocol = IPPROTO_TCP;                 // transport layer protocol : TCP
    ip->saddr = inet_addr(src_ip);              // source IP address
    ip->daddr = inet_addr(dest_ip);             // destination IP address

    /*------ filling the tcp header------*/
    tcp->source = tcp->dest;                        // source port
    tcp->dest = tcp->source;                        // Destination port
    tcp->seq = htonl(seq);                          // TCP seq number
    tcp->ack_seq = htonl(ack_seq);                  // TCP acknowledgement number
    tcp->doff = 5;                                  // Data offset(tcp header length = 5 * 4 = 20 bytes)
    tcp->syn = syn ? 1 : 0; 
    tcp->ack = ack ? 1 : 0;
    tcp->window = htons(8192);                        // window size
    tcp->check = 0;                                  // Kernel will compute the checksum



    /* sending the packet using sendto() */
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dst_port);
    dest_addr.sin_addr.s_addr = inet_addr(dst_ip);
    if(sendto(sock , packet , sizeof(packet) , 0 , (struct sockaddr*)&dest_addr , sizeof(dest_addr)) < 0){
        cout << "sendto() fail" << endl;
        exit(EXIT_FAILURE);
    }
    else{
        if(syn & !ack){
            cout << "[+] Sent SYN (seq=200)" << endl;

        }else if(!syn && ack){
            cout << "[+] Sent final ACK (seq=600)" << endl;
        }
    }
}
int main(){
    // creating a raw socket for the TCP Communication
    int sock = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);


    if(sock < 0){
        cout <<"socket creationg failed" << endl;
        exit(EXIT_FAILURE);
    }

    // Tell the kernel that headers are included in the packet
    int one = 1;
    if(setsockopt(sock , IPPROTO_IP , IP_HDRINCL , &one , sizeof(one)) < 0){
        cout << "setsocket failed" << endl;
        exit(EXIT_FAILURE);
    }


    // ---------------- building and sending the SYN Pacekt -------------------------
    // our syn packet carries sequence number 200.
    send_packet(sock , "127.0.0.1" , SERVER_IP , CLIENT_PORT , SERVER_PORT , 200 , 0 , true , false);


    // --------------waiting for the server's SYN-ACK response----------------------
    char buffer[65536];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    while(true){
        int data_size = recvfrom(sock , buffer , sizeof(buffer) , 0 , (struct scokaddr*) &source_addr , &addr_len);

        if(data_size < 0){
            cout << "recvfrom failed" << endl;
            continue;
        }
        // extract IP and TCP headers from the received packet.

        struct iphdr *ip = (struct iphdr *) buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip -> ihl * 4));
        // check if the packet is from the server(source IP should match server IP)
        // and the packet is destined to our choosen client port.
        if(strcmp(inet_ntoa(*(struct in_addr *)&ip->saddr) , SERVER_IP) != 0){
            continue;
        }
        if(ntohs(tcp->dest)!= CLIENT_PORT){
            continue;
        }
        // printing the TCP flags received for debugging
        print_output(tcp);



        // the server should send a SYN_ACK
        // according to the sever
        // - the sender should send a SYN-ACK with sequence number 400.
        // - the acknowledgement number should be 201 (client seq 200 + 1)
        if(tcp->syn == 1 && tcp->ack == 1 && ntohl(tcp -> seq) == 400){
            cout << "[+] Received SYN-ACK from "<< inet_ntoa(source_addr.sin_addr) << endl;

            // -----------------Send the final ACK ------------------
            // The final ACK packet uses sequence number 600
            // The acknowledgment number should be the servers sequence number + 1 , i.e., 401
            
            send_packet(sock, "127.0.0.1" , SERVER_IP , CLIENT_PORT , SERVER_PORT, 600 , 401 , false , true);

            cout << "[+] Handshake complete" << endl;
            break;
        }
    }
    close(sock);

}