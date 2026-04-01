#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main () {
  NodeContainer hub;
  hub.Create (1);

  NodeContainer spokes;
  spokes.Create (20);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  InternetStackHelper internet;
  internet.Install (hub);
  internet.Install (spokes);

  for (uint32_t i = 0; i < 20; i++) {
    p2p.Install (hub.Get(0), spokes.Get(i));
  }

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (hub);
  mobility.Install (spokes);

  hub.Get(0)->GetObject<MobilityModel>()->SetPosition (Vector (250,250,0));

  double angle;
  for (uint32_t i = 0; i < 20; i++) {
    angle = i * (360.0 / 20);
    double x = 250 + 200 * cos (angle * M_PI / 180);
    double y = 250 + 200 * sin (angle * M_PI / 180);
    spokes.Get(i)->GetObject<MobilityModel>()->SetPosition (Vector (x,y,0));
  }

  AnimationInterface anim ("star.xml");

  Simulator::Run ();
  Simulator::Destroy ();
}
