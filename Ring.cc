#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main () {
  NodeContainer nodes;
  nodes.Create (20);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  InternetStackHelper internet;
  internet.Install (nodes);

  for (uint32_t i = 0; i < 20; i++) {
    p2p.Install (nodes.Get(i), nodes.Get((i+1)%20));
  }

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  for (uint32_t i = 0; i < 20; i++) {
    double angle = i * (360.0 / 20);
    double x = 275 + 200 * cos (angle * M_PI / 180);
    double y = 275 + 200 * sin (angle * M_PI / 180);
    nodes.Get(i)->GetObject<MobilityModel>()->SetPosition (Vector (x,y,0));
  }

  AnimationInterface anim ("ring.xml");

  Simulator::Run ();
  Simulator::Destroy ();
}
