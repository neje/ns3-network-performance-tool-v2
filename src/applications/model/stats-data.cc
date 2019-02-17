/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Belgrade, Faculty of Transport and Traffic Engineering
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
 * Authors: Nenad Jevtic <n.jevtic@sf.bg.ac.rs>
 *                       <nen.jevtic@gmail.com>
 *          Marija Malnar <m.malnar@sf.bg.ac.rs>
 */
 
#include <sstream>
#include <iostream>
#include <algorithm>

#include "ns3/simulator.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/config.h"

#include "ns3/stats-header.h"
#include "ns3/stats-data.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StatsData");

/*******************************************************
 * Summary & RunSummary
 *******************************************************/
void
Summary::IterativeAdd (const Summary &s, unsigned iteration)
{
  duration = (duration * (iteration-1) + s.duration) / iteration;
  throughput = (throughput * (iteration-1) + s.throughput) / iteration;
  txPackets = (txPackets * (iteration-1) + s.txPackets) / iteration;
  rxPackets = (rxPackets * (iteration-1) + s.rxPackets) / iteration;
  lostPackets = (lostPackets * (iteration-1) + s.lostPackets) / iteration;
  lostRatio = (lostRatio * (iteration-1) + s.lostRatio) / iteration;
  e2eDelayMin = (e2eDelayMin * (iteration-1) + s.e2eDelayMin) / iteration;
  e2eDelayMax = (e2eDelayMax * (iteration-1) + s.e2eDelayMax) / iteration;
  e2eDelayAverage = (e2eDelayAverage * (iteration-1) + s.e2eDelayAverage) / iteration;
  e2eDelayMedianEstinate = (e2eDelayMedianEstinate * (iteration-1) + s.e2eDelayMedianEstinate) / iteration;
  e2eDelayJitter = (e2eDelayJitter * (iteration-1) + s.e2eDelayJitter) / iteration;
}

/******************************************************
 * NetFlowId
 *******************************************************/
std::string
NetFlowId::ToString ()
{
  std::ostringstream oss;
  oss << flowIndex << ": " << sourceNodeId << "-" << sourceAppId << "( ";

  if (InetSocketAddress::IsMatchingType (sourceAddr))
    {
      oss << InetSocketAddress::ConvertFrom (sourceAddr).GetIpv4 ()
          << ":" << InetSocketAddress::ConvertFrom (sourceAddr).GetPort ();
    }
  else if (Inet6SocketAddress::IsMatchingType (sourceAddr))
    {
      oss << "[" << Inet6SocketAddress::ConvertFrom (sourceAddr).GetIpv6 ()
          << "]:" << Inet6SocketAddress::ConvertFrom (sourceAddr).GetPort ();
    }
  else
    {
      oss << "<not valid>";
    }    
      
  oss << " ) ---> " << sinkNodeId << "-" << sinkAppId << "( ";

  if (InetSocketAddress::IsMatchingType (sinkAddr))
    {
      oss << InetSocketAddress::ConvertFrom (sinkAddr).GetIpv4 ()
          << ":" << InetSocketAddress::ConvertFrom (sinkAddr).GetPort ();
    }
  else if (Inet6SocketAddress::IsMatchingType (sinkAddr))
    {
      oss << "[" << Inet6SocketAddress::ConvertFrom (sinkAddr).GetIpv6 ()
          << "]:" << Inet6SocketAddress::ConvertFrom (sinkAddr).GetPort ();
    }
  else
    {
      oss << "<not valid>";
    }    

  oss << " )";
  return oss.str ();
}

