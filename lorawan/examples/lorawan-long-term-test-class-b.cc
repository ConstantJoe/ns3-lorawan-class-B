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

/*
 * Try to send data from two class A end devices to a gateway, data is
 * unconfirmed upstream data. Chain is LoRaWANMac -> LoRaWANPhy ->
 * SpectrumChannel -> LoRaWANPhy -> LoRaWANMac
 *
 * Trace Phy state changes, and Mac DataIndication and DataConfirm events
 * to stdout
 */
#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/ipv4-address.h>
#include <ns3/lorawan-module.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/mobility-module.h>
#include <ns3/applications-module.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/node.h>
#include <ns3/packet.h>

#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include <iostream>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("LORAWAN_EXAMPLE");

void
ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  uint64_t bytes = 0;
  while ((packet = socket->Recv ()))
    {
      bytes += packet->GetSize ();
    }

  NS_LOG_LOGIC("SOCKET received " << bytes << " bytes");
}

Ptr<Socket>
SetupPacketReceive (Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  sink->Bind ();
  sink->SetRecvCallback (MakeCallback (&ReceivePacket));
  return sink;
}

int main (int argc, char *argv[])
{

  uint32_t nNodes = 4;
  uint8_t  dr = 0;


  CommandLine cmd;
  cmd.AddValue("nNodes", "Number of nodes to add to simulation", nNodes);
  cmd.AddValue("dr", "Data rate to be used (up and down, a and b)", dr);
  cmd.Parse (argc, argv);

  dr &= (0b111); //this is a bit of a hack, changes inputed int to same as equivalent uint for small values

  NS_LOG_LOGIC("Start of Simulation!");
  NodeContainer endDeviceNodes;
  NodeContainer gatewayNodes;
  NodeContainer allNodes;

  endDeviceNodes.Create (nNodes);
  gatewayNodes.Create (1);
  allNodes.Add (endDeviceNodes);
  allNodes.Add (gatewayNodes);

  

  // position allocator, where nodes are randomly placed in a disk of discradius size.
  double m_discRadius = 6100.0;
  MobilityHelper edMobility;
  edMobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (m_discRadius));
  edMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  edMobility.Install (endDeviceNodes);


  // the gateway is placed at 0,0,0
  MobilityHelper gwMobility;
  Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // gateway
  gwMobility.SetPositionAllocator (nodePositionList);
  gwMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gwMobility.Install (gatewayNodes);


  LoRaWANHelper lorawanHelper;
  lorawanHelper.SetNbRep(1); // no retransmissions
  NetDeviceContainer lorawanEDDevices = lorawanHelper.Install (endDeviceNodes);

  lorawanHelper.SetDeviceType (LORAWAN_DT_GATEWAY);
  NetDeviceContainer lorawanGWDevices = lorawanHelper.Install (gatewayNodes);

  PacketSocketHelper packetSocket;
  packetSocket.Install (endDeviceNodes);
  packetSocket.Install (gatewayNodes);

  
  // install end device application on nodes
  LoRaWANEndDeviceHelper enddevicehelper;
  enddevicehelper.SetAttribute ("DataRateIndex", UintegerValue (dr));
  enddevicehelper.SetAttribute ("ClassBDataRateIndex", UintegerValue (dr));
  enddevicehelper.SetAttribute ("IsClassB", BooleanValue(true));

  ApplicationContainer enddeviceApps = enddevicehelper.Install (endDeviceNodes);

  // install gw application on gateways
  LoRaWANGatewayHelper gatewayhelper;
  gatewayhelper.SetAttribute ("DefaultClassBDataRateIndex", UintegerValue (dr));
  ApplicationContainer gatewayApps = gatewayhelper.Install (gatewayNodes);

  //run for a day
  gatewayApps.Start (Seconds (0.0));
  gatewayApps.Stop (Seconds (86400));

  enddeviceApps.Start (Seconds (0.0));
  enddeviceApps.Stop (Seconds (86400));

  Ptr<Socket> recvSink = SetupPacketReceive (gatewayNodes.Get (0));
 
  Simulator::Stop (Seconds (86400.0));
  Simulator::Run ();
  
  std::cout << "RESULTS START HERE" << std::endl;
  std::cout << nNodes << std::endl;

  Simulator::Destroy ();

  return 0;
}
