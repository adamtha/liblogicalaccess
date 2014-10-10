/**
 * \file tcpdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief TCP data transport.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace logicalaccess
{
    TcpDataTransport::TcpDataTransport() : d_ipAddress("127.0.0.1"), d_port(9559)
    {
    }

    TcpDataTransport::~TcpDataTransport()
    {
    }

    std::string TcpDataTransport::getIpAddress() const
    {
        return d_ipAddress;
    }

    void TcpDataTransport::setIpAddress(std::string ipAddress)
    {
        d_ipAddress = ipAddress;
    }

    int TcpDataTransport::getPort() const
    {
        return d_port;
    }

    void TcpDataTransport::setPort(int port)
    {
        d_port = port;
    }

    boost::shared_ptr<boost::asio::ip::tcp::socket> TcpDataTransport::getSocket() const
    {
        return d_socket;
    }

    bool TcpDataTransport::connect()
    {
        if (d_socket)
            d_socket->close();

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(getIpAddress()), getPort());
        d_socket.reset(new boost::asio::ip::tcp::socket(ios));

        try
        {
            d_socket->connect(endpoint);
            LOG(LogLevel::INFOS) << "Connected to " << getIpAddress() << " on port " << getPort() << ".";
        }
        catch (boost::system::system_error& ex)
        {
            LOG(LogLevel::ERRORS) << "Cannot establish connection on " << getIpAddress() << ":" << getPort() << " : " << ex.what();
            d_socket.reset();
        }

        return bool(d_socket);
    }

    void TcpDataTransport::disconnect()
    {
        if (d_socket)
        {
            LOG(LogLevel::INFOS) << "Disconnected.";
            d_socket->close();
            d_socket.reset();
        }
    }

    bool TcpDataTransport::isConnected()
    {
        return bool(d_socket);
    }

    std::string TcpDataTransport::getName() const
    {
        return d_ipAddress;
    }

    void TcpDataTransport::send(const std::vector<unsigned char>& data)
    {
        if (data.size() > 0)
        {
            boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getSocket();
            if (socket->is_open())
            {
                LOG(LogLevel::COMS) << "Send command: " << BufferHelper::getHex(data).c_str();
                socket->send(boost::asio::buffer(data));
            }
            else
            {
                LOG(LogLevel::ERRORS) << "TCP socket closed.";
            }
        }
    }

    std::vector<unsigned char> TcpDataTransport::receive(long int timeout)
    {
        std::vector<unsigned char> res;

        boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getSocket();

        long int currentWait = 0;
        size_t lenav = socket->available();
        while (lenav == 0 && (timeout == 0 || currentWait < timeout))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            currentWait += 250;

            lenav = socket->available();
        }

        if (lenav > 0)
        {
            std::vector<unsigned char> bufrcv(lenav);
            size_t len = socket->receive(boost::asio::buffer(bufrcv));
            res = std::vector<unsigned char>(bufrcv.begin(), bufrcv.begin() + len);
        }
        else
        {
            char buf[64];
            sprintf(buf, "Socket receive timeout (%ld > %ld).", currentWait, timeout);
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, buf);
        }

        LOG(LogLevel::COMS) << "Command response: " << BufferHelper::getHex(res);

        return res;
    }

    void TcpDataTransport::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("<xmlattr>.type", getTransportType());
        node.put("IpAddress", d_ipAddress);
        node.put("Port", d_port);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void TcpDataTransport::unSerialize(boost::property_tree::ptree& node)
    {
        d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
        d_port = node.get_child("Port").get_value<int>();
    }

    std::string TcpDataTransport::getDefaultXmlNodeName() const
    {
        return "TcpDataTransport";
    }
}