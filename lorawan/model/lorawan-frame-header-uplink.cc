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
#include "lorawan-frame-header-uplink.h"
#include "lorawan-mac.h"
#include <ns3/log.h>
#include <ns3/address-utils.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaWANFrameHeaderUplink");

/*

The LoRaWAN frame header is slightly different for uplink and downlink.
                    7   6         5   4        3..0
On uplink it is:    ADR ADRACKReq ACK Class_B  FOptsLen

The end device writes the uplink version and the NS reads it.
*/

LoRaWANFrameHeaderUplink::LoRaWANFrameHeaderUplink () : m_devAddr((uint32_t)0), m_frameControl(0), m_frameCounter(0), m_serializeFramePort(true)
{
}

//changed by Joe
LoRaWANFrameHeaderUplink::LoRaWANFrameHeaderUplink (Ipv4Address devAddr, bool adr, bool adrAckReq, bool ack, bool classB, uint8_t FOptsLen, uint16_t frameCounter, uint16_t framePort) : m_devAddr(devAddr), m_frameCounter(frameCounter), m_framePort(framePort)
{
  m_frameControl = 0;
  //m_status = 0;

  setAdr(adr);
  setAdrAckReq(adrAckReq);
  setAck(ack);
  setClassB(classB);

  m_serializeFramePort = true; // note that the caller should set m_serializeFramePort to true in case of framePort == 0
}

LoRaWANFrameHeaderUplink::~LoRaWANFrameHeaderUplink ()
{
}

Ipv4Address
LoRaWANFrameHeaderUplink::getDevAddr (void) const
{
  return m_devAddr;
}

void
LoRaWANFrameHeaderUplink::setDevAddr (Ipv4Address addr)
{
  m_devAddr = addr;
}

bool
LoRaWANFrameHeaderUplink::getAck () const
{
  return (m_frameControl & LORAWAN_FHDR_ACK_MASK);
}

void
LoRaWANFrameHeaderUplink::setAck (bool ack)
{
  if (ack)
    m_frameControl |= LORAWAN_FHDR_ACK_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_ACK_MASK;
}

bool
LoRaWANFrameHeaderUplink::getAdr () const
{
  return (m_frameControl & LORAWAN_FHDR_ADR_MASK);
}

void
LoRaWANFrameHeaderUplink::setAdr (bool adr)
{
  if (adr)
    m_frameControl |= LORAWAN_FHDR_ADR_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_ADR_MASK;
}

bool
LoRaWANFrameHeaderUplink::getAdrAckReq () const
{
  return (m_frameControl & LORAWAN_FHDR_ADRACKREQ_MASK);
}

void
LoRaWANFrameHeaderUplink::setAdrAckReq (bool adrAckReq)
{
  if (adrAckReq)
    m_frameControl |= LORAWAN_FHDR_ADRACKREQ_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_ADRACKREQ_MASK;
}

bool
LoRaWANFrameHeaderUplink::getClassB () const
{
  return (m_frameControl & LORAWAN_FHDR_CLASSB_MASK);
}

void
LoRaWANFrameHeaderUplink::setClassB (bool classB)
{
  if (classB)
    m_frameControl |= LORAWAN_FHDR_CLASSB_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_CLASSB_MASK;
}

uint16_t
LoRaWANFrameHeaderUplink::getFrameCounter () const
{
  return m_frameCounter;
}

void
LoRaWANFrameHeaderUplink::setFrameCounter (uint16_t frameCounter)
{
  m_frameCounter = frameCounter;
}

uint8_t
LoRaWANFrameHeaderUplink::getFramePort () const
{
  return m_framePort;
}

void
LoRaWANFrameHeaderUplink::setFramePort (uint8_t framePort)
{
  setSerializeFramePort(true);
  m_framePort = framePort;
}

bool
LoRaWANFrameHeaderUplink::getSerializeFramePort () const
{
  return m_serializeFramePort;
}

void
LoRaWANFrameHeaderUplink::setSerializeFramePort (bool serializeFramePort)
{
  m_serializeFramePort = serializeFramePort;
}

TypeId
LoRaWANFrameHeaderUplink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaWANFrameHeaderUplink")
    .SetParent<Header> ()
    .SetGroupName ("LoRaWAN")
    .AddConstructor<LoRaWANFrameHeaderUplink> ();
  return tid;
}

TypeId
LoRaWANFrameHeaderUplink::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LoRaWANFrameHeaderUplink::Print (std::ostream &os) const
{
  os << "Device Address = " << std::hex << m_devAddr  << ", frameControl = " << (uint16_t)m_frameControl << ", Frame Counter = " << std::dec << (uint32_t) m_frameCounter;

  if (m_serializeFramePort)
    os << ", Frame Port = " << (uint32_t)m_framePort;
}

uint32_t
LoRaWANFrameHeaderUplink::GetSerializedSize (void) const
{
  /*
   * Each frame header will consist of 7 bytes plus any frame options (optional) plus a frame port (optional)
   */

  uint8_t frameOptionsLength = m_frameControl & LORAWAN_FHDR_FOPTSLEN_MASK;
  uint8_t framePortLength = m_serializeFramePort == true ? 1 : 0;

  return 7 + frameOptionsLength + framePortLength;
}


void
LoRaWANFrameHeaderUplink::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU32 (m_devAddr.Get ());
  i.WriteU8 (m_frameControl); //calculated before this function is called, through the add.. functions
  i.WriteU16 (m_frameCounter);
  
  // TODO: frame options ...
  if (m_serializeFramePort) { //if framePort is 0, then the MAC layer messages are read directly from the payload. Else they are read from here (from FOpts)
    i.WriteU8 (m_framePort);
  }
    
  
}

uint32_t
LoRaWANFrameHeaderUplink::Deserialize (Buffer::Iterator start)
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

  if (frameControl & LORAWAN_FHDR_ADR_MASK) {
    setAdr(true);
  }
  if (frameControl & LORAWAN_FHDR_ADRACKREQ_MASK) {
    setAdrAckReq(true);
  }
  if (frameControl & LORAWAN_FHDR_ACK_MASK) {
    setAck(true);
  }
  if (frameControl & LORAWAN_FHDR_CLASSB_MASK) {
    setClassB(true);
  }
  
  // Frame counter
  nBytes += 2;
  uint16_t frameCounter = i.ReadU16();
  setFrameCounter(frameCounter);

  // The header does not indicate whether Frame Port is present, instead it
  // should be present if there is any Frame Payload. The caller should set
  // m_serializeFramePort to true if there is a frame port.
  if (m_serializeFramePort) {
    nBytes += 1;
    uint8_t framePort = i.ReadU8();
    m_framePort = framePort;
  }

  return nBytes;
}

bool
LoRaWANFrameHeaderUplink::IsAck () const
{
  return getAck();
}

} //namespace ns3
