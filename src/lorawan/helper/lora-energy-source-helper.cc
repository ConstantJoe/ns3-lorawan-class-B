/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 KU Leuven
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
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>, Peishuo Li <pressthunder@gmail.com>
 *					Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "lora-energy-source-helper.h"
#include "ns3/energy-source.h"

namespace ns3 {

LoRaEnergySourceHelper::LoRaEnergySourceHelper ()
{
  m_LoRaEnergySource.SetTypeId ("ns3::BasicEnergySource");
}

LoRaEnergySourceHelper::~LoRaEnergySourceHelper ()
{
}

void
LoRaEnergySourceHelper::Set (std::string name, const AttributeValue &v)
{
  m_LoRaEnergySource.Set (name, v);
}

Ptr<EnergySource>
LoRaEnergySourceHelper::DoInstall (Ptr<Node> node) const
{
  NS_ASSERT (node != NULL);
  Ptr<EnergySource> source = m_LoRaEnergySource.Create<EnergySource> ();
  NS_ASSERT (source != NULL);
  source->SetNode (node);
  return source;
}

} // namespace ns3
