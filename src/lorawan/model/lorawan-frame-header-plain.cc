/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 IDLab-imec
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
 * Author: Floris Van den Abeele <floris.vandenabeele@ugent.be>
 */
#include "lorawan-frame-header-plain.h"
#include "lorawan-mac.h"
#include <ns3/log.h>
#include <ns3/address-utils.h>

namespace ns3 {

/* This class is JUST used in lorawan-mac, which is common to both the gateway and the end device. 
 It is designed to be used with PeekHeader, so the deserialize method just deserializes what is needed in the MAC layer (getDevAddr, IsAck - which is contained within the frameControl byte)
 Those are used to decide whether to send the frame to the upper layers.
 Then in the upper layers, frame-header-downlink and frame-header-uplink are used appropriately. */

NS_LOG_COMPONENT_DEFINE ("LoRaWANFrameHeader");

LoRaWANFrameHeader::LoRaWANFrameHeader () : m_devAddr((uint32_t)0), m_frameControl(0)
{
}

LoRaWANFrameHeader::LoRaWANFrameHeader (Ipv4Address devAddr, bool ack)
{
  m_frameControl = 0;
  setAck(ack);
}

LoRaWANFrameHeader::~LoRaWANFrameHeader ()
{
}

Ipv4Address
LoRaWANFrameHeader::getDevAddr (void) const
{
  return m_devAddr;
}

void
LoRaWANFrameHeader::setDevAddr (Ipv4Address addr)
{
  m_devAddr = addr;
}

bool
LoRaWANFrameHeader::getAck () const
{
  return (m_frameControl & LORAWAN_FHDR_ACK_MASK);
}

void
LoRaWANFrameHeader::setAck (bool ack)
{
  if (ack)
    m_frameControl |= LORAWAN_FHDR_ACK_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_ACK_MASK;
}

TypeId
LoRaWANFrameHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaWANFrameHeader")
    .SetParent<Header> ()
    .SetGroupName ("LoRaWAN")
    .AddConstructor<LoRaWANFrameHeader> ();
  return tid;
}

TypeId
LoRaWANFrameHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LoRaWANFrameHeader::Print (std::ostream &os) const
{
  os << "Device Address = " << std::hex << m_devAddr  << ", frameControl = " << (uint16_t)m_frameControl;
}

uint32_t
LoRaWANFrameHeader::GetSerializedSize (void) const
{
  return 5;
}

void
LoRaWANFrameHeader::Serialize (Buffer::Iterator start) const
{

  /*
    The serialization is performed by frame-header-uplink and frame-header-downlink, not here
  */
  //TODO: log err 
}

uint32_t
LoRaWANFrameHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint32_t nBytes = 0;

  // Device address
  nBytes += 4;
  m_devAddr.Set (i.ReadU32());

  // Frame control field
  nBytes += 1;
  uint8_t frameControl = i.ReadU8();
  m_frameControl = 0;
  if (frameControl & LORAWAN_FHDR_ACK_MASK) {
    setAck(true);
  }

  return nBytes;
}

bool
LoRaWANFrameHeader::IsAck () const
{
  return getAck();
}

} //namespace ns3