std::string
NetFlowId::ToCsvString ()
{
  std::ostringstream oss;
  oss << flowIndex << "," << sourceNodeId << "," << sourceAppId << ",";

  if (InetSocketAddress::IsMatchingType (sourceAddr))
    {
      oss << InetSocketAddress::ConvertFrom (sourceAddr).GetIpv4 ()
          << ":" << InetSocketAddress::ConvertFrom (sourceAddr).GetPort ();
    }
  else if (Inet6SocketAddress::IsMatchingType (sourceAddr))
    {
      oss << "[" << Inet6SocketAddress::ConvertFrom (sourceAddr).GetIpv6 ()
          << "]:" << Inet6SocketAddress::ConvertFrom (sourceAddr).GetPort ();
    }
  else
    {
      oss << "<not valid>";
    }    
      
  oss << "," << sinkNodeId << "," << sinkAppId << ",";

  if (InetSocketAddress::IsMatchingType (sinkAddr))
    {
      oss << InetSocketAddress::ConvertFrom (sinkAddr).GetIpv4 ()
          << ":" << InetSocketAddress::ConvertFrom (sinkAddr).GetPort ();
    }
  else if (Inet6SocketAddress::IsMatchingType (sinkAddr))
    {
      oss << "[" << Inet6SocketAddress::ConvertFrom (sinkAddr).GetIpv6 ()
          << "]:" << Inet6SocketAddress::ConvertFrom (sinkAddr).GetPort ();
    }
  else
    {
      oss << "<not valid>";
    }    

  return oss.str ();
}

/*******************************************************
 * FlowData
 *******************************************************/
FlowData::FlowData (NetFlowId fid, std::string fn, bool scalarFileWriteEnable,
                    bool vectorFileWriteEnable, double histRes)
    : m_flowId (fid),
      m_fileName (fn),
      m_scalarData (histRes),
      m_delayVector ("Delay [us]"),
      m_scalarFileWriteEnable (scalarFileWriteEnable),
      m_vectorFileWriteEnable (vectorFileWriteEnable)
{
  NS_LOG_FUNCTION (this);
}

void 
FlowData::PacketReceived (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this);

  // If this is the first packet received at all, write file header
  if (IsVectorFileWriteEnabled () &&  m_flowId.flowIndex == 0 && m_scalarData.totalRxPackets == 0)
    {
      m_delayVector.WriteFileHeader (m_fileName + "-vec.csv");
    }

  StatsHeader statsHeader;
  packet->PeekHeader (statsHeader);
  
  // Scalar data
  m_scalarData.totalRxPackets++; // number of received packets
  m_scalarData.packetSizeInBytes = packet->GetSize (); // last packet's size
  m_scalarData.totalRxBytes += m_scalarData.packetSizeInBytes; // total bytes received
  uint32_t currentSequenceNumber = statsHeader.GetSeq (); // SeqNo is counting from 0, so (SeqNo + 1) is equal to the number of packets sent
  m_scalarData.lastPacketReceived = Simulator::Now ();
  Time lastPacketReceivedIsSent = statsHeader.GetTs ();
  m_scalarData.lastDelay = m_scalarData.lastPacketReceived - lastPacketReceivedIsSent;
  if (m_scalarData.totalRxPackets == 1) // first received packet
  {
    m_scalarData.firstPacketReceived = m_scalarData.lastPacketReceived;
    m_scalarData.firstDelay = m_scalarData.lastDelay; // Warning: actual first packet sent can be lost
  }
  m_scalarData.delayHist.AddValue (m_scalarData.lastDelay.GetSeconds ());

  // vector data
  if (IsVectorFileWriteEnabled ()) 
    {
      m_delayVector.WriteValueToFile (m_fileName + "-vec.csv", m_scalarData.lastPacketReceived, m_scalarData.lastDelay, m_flowId.flowIndex, currentSequenceNumber);
    }
}

void 
FlowData::PacketSent (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this);
  StatsHeader statsHeader;
  packet->PeekHeader (statsHeader);
  
  // Scalar data
  m_scalarData.totalTxPackets++; // number of transmitted packets
  m_scalarData.packetSizeInBytes = packet->GetSize (); // last packet's size
  m_scalarData.totalTxBytes += m_scalarData.packetSizeInBytes; // total bytes transmitted 

  m_scalarData.lastPacketSent = statsHeader.GetTs ();
  if (m_scalarData.totalTxPackets == 1) // first received packet
  {
    m_scalarData.firstPacketSent = m_scalarData.lastPacketSent;
  }
}

