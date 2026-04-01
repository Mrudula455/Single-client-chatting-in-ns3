#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main () {
  NodeContainer nodes;
  nodes.Create (25);

  InternetStackHelper internet;
  internet.Install (nodes);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  int index = 0;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      nodes.Get(index++)->GetObject<MobilityModel>()
        ->SetPosition (Vector (100*x, 100*y, 0));
    }
  }

  AnimationInterface anim ("grid.xml");

  Simulator::Run ();
  Simulator::Destroy ();
}
