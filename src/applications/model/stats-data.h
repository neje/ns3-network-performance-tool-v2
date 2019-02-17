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

#ifndef STATS_DATA_H
#define STATS_DATA_H

#include <string>
#include <vector>
#include <fstream>
#include <utility> // std::pair

#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/stats-hist.h"
#include "ns3/log.h"

namespace ns3 {

/*******************************************************
 * Summary & RunSummary
 *******************************************************/
struct Summary
{
	Summary ():
		duration (0),
		throughput (0),
		txPackets (0),
		rxPackets (0),
		lostPackets (0),
		lostRatio (0),
		e2eDelayMin (0),
		e2eDelayMax (0),
		e2eDelayAverage (0),
		e2eDelayMedianEstinate (0),
		e2eDelayJitter (0)
	{};
	void IterativeAdd (const Summary &s, unsigned iteration);

	double duration;
	double throughput;
	uint32_t txPackets;
	uint32_t rxPackets;
	uint32_t lostPackets;
	double lostRatio;
	double e2eDelayMin;
	double e2eDelayMax;
	double e2eDelayAverage;
	double e2eDelayMedianEstinate;
	double e2eDelayJitter;
};

struct RunSummary
{
	RunSummary () :
		numberOfFlows (0)
	{};

	unsigned int numberOfFlows;
	Summary aap; // All packets summary
	Summary aaf; // All flows summary
};

/*******************************************************
 * ScalarData
 *******************************************************/
struct ScalarData
{
  ScalarData (double histRes = 0.0001) 
    : totalRxPackets (0),
	    totalTxPackets (0),
	    totalRxBytes (0),
	    totalTxBytes (0),
	    delayHist (histRes)  // 0.1 ms, default
  {
    delayHist.Clear ();
  }

  void Clear ()
  {
    totalRxPackets = 0;
    totalTxPackets = 0;
	  totalRxBytes = 0;
	  totalTxBytes = 0;
	  delayHist.Clear ();
  }

  uint32_t totalRxPackets; // number of total packets receiced
  uint32_t totalTxPackets; // number of total packets sent
  uint64_t totalRxBytes;   // total received bytes
  uint64_t totalTxBytes;   // total transmitted bytes
  uint16_t packetSizeInBytes;
  Time firstPacketSent, lastPacketSent;
  Time firstPacketReceived, lastPacketReceived;
  Time firstDelay, lastDelay;
  StatsHist delayHist;
};

/*******************************************************
 * VectorData
 *******************************************************/
template<class T>
class VectorData
{
public:
  VectorData (std::string name = "Delay [us]")
    : m_name (name),
      m_numValuesWrittenToFile (0)
  {};
  void WriteValueToFile (std::string fileName, Time time, T t, uint16_t flowIndex = 0, uint32_t seqNo = 0);
  void WriteFileHeader (std::string fileName);
  int GetNValuesWrittenToFile () {return m_numValuesWrittenToFile; };
  void Clear () { m_numValuesWrittenToFile = 0; };
private:
  std::string m_name;
  int m_numValuesWrittenToFile;
};

template<class T>
void VectorData<T>::WriteValueToFile (std::string fileName, Time time, T t, uint16_t flowIndex, uint32_t seqNo)
{
  std::ofstream out;
  out.open (fileName.c_str (), std::ofstream::out | std::ofstream::app);
  out << flowIndex << ",";
  out << time.GetDouble () / 1000.0 << ",";
  out << seqNo << ",";
  
  for (int i=0; i < flowIndex; ++i)
    {
      out << ",";
    }
  
  Time *tp = dynamic_cast<Time*> (&t);
  if (tp != 0)
  { // if T is actually Time print time in micro seconds
    out << t.GetDouble () / 1000.0 << std::endl;  
  }
  else // class T must have defined operator<<
  {
    out << t << std::endl; 
  }
  out.close ();
  m_numValuesWrittenToFile++;
}

template<class T>
void VectorData<T>::WriteFileHeader (std::string fileName)
{
  std::ofstream out;
  out.open (fileName.c_str (), std::ofstream::out | std::ofstream::trunc);
  out << "Flow Index, Time [us], Sequence Id, " << m_name << std::endl;
  out.close ();
}

/*******************************************************
 * NetFlowId
 *******************************************************/
class NetFlowId
{
public:
  NetFlowId (uint32_t sonid, uint32_t soaid, Address siaddr, 
             uint32_t sinid = 0, uint32_t siaid = 0, Address soaddr = Address (), uint16_t i = 0) 
    : sourceNodeId (sonid),
      sourceAppId (soaid),
      sinkAddr (siaddr),
      sinkNodeId (sinid),
      sinkAppId (siaid),
      sourceAddr (soaddr),
      flowIndex (i)
  {};
  