Summary
FlowData::Finalize ()
{
  NS_LOG_FUNCTION (this);

  Summary s;
  Time endOfTransmition = (m_scalarData.lastPacketSent - m_scalarData.lastPacketReceived > 0)
                          ? (m_scalarData.lastPacketSent) : (m_scalarData.lastPacketReceived);
  s.duration = (endOfTransmition - m_scalarData.firstPacketSent).GetSeconds ();
  if (s.duration > 0)
    {
      s.throughput = 8.0 * (double)m_scalarData.totalRxBytes / s.duration;
    }
  else
    {
      s.throughput = 0.0;
    }
  s.txPackets = m_scalarData.totalTxPackets;
  s.rxPackets = m_scalarData.totalRxPackets;
  s.lostPackets = m_scalarData.totalTxPackets - m_scalarData.totalRxPackets;
  s.lostRatio = 100*s.lostPackets/s.txPackets;
  s.e2eDelayMin = m_scalarData.delayHist.GetMin ();
  s.e2eDelayMax = m_scalarData.delayHist.GetMax ();
  s.e2eDelayAverage = m_scalarData.delayHist.GetMean ();
  s.e2eDelayMedianEstinate = m_scalarData.delayHist.GetMedianEstimation ();
  s.e2eDelayJitter = m_scalarData.delayHist.GetStdDev ();

  if (IsScalarFileWriteEnabled ())
  {
    std::ofstream out;
    if (m_flowId.flowIndex == 0)
      { // all data go to one file, so when FlowId==0 then delete old file if it exists and start new file
        out.open ((m_fileName + "-sca.csv").c_str (), std::ofstream::out | std::ofstream::trunc);
      }
    else
      { // if FlowId>0 new file is already created and now just append to the end of the file
        out.open ((m_fileName + "-sca.csv").c_str (), std::ofstream::out | std::ofstream::app);
      }
    out << std::endl;
    out << "Flow Index, Source Node, Source App, Source Address, Sink Node, Sink App, Sink Address" << std::endl;
    out << m_flowId.ToCsvString () << std::endl;
    out << "Number of Rx packets for flow:," << s.rxPackets
        << ", written to file:, " << m_delayVector.GetNValuesWrittenToFile ()
        << ", written to hist:, " << m_scalarData.delayHist.GetCount ()
        << std::endl;
    out << std::endl;
    out << "Last packet length [B]:,"  << m_scalarData.packetSizeInBytes << std::endl;
    out << "Transmission duration [s]:,"  << s.duration << std::endl;
    out << "Throughput [bps]:," << s.throughput << std::endl;
    out << "Tx packets:," << s.txPackets << std::endl;
    out << "Rx packets:," << s.rxPackets << std::endl;
    out << "Lost packets:," << s.lostPackets << std::endl;
    out << "Lost packet ratio [%]:," << s.lostRatio << std::endl;
    out << "E2E delay - Min [ms]," << 1000.0*s.e2eDelayMin << std::endl;
    out << "E2E delay - Max [ms]," << 1000.0*s.e2eDelayMax << std::endl;
    out << "E2E delay - Average [ms]," << 1000.0*s.e2eDelayAverage << std::endl;
    out << "E2E delay - Median estimate (+/-" << 1000.0 * 0.5 *m_scalarData.delayHist.GetBinWidth () << ") [ms]:," << 1000.0*s.e2eDelayMedianEstinate << std::endl;
    out << "E2E delay - Jitter [ms]," << 1000.0*s.e2eDelayJitter << std::endl;
    out << std::endl;
    out << "Rx," << "First packet [us]:," << m_scalarData.firstPacketReceived.GetMicroSeconds () << std::endl;
    out << "Rx,"<< "Last packet [us]:," << m_scalarData.lastPacketReceived.GetMicroSeconds () << std::endl;
    Time diffRx = m_scalarData.lastPacketReceived - m_scalarData.firstPacketReceived;
    out << "Rx,"<< "Receiving duration [s]:,"  << diffRx.GetSeconds () << std::endl;
    out << "Rx,"<< "Count of packets:,"  << m_scalarData.totalRxPackets << std::endl;
    out << "Rx,"<< "Total bytes:,"  << m_scalarData.totalRxBytes << std::endl;
    if (diffRx.GetSeconds ())
      {
        out << "Rx,"<< "Throughput [bps]:,"  << 8.0 * (double)m_scalarData.totalRxBytes / diffRx.GetSeconds () << std::endl;
      }
    else
      {
        out << "Rx,"<< "Throughput [bps]:,"  << 0.0 << std::endl;
      }
    out << std::endl;
    out << "Tx," << "First packet [us]:," << m_scalarData.firstPacketSent.GetMicroSeconds () << std::endl;
    out << "Tx,"<< "Last packet [us]:," << m_scalarData.lastPacketSent.GetMicroSeconds () << std::endl;
    Time diffTx = m_scalarData.lastPacketSent - m_scalarData.firstPacketSent;
    out << "Tx,"<< "Sending duration [s]:,"  << diffTx.GetSeconds () << std::endl;
    out << "Tx,"<< "Count of packets:,"  << m_scalarData.totalTxPackets << std::endl;
    out << "Tx,"<< "Total bytes:,"  << (m_scalarData.totalTxBytes) << std::endl;
    if (diffTx.GetSeconds ())
      {
        out << "Tx,"<< "Throughput [bps]:,"  << 8.0 * (double)(m_scalarData.totalTxBytes) / diffTx.GetSeconds () << std::endl;
      }
    else
      {
        out << "Tx,"<< "Throughput [bps]:,"  << 0.0 << std::endl;
      }
    out << std::endl;
    out.close ();
  }
  return s;
}

