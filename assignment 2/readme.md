# DNS resolver implementation
This was the python implementation of both the recursive and iterative method for the DNS lookup mechanism. The program allows you to resolve domain names to IP addresses using different resolution approach

## Changes I have made
1. in `send_dns_query()`: I have added implementation to send the DNS query using UDP:
    - I used `dns.query.udp()` with the configured timeout to send queries to DNS servers.\

2. in `extract_next_nameservers()`: implemented resolving the NS hostname to IP address
    - I added a loop to resolve each nameserver hostname to its corresponding IP address
    - I added a proper error handling, which ensures that the program continues even if some nameservers can't be resolved

3. in `iterative_dns_lookup()`: I implemented stage progression logic
   - I added code to update the resolution stage 
   => (ROOT → TLD → AUTH) as we move through the DNS hierarchy
   - The switching was such that when the resolution stage of ROOT is crossed, it changes to TLD and when its TLD it changes to AUTH.

4. in `recursive_dns_lookup()`: I fixed and implemented recursive resolution
   - it first queries the NS records for the domain and prints them
   - after that it queries the A records for the domain and prints them
   - it provides the same output format as shown in the assignment examples

## How to Run

The program accepts two command-line arguments:
1. The resolution mode (`iterative` or `recursive`)
2. The domain name to resolve

Examples:
python3 dns_resolver.py iterative example.com
python3 dns_resolver.py recursive example.com

## Error Handling
The implementation includes error handling for various scenarios:
- timeouts during DNS queries
- failed nameserver resolution
- non-existent domains

## Dependencies
This program requires the dnspython library, which can be installed with:
 - pip install dnspython

