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

#ifndef LORAWAN_CURRENT_MODEL_H
#define LORAWAN_CURRENT_MODEL_H

#include "ns3/object.h"

namespace ns3 {

#define SX1272_TX_OPTIONS_PA_BOOST 19
#define SX1272_TX_OPTIONS_RFO 16

/**
 * \ingroup energy
 * 
 * \brief Modelize the transmit current as a function of the transmit power and mode
 *
 */
class LoRaWANCurrentModel : public Object
{
public:
  static TypeId GetTypeId (void);

  LoRaWANCurrentModel ();
  virtual ~LoRaWANCurrentModel ();

  /**
   * \param txPowerDbm the nominal tx power in dBm
   * \returns the transmit current (in Ampere)
   */
  virtual double CalcTxCurrent (double txPowerDbm) const = 0;

  /**
   * Convert from dBm to Watts.
   *
   * \param dbm the power in dBm
   * \return the equivalent Watts for the given dBm
   */
  static double DbmToW (double dbm);
};

/**
 * \ingroup energy
 *
 * \brief a linear model of the Wifi transmit current
 *
 * This model assumes that the transmit current is a linear function
 * of the nominal transmit power used to send the frame.
 * In particular, the power absorbed during the transmission of a frame \f$ W_{tx} \f$
 * is given by the power absorbed by the power amplifier \f$ W_{pa} \f$ plus the power
 * absorbed by the RF subsystem. The latter is assumed to be the same as the power
 * absorbed in the IDLE state \f$ W_{idle} \f$.
 * 
 * The efficiency \f$ \eta \f$ of the power amplifier is given by 
 * \f$ \eta = \frac{P_{tx}}{W_{pa}} \f$, where \f$ P_{tx} \f$ is the output power, i.e.,
 * the nominal transmit power. Hence, \f$ W_{pa} = \frac{P_{tx}}{\eta} \f$
 * 
 * It turns out that \f$ W_{tx} = \frac{P_{tx}}{\eta} + W_{idle} \f$. By dividing both
 * sides by the supply voltage \f$ V \f$: \f$ I_{tx} = \frac{P_{tx}}{V \cdot \eta} + I_{idle} \f$,
 * where \f$ I_{tx} \f$ and \f$ I_{idle} \f$ are, respectively, the transmit current and
 * the idle current.
 * 
 * For more information, refer to:
 * Francesco Ivan Di Piazza, Stefano Mangione, and Ilenia Tinnirello.
 * "On the Effects of Transmit Power Control on the Energy Consumption of WiFi Network Cards",
 * Proceedings of ICST QShine 2009, pp. 463--475
 * 
 * If the tx current corresponding to a given nominal transmit power is known, the efficiency
 * of the power amplifier is given by the above formula:
 * \f$ \eta = \frac{P_{tx}}{(I_{tx}-I_{idle})\cdot V} \f$
 * 
 */

/**
 * TODO: explain SX1272
 */

class SX1272LoRaWANCurrentModel : public LoRaWANCurrentModel
{
public:
  static TypeId GetTypeId (void);

  SX1272LoRaWANCurrentModel ();
  virtual ~SX1272LoRaWANCurrentModel ();
  
  /**
   * \param eta (dimension-less)
   *
   * Set the power amplifier efficiency.
   */
  void SetEta (double eta);

  /**
   * \param voltage (Volts)
   *
   * Set the supply voltage.
   */
  void SetVoltage (double voltage);

  /**
   * \param idleCurrent (Ampere)
   *
   * Set the current in the IDLE state.
   */
  void SetIdleCurrent (double idleCurrent);

  /**
   * \return the power amplifier efficiency.
   */
  double GetEta (void) const;

  /**
   * \return the supply voltage.
   */
  double GetVoltage (void) const;

  /**
   * \return the current in the IDLE state.
   */
  double GetIdleCurrent (void) const;

  double CalcTxCurrent (double txPowerDbm) const;

  bool GetPaBoost (void) const;
  
  void SetPaBoost (double paBoost);

private:
  double m_eta;
  double m_voltage;
  double m_idleCurrent;

  double m_txPowerUsePaBoost[SX1272_TX_OPTIONS_PA_BOOST];
  double m_txPowerUseRfo[SX1272_TX_OPTIONS_RFO];
};

} // namespace ns3

#endif /* LORAWAN_CURRENT_MODEL_H */