/******************************************************
 * StatsFlows
 *******************************************************/
StatsFlows::StatsFlows (uint64_t rngRun, std::string fn, bool scalarFileWriteEnable, bool vectorFileWriteEnable) :
    m_rngRun (rngRun),
    m_fileName (fn + "-Run_" + std::to_string (rngRun)),
    m_scalarFileWriteEnable (scalarFileWriteEnable),
    m_vectorFileWriteEnable (vectorFileWriteEnable),
    m_histResolution (0.0001) // 0.1 ms
{ 
  Config::ConnectWithoutContext ("/NodeList/*/ApplicationList/*/$ns3::StatsPacketSink/Rx", MakeCallback (&StatsFlows::PacketReceived, this));
  Config::ConnectWithoutContext ("/NodeList/*/ApplicationList/*/$ns3::StatsPacketSource/Tx", MakeCallback (&StatsFlows::PacketSent, this));
}
 
void
StatsFlows::PacketSent (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this);

  StatsHeader statsHeader;
  packet->PeekHeader (statsHeader);
  NS_LOG_INFO ("Packet header: " << statsHeader.ToString ());

  // All runs scalar statistics
  m_allFlowsScalarSummary.totalTxPackets++; // number of transmitted packets
  m_allFlowsScalarSummary.packetSizeInBytes = packet->GetSize (); // last packet's size
  m_allFlowsScalarSummary.totalTxBytes += m_allFlowsScalarSummary.packetSizeInBytes; // total bytes transmitted
  m_allFlowsScalarSummary.lastPacketSent = statsHeader.GetTs ();
  if (m_allFlowsScalarSummary.totalTxPackets == 1) // first received packet
  {
	  m_allFlowsScalarSummary.firstPacketSent = m_allFlowsScalarSummary.lastPacketSent;
  }
  NS_LOG_INFO ("Packet sent: " << m_allFlowsScalarSummary.totalTxPackets);

  // Detecting FlowId
  uint32_t sourceNodeId = statsHeader.GetNodeId ();
  uint32_t sourceAppId = statsHeader.GetApplicationId ();
  Address sinkAddr = statsHeader.GetRxAddress ();
  NetFlowId fid (sourceNodeId, sourceAppId, sinkAddr);
  NS_LOG_INFO ("Flow: " << fid.ToString ());
  
  // Search for existing FlowIds
  uint16_t i;
  for (i = 0; i < m_flowData.size(); i++)
  {
    if (fid == m_flowData[i].GetFlowId ()) break;
  }
  if (i == m_flowData.size()) // not found -> new FlowId
  {
    fid.flowIndex = i;
    FlowData fd (fid, m_fileName, m_scalarFileWriteEnable, m_vectorFileWriteEnable, m_histResolution);
    m_flowData.push_back (fd);
    NS_LOG_INFO ("New flow [size=" << m_flowData.size () << "]: " << m_flowData[i].GetFlowId ().ToString ());
  }
  else // found existing FlowId
  {
    NS_LOG_INFO ("Flow found: [size=" << m_flowData.size () << "]: " << m_flowData[i].GetFlowId ().ToString ());
  }
  
  m_flowData[i].PacketSent (packet); // call statistics calculations for this particular FlowId
}

