/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 National University of Ireland, Maynooth
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
 * Author: Joseph Finnegan <joseph.finnegan@mu.ie>
 */

#include "lorawan-radio-energy-model-helper.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lorawan-net-device.h"
#include "ns3/lorawan-phy.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/lorawan-current-model.h"

namespace ns3 {

LoRaWANRadioEnergyModelHelper::LoRaWANRadioEnergyModelHelper ()
{
  m_radioEnergy.SetTypeId ("ns3::LoRaWANRadioEnergyModelHelper");
  m_depletionCallback.Nullify ();
  m_rechargedCallback.Nullify ();
}

LoRaWANRadioEnergyModelHelper::~LoRaWANRadioEnergyModelHelper ()
{
}

void
LoRaWANRadioEnergyModelHelper::Set (std::string name, const AttributeValue &v)
{
  m_radioEnergy.Set (name, v);
}

void
LoRaWANRadioEnergyModelHelper::SetDepletionCallback (
  WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback)
{
  m_depletionCallback = callback;
}

void
LoRaWANRadioEnergyModelHelper::SetRechargedCallback (
  WifiRadioEnergyModel::WifiRadioEnergyRechargedCallback callback)
{
  m_rechargedCallback = callback;
}


//For example, SX1272LoRaWANRadioEnergyModel
void
LoRaWANRadioEnergyModelHelper::SetCurrentModel (std::string name,
                                               /*std::string n0, const AttributeValue& v0,
                                               std::string n1, const AttributeValue& v1,
                                               std::string n2, const AttributeValue& v2,
                                               std::string n3, const AttributeValue& v3,
                                               std::string n4, const AttributeValue& v4,
                                               std::string n5, const AttributeValue& v5,
                                               std::string n6, const AttributeValue& v6,
                                               std::string n7, const AttributeValue& v7*/)
{
  ObjectFactory factory;
  factory.SetTypeId (name);
  /*factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);*/
  m_currentModel = factory;

  //??
}


/*
 * Private function starts here.
 */

Ptr<DeviceEnergyModel>
LoRaWANRadioEnergyModelHelper::DoInstall (Ptr<NetDevice> device,
                                       Ptr<EnergySource> source) const
{
  NS_ASSERT (device != NULL);
  NS_ASSERT (source != NULL);
  // check if device is WifiNetDevice
  std::string deviceName = device->GetInstanceTypeId ().GetName ();
  if (deviceName.compare ("ns3::LoRaWANNetDevice") != 0)
    {
      NS_FATAL_ERROR ("NetDevice type is not LoRaWANNetDevice!");
    }
  Ptr<Node> node = device->GetNode ();
  Ptr<LoRaWANRadioEnergyModel> model = m_radioEnergy.Create ()->GetObject<LoRaWANRadioEnergyModel> ();
  NS_ASSERT (model != NULL);
  // set energy source pointer
  model->SetEnergySource (source);
  // set energy depletion callback
  // if none is specified, make a callback to WifiPhy::SetSleepMode
  Ptr<LoRaWANNetDevice> LoRaWANDevice = DynamicCast<LoRaWANNetDevice> (device);
  Ptr<LoRaWANPhy> LoRaWANPhy = LoRaWANDevice->GetPhy ();



  //TODO: are these needed?
  /*if (m_depletionCallback.IsNull ())
    {
      model->SetEnergyDepletionCallback (MakeCallback (&WifiPhy::SetSleepMode, wifiPhy));
    }
  else
    {
      model->SetEnergyDepletionCallback (m_depletionCallback);
    }
  // set energy recharged callback
  // if none is specified, make a callback to WifiPhy::ResumeFromSleep
  if (m_rechargedCallback.IsNull ())
    {
      model->SetEnergyRechargedCallback (MakeCallback (&WifiPhy::ResumeFromSleep, wifiPhy));
    }
  else
    {
      model->SetEnergyRechargedCallback (m_rechargedCallback);
    }*/
  // add model to device model list in energy source
  source->AppendDeviceEnergyModel (model);
  

  // create and register energy model phy listener
  //wifiPhy->RegisterListener (model->GetPhyListener ());
  LoRaPhy -> TraceConnectWithoutContext ("TrxState",MakeCallback(&LoRaRadioEnergyModel::ChangeLoRaState, model));
  LoRaPhy -> TraceConnectWithoutContext ("TxPower",MakeCallback(&LoRaRadioEnergyModel::SetTxCurrentA, model));


  if (m_currentModel.GetTypeId ().GetUid ())
    {
      Ptr<LoRaWANCurrentModel> current = m_currentModel.Create<LoRaWANCurrentModel> ();
      model->SetCurrentModel (current);
    }
  return model;
}

} // namespace ns3
