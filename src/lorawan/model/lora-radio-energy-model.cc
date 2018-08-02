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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 *         Peishuo Li <pressthunder@gmail.com>
 *				 Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "ns3/energy-source.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/lora-radio-energy-model.h"
#include "ns3/lorawan-phy.h"

NS_LOG_COMPONENT_DEFINE ("LoRaRadioEnergyModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoRaRadioEnergyModel);

TypeId
LoRaRadioEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaRadioEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .AddConstructor<LoRaRadioEnergyModel> ()
    .AddAttribute ("StandbyCurrentA",
                   "The default radio standby current in Ampere.",
                   DoubleValue (0.0015),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetStandbyCurrentA,
                                       &LoRaRadioEnergyModel::GetStandbyCurrentA),
                   MakeDoubleChecker<double> ())
    /*.AddAttribute ("TxCurrentA", //TODO: this depends on the tx power level chosen - is this modeled in the phy layer? Check. Using +13 from SX1272 for now
                   "The radio Tx, in dBm.",
                   DoubleValue (2),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetTxCurrentA,
                                       &LoRaRadioEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())*/
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current.",
                   DoubleValue (0.0105),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetRxCurrentA,
                                       &LoRaRadioEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SleepCurrentA",
                   "The radio Sleep current.",
                   DoubleValue (0.0000005),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetSleepCurrentA,
                                       &LoRaRadioEnergyModel::GetSleepCurrentA),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoRaRadioEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValue::DoubleCallback")
    .AddTraceSource ("CurrentEnergyState",
                     "Current Phy layer state and corresponding energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoRaRadioEnergyModel::m_EnergyStateLogger),
                     "ns3::LoRaRadioEnergyModel::EnergyStateTracedCallback")
  ; 
  return tid;
}

LoRaRadioEnergyModel::LoRaRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  //m_energyDepletionCallback.Nullify ();
  m_source = NULL;
  m_currentState = LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF;
  m_sourceEnergyUnlimited = 0;
  m_remainingBatteryEnergy = 0;
  m_sourcedepleted = 0;

  m_usePaBoost = true;

  //note there are also a bit lower than expected - going to repeat with second device
  //how much after decimal points figures to use?
  m_txPowerUsePaBoost[0] = 0.0327329143; //2dBm
  m_txPowerUsePaBoost[1] = 0.0339984; //3dBm
  m_txPowerUsePaBoost[2] = 0.0352840571; //4dBm
  m_txPowerUsePaBoost[3] = 0.0362664857; //5dBm
  m_txPowerUsePaBoost[4] = 0.0377083936; //6dBm
  m_txPowerUsePaBoost[5] = 0.0391588639; //7dBm
  m_txPowerUsePaBoost[6] = 0.0407067311; //8dBm
  m_txPowerUsePaBoost[7] = 0.0422184891; //9dBm
  m_txPowerUsePaBoost[8] = 0.0443943; //10dBm
  m_txPowerUsePaBoost[9] = 0.0468126286; //11dBm
  m_txPowerUsePaBoost[10] = 0.0496468683; //12dBm
  m_txPowerUsePaBoost[11] = 0.0526335714; //13dBm
  m_txPowerUsePaBoost[12] = 0.0567028; //14dBm
  m_txPowerUsePaBoost[13] = 0.0634942286; //15dBm
  m_txPowerUsePaBoost[14] = 0.0718378413; //16dBm
  m_txPowerUsePaBoost[15] = 0.0803444; //17dBm

  m_txPowerUsePaBoost[16] = 0.086461646; //18dBm
  m_txPowerUsePaBoost[17] = 0.0959928444; //19dBm
  m_txPowerUsePaBoost[18] = 0.1070608683; //20dBm

  m_txPowerUseRfo[0] = 0;

}

LoRaRadioEnergyModel::~LoRaRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

void
LoRaRadioEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
  m_energyToDecrease = 0;
  m_remainingBatteryEnergy = m_source->GetInitialEnergy();
}

double
LoRaRadioEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

double
LoRaRadioEnergyModel::GetStandbyCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_StandbyCurrentA;
}

void
LoRaRadioEnergyModel::SetStandbyCurrentA (double standbyCurrentA)
{
  NS_LOG_FUNCTION (this << standbyCurrentA);
  m_StandbyCurrentA = standbyCurrentA;
}

double
LoRaRadioEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TxCurrentA;
}

void
LoRaRadioEnergyModel::SetPaBoost (double paBoost)
{
  NS_LOG_FUNCTION (this << paBoost);
  m_usePaBoost = paBoost; // todo: is this enough?
}

bool
LoRaRadioEnergyModel::GetPaBoost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_usePaBoost;
}

