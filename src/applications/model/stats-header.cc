/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/stats-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StatsHeader");

NS_OBJECT_ENSURE_REGISTERED (StatsHeader);

StatsHeader::StatsHeader ()
  : m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ()),
    m_nodeId (0),
    m_appId (0)
{
  NS_LOG_FUNCTION (this);
}

void
StatsHeader::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}
uint32_t
StatsHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

Time
StatsHeader::GetTs (void) const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

void
StatsHeader::SetNodeId (uint32_t nodeId)
{
  NS_LOG_FUNCTION (this << m_nodeId);
  m_nodeId = nodeId;
}

uint32_t
StatsHeader::GetNodeId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nodeId;
}

void
StatsHeader::SetApplicationId (uint32_t appId)
{
  NS_LOG_FUNCTION (this << m_appId);
  m_appId = appId;
}

uint32_t
StatsHeader::GetApplicationId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_appId;
}

TypeId
StatsHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StatsHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<StatsHeader> ()
  ;
  return tid;
}

TypeId
StatsHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
StatsHeader::SetRxAddress (const Address &addr)
{
  if (InetSocketAddress::IsMatchingType (addr) || Inet6SocketAddress::IsMatchingType (addr)) 
    {
      m_rxAddress = addr;
    }
  else
    {
      NS_ASSERT_MSG (0, "SetRxAddress: Address is not correct! Type must be InetSocketAddress or Inet6SocketAddress");
    }
}

void
StatsHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << ToString ();
}

std::string
StatsHeader::ToString () const
{
  NS_LOG_FUNCTION (this);
  std::ostringstream os;
  os << "(seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << " nodeId=" << m_nodeId << " appId=" << m_appId << " Ip=";
  if (InetSocketAddress::IsMatchingType (m_rxAddress))
    {
      os << InetSocketAddress::ConvertFrom (m_rxAddress).GetIpv4 ()
          << ":" << InetSocketAddress::ConvertFrom (m_rxAddress).GetPort ();
    }
  else if (Inet6SocketAddress::IsMatchingType (m_rxAddress))
    {
      os << "[" << Inet6SocketAddress::ConvertFrom (m_rxAddress).GetIpv6 ()
          << "]:" << Inet6SocketAddress::ConvertFrom (m_rxAddress).GetPort ();
    }
  else
    {
      os << "<not valid>";
    }    
  os << ")";
  return os.str ();
}


uint32_t
StatsHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t size = 0;
  if (InetSocketAddress::IsMatchingType (m_rxAddress)) 
    {
      size = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(Ipv4Address) + sizeof(uint16_t);
      //size = 4+8+4+4+1+4+2; // seq+ts+nodeId+appId+addrType+ipv4+port
    }
  else if (Inet6SocketAddress::IsMatchingType (m_rxAddress))
    {
      size = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(Ipv6Address) + sizeof(uint16_t);
      //size = 4+8+4+4+1+16+2; // seq+ts+nodeId+appId+addrType+ipv6+port
    }
  else
    {
      NS_ASSERT_MSG (0, "GetSerializedSize: Address is not correct! Type must be InetSocketAddress or Inet6SocketAddress");
    }
//  NS_LOG_UNCOND ("GetSerializedSize: " << size);
  return size;
}

void
StatsHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU64 (m_ts);
  i.WriteHtonU32 (m_nodeId);
  i.WriteHtonU32 (m_appId);
  if (InetSocketAddress::IsMatchingType (m_rxAddress)) 
    {
      i.WriteU8 (4);
      InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom (m_rxAddress);
      Ipv4Address ipAddr = inetAddr.GetIpv4 ();
      uint8_t buf[4];
      ipAddr.Serialize (buf);
      for (int k=0; k<4; k++)
        {
          i.WriteU8 (buf[k]); // network order
        }
      uint16_t port = inetAddr.GetPort ();
      i.WriteHtonU16 (port);
    }
  else if (Inet6SocketAddress::IsMatchingType (m_rxAddress))
    {
      i.WriteU8 (6);
      Inet6SocketAddress inetAddr = Inet6SocketAddress::ConvertFrom (m_rxAddress);
      Ipv6Address ipAddr = inetAddr.GetIpv6 ();
      uint8_t buf[16];
      ipAddr.Serialize (buf);
      for (int k=0; k<16; k++)
        {
          i.WriteU8 (buf[k]); 
        }
      uint16_t port = inetAddr.GetPort ();
      i.WriteHtonU16 (port);
    }
  else
    {
      NS_ASSERT_MSG (0, "Serialize: Address is not correct! Type must be InetSocketAddress or Inet6SocketAddress");
    }
}

uint32_t
StatsHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  m_nodeId = i.ReadNtohU32 ();
  m_appId = i.ReadNtohU32 ();
  uint8_t addrType = i.ReadU8 ();
  if (addrType == 4) 
    {
      uint8_t buf[4];
      for (int k=0; k<4; k++)
        {
          buf[k] = i.ReadU8 (); // network order
        }
      Ipv4Address ipAddr = Ipv4Address::Deserialize (buf);
      uint16_t port = i.ReadNtohU16 ();
      m_rxAddress = InetSocketAddress (ipAddr, port);
    }
  else if (addrType == 6)
    {
      uint8_t buf[16];
      for (int k=0; k<16; k++)
        {
          buf[k] = i.ReadU8 (); 
        }
      Ipv6Address ipAddr = Ipv6Address::Deserialize (buf);
      uint16_t port = i.ReadNtohU16 ();
      m_rxAddress = Inet6SocketAddress (ipAddr, port);
    }
  else
    {
      NS_ASSERT_MSG (0, "Deserialize: Address is not correct! Type must be InetSocketAddress or Inet6SocketAddress");
    }  
  return GetSerializedSize ();
}

} // namespace ns3
