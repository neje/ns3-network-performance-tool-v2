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

#ifndef STATS_HEADER_H
#define STATS_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"
#include "ns3/address.h"

namespace ns3 {
/**
 * \ingroup udpclientserver
 *
 * \brief Packet header for UDP client/server application.
 *
 * The header is made of a 32bits sequence number followed by
 * a 64bits time stamp, followed by 32 bit NodeId.
 */
class StatsHeader : public Header
{
public:
  StatsHeader ();

  /**
   * \param seq the sequence number
   */
  void SetSeq (uint32_t seq);
  /**
   * \return the sequence number
   */
  uint32_t GetSeq (void) const;
  /**
   * \return the time stamp
   */
  Time GetTs (void) const;

  /**
   * \param nodeId the sender's node ID
   */
  void SetNodeId (uint32_t nodeId);
  /**
   * \return the sender's node ID
   */
  uint32_t GetNodeId (void) const;
  
  /**
   * \param appId the sender's application ID
   */
  void SetApplicationId (uint32_t appId);
  /**
   * \return the sender's application ID
   */
  uint32_t GetApplicationId (void) const;
  
  /**
   * \brief Setting IP address and port (InetSocketAddress or Inet6SocketAddress) of the receiver in packet header
   * \param addr the receiver's IP address and port (InetSocketAddress or Inet6SocketAddress)
   */
  void SetRxAddress (const Address &addr);
  /**
   * \brief Getting IP address and port (InetSocketAddress or Inet6SocketAddress) of the receiver from packet header
   * \return the receiver's IP address and port (InetSocketAddress or Inet6SocketAddress)
   */
  Address GetRxAddress (void) const
    {
      return m_rxAddress;
    };

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  std::string ToString () const;

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_seq; //!< Sequence number
  uint64_t m_ts; //!< Time stamp
  uint32_t m_nodeId; //!< Sender Node Id
  uint32_t m_appId; //!< Sender Application Id
  Address m_rxAddress; //!< Receiver IP address (IPv4 or IPv6) and port (InetSocketAddress or Inet6SocketAddress)
};

} // namespace ns3

#endif /* STATS_HEADER_H */