void
LoRaRadioEnergyModel::SetTxCurrentA (double oldTxCurrent_dBm, double newTxCurrent_dBm)
{
  NS_LOG_FUNCTION (this << newTxCurrent_dBm);

  /*
  TODO:
  the values used here will be gained from measurements of the current consumption for every possible TX power option on the SX1272.
  For now, just adding in the values specified in the datasheet, which are: 
  */

  //these are the ones from the datasheet
  /*switch (newTxCurrent_dBm)
  {
    case 20:
      m_TxCurrentA = 0.125;
      break;
    case 17:
      m_TxCurrentA = 0.090;
      break;
    case 13:
      m_TxCurrentA = 0.028;
      break;
    case 7:
      m_TxCurrentA = 0.018;
      break;
    default:
      NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined tx current used: " << newTxCurrent_dBm);
  }*/

  /*Tx power is currently set prior to going into Tx mode, so there's no situation where Tx power changes during a send (and so no current calculations have to be done here)*/

  /*TODO: the max subband Tx power used in each channels (and since there is no ADR, the only Tx powers), are 14 and 27dBm.
    So the switch statement above will always fail

    The max EIRP for each subband, according to the ETSI regulations, are as follows:

    subband | spectrum access | edge frequencies | max eirp | equivalent dBm
    g       | 1% or LBT AFA   | 865-868MHz       | 10mW     | 10 dBm
    g1      | 1% or LBT AFA   | 868-868.6MHz     | 25mW     | 14 dBm
    g2      | 0.1% or LBT AFA | 868.7-869.2MHz   | 25mW     | 14 dBm
    g3      | 10% or LBT AFA  | 869.4-869.65MHz  | 500mW    | 27 dBm
    g4      | No Requirement  | 867-870MHz       | 5mW      | 7  dBm
    g4      | 1% or LBT AFA   | 867-870MHz       | 25mW     | 14 dBm

    The TXPower levels in LoRaWAN, which are used as part of the ADR, are defined as relative to the Max EIRP of the subband of the channel used for transmission, and are as follows:


    TXPower | Configuration (EIRP)
    0       | Max EIRP
    1       | Max EIRP - 2dB
    2       | Max EIRP - 4dB 
    3       | Max EIRP - 6dB
    4       | Max EIRP - 8dB
    5       | Max EIRP - 10dB
    6       | Max EIRP - 12dB
    7       | Max EIRP - 14dB
    8-14    | RFU (Reserved For Use)
    15      | Defined in LoRaWAN

    so, for example, the TXPower levels for a channel deployed in the g1 subband (e.g. 868.1, one of the mandetory ones) would be 14, 12, 10, 8, 6, 4, 2, 0.
    and the TXPower levels for a channel deployed in the g3 band would be 27, 25, 23, 21, 19, 17, 15, 13.
  
    The last one ("defined in LoRaWAN") is described in section 5.3 of the protocol: The LinkADRReq command is used by the NS to change the data rate and tx power of an end device.
    The requested data rate and tx power are encoded together in a single byte (4 bits each). A value of 15 specifies that the end device is to keep its current parameter value
     (i.e. a TXpower of 15 means stay at whatever TXpower the device is at already).  

    We base our energy calculations off of the SX1272, which can only reach a max TX power of 20dBm.
  */

  //m_TxCurrentA = txCurrentA;

  if(m_usePaBoost)
  {
    if(newTxCurrent_dBm < 2)
    {
      NS_LOG_LOGIC (this << " Chosen dBm of " << newTxCurrent_dBm << " is less than the sx1272 min of 2dBm, using 2dBm instead.");
      newTxCurrent_dBm = 2;
     
    }
    else if(newTxCurrent_dBm > 20)
    {
      NS_LOG_LOGIC (this << " Chosen dBm of " << newTxCurrent_dBm << " is more than the sx1272 max of 20dBm, using 20dBm instead.");
      newTxCurrent_dBm = 20; 
    }
    
    int ind = (int) newTxCurrent_dBm;
    ind -= 2;
    m_TxCurrentA = m_txPowerUsePaBoost[ind];  
    
  }
  else 
  {
    NS_FATAL_ERROR ("LoRaRadioEnergyModel:values for RFO not yet defined");
  }
}

double
LoRaRadioEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_RxCurrentA;
}

void
LoRaRadioEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_RxCurrentA = rxCurrentA;
}

double
LoRaRadioEnergyModel::GetSleepCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_SleepCurrentA;
}

void
LoRaRadioEnergyModel::SetSleepCurrentA (double sleepCurrentA)
{
  NS_LOG_FUNCTION (this << sleepCurrentA);
  m_SleepCurrentA = sleepCurrentA;
}


LoRaWANPhyEnumeration
LoRaRadioEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
LoRaRadioEnergyModel::HandleEnergyDepletion (){
}

void 
LoRaRadioEnergyModel::HandleEnergyRecharged (){
}

void
LoRaRadioEnergyModel::HandleEnergyChanged (){
}

