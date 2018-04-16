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
#include "ns3/lorawan-enddevice-application.h"

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

//used to give % of simulation finished
void
PrintTime(uint8_t i)
{
  //std::cout << "Current time: " << Simulator::Now() << std::endl;
  printf("Percentage finished: %d\n", i);
}

int main (int argc, char *argv[])
{

  uint32_t nNodes = 4;
  uint8_t  dr = 0;

  CommandLine cmd;
  cmd.AddValue("nNodes", "Number of nodes to add to simulation", nNodes);
  cmd.AddValue("dr", "Data rate to be used (up and down, a and b)", dr);
  cmd.Parse (argc, argv);

  dr &= (0b111); //this is a hack, ask Shahwaiz about it.

  
  //printf("%u\n", dr);

  std::cout << nNodes << std::endl;
  /*
  std::cout << UintegerValue(dr) << std::endl;
  std::cout << UintegerValue(nNodes) << std::endl;*/
  


  /*
    Values to be added to cmd:
      dr
      cr
      throughputB
      throughputA
      probably more
  */

  std::cout << nNodes << std::endl;

  NS_LOG_LOGIC("Start of Simulation!");
  NodeContainer endDeviceNodes;
  NodeContainer gatewayNodes;
  NodeContainer allNodes;

  //int numEndDevices = 1;

  endDeviceNodes.Create (nNodes);
  gatewayNodes.Create (1);
  allNodes.Add (endDeviceNodes);
  allNodes.Add (gatewayNodes);

 
  
  // new position allocator, where nodes are randomly placed in a disk of discradius size.
  double m_discRadius = 6100.0;

  MobilityHelper edMobility;
  edMobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (m_discRadius));
  edMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  edMobility.Install (endDeviceNodes);

  MobilityHelper gwMobility;
  Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // gateway
  gwMobility.SetPositionAllocator (nodePositionList);
  gwMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gwMobility.Install (gatewayNodes);

  //MobilityHelper mobility;
  //Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();

  // old position allocator, which looked something like:
  //    e       e
  //
  //        g
  //
  //    e       e

  /*for(uint32_t i = 0; i < nNodes/4 + 1; i++){
    if(i == nNodes/4){
      if(i % nNodes/4 < 1) {
        nodePositionList->Add (Vector ( 5.0*(i+1),  5.0*(i+1), 0.0));
      }
      if(i % nNodes/4 < 2) {
        nodePositionList->Add (Vector (-5.0*(i+1),  5.0*(i+1), 0.0));
      }
      if(i % nNodes/4 < 3) {
         nodePositionList->Add (Vector ( 5.0*(i+1), -5.0*(i+1), 0.0));
      }
      if(i % nNodes/4 < 4) {
        nodePositionList->Add (Vector (-5.0*(i+1), -5.0*(i+1), 0.0));
      }  
    }
    else {
      nodePositionList->Add (Vector ( 5.0*(i+1),  5.0*(i+1), 0.0));  // end device1
      nodePositionList->Add (Vector (-5.0*(i+1),  5.0*(i+1), 0.0));  // end device2
      nodePositionList->Add (Vector ( 5.0*(i+1), -5.0*(i+1), 0.0));  // end device3
      nodePositionList->Add (Vector (-5.0*(i+1), -5.0*(i+1), 0.0));  // end device4
    }
  }
  
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // gateway
  mobility.SetPositionAllocator (nodePositionList);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);*/

  LoRaWANHelper lorawanHelper;

  

  //lorawanHelper.EnableLogComponents(LOG_LEVEL_ALL);

  lorawanHelper.SetNbRep(1);
  NetDeviceContainer lorawanEDDevices = lorawanHelper.Install (endDeviceNodes);

  lorawanHelper.SetDeviceType (LORAWAN_DT_GATEWAY);
  NetDeviceContainer lorawanGWDevices = lorawanHelper.Install (gatewayNodes);

  //lorawanHelper.EnablePcapAll ("myfirst", false); //not using PCAPs for now
  
  // Trace state changes in the phy
  //dev0->GetPhy ()->TraceConnect ("TrxState", std::string ("phy0"), MakeCallback (&StateChangeNotification));
  //dev1->GetPhy ()->TraceConnect ("TrxState", std::string ("phy1"), MakeCallback (&StateChangeNotification));
  //dev2->GetPhy ()->TraceConnect ("TrxState", std::string ("phy2"), MakeCallback (&StateChangeNotification));
  //dev3->GetPhy ()->TraceConnect ("TrxState", std::string ("phy3"), MakeCallback (&StateChangeNotification));
  //for (auto &i : devgw->GetPhys()) {
  //  i->TraceConnect ("TrxState", std::string ("phy-gw"), MakeCallback (&StateChangeNotification));
  //}

  PacketSocketHelper packetSocket;
  packetSocket.Install (endDeviceNodes);
  packetSocket.Install (gatewayNodes);

  LoRaWANGatewayHelper gatewayhelper;
  

  gatewayhelper.SetAttribute ("DefaultClassBDataRateIndex", UintegerValue (dr));

  //leaving attributes set static for now
  ApplicationContainer gatewayApps = gatewayhelper.Install (gatewayNodes);

  LoRaWANEndDeviceHelper enddevicehelper;
  
  enddevicehelper.SetAttribute ("DataRateIndex", UintegerValue (dr));
  enddevicehelper.SetAttribute ("ClassBDataRateIndex", UintegerValue (dr));

  //leaving attributes set to default for now
  ApplicationContainer enddeviceApps = enddevicehelper.Install (endDeviceNodes);
  
  /*for (uint8_t i = 0; i < enddeviceApps.GetN(); i++) { 
     enddeviceApps.Get(i)->SetAttribute ("ClassBDataRateIndex", UintegerValue (4));
     enddeviceApps.Get(i)->SetAttribute ("DataRateIndex", UintegerValue (4));
   }*/
 
  

  //run for a day
  gatewayApps.Start (Seconds (0.0));
  gatewayApps.Stop (Seconds (86400));
  //gatewayApps.Stop (Years (1.0));

  enddeviceApps.Start (Seconds (0.0));
  enddeviceApps.Stop (Seconds (86400));
  //enddeviceApps.Stop (Years (1.0));

  Ptr<Socket> recvSink = SetupPacketReceive (gatewayNodes.Get (0));


  for(uint8_t i=0; i<20; i++){
    Time t = Seconds (i*(86400.0/2000));
    Simulator::Schedule (t, &PrintTime, i*5);
  }
 

  Simulator::Stop (Seconds (86400.0));
  //Simulator::Stop (Years (1.0));

  Simulator::Run ();
  //std::cout << "Going to start running!" << std::endl;

   /*for (uint8_t i = 0; i < enddeviceApps.GetN(); i++) { 
     enddeviceApps.Get(i)->;
   }*/
  
  std::cout << "RESULTS START HERE" << std::endl;
  std::cout << nNodes << std::endl;
  /*std::cout << "End Device " << "ID" << "\t" << "US_A" << "\t" << "DS_RW1" << "\t" << "DS_RW2" << "\t" << "DS_B" << "\t" << "DS_Bea" << "\t" << std::endl;
  std::cout << "NS " << "ID" << "\t" <<  "DS_A_Gen" <<  
      "\t" << "DS_Sent" << "\t" << "DS_RW1" << "\t" << "DS_RW2" << 
      "\t" << "DS_Retr" << "\t" << "DS_Acks" << "\t" << "DS_Bea" << 
      "\t" << "DS_B_Gen" << "\t" << "DS_B" << "\t" << "US_A" << std::endl;
  std::cout << "Net Device " << "ID" <<  "\t" << "TX_busy" << "\t" << "TX_dc" << "\t" << "RX_bea" << "\t" << "RX_dl" << std::endl;;*/

  Simulator::Destroy ();

  return 0;
}