void
StatsFlows::PacketReceived (Ptr<const Packet> packet, uint32_t sinkNodeId, uint32_t sinkAppId, Address sourceAddr)
{
  NS_LOG_FUNCTION (this);

  StatsHeader statsHeader;
  packet->PeekHeader (statsHeader);
  NS_LOG_INFO ("Packet header: " << statsHeader.ToString ());

  // Scalar data for all runs
  m_allFlowsScalarSummary.totalRxPackets++; // number of received packets
  m_allFlowsScalarSummary.packetSizeInBytes = packet->GetSize (); // last packet's size
  m_allFlowsScalarSummary.totalRxBytes += m_allFlowsScalarSummary.packetSizeInBytes; // total bytes received
  m_allFlowsScalarSummary.lastPacketReceived = Simulator::Now ();
  Time lastPacketReceivedIsSent = statsHeader.GetTs ();
  m_allFlowsScalarSummary.lastDelay = m_allFlowsScalarSummary.lastPacketReceived - lastPacketReceivedIsSent;
  if (m_allFlowsScalarSummary.totalRxPackets == 1) // first received packet
    {
      m_allFlowsScalarSummary.firstPacketReceived = m_allFlowsScalarSummary.lastPacketReceived;
      m_allFlowsScalarSummary.firstDelay = m_allFlowsScalarSummary.lastDelay; // Warning: actual first packet sent can be lost
    }
  m_allFlowsScalarSummary.delayHist.AddValue (m_allFlowsScalarSummary.lastDelay.GetSeconds ());
  NS_LOG_INFO ("Packet received: " << m_allFlowsScalarSummary.totalRxPackets);
  NS_LOG_INFO (m_allFlowsScalarSummary.totalRxPackets);

  // Detecting FlowId
  uint32_t sourceNodeId = statsHeader.GetNodeId ();
  uint32_t sourceAppId = statsHeader.GetApplicationId ();
  Address sinkAddr = statsHeader.GetRxAddress ();
  NetFlowId fid (sourceNodeId, sourceAppId, sinkAddr, sinkNodeId, sinkAppId, sourceAddr);
  
  // Search for existing FlowIds
  uint16_t i;
  for (i = 0; i < m_flowData.size(); i++)
  {
    if (fid == m_flowData[i].GetFlowId ()) break;
  }
  if (i == m_flowData.size()) // not found in existing FlowIds
  {
    NS_LOG_INFO (">>>>>>>>>>>>>>  New Flow, not possible, assert!!! >>>>>>>>>>>>>>>>>>>>>");
    NS_LOG_INFO ("Packet stats header: " << statsHeader.ToString ());
    NS_LOG_INFO ("Existing flows:");
    for (i = 0; i < m_flowData.size(); i++)
    {
       NS_LOG_INFO (m_flowData[i].GetFlowId ().ToString ());
    }
    NS_LOG_INFO ("New Flow: [size=" << m_flowData.size () << "]: " << fid.ToString ());
    NS_ASSERT_MSG (0, "New Flow, not possible, assert!!!");
  }
  else // Flow exists
  {
    if (m_flowData[i].IsFirstPacketReceived ())
      {
        m_flowData[i].GetFlowId ().sinkNodeId = sinkNodeId;
        m_flowData[i].GetFlowId ().sinkAppId = sinkAppId;
        m_flowData[i].GetFlowId ().sourceAddr = sourceAddr;
      }
    NS_LOG_INFO ("Flow found: [size=" << m_flowData.size () << "]: " << m_flowData[i].GetFlowId ().ToString ());      
  }
  
  m_flowData[i].PacketReceived (packet); // call statistics calculations for this particular FlowId
}

