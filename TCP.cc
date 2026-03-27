// File: scratch/tcp-client-server.cc
// Run: ./ns3 run scratch/tcp-client-server

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpClientServer");

// -------- SERVER: called when data arrives --------
void HandleRead (Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from)))
    {
        std::cout << "[SERVER] TCP Received " << packet->GetSize ()
                  << " bytes at time " << Simulator::Now ().GetSeconds ()
                  << " s from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                  << std::endl;
    }
}

// -------- SERVER: called when a new connection is accepted --------
void HandleAccept (Ptr<Socket> socket, const Address &from)
{
    std::cout << "[SERVER] Accepted TCP connection from "
              << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
              << " at " << Simulator::Now ().GetSeconds () << " s" << std::endl;
    socket->SetRecvCallback (MakeCallback (&HandleRead));
}

// -------- CLIENT: recursive send --------
void SendTcpPacket (Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval)
{
    if (pktCount > 0)
    {
        int sent = socket->Send (Create<Packet> (pktSize));
        if (sent > 0)
        {
            std::cout << "[CLIENT] TCP Sent " << pktSize << " bytes at time "
                      << Simulator::Now ().GetSeconds () << " s"
                      << " | Packets remaining: " << pktCount - 1
                      << std::endl;
        }
        Simulator::Schedule (pktInterval, &SendTcpPacket,
                             socket, pktSize, pktCount - 1, pktInterval);
    }
    else
    {
        socket->Close ();
        std::cout << "[CLIENT] All packets sent. Socket closed at "
                  << Simulator::Now ().GetSeconds () << " s" << std::endl;
    }
}

int main (int argc, char *argv[])
{
    Time::SetResolution (Time::NS);

    // ----------- Create 2 nodes -----------
    NodeContainer nodes;
    nodes.Create (2);

    // ----------- Point-to-Point link -----------
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer devices = p2p.Install (nodes);

    // ----------- Internet Stack -----------
    InternetStackHelper internet;
    internet.Install (nodes);

    // ----------- IP Addresses -----------
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    uint16_t port = 9090;

    // ================ TCP SERVER (Node 1) ================
    Ptr<Socket> serverSocket = Socket::CreateSocket (
        nodes.Get (1), TcpSocketFactory::GetTypeId ());

    serverSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), port));
    serverSocket->Listen ();
    serverSocket->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeCallback (&HandleAccept));

    std::cout << "[SERVER] TCP Server listening on "
              << interfaces.GetAddress (1) << ":" << port << std::endl;

    // ================ TCP CLIENT (Node 0) ================
    Ptr<Socket> clientSocket = Socket::CreateSocket (
        nodes.Get (0), TcpSocketFactory::GetTypeId ());

    InetSocketAddress remote = InetSocketAddress (interfaces.GetAddress (1), port);

    // Connect immediately (TCP handshake happens at sim start)
    clientSocket->Connect (remote);

    std::cout << "[CLIENT] TCP Client connecting to "
              << interfaces.GetAddress (1) << ":" << port << std::endl;

    // Schedule sending AFTER connection is established (t=2s)
    // Simulator::Schedule binds args directly — no MakeCallback needed
    Simulator::Schedule (Seconds (2.0), &SendTcpPacket,
                         clientSocket,
                         (uint32_t)1024,   // packet size in bytes
                         (uint32_t)10,     // number of packets
                         Seconds (1.0));   // interval between packets

    Simulator::Stop (Seconds (20.0));
    Simulator::Run ();
    Simulator::Destroy ();

    std::cout << "\n[SIMULATION] TCP simulation complete." << std::endl;
    return 0;
}
