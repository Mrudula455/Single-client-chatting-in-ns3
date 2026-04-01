#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UnifiedChatApp");

class ChatServerApp : public Application
{
public:
  ChatServerApp () : m_socket (0), m_port (9000) {}
  void Setup (uint16_t port) { m_port = port; }

private:
  virtual void StartApplication ()
  {
    m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
    m_socket->Bind (local);

    m_socket->SetRecvCallback (MakeCallback (&ChatServerApp::HandleRead, this));
  }

  void HandleRead (Ptr<Socket> socket)
  {
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom (from)))
    {
      uint32_t size = packet->GetSize();
      double time = Simulator::Now ().GetSeconds ();

      std::stringstream msg;
      msg<<"ACK | Time:"<<time<<" | Size:"<<size<<" bytes";

      std::string response = msg.str();

      NS_LOG_UNCOND ("Server received packet");

      Ptr<Packet> p = Create<Packet>((uint8_t*)response.c_str(),response.length());
      socket->SendTo(p,0,from);
    }
  }

  Ptr<Socket> m_socket;
  uint16_t m_port;
};


class ChatClientApp : public Application
{
public:
  ChatClientApp () : m_socket (0) {}

  void Setup (Address address, Time interval, std::string name)
  {
    m_peer = address;
    m_interval = interval;
    m_name = name;
  }

private:

  virtual void StartApplication ()
  {
    m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    m_socket->Connect (m_peer);

    m_socket->SetRecvCallback (MakeCallback (&ChatClientApp::HandleRead,this));

    SendPacket ();
  }

  void SendPacket ()
  {
    std::string msg="Hello from "+m_name;

    Ptr<Packet> p=Create<Packet>((uint8_t*)msg.c_str(),msg.length());

    m_socket->Send(p);

    Simulator::Schedule(m_interval,&ChatClientApp::SendPacket,this);
  }

  void HandleRead (Ptr<Socket> socket)
  {
    Ptr<Packet> packet = socket->Recv();

    std::vector<uint8_t> buf(packet->GetSize());
    packet->CopyData(buf.data(),packet->GetSize());

    std::string data((char*)buf.data(),packet->GetSize());

    NS_LOG_UNCOND("["<<m_name<<"] Received -> "<<data);
  }

  Ptr<Socket> m_socket;
  Address m_peer;
  Time m_interval;
  std::string m_name;
};


int main ()
{
  NodeContainer nodes;
  nodes.Create(3);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay",StringValue("2ms"));

  NetDeviceContainer d1 = p2p.Install(nodes.Get(1),nodes.Get(0));
  NetDeviceContainer d2 = p2p.Install(nodes.Get(2),nodes.Get(0));

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;

  address.SetBase("10.1.1.0","255.255.255.0");
  Ipv4InterfaceContainer i1 = address.Assign(d1);

  address.SetBase("10.2.1.0","255.255.255.0");
  Ipv4InterfaceContainer i2 = address.Assign(d2);

  Ptr<ChatServerApp> server = CreateObject<ChatServerApp>();
  server->Setup(9000);
  nodes.Get(0)->AddApplication(server);

  Ptr<ChatClientApp> clientA = CreateObject<ChatClientApp>();
  clientA->Setup(InetSocketAddress(i1.GetAddress(1),9000),Seconds(3),"CLIENT_A");
  nodes.Get(1)->AddApplication(clientA);

  Ptr<ChatClientApp> clientB = CreateObject<ChatClientApp>();
  clientB->Setup(InetSocketAddress(i2.GetAddress(1),9000),Seconds(5),"CLIENT_B");
  nodes.Get(2)->AddApplication(clientB);

  Simulator::Stop(Seconds(15));

  Simulator::Run();
  Simulator::Destroy();
}
