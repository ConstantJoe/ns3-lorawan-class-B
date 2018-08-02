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
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>,
 *          Peishuo Li <pressthunder@gmail.com>
 *					Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#ifndef LORA_RADIO_ENERGY_MODEL_H
#define LORA_RADIO_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/energy-source.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/lorawan-phy.h"
#include <ns3/traced-callback.h>

namespace ns3 {

/**
 * \ingroup energy
 * \brief A LoRa radio energy model.
 * 
 * 3 states are defined for the radio: TX,RX and IDLE.
 * Inherit from LoRaPhy::State
 * The different types of tansactirons that are defined are:
 *  1. TX: Transmitter is enabled.
 *  2. RX: Receiver is enabled.
 *  3. IDLE: Transceiver is disabled.
 * The class keeps track of what state the radio is currently in.
 *
 * Energy calculation: For each transaction, this model notifies EnergySource
 * object. The EnergySource object will query this model for the total current.
 * Then the EnergySource object uses the total current to calculate energy.
 */

#define SX1272_TX_OPTIONS_PA_BOOST 19
#define SX1272_TX_OPTIONS_RFO 16

class LoRaRadioEnergyModel : public DeviceEnergyModel
{
public:
  /**
   * Callback type for energy depletion handling.
   */
  typedef Callback<void> LoRaRadioEnergyDepletionCallback;

public:
  static TypeId GetTypeId (void);
  LoRaRadioEnergyModel ();
  virtual ~LoRaRadioEnergyModel ();

  /**
   * \brief Sets pointer to EnergySouce installed on node.
   *
   * \param source Pointer to EnergySource installed on node.
   *
   * Implements DeviceEnergyModel::SetEnergySource.
   */
  virtual void SetEnergySource (Ptr<EnergySource> source);

  /**
   * \returns Total energy consumption of the LoRa device.
   *
   * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
   */
  virtual double GetTotalEnergyConsumption (void) const;

  // Setter & getters for state power consumption.
  double GetStandbyCurrentA (void) const;
  void SetStandbyCurrentA (double standbyCurrentA);
  double GetRxCurrentA (void) const;
  void SetRxCurrentA (double rxCurrentA);
  double GetTxCurrentA (void) const;
  void SetTxCurrentA (double oldTxCurrent_dBm, double newTxCurrent_dBm);
  double GetSleepCurrentA (void) const;
  void SetSleepCurrentA (double sleepCurrentA);
  bool GetPaBoost (void) const;
  void SetPaBoost (double paBoost);


  /**
   * \returns Current state.
   */
	LoRaWANPhyEnumeration GetCurrentState (void) const;

  /**
   * \brief Inherit from DeviceEnergyModel, not used
   */
  virtual void ChangeState (int newState);

  /**
   * \brief Handles energy depletion.
   *
   * Implements DeviceEnergyModel::HandleEnergyDepletion
   */
  void HandleEnergyDepletion (void);


  /**
   * Implements DeviceEnergyModel::HandleEnergyRecharged
   */
  void HandleEnergyRecharged (void);

  /**
   * Implements DeviceEnergyModel::HandleEnergyChanged
   */
  void HandleEnergyChanged (void);

  /**
   * \brief Change state of the LoRaRadioEnergyMode
   */
  void ChangeLoRaState (LoRaWANPhyEnumeration oldstate, LoRaWANPhyEnumeration newstate);

private:
  /**
   * Implement real destruction code and chain up to the parent's implementation once done
   */
  void DoDispose (void);

  /**
   * \returns Current draw of device, at current state.
   *
   * Implements DeviceEnergyModel::GetCurrentA.
   */
  virtual double DoGetCurrentA (void) const;

  /**
   * \param state New state the radio device is currently in.
   *
   * Sets current state. This function is private so that only the energy model
   * can change its own state.
   */
  void SetLoRaRadioState (const LoRaWANPhyEnumeration state);

private:
  Ptr<EnergySource> m_source;

  // Member variables for current draw in different radio modes.
  double m_TxCurrentA;
  double m_RxCurrentA;
  double m_StandbyCurrentA;
  double m_SleepCurrentA;

  bool m_usePaBoost;

  // This variable keeps track of the total energy consumed by this model.
  TracedValue<double> m_totalEnergyConsumption;

  TracedCallback<std::string, std::string, bool, double, double, double> m_EnergyStateLogger;

  // State variables.
	LoRaWANPhyEnumeration m_currentState;  // current state the radio is in
  Time m_lastUpdateTime;                // time stamp of previous energy update
  double m_energyToDecrease;            // consumed energy of lastest LoRaRadioEnergyMode
  double m_remainingBatteryEnergy;      // remaining battery energy of the energy source attaching to the node
  bool m_sourceEnergyUnlimited;         // battery energy of the energy source attaching to the node unlimited or not
  bool m_sourcedepleted;                // battery energy of the energy source depleted or not

  double m_txPowerUsePaBoost[SX1272_TX_OPTIONS_PA_BOOST];
  double m_txPowerUseRfo[SX1272_TX_OPTIONS_RFO];

};

} // namespace ns3

#endif /* LORA_RADIO_ENERGY_MODEL_H */
