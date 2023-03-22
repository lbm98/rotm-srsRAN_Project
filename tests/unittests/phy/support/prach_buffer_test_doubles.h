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

#include "../../../lib/phy/support/prach_buffer_impl.h"
#include "srsran/adt/tensor.h"
#include "srsran/phy/support/support_factories.h"
#include "srsran/srsvec/copy.h"
#include "srsran/support/error_handling.h"

namespace srsran {

class prach_buffer_spy : public prach_buffer
{
public:
  struct entry_t {
    unsigned i_port;
    unsigned i_td_occasion;
    unsigned i_fd_occasion;
    unsigned i_symbol;
  };

  prach_buffer_spy() : buffer(0, 0, 0, 0, 0) {}

  prach_buffer_spy(span<cf_t> data_,
                   unsigned   nof_td_occasions,
                   unsigned   nof_fd_occasions,
                   unsigned   nof_symbols,
                   unsigned   sequence_length) :
    buffer(1, nof_td_occasions, nof_fd_occasions, nof_symbols, sequence_length)
  {
    span<const cf_t> data      = data_;
    unsigned         nof_ports = get_max_nof_ports();

    report_fatal_error_if_not(data.size() == nof_td_occasions * nof_fd_occasions * sequence_length,
                              "The symbols data size is not consistent with the symbol size and number of symbols.");

    for (unsigned i_td_occasion = 0; i_td_occasion != nof_td_occasions; ++i_td_occasion) {
      for (unsigned i_fd_occasion = 0; i_fd_occasion != nof_fd_occasions; ++i_fd_occasion) {
        span<const cf_t> occasion_data = data.first(sequence_length);
        data                           = data.last(data.size() - sequence_length);
        for (unsigned i_port = 0; i_port != nof_ports; ++i_port) {
          for (unsigned i_symbol = 0; i_symbol != nof_symbols; ++i_symbol) {
            srsvec::copy(buffer.get_symbol(i_port, i_td_occasion, i_fd_occasion, i_symbol), occasion_data);
          }
        }
      }
    }
  }

  unsigned get_max_nof_ports() const override
  {
    ++count_get_max_nof_ports;
    return buffer.get_max_nof_ports();
  }

  unsigned get_max_nof_td_occasions() const override
  {
    ++count_get_max_nof_td_occasions;
    return buffer.get_max_nof_td_occasions();
  }

  unsigned get_max_nof_fd_occasions() const override
  {
    ++count_get_max_nof_fd_occasions;
    return buffer.get_max_nof_fd_occasions();
  }

  unsigned get_max_nof_symbols() const override
  {
    ++count_get_max_nof_symbols;
    return buffer.get_max_nof_symbols();
  }

  unsigned get_sequence_length() const override
  {
    ++count_get_sequence_length;
    return buffer.get_sequence_length();
  }

  span<cf_t> get_symbol(unsigned i_port, unsigned i_td_occasion, unsigned i_fd_occasion, unsigned i_symbol) override
  {
    get_symbol_entries.emplace_back();
    entry_t& entry      = get_symbol_entries.back();
    entry.i_port        = i_port;
    entry.i_td_occasion = i_td_occasion;
    entry.i_fd_occasion = i_fd_occasion;
    entry.i_symbol      = i_symbol;
    return buffer.get_symbol(i_port, i_td_occasion, i_fd_occasion, i_symbol);
  }

  span<const cf_t>
  get_symbol(unsigned i_port, unsigned i_td_occasion, unsigned i_fd_occasion, unsigned i_symbol) const override
  {
    get_symbol_const_entries.emplace_back();
    entry_t& entry      = get_symbol_const_entries.back();
    entry.i_port        = i_port;
    entry.i_td_occasion = i_td_occasion;
    entry.i_fd_occasion = i_fd_occasion;
    entry.i_symbol      = i_symbol;
    return buffer.get_symbol(i_port, i_td_occasion, i_fd_occasion, i_symbol);
  }

  unsigned get_total_count() const
  {
    return count_get_max_nof_ports + count_get_max_nof_td_occasions + count_get_max_nof_fd_occasions +
           count_get_max_nof_symbols + count_get_sequence_length + get_symbol_entries.size() +
           get_symbol_const_entries.size();
  }

  const std::vector<entry_t>& get_get_symbol_entries() { return get_symbol_entries; };
  const std::vector<entry_t>& get_get_symbol_const_entries() { return get_symbol_const_entries; };

  void clear()
  {
    count_get_max_nof_ports        = 0;
    count_get_max_nof_td_occasions = 0;
    count_get_max_nof_fd_occasions = 0;
    count_get_max_nof_symbols      = 0;
    count_get_sequence_length      = 0;
  }

private:
  /// Underlying buffer.
  prach_buffer_impl buffer;

  mutable unsigned             count_get_max_nof_ports        = 0;
  mutable unsigned             count_get_max_nof_td_occasions = 0;
  mutable unsigned             count_get_max_nof_fd_occasions = 0;
  mutable unsigned             count_get_max_nof_symbols      = 0;
  mutable unsigned             count_get_sequence_length      = 0;
  std::vector<entry_t>         get_symbol_entries;
  mutable std::vector<entry_t> get_symbol_const_entries;
};

} // namespace srsran
