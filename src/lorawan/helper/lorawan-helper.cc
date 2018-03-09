/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "lorawan-helper.h"
#include <ns3/lorawan-net-device.h>
#include <ns3/simulator.h>
#include <ns3/mobility-model.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/names.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaWANHelper");

/* ... */
LoRaWANHelper::LoRaWANHelper (void) : m_deviceType (LORAWAN_DT_END_DEVICE_CLASS_A)
{
  m_channel = CreateObject<SingleModelSpectrumChannel> ();

  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
}

LoRaWANHelper::LoRaWANHelper (bool useMultiModelSpectrumChannel) : m_deviceType (LORAWAN_DT_END_DEVICE_CLASS_A)
{
  if (useMultiModelSpectrumChannel)
    {
      m_channel = CreateObject<MultiModelSpectrumChannel> ();
    }
  else
    {
      m_channel = CreateObject<SingleModelSpectrumChannel> ();
    }
  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
}

LoRaWANHelper::~LoRaWANHelper (void)
{
  //m_channel->Dispose ();
  m_channel = 0;
}

void
LoRaWANHelper::AddMobility (Ptr<LoRaWANPhy> phy, Ptr<MobilityModel> m)
{
  phy->SetMobility (m);
}

void
LoRaWANHelper::SetDeviceType (LoRaWANDeviceType deviceType)
{
  m_deviceType = deviceType;
}

void
LoRaWANHelper::SetNbRep (uint8_t nbRep)
{
  m_nbRep = nbRep;
}

void
LoRaWANHelper::EnableLogComponents (enum LogLevel level)
{
  //LogComponentEnableAll (LOG_PREFIX_TIME);
  //LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_ALL);

  LogComponentEnable ("LoRaWANSpectrumValueHelper", level);
  LogComponentEnable ("LoRaWANPhy", level);
  LogComponentEnable ("LoRaWANGatewayApplication", level);
  LogComponentEnable ("LoRaWANErrorModel", level);
  LogComponentEnable ("LoRaWANMac", level);
  LogComponentEnable ("LoRaWANNetDevice", level);
  LogComponentEnable ("LoRaWANInterferenceHelper", level);
  LogComponentEnable ("LoRaWANSpectrumSignalParameters", level);
  LogComponentEnable ("LoRaWANEndDeviceApplication", level);
  LogComponentEnable ("LoRaWANFrameHeader", level);
}

NetDeviceContainer
LoRaWANHelper::Install (NodeContainer c)
{
  NetDeviceContainer devices;
  static uint32_t addressCounter = 1;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;

      Ptr<LoRaWANNetDevice> netDevice = CreateObject<LoRaWANNetDevice> (m_deviceType);

      netDevice->SetChannel (m_channel); // will also set channel on underlying phy(s)
      netDevice->SetNode (node);

      if (m_deviceType != LORAWAN_DT_GATEWAY) {
        netDevice->SetAddress (Ipv4Address(addressCounter++)); // will also set channel on underlying phy(s)
        netDevice->SetAttribute ("NbRep", UintegerValue (m_nbRep)); // set number of repetitions
      }

      node->AddDevice (netDevice);
      devices.Add (netDevice);
    }
  return devices;
}


Ptr<SpectrumChannel>
LoRaWANHelper::GetChannel (void)
{
  return m_channel;
}

void
LoRaWANHelper::SetChannel (Ptr<SpectrumChannel> channel)
{
  m_channel = channel;
}

void
LoRaWANHelper::SetChannel (std::string channelName)
{
  Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
  m_channel = channel;
}

int64_t
LoRaWANHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<LoRaWANNetDevice> lorawan = DynamicCast<LoRaWANNetDevice> (netDevice);
      if (lorawan)
        {
          currentStream += lorawan->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}

/**
 * @brief Write a packet in a PCAP file
 * @param file the output file
 * @param packet the packet
 */
static void
PcapSniffLoRaWAN (Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
  file->Write (Simulator::Now (), packet);
}

//based on LrWANHelper's function of the same name.
void
LoRaWANHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  std::cout << "in enable pcap internal" << std::endl; //TODO: this isn't getting called.
  NS_LOG_FUNCTION (this << prefix << nd << promiscuous << explicitFilename);
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.
  //

  // In the future, if we create different NetDevice types, we will
  // have to switch on each type below and insert into the right
  // NetDevice type
  //
  Ptr<LoRaWANNetDevice> device = nd->GetObject<LoRaWANNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("LoRaWANNetDevice::EnablePcapInternal(): Device " << device << " not of type ns3::LoRaWANNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_LORATAP); //270 is DLT_LORATAP, see here: http://www.tcpdump.org/linktypes.html
                                                      // But though it has been given a DLT, the implementation is not part of the version of Wireshark in Ubuntu's current LTS repos. Going to try out the newer version.
                                                      // it looks like there is a packet_lorawan in the newest wireshark, that could be even more useful than LoRaTAP (which is phy layer only)
  if (promiscuous == true)
    {

      //device->GetMac ()->TraceConnectWithoutContext ("PromiscSniffer", MakeBoundCallback (&PcapSniffLoRaWAN, file));
      if (device->GetDeviceType() == LORAWAN_DT_END_DEVICE_CLASS_A){
        std::cout << "device is class a" << std::endl;
        device->GetMac ()->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffLoRaWAN, file));
      }
      else if (device->GetDeviceType() == LORAWAN_DT_GATEWAY) {
        std::cout << "device is gateway" << std::endl;
        std::vector<Ptr<LoRaWANMac> > macs = device->GetMacs ();
        for (auto mac = macs.cbegin(); mac != macs.cend(); mac++) {
          (*mac)->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffLoRaWAN, file));
        }
      }
      else {
        std::cout << "device type err" << std::endl;
      }
    }
  else
    {
      if (device->GetDeviceType() == LORAWAN_DT_END_DEVICE_CLASS_A){
        std::cout << "device is class a" << std::endl;
        device->GetMac ()->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffLoRaWAN, file));
      }
      else if (device->GetDeviceType() == LORAWAN_DT_GATEWAY) {
        std::cout << "device is gateway" << std::endl;
        std::vector<Ptr<LoRaWANMac> > macs = device->GetMacs ();
        for (auto mac = macs.cbegin(); mac != macs.cend(); mac++) {
          (*mac)->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffLoRaWAN, file));
        }
      }
      else {
        std::cout << "device type err" << std::endl;
      }
      
    }
}

}
