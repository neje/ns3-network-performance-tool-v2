/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Belgrade
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Nenad Jevtic (n.jevtic@sf.bg.ac.rs), Marija Malnar (m.malnar@sf.bg.ac.rs)
 */

#include <fstream>
#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
//#include "ns3/aodv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
//#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("paper-for-electronics-and-electrical-engineering");

int
main (int argc, char **argv)
{
//  Packet::EnablePrinting ();

  uint32_t nNodes = 60; // number of nodes
  uint32_t nSources = 3; // number of source nodes for application traffic (number of sink nodes is the same in this example)

  double simulationDuration = 200.0; // in seconds
  double netStartupTime = 10.0; // [s] time before any application starts sending data

  std::string rate ("50kbps"); // application layer data rate
  uint8_t appStartDiff = 0; // [s] time difference between start of two following applications
  std::string protocol = "ns3::UdpSocketFactory"; // protocol for transport layer
  uint32_t packetSize = 128; // Bytes
  uint32_t port = 80;

  double txp = 17; // dBm, transmission power
  std::string phyMode ("DsssRate5_5Mbps"); // physical data rate and modulation type

  double nodeSpeed = 1.0; // m/s
  double nodePause = 0.0; // s
  double simAreaX = 2000.0; // m
  double simAreaY = 2000.0; // m
  
  std::string m_csvFileNamePrefix ("Net");

  CommandLine cmd;
  cmd.AddValue ("csvFileNamePrefix", "The name prefix of the CSV output file (without .csv extension)", m_csvFileNamePrefix);
  cmd.AddValue ("nNodes", "Number of nodes in simulation", nNodes);
  cmd.AddValue ("nSources", "Number of nodes that send data (max = nNodes/2)", nSources);
  cmd.AddValue ("simTime", "Duration of one simulation run.", simulationDuration);
  cmd.AddValue ("width", "Width of simulation area (X-axis).", simAreaX);
  cmd.AddValue ("height", "Height of simulation area (Y-axis).", simAreaY);
  cmd.AddValue ("simTime", "Height of simulation area (Y-axis).", simulationDuration);
  cmd.AddValue ("dataRate", "Application data rate.", rate);
  cmd.AddValue ("packetSize", "Application test packet size.", packetSize);
  cmd.AddValue ("nodeSpeed", "Application data rate.", nodeSpeed);
  cmd.Parse (argc, argv);

  // File name
  if (m_csvFileNamePrefix == "Net")
    {
	    m_csvFileNamePrefix += "-area" + std::to_string (simAreaX) + "mx" + std::to_string (simAreaY) + "m"
                        + "-nodes" + std::to_string (nSources) + "of" + std::to_string (nNodes)
                        + "-speed" + std::to_string (nodeSpeed)
                        + "-rate" + rate
                        + "-packets" + std::to_string (packetSize) + "B";
    }

  // Disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // Turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));

  // Creating nodes
  NodeContainer adhocNodes;
  adhocNodes.Create (nNodes);

  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);

  // Mobility
  MobilityHelper mobilityAdhoc;
//  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

  std::stringstream ssX;
  ssX << "ns3::UniformRandomVariable[Min=0.0|Max=" << simAreaX << "]";
  std::stringstream ssY;
  ssY << "ns3::UniformRandomVariable[Min=0.0|Max=" << simAreaY << "]";
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue (ssX.str ()));
  pos.Set ("Y", StringValue (ssY.str ()));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
//  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
  mobilityAdhoc.Install (adhocNodes);
