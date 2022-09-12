/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/adt/byte_buffer.h"
#include "srsgnb/adt/byte_buffer_slice_chain.h"
#include "srsgnb/ran/du_types.h"
#include "srsgnb/ran/lcid.h"
#include "srsgnb/rlc/rlc_config_messages.h"

/*
 * This file will hold the interfaces and notifiers for the RLC entity.
 * They follow the following nomenclature:
 *
 *   rlc_{tx/rx}_{lower/upper}_layer_{[control/data]}_{interface/notifier}
 *
 * 1. TX/RX indicates whether the interface is intended for the
 *    TX or RX side of the entity
 * 2. Lower/Upper indicates whether the interface/notifier interacts
 *    with the upper or lower layers.
 * 3. Control/Data: indicates whether this interface is necessary for "control"
 *    purposes (e.g., notifying the RRC of max retransmissions, or that we are
 *    near max HFN) or "data" purposes (e.g. handling SDUs).
 *    This distinction is only necessary when interfacing with the upper layers,
 *    and as such, we omit it in the interfaces with the lower layers.
 * 4. Interface/Notifier: whether this is an interface the RLC entity will
 *    inherit or a notifier that the RLC will keep as a member.
 *
 */
namespace srsgnb {

/****************************************
 * Interfaces/notifiers for upper layers
 ****************************************/
/// Structure used to represent an RLC SDU. An RLC SDU
/// must be accompanied with the corresponding PDCP COUNT
/// so that RLC AM can notify the PDCP of ACKs
struct rlc_sdu {
  uint32_t    pdcp_count = 0;
  byte_buffer buf        = {};
  rlc_sdu()              = default;
  rlc_sdu(uint32_t pdcp_count, byte_buffer buf) : pdcp_count(pdcp_count), buf(std::move(buf)) {}
};

/// This interface represents the data entry point of the transmitting side of a RLC entity.
/// The upper-layers will use this call to pass RLC SDUs into the TX entity.
class rlc_tx_upper_layer_data_interface
{
public:
  rlc_tx_upper_layer_data_interface()                                                     = default;
  virtual ~rlc_tx_upper_layer_data_interface()                                            = default;
  rlc_tx_upper_layer_data_interface(const rlc_tx_upper_layer_data_interface&)             = delete;
  rlc_tx_upper_layer_data_interface& operator=(const rlc_tx_upper_layer_data_interface&)  = delete;
  rlc_tx_upper_layer_data_interface(const rlc_tx_upper_layer_data_interface&&)            = delete;
  rlc_tx_upper_layer_data_interface& operator=(const rlc_tx_upper_layer_data_interface&&) = delete;

  /// \brief Interface for higher layers to pass SDUs into RLC
  /// \param sdu SDU to be handled
  virtual void handle_sdu(rlc_sdu sdu) = 0;

  // TODO: virtual bool discard_sdu(uint32_t pdcp_count) = 0;
};

/// This interface represents the data upper layer that the RLC bearer must notify on successful delivery of an SDU, so
/// it can stop its discard timer. For RLC AM this is on reception of a corresponding ACK; for RLC UM this is the moment
/// the transmission of the SDU begins.
class rlc_tx_upper_layer_data_notifier
{
public:
  virtual ~rlc_tx_upper_layer_data_notifier() = default;

  virtual void on_delivered_sdu(uint32_t pdcp_count) = 0;
};

/// This interface represents the control upper layer that the
/// TX RLC bearer must notify in case of protocol errors,
/// or, in the case of AM bearers, maximum retransmissions reached.
class rlc_tx_upper_layer_control_notifier
{
public:
  rlc_tx_upper_layer_control_notifier()                                                       = default;
  virtual ~rlc_tx_upper_layer_control_notifier()                                              = default;
  rlc_tx_upper_layer_control_notifier(const rlc_tx_upper_layer_control_notifier&)             = delete;
  rlc_tx_upper_layer_control_notifier& operator=(const rlc_tx_upper_layer_control_notifier&)  = delete;
  rlc_tx_upper_layer_control_notifier(const rlc_tx_upper_layer_control_notifier&&)            = delete;
  rlc_tx_upper_layer_control_notifier& operator=(const rlc_tx_upper_layer_control_notifier&&) = delete;

  virtual void on_protocol_failure() = 0;
  virtual void on_max_retx()         = 0;
};

/***************************************
 * Interfaces/notifiers for lower layers
 ***************************************/
/// This interface represents the data exit point of the transmitting side of a RLC entity.
/// The lower layers will use this interface to pull a PDU from the RLC, or to
/// query the current buffer state of the RLC bearer.
class rlc_tx_lower_layer_interface
{
public:
  rlc_tx_lower_layer_interface()                                                = default;
  virtual ~rlc_tx_lower_layer_interface()                                       = default;
  rlc_tx_lower_layer_interface(const rlc_tx_lower_layer_interface&)             = delete;
  rlc_tx_lower_layer_interface& operator=(const rlc_tx_lower_layer_interface&)  = delete;
  rlc_tx_lower_layer_interface(const rlc_tx_lower_layer_interface&&)            = delete;
  rlc_tx_lower_layer_interface& operator=(const rlc_tx_lower_layer_interface&&) = delete;

  /// \brief Pulls a PDU from the lower end of the RLC TX entity
  /// An empty PDU is returned if nof_bytes is insufficient or the TX buffer is empty.
  /// \param nof_bytes Limits the maximum size of the requested PDU.
  /// \return One PDU
  virtual byte_buffer_slice_chain pull_pdu(uint32_t nof_bytes) = 0;

  /// \brief Get the buffer status information
  /// This function provides the current buffer state of the RLC TX entity.
  /// This is the gross total size required to fully flush the TX entity (potentially by multiple calls to pull_pdu).
  /// \return Provides the current buffer state
  virtual uint32_t get_buffer_state() = 0;
};

class rlc_tx_lower_layer_notifier
{
public:
  rlc_tx_lower_layer_notifier()                                               = default;
  virtual ~rlc_tx_lower_layer_notifier()                                      = default;
  rlc_tx_lower_layer_notifier(const rlc_tx_lower_layer_notifier&)             = delete;
  rlc_tx_lower_layer_notifier& operator=(const rlc_tx_lower_layer_notifier&)  = delete;
  rlc_tx_lower_layer_notifier(const rlc_tx_lower_layer_notifier&&)            = delete;
  rlc_tx_lower_layer_notifier& operator=(const rlc_tx_lower_layer_notifier&&) = delete;

  /// \brief Method called by RLC bearer whenever its buffer state is updated and the respective result
  /// needs to be forwarded to lower layers.
  virtual void on_buffer_state_update(unsigned bsr) = 0;
};
} // namespace srsgnb
