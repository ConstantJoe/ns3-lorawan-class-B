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

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/energy-source.h"
#include "lorawan-radio-energy-model.h"
#include "lorawan-current-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaWANRadioEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (LoRaWANRadioEnergyModel);

TypeId
LoRaWANRadioEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaWANRadioEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .SetGroupName ("Energy")
    .AddConstructor<LoRaWANRadioEnergyModel> ()
    .AddAttribute ("IdleCurrentA",
                   "The default radio Idle current in Ampere.",
                   DoubleValue (0.273),  // idle mode = 273mA
                   MakeDoubleAccessor (&LoRaWANRadioEnergyModel::SetIdleCurrentA,
                                       &LoRaWANRadioEnergyModel::GetIdleCurrentA),
                   MakeDoubleChecker<double> ())
    /*.AddAttribute ("CcaBusyCurrentA",
                   "The default radio CCA Busy State current in Ampere.",
                   DoubleValue (0.273),  // default to be the same as idle mode
                   MakeDoubleAccessor (&WifiRadioEnergyModel::SetCcaBusyCurrentA,
                                       &WifiRadioEnergyModel::GetCcaBusyCurrentA),
                   MakeDoubleChecker<double> ())*/
    .AddAttribute ("TxCurrentA",
                   "The radio Tx current in Ampere.",
                   DoubleValue (0.380),    // transmit at 0dBm = 380mA
                   MakeDoubleAccessor (&LoRaWANRadioEnergyModel::SetTxCurrentA,
                                       &LoRaWANRadioEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current in Ampere.",
                   DoubleValue (0.313),    // receive mode = 313mA
                   MakeDoubleAccessor (&LoRaWANRadioEnergyModel::SetRxCurrentA,
                                       &LoRaWANRadioEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    /*.AddAttribute ("SwitchingCurrentA",
                   "The default radio Channel Switch current in Ampere.",
                   DoubleValue (0.273),  // default to be the same as idle mode
                   MakeDoubleAccessor (&WifiRadioEnergyModel::SetSwitchingCurrentA,
                                       &WifiRadioEnergyModel::GetSwitchingCurrentA),
                   MakeDoubleChecker<double> ())*/
    .AddAttribute ("SleepCurrentA",
                   "The radio Sleep current in Ampere.",
                   DoubleValue (0.033),  // sleep mode = 33mA
                   MakeDoubleAccessor (&LoRaWANRadioEnergyModel::SetSleepCurrentA,
                                       &LoRaWANRadioEnergyModel::GetSleepCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentModel", "A pointer to the attached tx current model.",
                   PointerValue (),
                   MakePointerAccessor (&LoRaWANRadioEnergyModel::m_txCurrentModel),
                   MakePointerChecker<LoRaWANCurrentModel> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoRaWANRadioEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValueCallback::Double")
  ; 
  return tid;
}

LoRaWANRadioEnergyModel::LoRaWANRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_currentState = WifiPhy::IDLE;  // initially IDLE
  m_lastUpdateTime = Seconds (0.0);
  m_nPendingChangeState = 0; //needed?
  m_isSupersededChangeState = false; //needed?
  m_energyDepletionCallback.Nullify ();
  m_source = NULL;
  
  //this is what links the SetTxCurrentFromModel to the Tx power. 
  //so the listener is needed.

  // set callback for WifiPhy listener
  //m_listener = new WifiRadioEnergyModelPhyListener;
  //m_listener->SetChangeStateCallback (MakeCallback (&DeviceEnergyModel::ChangeState, this));
  // set callback for updating the tx current
  //m_listener->SetUpdateTxCurrentCallback (MakeCallback (&WifiRadioEnergyModel::SetTxCurrentFromModel, this));
}

LoRaWANRadioEnergyModel::~LoRaWANRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  //delete m_listener;
}

void
LoRaWANRadioEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

double
LoRaWANRadioEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

double
LoRaWANRadioEnergyModel::GetIdleCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_idleCurrentA;
}

void
LoRaWANRadioEnergyModel::SetIdleCurrentA (double idleCurrentA)
{
  NS_LOG_FUNCTION (this << idleCurrentA);
  m_idleCurrentA = idleCurrentA;
}

/*double
WifiRadioEnergyModel::GetCcaBusyCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ccaBusyCurrentA;
}

void
WifiRadioEnergyModel::SetCcaBusyCurrentA (double CcaBusyCurrentA)
{
  NS_LOG_FUNCTION (this << CcaBusyCurrentA);
  m_ccaBusyCurrentA = CcaBusyCurrentA;
}*/

double
LoRaWANRadioEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txCurrentA;
}

void
LoRaWANRadioEnergyModel::SetTxCurrentA (double txCurrentDbm)
{
  NS_LOG_FUNCTION (this << txCurrentDbm);
  //m_txCurrentA = txCurrentA;
  SetTxCurrentFromModel(txCurrentDbm);
}

double
LoRaWANRadioEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxCurrentA;
}

void
LoRaWANRadioEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_rxCurrentA = rxCurrentA;
}

/*double
WifiRadioEnergyModel::GetSwitchingCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_switchingCurrentA;
}

void
WifiRadioEnergyModel::SetSwitchingCurrentA (double switchingCurrentA)
{
  NS_LOG_FUNCTION (this << switchingCurrentA);
  m_switchingCurrentA = switchingCurrentA;
}*/

