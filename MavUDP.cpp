#include "MavUDP.h"

MavUDP::MavUDP(const std::string &host, uint16_t port, uint8_t gcs) :
    m_hostName(host),
    m_port(port),
    m_gcsID(gcs),
    MAX_LINE(2048)
{
    m_stop.store(false);
    m_thread = new std::thread(&MavUDP::run, this);
}

MavUDP::~MavUDP()
{
    m_stop.store(true);
    if (m_thread->joinable())
        m_thread->join();
    delete m_thread;
}

void MavUDP::run()
{
    UDPSocket sock = -1;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton(m_hostName.c_str(), &servaddr.sin_addr);
    char *data = new char[MAX_LINE];

    while (!m_stop.load())
    {
        if (sock <= 0)
        {
            // reconnect
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

            if (sock > 0)
            {
                int optval = 1;
                setsockopt(sock, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &optval, sizeof(optval));

                servaddr.sin_port = htons(m_port);
                if (bind(sock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                {
                    close(sock);
                    sock = -1;
                    std::cout << "Failed bind UDP connection " << m_hostName << ":" << m_port << strerror(errno);
                }
                else
                    std::cout << "Done bind server " << m_port;
            }
            else
            {
                std::cout << "Failed to create UDP connection " << m_hostName << ":" << m_port;
                sock = -1;
            }
        }
        else
        {
            struct sockaddr_in cliaddr;
            memset(&cliaddr, 0, sizeof(cliaddr));
            socklen_t len = sizeof(cliaddr); // len is value/resuslt

            int n = recvfrom(sock, (char *)data, MAX_LINE, MSG_DONTWAIT, (struct sockaddr *)&cliaddr, &len);
            if (n > 0)
            {
                if (m_clients.find(cliaddr) == m_clients.end())
                {
                    std::cout << "New client " << cliaddr.sin_port;
                    // create new
                    MavlinkParser *c = new MavlinkParser(m_gcsID);
                    m_clients.insert(std::pair(cliaddr, c));
                }
                m_clients.at(cliaddr)->read(data, n);
            }

            // to readed buffer
            // prepare data to transmit
            for (const std::pair<const SockAddr, MavlinkParser *> &value : m_clients)
            {
                const int dataLength = value.second->write(data, MAX_LINE);
                if (dataLength > 0)
                {
                    const size_t l = sizeof(value.first.addr);
                    sendto(sock, (const char *)data, dataLength,
                           MSG_DONTWAIT | MSG_NOSIGNAL,
                           (const struct sockaddr *)&value.first.addr, l);
                }
            }

            bool dead = true;
            while (dead) {
                dead = false;
                for (const std::pair<const SockAddr, MavlinkParser *> &value : m_clients)
                {
                    if (value.second->lifetime() > 1000 * 10)
                    {
                        delete value.second;
                        if (m_clients.erase(value.first) > 0)
                            dead = true;
                        break;
                    }
                }
            }
        }
        usleep((sock <= 0) ? 1000000 : 5000);
    }
    if (sock > 0)
        close(sock);
    delete [] data;
}
