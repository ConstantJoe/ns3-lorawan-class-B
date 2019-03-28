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
#ifndef LORAWAN_FRAME_HEADER_DOWNLINK_H
#define LORAWAN_FRAME_HEADER_DOWNLINK_H

#include <ns3/header.h>
#include "ns3/ipv4-address.h"

//common to both uplink and downlink
#define LORAWAN_FHDR_ADR_MASK 0x80
#define LORAWAN_FHDR_ACK_MASK 0x20
#define LORAWAN_FHDR_FOPTSLEN_MASK 0x0F

//only downlink
//0x40 is RFU for downlink
#define LORAWAN_FHDR_FPENDING_MASK 0x10

#define LORAWAN_FHDR_FOPTSLEN_MAX_SIZE 15

namespace ns3 {

/**
 * \ingroup lorawan
 * Represent the Downlink Frame Header (FHDR) in LoRaWAN
 */
class LoRaWANFrameHeaderDownlink : public Header
{
public:
  LoRaWANFrameHeaderDownlink (void);

  LoRaWANFrameHeaderDownlink (Ipv4Address devAddr, bool adr, bool adrAckReq, bool ack, bool framePending, uint8_t FOptsLen, uint16_t frameCounter, uint16_t framePort);
  ~LoRaWANFrameHeaderDownlink (void);

  Ipv4Address getDevAddr(void) const;
  void setDevAddr(Ipv4Address);

  bool getAck() const;
  void setAck(bool);

  bool getAdr() const;
  void setAdr(bool);

  bool getFramePending() const;
  void setFramePending(bool);

  uint16_t getFrameCounter() const;
  void setFrameCounter(uint16_t);

  uint8_t getFrameOptionsLength () const;

  uint8_t getFramePort() const;
  void setFramePort(uint8_t);

  bool getSerializeFramePort() const;
  void setSerializeFramePort(bool);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  bool IsAck() const;
  bool IsFramePending() const;
  
private:
  Ipv4Address m_devAddr; //!< Short device address of end-device
  uint8_t m_frameControl;
  uint16_t m_frameCounter;
  uint8_t m_framePort; // Not actually part of the frame header, but we include it here for ease of use
  bool m_serializeFramePort;

}; //LoRaWANFrameHeaderDownlink

}; // namespace ns-3

#endif /* LORAWAN_FRAME_HEADER_DOWNLINK_H */