  std::string ToString ();
  std::string ToCsvString ();
  
  friend bool operator== (NetFlowId f1, NetFlowId f2);
  uint32_t sourceNodeId;
  uint32_t sourceAppId;
  Address sinkAddr;
  uint32_t sinkNodeId;
  uint32_t sinkAppId;
  Address sourceAddr;
  uint16_t flowIndex;
};

inline bool 
operator== (NetFlowId f1, NetFlowId f2) 
{
  bool b=0; 
  if ( (InetSocketAddress::IsMatchingType (f1.sinkAddr) || Inet6SocketAddress::IsMatchingType (f1.sinkAddr)) &&
       (InetSocketAddress::IsMatchingType (f2.sinkAddr) || Inet6SocketAddress::IsMatchingType (f2.sinkAddr)) )
    {
      b = (f1.sourceNodeId==f2.sourceNodeId) && (f1.sourceAppId==f2.sourceAppId) && (f1.sinkAddr==f2.sinkAddr);
    }
  else
    {
      NS_ASSERT_MSG (0, "Flow not valid!!! Not valid sink IP address.");
    }    
  return b;    
}

/*******************************************************
 * FlowData
 *******************************************************/
class FlowData
{
public:
  FlowData (NetFlowId fid, std::string fn, bool scalarFileWriteEnable = false, bool vectorFileWriteEnable = false,
            double histRes = 0.0001);

  void PacketReceived (Ptr<const Packet> packet);
  void PacketSent (Ptr<const Packet> packet);
  
//  void SetFileName (std::string fileName) { m_fileName = fileName; };

//  void SetScalarFileWriteEnable (bool b) { m_scalarFileWriteEnable = b; };
  bool IsScalarFileWriteEnabled () { return m_scalarFileWriteEnable; } ;

//  void SetVectorFileWriteEnable (bool b) { m_vectorFileWriteEnable = b; };
  bool IsVectorFileWriteEnabled () { return m_vectorFileWriteEnable; } ;

  Summary Finalize (); // Final calculations and write to file
  NetFlowId& GetFlowId () { return m_flowId; };
  bool IsFirstPacketReceived () { return m_scalarData.totalRxPackets == 0; };

  void Clear () { m_scalarData.Clear (); m_delayVector.Clear (); };
private:
  NetFlowId m_flowId;
  std::string m_fileName;
  ScalarData m_scalarData;
  VectorData<Time> m_delayVector;
  bool m_scalarFileWriteEnable;
  bool m_vectorFileWriteEnable;
};

/*******************************************************
 * StatsFlows
 *******************************************************/
class StatsFlows
{
public:
  StatsFlows (uint64_t rngRun, std::string fn = "noname", bool scalarFileWriteEnable = false, bool vectorFileWriteEnable = false);
  void PacketReceived (Ptr<const Packet> packet, uint32_t sinkNodeId, uint32_t sinkAppId, Address sourceAddr);
  void PacketSent (Ptr<const Packet> packet);
  RunSummary Finalize ();

  void SetFileName (std::string fileName) { m_fileName = fileName; };

  void SetHistResolution (double res) { m_histResolution = res; };
  double GetHistResolution () { return m_histResolution; };

  void SetScalarFileWriteEnable (bool b) { m_scalarFileWriteEnable = b; };
  bool IsScalarFileWriteEnabled () { return m_scalarFileWriteEnable; } ;

  void SetVectorFileWriteEnable (bool b) { m_vectorFileWriteEnable = b; };
  bool IsVectorFileWriteEnabled () { return m_vectorFileWriteEnable; } ;

  void Clear ();

private:
  uint64_t m_rngRun;
  std::vector<FlowData> m_flowData;
  std::string m_fileName;
  ScalarData m_allFlowsScalarSummary;
  bool m_scalarFileWriteEnable;
  bool m_vectorFileWriteEnable;
  double m_histResolution;
};

} // namespace ns3

#endif // STATS_DATA_H