double
LoRaWANRadioEnergyModel::GetSleepCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sleepCurrentA;
}

void
LoRaWANRadioEnergyModel::SetSleepCurrentA (double sleepCurrentA)
{
  NS_LOG_FUNCTION (this << sleepCurrentA);
  m_sleepCurrentA = sleepCurrentA;
}

LoRaWANPhyEnumeration
LoRaWANRadioEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
LoRaWANRadioEnergyModel::SetEnergyDepletionCallback (
  LoRaWANRadioEnergyDepletionCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Setting NULL energy depletion callback!");
    }
  m_energyDepletionCallback = callback;
}

void
LoRaWANRadioEnergyModel::SetEnergyRechargedCallback (
  LoRaWANRadioEnergyRechargedCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Setting NULL energy recharged callback!");
    }
  m_energyRechargedCallback = callback;
}

void
LoRaWANRadioEnergyModel::SetTxCurrentModel (Ptr<WifiTxCurrentModel> model)
{
  m_txCurrentModel = model;
}

void
LoRaWANRadioEnergyModel::SetTxCurrentFromModel (double txPowerDbm)
{
  if (m_currentModel)
    {
      m_txCurrentA = m_currentModel->CalcTxCurrent (txPowerDbm);
    }
}

void
LoRaWANRadioEnergyModel::SetRxCurrentFromModel (bool x)
{
  if (m_currentModel)
    {
      m_rxCurrentA = m_currentModel->CalcRxCurrent (x);
    }
}


void
LoRaWANRadioEnergyModel::ChangeState (int newState)
{
  NS_LOG_FUNCTION (this << newState);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();

  switch (m_currentState)
    {
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF:
      energyToDecrease = duration.GetSeconds () * m_SleepCurrentA * supplyVoltage;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_IDLE:
      energyToDecrease = duration.GetSeconds () * m_StandbyCurrentA * supplyVoltage; //TODO: double-check use of this.
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_RX_ON:
      energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TX_ON:
      energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_RX:
      energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_TX:
      energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
      break;
    default:
      NS_FATAL_ERROR ("LoRaWANRadioEnergyModel:Undefined radio state: " << m_currentState);
    }

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();

  //m_nPendingChangeState++;

  // notify energy source
  m_source->UpdateEnergySource ();


  SetLoRaWANRadioState ((LoRaWANPhyEnumeration) newState);

  NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Total energy consumption is " << m_totalEnergyConsumption << "J");

  /*
  // in case the energy source is found to be depleted during the last update, a callback might be
  // invoked that might cause a change in the Wifi PHY state (e.g., the PHY is put into SLEEP mode).
  // This in turn causes a new call to this member function, with the consequence that the previous
  // instance is resumed after the termination of the new instance. In particular, the state set
  // by the previous instance is erroneously the final state stored in m_currentState. The check below
  // ensures that previous instances do not change m_currentState.

  if (!m_isSupersededChangeState)
    {
      // update current state & last update time stamp
      SetLoRaWANRadioState ((LoRaWANPhyEnumeration) newState);

      // some debug message
      NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }

  m_isSupersededChangeState = (m_nPendingChangeState > 1);

  m_nPendingChangeState--;*/
}

void
LoRaWANRadioEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Energy is depleted!");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      m_energyDepletionCallback ();
    }
}

void
LoRaWANRadioEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Energy is recharged!");
  // invoke energy recharged callback, if set.
  if (!m_energyRechargedCallback.IsNull ())
    {
      m_energyRechargedCallback ();
    }
}

/*WifiRadioEnergyModelPhyListener *
WifiRadioEnergyModel::GetPhyListener (void)
{
  NS_LOG_FUNCTION (this);
  return m_listener;
}*/

/*
 * Private functions start here.
 */

void
LoRaWANRadioEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
  m_energyDepletionCallback.Nullify ();
}

double
LoRaWANRadioEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);

   switch (m_currentState)
    {
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF:
      return m_SleepCurrentA;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_IDLE:
      return m_StandbyCurrentA;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_RX_ON:
      return m_RxCurrentA;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TX_ON:
      return m_TxCurrentA;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_RX:
      return m_RxCurrentA;
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_TX:
      return m_TxCurrentA;
      break;
    default:
      NS_FATAL_ERROR ("LoRaWANRadioEnergyModel:Undefined radio state: " << m_currentState);
    }
}

void
LoRaWANRadioEnergyModel::SetLoRaWANRadioState (const LoRaWANPhyEnumeration state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;

  switch (m_currentState)
    {
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF:
      stateName = "SLEEP";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_IDLE:
      stateName = "STANDBY";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_RX_ON:
      stateName = "RX_ON";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TX_ON:
      stateName = "TX_ON";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_RX:
      stateName = "RX_BUSY";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_TX:
      stateName = "TX_BUSY";
      break;
    default:
      NS_FATAL_ERROR ("LoRaWANRadioEnergyModel:Undefined radio state: " << m_currentState);
    }
  NS_LOG_DEBUG ("LoRaWANRadioEnergyModel:Switching to state: " << stateName <<
                " at time = " << Simulator::Now ());
}

} // namespace ns3
