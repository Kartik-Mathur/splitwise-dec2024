#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <map>
#include <list>
#include <set>
#include <unordered_map>

using namespace std;
using std::list;
using std::map;
using std::multiset;
using std::ostringstream;
using std::pair;
using std::string;
using std::unordered_map;

class Graph
{
public:
    map<string, list<pair<string, int>>> adj;

    void addEdge(string u, string v, int d, bool bidir = false)
    {
        adj[u].push_back(make_pair(v, d));
        if (bidir)
        {
            adj[v].push_back({u, d});
        }
    }

    string splitwise()
    {
        unordered_map<string, int> money;
        multiset<pair<int, string>> s;
        ostringstream out;

        for (auto &p : adj)
        {
            for (auto &child_pair : p.second)
            {
                money[p.first] -= child_pair.second;
                money[child_pair.first] += child_pair.second;
            }
        }

        for (auto &p : money)
        {
            if (p.second != 0)
            {
                s.insert({p.second, p.first});
            }
        }

        while (!s.empty())
        {
            auto low = s.begin();
            auto high = prev(s.end());
            int amount = min(-low->first, high->first);
            string lowPerson = low->second;
            string highPerson = high->second;
            // Print nahi krega just store karta jaega saara output inside it
            // out << lowPerson << " --> " << highPerson << " paid: " << amount << endl;
            out << lowPerson << " --> " << highPerson << " paid: " << amount;

            pair<int, string> newLow = {low->first + amount, lowPerson};
            pair<int, string> newHigh = {high->first - amount, highPerson};

            s.erase(low);
            s.erase(high);

            if (newLow.first != 0)
            {
                s.insert(newLow);
            }
            if (newHigh.first != 0)
            {
                s.insert(newHigh);
            }
        }
        // Finally jo bhi store kia hai out ke andar usse string bana kar bhejdo function se
        return out.str();
    }
};

