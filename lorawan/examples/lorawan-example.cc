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
  NS_LOG_LOGIC("Start of Example Simulation!");
  NodeContainer endDeviceNodes;
  NodeContainer gatewayNodes;
  NodeContainer allNodes;

  endDeviceNodes.Create (4);
  gatewayNodes.Create (1);
  allNodes.Add (endDeviceNodes);
  allNodes.Add (gatewayNodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
  nodePositionList->Add (Vector (5.0, 5.0, 0.0));  // end device1
  nodePositionList->Add (Vector (-5.0, 5.0, 0.0));  // end device2
  nodePositionList->Add (Vector (5.0, -5.0, 0.0));  // end device3
  nodePositionList->Add (Vector (-5.0, -5.0, 0.0));  // end device4
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // gateway
  mobility.SetPositionAllocator (nodePositionList);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);

  LoRaWANHelper lorawanHelper;

  

  //lorawanHelper.EnableLogComponents(LOG_LEVEL_ALL);

  lorawanHelper.SetNbRep(1);
  NetDeviceContainer lorawanEDDevices = lorawanHelper.Install (endDeviceNodes);

  lorawanHelper.SetDeviceType (LORAWAN_DT_GATEWAY);
  NetDeviceContainer lorawanGWDevices = lorawanHelper.Install (gatewayNodes);

  lorawanHelper.EnablePcapAll ("myfirst", false);
  
  // Trace state changes in the phy
  //dev0->GetPhy ()->TraceConnect ("TrxState", std::string ("phy0"), MakeCallback (&StateChangeNotification));
  //dev1->GetPhy ()->TraceConnect ("TrxState", std::string ("phy1"), MakeCallback (&StateChangeNotification));
  //dev2->GetPhy ()->TraceConnect ("TrxState", std::string ("phy2"), MakeCallback (&StateChangeNotification));
  //dev3->GetPhy ()->TraceConnect ("TrxState", std::string ("phy3"), MakeCallback (&StateChangeNotification));
  //for (auto &i : devgw->GetPhys()) {
  //  i->TraceConnect ("TrxState", std::string ("phy-gw"), MakeCallback (&StateChangeNotification));
  //}


  // Note to self: PacketSocketHelper::Install adds a PacketSocketFactory
  // object as an aggregate object to each of the nodes in the NodeContainer

  // are the application level classes being used here??
  // no - use a different application level to what's currently being used here (not OnOffHelper)
  PacketSocketHelper packetSocket;
  packetSocket.Install (endDeviceNodes);
  packetSocket.Install (gatewayNodes);

  // Not sure what this does
 // PacketSocketAddress socket;
 // socket.SetSingleDevice (lorawanEDDevices.Get (0)->GetIfIndex ()); // Set the address to match only a specified NetDevice...
  // socket.SetPhysicalAddress (lorawanGWDevices.Get (0)->GetAddress ()); // Set destination address
  //socket.SetProtocol (1); // Set the protocol

  /*OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
  onoff.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=100]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=10]"));
  onoff.SetAttribute ("DataRate", DataRateValue (DataRate ("0.4Mbps")));
  onoff.SetAttribute ("DataRate", DataRateValue (DataRate (3*8))); // 3 bytes per second
  onoff.SetAttribute ("DataRate", DataRateValue (DataRate (6*8))); // 3 bytes per second
  onoff.SetAttribute ("PacketSize", UintegerValue (30));

  ApplicationContainer apps = onoff.Install (endDeviceNodes.Get (0));*/

  LoRaWANGatewayHelper gatewayhelper;
  //leaving attributes set static for now
  ApplicationContainer gatewayApps = gatewayhelper.Install (gatewayNodes);

  LoRaWANEndDeviceHelper enddevicehelper;
  //leaving attributes set to default for now
  ApplicationContainer enddeviceApps = enddevicehelper.Install (endDeviceNodes);

  /*apps.Start (Seconds (0.0));
  apps.Stop (Seconds (100));*/

  gatewayApps.Start (Seconds (0.0));
  gatewayApps.Stop (Seconds (1000));
  
  enddeviceApps.Start (Seconds (0.0));
  enddeviceApps.Stop (Seconds (1000));



  Ptr<Socket> recvSink = SetupPacketReceive (gatewayNodes.Get (0));

  Simulator::Stop (Seconds (1000.0));

  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