//  streamIndex += mobilityAdhoc.AssignStreams (adhocNodes, streamIndex);
//  NS_UNUSED (streamIndex); // From this point, streamIndex is unused

  // Routing & Internet
  InternetStackHelper internet;
  DsrMainHelper dsrMain;
  DsrHelper dsr;
  internet.Install (adhocNodes);
  dsrMain.Install (dsr, adhocNodes);
  /*
  AodvHelper aodv;
  aodv.Set ("EnableHello", BooleanValue (false));
  InternetStackHelper internet;
  internet.SetRoutingHelper(aodv);
  internet.Install (adhocNodes);
  */

  //Assigning ip address
  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer adhocInterfaces;
  adhocInterfaces = addressAdhoc.Assign (adhocDevices);

  // Applications
  Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
  for (uint32_t i = 0; i<nSources; i++)
  {
    std::ostringstream oss;
    oss <<  "10.1.1." << i+1;
    InetSocketAddress destinationAddress = InetSocketAddress (Ipv4Address (oss.str().c_str ()), port); // destination address for sorce apps
    InetSocketAddress sinkReceivingAddress = InetSocketAddress (Ipv4Address::GetAny (), port); // sink nodes receive from any address
    double appJitter = var->GetValue (0.0,0.5); // half of a second jitter
   
    // Source
    StatsSourceHelper sourceAppH (protocol, destinationAddress);
    sourceAppH.SetConstantRate (DataRate (rate));
    sourceAppH.SetAttribute ("PacketSize", UintegerValue(packetSize));
    ApplicationContainer sourceApps = sourceAppH.Install (adhocNodes.Get (nNodes-1-i));
    sourceApps.Start (Seconds (netStartupTime+i*appStartDiff+appJitter)); // Every app starts "appStartDiff" seconds after previous one
    sourceApps.Stop (Seconds (netStartupTime+simulationDuration+appJitter)); // Every app stops after finishes runnig of "simulationDuration" seconds
   
    // Sink 
    StatsSinkHelper sink (protocol, sinkReceivingAddress);
    ApplicationContainer sinkApps = sink.Install (adhocNodes.Get (i));
    sinkApps.Start (Seconds (0.0)); // start at the begining and wait for first packet
    sinkApps.Stop (Seconds (netStartupTime+simulationDuration+1)); // stop a bit later then source to receive the last packet
  }
 
  // Tracing
  StatsFlows oneRunStats (RngSeedManager::GetRun	(), m_csvFileNamePrefix, true, false); // current RngRun, file name, RunSummary to file, EveryPacket to file
  //StatsFlows oneRunStats (m_rngRun, m_csvFileNamePrefix); // current RngRun, file name, false, false
  oneRunStats.SetHistResolution (0.0001); // sets resolution in seconds

  //sf.EnableWriteEvryRunSummary (); or sf.DisableWriteEvryRunSummary (); -> file: <m_csvFileNamePrefix>-Run<RngRun>.csv
  //sf.DisableWriteEveryPacket ();   or sf.EnableWriteEveryPacket ();    -> file: <m_csvFileNamePrefix>-Run<RngRun>.csv

  // Start-stop simulation
  // Stop event is set so that all applications have enough tie to finish 
  Simulator::Stop (Seconds (netStartupTime+simulationDuration+1));
  Simulator::Run ();

  // Write final statistics to file and return run summary
  RunSummary srs = oneRunStats.Finalize ();
  NS_LOG_UNCOND ("Rng Run: " << RngSeedManager::GetRun	());
  NS_LOG_UNCOND ("Number of Flows: " << srs.numberOfFlows);
  NS_LOG_UNCOND ("Throughput [bps]: AAF " << srs.aaf.throughput << ", AAP " << srs.aap.throughput);
  NS_LOG_UNCOND ("Lost Ratio [%]: AAF " << srs.aaf.lostRatio << ", AAP "<< srs.aap.lostRatio);
  NS_LOG_UNCOND ("E2E Delay Average [ms]: AAF " << srs.aaf.e2eDelayAverage * 1000.0 << ", AAP " << srs.aap.e2eDelayAverage * 1000.0);
  NS_LOG_UNCOND ("E2E Delay Median Estimate [ms]: AAF " << srs.aaf.e2eDelayMedianEstinate * 1000.0 << ", AAP " << srs.aap.e2eDelayMedianEstinate * 1000.0);
  NS_LOG_UNCOND ("E2E Delay Jitter [ms]: AAF " << srs.aaf.e2eDelayJitter * 1000.0 << ", AAP " << srs.aap.e2eDelayJitter * 1000.0);

  // End of simulation
  Simulator::Destroy ();
  return 0;
}