RunSummary
StatsFlows::Finalize ()
{
  NS_LOG_FUNCTION (this);
  
  RunSummary srs;
  srs.numberOfFlows = m_flowData.size();

  // All packets average summary
  Time endOfTransmition = (m_allFlowsScalarSummary.lastPacketSent - m_allFlowsScalarSummary.lastPacketReceived > 0)
                          ? (m_allFlowsScalarSummary.lastPacketSent) : (m_allFlowsScalarSummary.lastPacketReceived);
  srs.aap.duration = (endOfTransmition - m_allFlowsScalarSummary.firstPacketSent).GetSeconds ();
  srs.aap.throughput = (double)m_allFlowsScalarSummary.totalRxBytes * 8.0 / srs.aap.duration;
  srs.aap.txPackets = m_allFlowsScalarSummary.totalTxPackets;
  srs.aap.rxPackets = m_allFlowsScalarSummary.totalRxPackets;
  srs.aap.lostPackets = m_allFlowsScalarSummary.totalTxPackets - m_allFlowsScalarSummary.totalRxPackets;
  srs.aap.lostRatio = 100.0* (double)srs.aap.lostPackets / (double)srs.aap.txPackets;
  srs.aap.e2eDelayMin = m_allFlowsScalarSummary.delayHist.GetMin ();
  srs.aap.e2eDelayMax = m_allFlowsScalarSummary.delayHist.GetMax ();
  srs.aap.e2eDelayAverage = m_allFlowsScalarSummary.delayHist.GetMean ();
  srs.aap.e2eDelayMedianEstinate = m_allFlowsScalarSummary.delayHist.GetMedianEstimation ();
  srs.aap.e2eDelayJitter = m_allFlowsScalarSummary.delayHist.GetStdDev ();

  // All flows average summary
  for (uint16_t i = 0; i < srs.numberOfFlows; i++)
    {
      NS_LOG_INFO ("FINALIZE: call Finalize() for flowId=" << i);
      // Calculates all flows average and also writes flow summary to scalar file if enabled
      srs.aaf.IterativeAdd(m_flowData[i].Finalize (), i+1);
    }

  if (IsScalarFileWriteEnabled ())
    {
      std::ofstream out;
      if (srs.numberOfFlows == 0)
        { // no flows, so open new file (delete old if exists)
          out.open ((m_fileName + "-sca.csv").c_str (), std::ofstream::out | std::ofstream::trunc);
        }
      else
        { // file is already used when writing flow sumary
          out.open ((m_fileName + "-sca.csv").c_str (), std::ofstream::out | std::ofstream::app);
        }
      out << std::endl;
      out << "AVERAGE RESULTS, Average of all flows (" << srs.numberOfFlows << "), Average of all packets" << std::endl;
      out << "Transmission duration [s]:," << srs.aaf.duration << "," << srs.aap.duration << std::endl;
      out << "Throughput [bps]:," << srs.aaf.throughput << "," << srs.aap.throughput << std::endl;
      out << "Tx packets:," << srs.aaf.txPackets << "," << srs.aap.txPackets << std::endl;
      out << "Rx packets:," << srs.aaf.rxPackets << "," << srs.aap.rxPackets << std::endl;
      out << "Lost packets:," << srs.aaf.lostPackets << "," << srs.aap.lostPackets << std::endl;
      out << "Lost packet ratio [%]:," << srs.aaf.lostRatio << "," << srs.aap.lostRatio << std::endl;
      out << "E2E delay - Min [ms]:," << 1000.0*srs.aaf.e2eDelayMin << "," << 1000.0*srs.aap.e2eDelayMin << std::endl;
      out << "E2E delay - Max [ms]:," << 1000.0*srs.aaf.e2eDelayMax << "," << 1000.0*srs.aap.e2eDelayMax << std::endl;
      out << "E2E delay - Average [ms]:," << 1000.0*srs.aaf.e2eDelayAverage << "," << 1000.0*srs.aap.e2eDelayAverage << std::endl;
      out << "E2E delay - Median estimate (+/-" << 1000.0 * 0.5 *m_allFlowsScalarSummary.delayHist.GetBinWidth () << ") [ms]:," << 1000.0*srs.aaf.e2eDelayMedianEstinate << "," << 1000.0*srs.aap.e2eDelayMedianEstinate << std::endl;
      out << "E2E delay - Jitter [ms]:," << 1000.0*srs.aaf.e2eDelayJitter << "," << 1000.0*srs.aap.e2eDelayJitter << std::endl;
      out << std::endl;
      out.close ();
      m_allFlowsScalarSummary.delayHist.WriteToCsvFile (m_fileName + "-sca.csv", 0.0001, "E2E Delay Hist:");
    }
  Clear ();
  return srs;
}

void
StatsFlows::Clear ()
{
  m_allFlowsScalarSummary.Clear ();
  for (uint16_t i = 0; i < m_flowData.size(); i++)
    {
      m_flowData[i].Clear ();
    }
  m_flowData.clear ();
}


} // namespace ns3
