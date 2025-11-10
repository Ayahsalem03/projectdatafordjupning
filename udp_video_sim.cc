#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
  double simTime = 10.0;
  uint32_t packetSize = 1200;
  double videoRateMbps = 3.0;
  double bottleneckMbps = 5.0;
  uint32_t delayMs = 20;

  CommandLine cmd(__FILE__);
  cmd.AddValue("simTime", "Simuleringstid (sek)", simTime);
  cmd.AddValue("videoRateMbps", "Videobitrate (Mbit/s)", videoRateMbps);
  cmd.AddValue("bottleneckMbps", "Länkbandbredd (Mbit/s)", bottleneckMbps);
  cmd.AddValue("delayMs", "Fördröjning (ms)", delayMs);
  cmd.Parse(argc, argv);

  Time::SetResolution(Time::NS);
  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(bottleneckMbps) + "Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue(std::to_string(delayMs) + "ms"));
  NetDeviceContainer devices = p2p.Install(nodes);

  InternetStackHelper stack;
  stack.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  uint16_t port = 4000;
  UdpServerHelper server(port);
  ApplicationContainer serverApp = server.Install(nodes.Get(1));
  serverApp.Start(Seconds(0.0));
  serverApp.Stop(Seconds(simTime));

  double rateBps = videoRateMbps * 1e6;
  double interval = (packetSize * 8) / rateBps;
  UdpClientHelper client(interfaces.GetAddress(1), port);
  client.SetAttribute("MaxPackets", UintegerValue(1000000));
  client.SetAttribute("Interval", TimeValue(Seconds(interval)));
  client.SetAttribute("PacketSize", UintegerValue(packetSize));
  ApplicationContainer clientApp = client.Install(nodes.Get(0));
  clientApp.Start(Seconds(0.5));
  clientApp.Stop(Seconds(simTime));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

  for (auto &flow : stats) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);
    std::cout << "==== RESULTAT ====" << std::endl;
    std::cout << "Källa: " << t.sourceAddress << "  -->  Mottagare: " << t.destinationAddress << std::endl;
    std::cout << "Paket skickade: " << flow.second.txPackets << std::endl;
    std::cout << "Paket mottagna: " << flow.second.rxPackets << std::endl;
    std::cout << "Förlust (%): " << (100.0 * (flow.second.txPackets - flow.second.rxPackets) / flow.second.txPackets) << std::endl;
    std::cout << "Medelfördröjning (ms): " << (flow.second.delaySum.GetSeconds() * 1000 / flow.second.rxPackets) << std::endl;
    std::cout << "Genomsnittlig jitter (ms): " << (flow.second.jitterSum.GetSeconds() * 1000 / (flow.second.rxPackets - 1)) << std::endl;
    std::cout << "===================" << std::endl;
  }

  Simulator::Destroy();
  return 0;
}

