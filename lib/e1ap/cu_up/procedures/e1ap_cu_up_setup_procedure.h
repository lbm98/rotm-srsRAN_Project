/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "../../common/e1ap_asn1_utils.h"
#include "e1ap_cu_up_event_manager.h"
#include "srsran/e1ap/common/e1_setup_messages.h"
#include "srsran/e1ap/cu_up/e1ap_cu_up.h"
#include "srsran/f1ap/du/f1ap_du.h"
#include "srsran/support/async/async_task.h"

namespace srsran {
namespace srs_cu_up {

struct f1ap_du_context;

/// E1 Setup Procedure for the gNB-CU-UP as per TS 38.463, 8.2.3.
class e1ap_cu_up_setup_procedure
{
public:
  e1ap_cu_up_setup_procedure(const cu_up_e1_setup_request& request_,
                             e1ap_message_notifier&        cu_cp_notif_,
                             e1ap_event_manager&           ev_mng_,
                             timer_factory                 timers_);

  void operator()(coro_context<async_task<cu_up_e1_setup_response>>& ctx);

private:
  /// Send E1 SETUP REQUEST to CU-CP.
  void send_cu_up_e1_setup_request();

  /// Checks whether the CU-UP should attempt again to connect to CU-CP.
  bool retry_required();

  /// Creates procedure result to send back to procedure caller.
  cu_up_e1_setup_response create_e1_setup_result();

  srslog::basic_logger&        logger;
  const cu_up_e1_setup_request request;
  e1ap_message_notifier&       cu_cp_notifier;
  e1ap_event_manager&          ev_mng;
  timer_factory                timers;

  unique_timer e1_setup_wait_timer;

  e1ap_transaction     transaction;
  unsigned             e1_setup_retry_no = 0;
  std::chrono::seconds time_to_wait{0};
};

} // namespace srs_cu_up
} // namespace srsran
