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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
//#include "ns3/aodv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("paper-for-electronics-and-electrical-engineering");

/////////////////////////////////////////////
// class RoutingExperiment
// controls one program execution (run)
// holds data from current run
/////////////////////////////////////////////
class RoutingExperiment
{
public:
  RoutingExperiment (uint64_t stopRun = 1, std::string fn = "Net-Parameters"); // default is only one simulation run
  RoutingExperiment (uint64_t startRun, uint64_t stopRun, std::string fn = "Net-Parameters");
  RunSummary Run (int argc, char **argv);
  void WriteToSummaryFile (RunSummary srs);

  void SetRngRun (uint64_t run) { m_rngRun = run; };
  uint64_t GetRngRun () { return m_rngRun; };
  void SetStartRngRun (uint64_t run) { m_startRngRun = run; };
  uint64_t GetStartRngRun () { return m_startRngRun; };
  void SetStopRngRun (uint64_t run) { m_stopRngRun = run; };
  uint64_t GetStopRngRun () { return m_stopRngRun; };
  bool IsExternalRngRunControl () { return m_externalRngRunControl; };

private:
  uint64_t m_startRngRun; // first RngRun
  uint64_t m_stopRngRun; // last RngRun
  uint64_t m_rngRun; // current value for RngRun
  bool m_externalRngRunControl; // internal or external control of rng run numbers
  std::string m_csvFileNamePrefix; // file name for writing simulation summary results
};


RoutingExperiment::RoutingExperiment (uint64_t stopRun, std::string fn):
    m_startRngRun (1), 
    m_stopRngRun (stopRun),
    m_rngRun (1),
    m_externalRngRunControl (false), // default is internal control
    m_csvFileNamePrefix (fn) // Default name is Net-Parameters-Summary
{
	NS_ASSERT_MSG (m_startRngRun <= m_stopRngRun, "First run number must be less or equal to last.");
}

