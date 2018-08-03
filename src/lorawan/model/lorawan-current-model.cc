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

#include "lora-sx1272-current-model.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaWANCurrentModel");

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (LoRaWANSX1272CurrentModel);

TypeId 
LoRaWANCurrentModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaWANCurrentModel")
    .SetParent<Object> ()
    .SetGroupName ("LoRaWAN")
  ;
  return tid;
}

LoRaWANCurrentModel::LoRaWANCurrentModel()
{
}

LoRaWANCurrentModel::~LoRaWANCurrentModel()
{
}

double
LoRaWANCurrentModel::DbmToW (double dbm)
{
  double mW = std::pow (10.0, dbm / 10.0);
  return mW / 1000.0;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (SX1272LoRaWANCurrentModel);

TypeId 
SX1272LoRaWANCurrentModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SX1272LoRaWANCurrentModel")
    .SetParent<LoRaWANCurrentModel> ()
    .SetGroupName ("LoRaWAN")
    .AddConstructor<SX1272LoRaWANCurrentModel> ()
    .AddAttribute ("Eta", "The efficiency of the power amplifier.",
                   DoubleValue (0.10), //to change
                   MakeDoubleAccessor (&SX1272LoRaWANCurrentModel::SetEta,
                                       &SX1272LoRaWANCurrentModel::GetEta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Voltage", "The supply voltage (in Volts).",
                   DoubleValue (3.0), //to change
                   MakeDoubleAccessor (&SX1272LoRaWANCurrentModel::SetVoltage,
                                       &SX1272LoRaWANCurrentModel::GetVoltage),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("IdleCurrent", "The current in the IDLE state (in Watts).",
                   DoubleValue (0.273333), //to change
                   MakeDoubleAccessor (&SX1272LoRaWANCurrentModel::SetIdleCurrent,
                                       &SX1272LoRaWANCurrentModel::GetIdleCurrent),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

SX1272LoRaWANCurrentModel::SX1272LoRaWANCurrentModel ()
{
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

SX1272LoRaWANCurrentModel::~SX1272LoRaWANCurrentModel()
{
}

/*void
SX1272LoRaWANCurrentModel::SetEta (double eta)
{
  m_eta = eta;
}*/

void
SX1272LoRaWANCurrentModel::SetVoltage (double voltage)
{
  m_voltage = voltage;
}

void
SX1272LoRaWANCurrentModel::SetIdleCurrent (double idleCurrent)
{
  m_idleCurrent = idleCurrent;
}

/*double
SX1272LoRaWANCurrentModel::GetEta (void) const
{
  return m_eta;
}*/

double
SX1272LoRaWANCurrentModel::GetVoltage (void) const
{
  return m_voltage;
}

double
SX1272LoRaWANCurrentModel::GetIdleCurrent (void) const
{
  return m_idleCurrent;
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

double
SX1272LoRaWANCurrentModel::CalcTxCurrent (double txPowerDbm) const
{
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


  //return DbmToW (txPowerDbm) / (m_voltage * m_eta) + m_idleCurrent;

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
    return m_txPowerUsePaBoost[ind];  
    
  }
  else 
  {
    NS_FATAL_ERROR ("LoRaRadioEnergyModel:values for RFO not yet defined");
  }
}

// ------------------------------------------------------------------------- //

} // namespace ns3