int main()
{
    int server_fd, new_socket;
    /*
    The server_fd(server file descriptor) is acting like a file descriptor for the socket
    that listens for incoming connections. It is used to manage the socket's
    lifecycle like binding to an address and listening for connections,
    and then closing the socket connection as well.
    1. Creating the socket: int socket(int domain, int type, int protocol);
        - int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        SYNTAX: int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        Where AF_INET: Denotes using IPv4 addresses
        SOCK_STREAM: These services are provided by the TCP protocol
                    - Two way communication that is error free and guarantee
                    data is properly ordered.
        0: It automatically choses the correct protocol for AF_INET and SOCK_STREAM
        that is TCP in our case
    2. The socket is created and is not attached to an address.
        - bind is used to attach it to an address
        - listen() is used to ensure that the socket accepts incoming request
        - connect() is used to add a path to send request to.
    */

    /*
     new_socket(int):
     - When a connection is created the new_socket gets initialised and this
     is what is used to talk to a specific client to send and recieve data
     When connection closes the new_socket would no longer exist
     but the server_fd would keep on listening
     Syntax:int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
     Code: new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)
     # server_fd:Bucket explained already
     # (struct sockaddr *)&address: A pointer to a sockaddr structure
         - It will hold the address of the connecting client
     # (socklen_t *)&addrlen: addrlen is a variable of type socklen_t
         that initially contains the size of the address structure
    */
    struct sockaddr_in address;
    /*
        struct containing the server address defined inside <netinet/in.h>
        This store the internet addresses
        The internal structure is:
        struct sockaddr_in {
            short            sin_family;   // Address family (AF_INET for IPv4)
            unsigned short   sin_port;     // Port number of socket: Stored in network byte order, which is big-endian (most significant byte first)
                - htons() (host to network short) are used to set this field
            struct in_addr   sin_addr;     // Internet address
            char             sin_zero[8];  // Padding to make the structure the same size as struct sockaddr
        };

        # Storing 19(1  0  0  1  1) with and without htons
        Without htons():
        Memory: | 1  1  0  0  1 |
        With htons():
        Memory: | 1  0  0  1  1 |

        All the values are need to be set
    */
    int opt = 1;
    /*
        setsockopt(): Uses this typically set to 1,
        let's say server restarted then there is sometime we need to wait
        "address already in use", this is avoided by setting it to 1
    */
    int addrlen = sizeof(address);
    // Used inside the accept and bind functions for internal implementation

    // To store incoming data from request, we create buffer[]
    // The size of the buffer (4096 bytes) is generally large enough to accommodate the
    // maximum amount of data that might be received in one segment of a TCP/IP transmission
    char buffer[4096] = {0};
    Graph g;
    g.addEdge("Aadeep", "Agrim", 1000);
    g.addEdge("Agrim", "Anurag", 200);
    g.addEdge("Niyati", "Aadeep", 1100);
    g.addEdge("Harman", "Niyati", 500);
    g.addEdge("Aryan", "Agrim", 800);
    g.addEdge("Yogesh", "Kanak", 800);

    string transactions = g.splitwise();
    // socket(AF_INET, SOCK_STREAM, 0)) == 0 Comparing if the output is zero
    // thus true would be used and it means socket is created successfully.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /*
        SOL_SOCKET
            - Means the options are to be configured at the socket level.
        SO_REUSEADDR
            - It's useful for server to restart but might still have the port
            they bind to in the TIME_WAIT state. Without setting this option, bind() could
            fail with EADDRINUSE if the socket address is still considered in use.
    */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int result = ::bind(server_fd, (struct sockaddr *)&address, addrlen);
    /*
        bind(): function is used to associate the server's socket,
        identified by server_fd, with a specific local IP address and port number
        specified in the address structure.
    */
    //  Result is 0 on success and -1 on fail
    if (result < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // Syntax: int listen(int sockfd, int backlog);
    /*
        This parameter specifies the maximum length to which the queue of
        pending connections for sockfd may grow. This is not necessarily the
        exact number of connections that will be queued; it is an upper limit.
    */
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // There is a loop that keep on acception new connections
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)))
    {
        if (new_socket < 0)
        {
            perror("accept");
            continue;
        }

        memset(buffer, 0, 4096);                         // Empty the buffer
        int bytes_read = read(new_socket, buffer, 4096); // reads the data sent by client into buffer
        string request(buffer, bytes_read);              // change the buffer to string

        string response;
        /*
            request.find("GET /data HTTP/1.1"): This line searches the request
            string for the substring "GET /data HTTP/1.1"
        */
        /*
            GET /data HTTP/1.1 we are trying to find in the request if it is found then it returns the
            index, else it returns std::string::npos
        */
        if (request.find("GET /data HTTP/1.1") != std::string::npos)
        {
            ostringstream jsonStream;
            /*
            ostringstream jsonStream;: An ostringstream object is used here to construct a JSON string
            jsonStream << ...: This line constructs a JSON object containing one key-value pair,
            where "transactions" is the key, and transactions (a variable containing transaction data)
            is the value
            */
            jsonStream << "{"
                       << "\"transactions\": \"" << transactions << "\""
                       << "}";
            string jsonResponse = jsonStream.str();
            /*
                jsonResponse: After constructing the JSON string, it is converted to a
                standard string using jsonStream.str()
            */
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json\r\n"
                       "Access-Control-Allow-Origin: *\r\n"
                       "Content-Length: " +
                       to_string(jsonResponse.length()) + "\r\n\r\n" + jsonResponse;
            /*
                Headers:
                    Content-Type: application/json: Specifies that the response body is in JSON format.
                    Access-Control-Allow-Origin: *: This header allows all domains to request data from this server, which is crucial for APIs that might be accessed from different domains (helpful in development and certain production scenarios).
                    Content-Length:: This header indicates the length of the response body, crucial for proper loading and processing by the client.
                    Body: The actual JSON response is appended after headers and a blank line.
            */
        }
        else
        {
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: 13\r\n\r\n"
                       "404 Not Found";
        }

        send(new_socket, response.c_str(), response.size(), 0);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
