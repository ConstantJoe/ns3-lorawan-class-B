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
#include "lorawan-frame-header-downlink.h"
#include "lorawan-mac.h"
#include <ns3/log.h>
#include <ns3/address-utils.h>


#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaWANFrameHeaderDownlink");


/*
The LoRaWAN frame header is slightly different for uplink and downlink.
                    7   6         5   4        3..0
On downlink it is:  ADR RFU       ACK FPending FOptsLen
The NS writes the downlink version and the end device reads it.
*/


LoRaWANFrameHeaderDownlink::LoRaWANFrameHeaderDownlink () : m_devAddr((uint32_t)0), m_frameControl(0), m_frameCounter(0), m_serializeFramePort(true)
{
}

LoRaWANFrameHeaderDownlink::LoRaWANFrameHeaderDownlink (Ipv4Address devAddr, bool adr, bool adrAckReq, bool ack, bool framePending, uint8_t FOptsLen, uint16_t frameCounter, uint16_t framePort) : m_devAddr(devAddr), m_frameCounter(frameCounter), m_framePort(framePort)
{
  m_frameControl = 0;

  setAdr(adr);
  setAck(ack);
  setFramePending(framePending); //DL only

  m_serializeFramePort = true; // note that the caller should set m_serializeFramePort to true in case of framePort == 0
}

LoRaWANFrameHeaderDownlink::~LoRaWANFrameHeaderDownlink ()
{
}

Ipv4Address
LoRaWANFrameHeaderDownlink::getDevAddr (void) const
{
  return m_devAddr;
}

void
LoRaWANFrameHeaderDownlink::setDevAddr (Ipv4Address addr)
{
  m_devAddr = addr;
}

bool
LoRaWANFrameHeaderDownlink::getAdr () const
{
  return (m_frameControl & LORAWAN_FHDR_ADR_MASK);
}

void
LoRaWANFrameHeaderDownlink::setAdr (bool adr)
{
  if (adr)
    m_frameControl |= LORAWAN_FHDR_ADR_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_ADR_MASK;
}

bool
LoRaWANFrameHeaderDownlink::getAck () const
{
  return (m_frameControl & LORAWAN_FHDR_ACK_MASK);
}

void
LoRaWANFrameHeaderDownlink::setAck (bool ack)
{
  if (ack)
    m_frameControl |= LORAWAN_FHDR_ACK_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_ACK_MASK;
}

bool
LoRaWANFrameHeaderDownlink::getFramePending () const
{
  return (m_frameControl & LORAWAN_FHDR_FPENDING_MASK);
}

void
LoRaWANFrameHeaderDownlink::setFramePending (bool framePending)
{
  if (framePending)
    m_frameControl |= LORAWAN_FHDR_FPENDING_MASK;
  else
    m_frameControl &= ~LORAWAN_FHDR_FPENDING_MASK;
}

uint16_t
LoRaWANFrameHeaderDownlink::getFrameCounter () const
{
  return m_frameCounter;
}

void
LoRaWANFrameHeaderDownlink::setFrameCounter (uint16_t frameCounter)
{
  m_frameCounter = frameCounter;
}

uint8_t 
LoRaWANFrameHeaderDownlink::getFrameOptionsLength () const
{
  return (m_frameControl & LORAWAN_FHDR_FOPTSLEN_MASK);
}

uint8_t
LoRaWANFrameHeaderDownlink::getFramePort () const
{
  return m_framePort;
}

void
LoRaWANFrameHeaderDownlink::setFramePort (uint8_t framePort)
{
  setSerializeFramePort(true);
  m_framePort = framePort;
}

bool
LoRaWANFrameHeaderDownlink::getSerializeFramePort () const
{
  return m_serializeFramePort;
}

void
LoRaWANFrameHeaderDownlink::setSerializeFramePort (bool serializeFramePort)
{
  m_serializeFramePort = serializeFramePort;
}

TypeId
LoRaWANFrameHeaderDownlink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaWANFrameHeaderDownlink")
    .SetParent<Header> ()
    .SetGroupName ("LoRaWAN")
    .AddConstructor<LoRaWANFrameHeaderDownlink> ();
  return tid;
}

TypeId
LoRaWANFrameHeaderDownlink::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LoRaWANFrameHeaderDownlink::Print (std::ostream &os) const
{
  os << "Device Address = " << std::hex << m_devAddr  << ", frameControl = " << (uint16_t)m_frameControl << ", Frame Counter = " << std::dec << (uint32_t) m_frameCounter;

  if (m_serializeFramePort)
    os << ", Frame Port = " << (uint32_t)m_framePort;
}

uint32_t
LoRaWANFrameHeaderDownlink::GetSerializedSize (void) const
{
  /*
   * Each frame header will consist of 7 bytes plus any frame options (optional) plus a frame port (optional)
   */

  uint8_t frameOptionsLength = m_frameControl & LORAWAN_FHDR_FOPTSLEN_MASK;
  uint8_t framePortLength = m_serializeFramePort == true ? 1 : 0;

  return 7 + frameOptionsLength + framePortLength;
}


void
LoRaWANFrameHeaderDownlink::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU32 (m_devAddr.Get ());
  i.WriteU8 (m_frameControl);
  i.WriteU16 (m_frameCounter);

  // TODO: frame options ...
  if (m_serializeFramePort) { //if framePort is 0, then the MAC layer messages are read directly from the payload. Else they are read from here (from FOpts)
    i.WriteU8 (m_framePort);
    
  }
}

uint32_t
LoRaWANFrameHeaderDownlink::Deserialize (Buffer::Iterator start)
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
  if (frameControl & LORAWAN_FHDR_ACK_MASK) {
    setAck(true);
  }
  if (frameControl & LORAWAN_FHDR_FPENDING_MASK) { 
    setFramePending(true);
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
LoRaWANFrameHeaderDownlink::IsAck () const
{
  return getAck();
}

bool
LoRaWANFrameHeaderDownlink::IsFramePending () const
{
  return getFramePending();
}

} //namespace ns3