void
LoRaRadioEnergyModel::ChangeLoRaState (LoRaWANPhyEnumeration oldstate, LoRaWANPhyEnumeration newstate)
{
  /*Change state identifiers here*/
  /*
  these are the LoRaWAN PHY states in this simulator.
  
  LoRaWANPhyEnumeration:

  LORAWAN_PHY_TRX_OFF = 0x00,
  LORAWAN_PHY_IDLE = 0x01,
  LORAWAN_PHY_RX_ON = 0x02,
  LORAWAN_PHY_TX_ON = 0x03,
  LORAWAN_PHY_BUSY_RX = 0x04,
  LORAWAN_PHY_BUSY_TX = 0x05,
  LORAWAN_PHY_SUCCESS = 0x06,
  LORAWAN_PHY_UNSPECIFIED = 0x07,
  LORAWAN_PHY_FORCE_TRX_OFF = 0x08,

  but SUCCESS and UNSPECIFIED are not used as states in the machine, just as return values by functions. And FORCE_TRX_OFF is only used by the Gateway, which we are not modelling.
  */


  NS_LOG_FUNCTION (this << newstate);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid

  // energy to decrease = current * voltage * time
      m_energyToDecrease = 0.0;
      double supplyVoltage = m_source->GetSupplyVoltage ();

      switch (m_currentState)
        {

					/*case LoRaPhy::State::IDLE:
          m_energyToDecrease = duration.GetSeconds () * m_IdleCurrentA * supplyVoltage;
          break;
        case LoRaPhy::State::TX:
          m_energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
          break;
        case LoRaPhy::State::RX:
          m_energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
          break;
				default:
          NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);*/

          case LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF:
            m_energyToDecrease = duration.GetSeconds () * m_SleepCurrentA * supplyVoltage;
            break;
          case LoRaWANPhyEnumeration::LORAWAN_PHY_IDLE:
            m_energyToDecrease = duration.GetSeconds () * m_StandbyCurrentA * supplyVoltage; //TODO: double-check use of this.
            break;
          case LoRaWANPhyEnumeration::LORAWAN_PHY_RX_ON:
            m_energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
            break;
          case LoRaWANPhyEnumeration::LORAWAN_PHY_TX_ON:
            m_energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
            break;
          case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_RX:
            m_energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
            break;
          case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_TX:
            m_energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
            break;
          default:
            NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);
        }

      // update total energy consumption
      m_totalEnergyConsumption += m_energyToDecrease;

      // update last update time stamp
      m_lastUpdateTime = Simulator::Now ();

      // notify energy source
      m_source->UpdateEnergySource ();

  if (!m_sourcedepleted)
    {
      SetLoRaRadioState (newstate);
      NS_LOG_DEBUG ("LoRaRadioEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }
}

 void
 LoRaRadioEnergyModel::ChangeState (int newState)
{
}

void
LoRaRadioEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
}


double
LoRaRadioEnergyModel::DoGetCurrentA (void) const
{
  /*
  TODO: update this
  */
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
      NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);

    /*case LoRaPhy::State::IDLE:
      return m_IdleCurrentA;
    case LoRaPhy::State::TX:
      return m_TxCurrentA;
    case LoRaPhy::State::RX:
      return m_RxCurrentA;
    default:
      NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state:" << m_currentState);*/
    }
}


void
LoRaRadioEnergyModel::SetLoRaRadioState (const LoRaWANPhyEnumeration state)
{
 
  /*
  TODO: update this
  TODO: add in other identified states
  */

  NS_LOG_FUNCTION (this << state);

  std::string preStateName;
  switch (m_currentState)
    {
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF:
      preStateName = "SLEEP";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_IDLE:
      preStateName = "STANDBY";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_RX_ON:
      preStateName = "RX_ON";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TX_ON:
      preStateName = "TX_ON";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_RX:
      preStateName = "RX_BUSY";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_TX:
      preStateName = "TX_BUSY";
      break;
    default:
      NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);
      
		/*case LoRaPhy::State::IDLE:
      preStateName = "IDLE";
      break;
    case LoRaPhy::State::TX:
      preStateName = "TX";
      break;
		case LoRaPhy::State::RX: 
      preStateName = "RX";
      break;
  default:
    NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);*/
  }

  m_currentState = state;
  std::string curStateName;
  switch (state)
    {
      case LoRaWANPhyEnumeration::LORAWAN_PHY_TRX_OFF:
      preStateName = "SLEEP";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_IDLE:
      preStateName = "STANDBY";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_RX_ON:
      preStateName = "RX_ON";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_TX_ON:
      preStateName = "TX_ON";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_RX:
      preStateName = "RX_BUSY";
      break;
    case LoRaWANPhyEnumeration::LORAWAN_PHY_BUSY_TX:
      preStateName = "TX_BUSY";
      break;
    default:
      NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);

    /*case LoRaPhy::State::IDLE:
      curStateName = "IDLE";
      break;
    case LoRaPhy::State::RX:
      curStateName = "RX";
      break;
    case LoRaPhy::State::TX:
      curStateName = "TX";
      break;
  default:
    NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);*/
  }

  m_remainingBatteryEnergy = m_source -> GetRemainingEnergy();

  //TODO: check this is used properly
  m_EnergyStateLogger (preStateName, curStateName, m_sourceEnergyUnlimited, m_energyToDecrease, m_remainingBatteryEnergy, m_totalEnergyConsumption);

  NS_LOG_DEBUG ("LoRaRadioEnergyModel:Switching to state: " << curStateName <<
                " at time = " << Simulator::Now ());
}


// -------------------------------------------------------------------------- //


/*
 * Private function state here.
 */



} // namespace ns3
