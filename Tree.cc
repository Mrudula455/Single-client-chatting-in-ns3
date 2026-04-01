#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main () {
  NodeContainer nodes;
  nodes.Create (21);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  InternetStackHelper internet;
  internet.Install (nodes);

  // Root → level 1
  for (int i = 1; i <= 4; i++)
    p2p.Install (nodes.Get(0), nodes.Get(i));

  // level 1 → level 2
  int child = 5;
  for (int i = 1; i <= 4; i++)
    for (int j = 0; j < 4; j++)
      p2p.Install (nodes.Get(i), nodes.Get(child++));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  nodes.Get(0)->SetPosition (Vector (320, 20, 0));

  for (int i = 1; i <= 4; i++)
    nodes.Get(i)->SetPosition (Vector (160*i, 125, 0));

  int pos = 5;
  for (int i = 0; i < 16; i++)
    nodes.Get(pos++)->SetPosition (Vector (80*(i+1), 250, 0));

  AnimationInterface anim ("tree.xml");

  Simulator::Run ();
  Simulator::Destroy ();
}
