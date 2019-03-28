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
#ifndef LORAWAN_FRAME_HEADER_H
#define LORAWAN_FRAME_HEADER_H

#include <ns3/header.h>
#include "ns3/ipv4-address.h"

#define LORAWAN_FHDR_ACK_MASK 0x20

namespace ns3 {

/**
 * \ingroup lorawan
 * Represent the Frame Header (FHDR) in LoRaWAN
 */
class LoRaWANFrameHeader : public Header
{
public:
  LoRaWANFrameHeader (void);

  LoRaWANFrameHeader (Ipv4Address devAddr, bool ack);
  ~LoRaWANFrameHeader (void);

  Ipv4Address getDevAddr(void) const;
  void setDevAddr(Ipv4Address);

  bool getAck() const;
  void setAck(bool);

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

}; //LoRaWANFrameHeader

}; // namespace ns-3

#endif /* LORAWAN_FRAME_HEADER_H */