RoutingExperiment::RoutingExperiment (uint64_t startRun, uint64_t stopRun, std::string fn):
    m_startRngRun (startRun), // default is only one simulation run
    m_stopRngRun (stopRun),
    m_rngRun (startRun),
	m_csvFileNamePrefix (fn) // Default name is Net-Parameters-Summary
{
	NS_ASSERT_MSG (m_startRngRun <= m_stopRngRun, "First run number must be less or equal to last.");
}
void
RoutingExperiment::WriteToSummaryFile (RunSummary srs)
{
  std::ofstream out;
  if (m_rngRun == m_startRngRun)
    {
      out.open ((m_csvFileNamePrefix + "-Summary.csv").c_str (), std::ofstream::out | std::ofstream::trunc);
      out << "Rng Run, Number of Flows, Throughput [bps],, Tx Packets,, Rx Packets,, Lost Packets,, Lost Ratio [%],, "
          << "E2E Delay Min [ms],, E2E Delay Max [ms],, E2E Delay Average [ms],, E2E Delay Median Estimate [ms],, E2E Delay Jitter [ms]"
          << std::endl;
      out << ", , all flows avg, all packets avg, all flows avg, all packets avg, all flows avg, all packets avg, all flows avg, all packets avg, all flows avg, all packets avg"
          << "  , all flows avg, all packets avg, all flows avg, all packets avg, all flows avg, all packets avg, all flows avg, all packets avg, all flows avg, all packets avg"
          << std::endl;
    }
  else
    {
      out.open ((m_csvFileNamePrefix + "-Summary.csv").c_str (), std::ofstream::out | std::ofstream::app);
    }
  out << m_rngRun << "," << srs.numberOfFlows << ","
      << srs.afa.throughput << "," << srs.apa.throughput << ","
      << srs.afa.txPackets << "," << srs.apa.txPackets << ","
      << srs.afa.rxPackets << "," << srs.apa.rxPackets << ","
      << srs.afa.lostPackets << "," << srs.apa.lostPackets << ","
      << srs.afa.lostRatio << ","<< srs.apa.lostRatio << ","
      << srs.afa.e2eDelayMin * 1000.0 << "," << srs.apa.e2eDelayMin * 1000.0 << ","
      << srs.afa.e2eDelayMax * 1000.0 << "," << srs.apa.e2eDelayMax * 1000.0 << ","
      << srs.afa.e2eDelayAverage * 1000.0 << "," << srs.apa.e2eDelayAverage * 1000.0 << ","
      << srs.afa.e2eDelayMedianEstinate * 1000.0 << "," << srs.apa.e2eDelayMedianEstinate * 1000.0 << ","
      << srs.afa.e2eDelayJitter * 1000.0 << "," << srs.apa.e2eDelayJitter * 1000.0
      << std::endl;
  if (m_rngRun == m_stopRngRun)
    {
      out << std::endl;
      out << "," << "Min,"
                    << "=MIN(C3:C" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(D3:D" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(E3:E" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(F3:F" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(G3:G" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(H3:H" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(I3:I" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(J3:J" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(K3:K" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(L3:L" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(M3:M" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(N3:N" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(O3:O" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(P3:P" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(Q3:Q" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(R3:R" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(S3:S" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(T3:T" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(U3:U" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MIN(V3:V" << m_stopRngRun - m_startRngRun + 3 << ")"
                    << std::endl;
      out << "," << "Max,"
                    << "=MAX(C3:C" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(D3:D" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(E3:E" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(F3:F" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(G3:G" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(H3:H" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(I3:I" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(J3:J" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(K3:K" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(L3:L" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(M3:M" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(N3:N" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(O3:O" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(P3:P" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(Q3:Q" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(R3:R" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(S3:S" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(T3:T" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(U3:U" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MAX(V3:V" << m_stopRngRun - m_startRngRun + 3 << ")"
                    << std::endl;
      out << "," << "Average,"
                    << "=AVERAGE(C3:C" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(D3:D" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(E3:E" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(F3:F" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(G3:G" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(H3:H" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(I3:I" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(J3:J" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(K3:K" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(L3:L" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(M3:M" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(N3:N" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(O3:O" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(P3:P" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(Q3:Q" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(R3:R" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(S3:S" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(T3:T" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(U3:U" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=AVERAGE(V3:V" << m_stopRngRun - m_startRngRun + 3 << ")"
                    << std::endl;
      out << "," << "Median,"
                    << "=MEDIAN(C3:C" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(D3:D" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(E3:E" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(F3:F" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(G3:G" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(H3:H" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(I3:I" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(J3:J" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(K3:K" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(L3:L" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(M3:M" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(N3:N" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(O3:O" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(P3:P" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(Q3:Q" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(R3:R" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(S3:S" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(T3:T" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(U3:U" << m_stopRngRun - m_startRngRun + 3 << "),"
                    << "=MEDIAN(V3:V" << m_stopRngRun - m_startRngRun + 3 << ")"
                    << std::endl;
      out << "," << "Std. deviation,"
                    << "=STDEV(C3:C" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(D3:D" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(E3:E" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(F3:F" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(G3:G" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(H3:H" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(I3:I" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(J3:J" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(K3:K" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(L3:L" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(M3:M" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(N3:N" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(O3:O" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(P3:P" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(Q3:Q" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(R3:R" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(S3:S" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(T3:T" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(U3:U" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << "),"
                    << "=STDEV(V3:V" << m_stopRngRun - m_startRngRun + 3 << ")/" << "SQRT(" << m_stopRngRun - m_startRngRun + 1 << ")"
                    << std::endl;
    }
  out.close ();
};

RunSummary
RoutingExperiment::Run (int argc, char **argv)
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

  CommandLine cmd;
  cmd.AddValue ("csvFileNamePrefix", "The name prefix of the CSV output file (without .csv extension)", m_csvFileNamePrefix);
  cmd.AddValue ("nNodes", "Number of nodes in simulation", nNodes);
  cmd.AddValue ("nSources", "Number of nodes that send data (max = nNodes/2)", nSources);
  // User can set current rng run manualy (externaly) or authomaticaly
  cmd.AddValue ("externalRngRunControl", "Generation of 0=internal or 1=external current rng run number. If '1', then it must be used with --currentRngRun to externaly set current rng run number.", m_externalRngRunControl);
  // !!! Do not use this parameter if you enable authomatic control of current rng run !!!
  cmd.AddValue ("currentRngRun", "Current number of RngRun if external rng run control is used. It must be used with --externalRngRunControl=1 to prevent authomatic rng run control. Also, must be between startRngRun and stopRngRun. Otherwise can produce unpredictable result.", m_rngRun);
  // User should set start and stop rng run number if set internal control of rng run numbers
  // If user want to externaly control rng runs this also must be set
  cmd.AddValue ("startRngRun", "Start number of RngRun. Used in both internal and external rng run generation.", m_startRngRun);
  cmd.AddValue ("stopRngRun", "End number of RngRun (must be greater then or equal to startRngNum). Used in both internal and external rng run generation.", m_stopRngRun);
  cmd.AddValue ("simTime", "Duration of one simulation run.", simulationDuration);
  cmd.AddValue ("width", "Width of simulation area (X-axis).", simAreaX);
  cmd.AddValue ("height", "Height of simulation area (Y-axis).", simAreaY);
  cmd.AddValue ("simTime", "Height of simulation area (Y-axis).", simulationDuration);
  cmd.AddValue ("dataRate", "Application data rate.", rate);
  cmd.AddValue ("packetSize", "Application test packet size.", packetSize);
  cmd.AddValue ("nodeSpeed", "Application data rate.", nodeSpeed);
  cmd.Parse (argc, argv);

  // Should be placed after cmd.Parse () because user can overload rng run number with command line option "--currentRngRun"
  RngSeedManager::SetRun (m_rngRun);

  // File name
  if (m_csvFileNamePrefix == "Net-Parameters")
    {
	    m_csvFileNamePrefix += "-" + std::to_string (simAreaX) + "mx" + std::to_string (simAreaY) + "m"
                        + "-nodes" + std::to_string (nSources) + "_" + std::to_string (nNodes)
                        + "-" + rate
                        + "-packet" + std::to_string (packetSize) + "B";
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
  StatsFlows oneRunStats (m_rngRun, m_csvFileNamePrefix, true, false); // current RngRun, file name, RunSummary to file, EveryPacket to file
  //StatsFlows oneRunStats (m_rngRun, m_csvFileNamePrefix); // current RngRun, file name, false, false
  oneRunStats.SetHistResolution (0.0001); // sets resolution in seconds

  //sf.EnableWriteEvryRunSummary (); or sf.DisableWriteEvryRunSummary (); -> file: <m_csvFileNamePrefix>-Run<RngRun>.csv
  //sf.DisableWriteEveryPacket ();   or sf.EnableWriteEveryPacket ();    -> file: <m_csvFileNamePrefix>-Run<RngRun>.csv

  // Start-stop simulation
  // Stop event is set so that all applications have enough tie to finish 
  Simulator::Stop (Seconds (netStartupTime+simulationDuration+1));
  NS_LOG_INFO ("Current simulation run [" << m_startRngRun << "->" << m_stopRngRun << "]: " << m_rngRun);
  Simulator::Run ();

  // Write final statistics to file and return run summary
  RunSummary srs = oneRunStats.Finalize ();

  // End of simulation
  Simulator::Destroy ();
  return srs;
}


//////////////////////////////////////////////
// main function
// controls multiple simulation execution (multiple runs)
// holds data between runs 
////////////////////////////////////////////// 
int
main (int argc, char *argv[])
{
  RoutingExperiment experiment;
  

  // Run the same experiment several times for different RngRun numbers
  while (true)
    {
      // Run the experiment
      // Also parse command line arguments if any (this includes rng run parameters)
      RunSummary srs = experiment.Run (argc,argv);
      experiment.WriteToSummaryFile (srs); // -> file: <m_csvFileNamePrefix>-Summary.csv

      // Control rng run
      if (experiment.IsExternalRngRunControl ()) break;
      experiment.SetRngRun (experiment.GetRngRun () + 1);
      if (experiment.GetRngRun () < experiment.GetStartRngRun () || experiment.GetRngRun () > experiment.GetStopRngRun ()) break;
    }
  return 0;
}




