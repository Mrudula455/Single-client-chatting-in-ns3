#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main () {
  NodeContainer nodes;
  nodes.Create (20);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  csma.Install (nodes);

  InternetStackHelper internet;
  internet.Install (nodes);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  for (uint32_t i = 0; i < 20; i++) {
    nodes.Get(i)->GetObject<MobilityModel>()->SetPosition (Vector (20*i, 45, 0));
  }

  AnimationInterface anim ("bus.xml");

  Simulator::Run ();
  Simulator::Destroy ();
}
