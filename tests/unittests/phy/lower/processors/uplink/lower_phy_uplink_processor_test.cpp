/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "prach/prach_processor_notifier_test_doubles.h"
#include "prach/prach_processor_test_doubles.h"
#include "puxch/puxch_processor_notifier_test_doubles.h"
#include "puxch/puxch_processor_test_doubles.h"
#include "uplink_processor_notifier_test_doubles.h"
#include "srsran/gateways/baseband/baseband_gateway_buffer.h"
#include "srsran/phy/lower/processors/uplink/uplink_processor_baseband.h"
#include "srsran/phy/lower/processors/uplink/uplink_processor_factories.h"
#include "fmt/ostream.h"
#include <gtest/gtest.h>
#include <random>

using namespace srsran;

namespace srsran {

std::ostream& operator<<(std::ostream& os, span<const cf_t> data)
{
  fmt::print(os, "{}", data);
  return os;
}

std::ostream& operator<<(std::ostream& os, const lower_phy_rx_symbol_context& context)
{
  fmt::print(os, "{} {} {}", context.slot, context.nof_symbols, context.sector);
  return os;
}

std::ostream& operator<<(std::ostream& os, const prach_processor_baseband::symbol_context& context)
{
  fmt::print(os, "{} {} {} {}", context.slot, context.symbol, context.sector, context.port);
  return os;
}

std::ostream& operator<<(std::ostream& os, const puxch_processor_configuration& config)
{
  fmt::print(os,
             "CP={} SCS={} SRate={} BW={} DftWindowOffset={} CenterFreq={}Hz NofRxPorts={}",
             config.cp,
             to_string(config.scs),
             config.srate,
             config.bandwidth_rb,
             config.dft_window_offset,
             config.center_freq_Hz,
             config.nof_rx_ports);
  return os;
}

bool operator==(const prach_processor_baseband::symbol_context left,
                const prach_processor_baseband::symbol_context right)
{
  return (left.slot == right.slot) && (left.symbol == right.symbol) && (left.sector == right.sector) &&
         (left.port == right.port);
}

bool operator==(const lower_phy_rx_symbol_context left, const lower_phy_rx_symbol_context right)
{
  return (left.slot == right.slot) && (left.nof_symbols == right.nof_symbols) && (left.sector == right.sector);
}

bool operator==(span<const cf_t> left, span<const cf_t> right)
{
  return std::equal(left.begin(), left.end(), right.begin(), right.end());
}

bool operator==(const baseband_gateway_buffer& left, const baseband_gateway_buffer& right)
{
  if (left.get_nof_channels() != right.get_nof_channels()) {
    return false;
  }
  unsigned nof_channels = left.get_nof_channels();

  for (unsigned i_channel = 0; i_channel != nof_channels; ++i_channel) {
    span<const cf_t> left_channel  = left.get_channel_buffer(i_channel);
    span<const cf_t> right_channel = right.get_channel_buffer(i_channel);
    if (!std::equal(left_channel.begin(), left_channel.end(), right_channel.begin(), right_channel.end())) {
      return false;
    }
  }

  return true;
}

bool operator==(const puxch_processor_configuration& left, const puxch_processor_configuration& right)
{
  return (left.cp == right.cp) && (left.scs == right.scs) && (left.srate == right.srate) &&
         (left.bandwidth_rb == right.bandwidth_rb) && (left.dft_window_offset == right.dft_window_offset) &&
         (left.center_freq_Hz == right.center_freq_Hz) && (left.nof_rx_ports == right.nof_rx_ports);
}

} // namespace srsran

using LowerPhyUplinkProcessorParams = std::tuple<unsigned, sampling_rate, subcarrier_spacing, cyclic_prefix>;

namespace {

class LowerPhyUplinkProcessorFixture : public ::testing::TestWithParam<LowerPhyUplinkProcessorParams>
{
protected:
  static void SetUpTestSuite()
  {
    if (ul_proc_factory == nullptr) {
      prach_proc_factory = std::make_shared<prach_processor_factory_spy>();
      ASSERT_NE(prach_proc_factory, nullptr);

      puxch_proc_factory = std::make_shared<puxch_processor_factory_spy>();
      ASSERT_NE(puxch_proc_factory, nullptr);

      ul_proc_factory = create_uplink_processor_factory_sw(prach_proc_factory, puxch_proc_factory);
      ASSERT_NE(ul_proc_factory, nullptr);
    }
  }

  void SetUp() override
  {
    ASSERT_NE(ul_proc_factory, nullptr);

    // Select parameters.
    unsigned           nof_rx_ports = std::get<0>(GetParam());
    sampling_rate      srate        = std::get<1>(GetParam());
    subcarrier_spacing scs          = std::get<2>(GetParam());
    cyclic_prefix      cp           = std::get<3>(GetParam());

    // Prepare configurations.
    config.sector_id           = dist_sector_id(rgen);
    config.scs                 = scs;
    config.cp                  = cp;
    config.rate                = srate;
    config.bandwidth_prb       = dist_bandwidth_prb(rgen);
    config.center_frequency_Hz = dist_center_freq_Hz(rgen);
    config.nof_rx_ports        = nof_rx_ports;
    config.initial_slot_index  = dist_initial_slot(rgen);

    // Create processor.
    ul_processor = ul_proc_factory->create(config);
    ASSERT_NE(ul_processor, nullptr);

    // Select PRACH processor spy.
    prach_proc_spy = &prach_proc_factory->get_spy();

    // Select PUxCH processor spy.
    puxch_proc_spy = &puxch_proc_factory->get_spy();
  }

  static constexpr unsigned                                  nof_frames_test = 10;
  static std::mt19937                                        rgen;
  static std::uniform_int_distribution<unsigned>             dist_sector_id;
  static std::uniform_int_distribution<unsigned>             dist_initial_slot;
  static std::uniform_int_distribution<unsigned>             dist_bandwidth_prb;
  static std::uniform_real_distribution<double>              dist_center_freq_Hz;
  static std::uniform_real_distribution<float>               dist_sample;
  static std::shared_ptr<prach_processor_factory_spy>        prach_proc_factory;
  static std::shared_ptr<puxch_processor_factory_spy>        puxch_proc_factory;
  static std::shared_ptr<lower_phy_uplink_processor_factory> ul_proc_factory;

  uplink_processor_configuration              config;
  std::unique_ptr<lower_phy_uplink_processor> ul_processor   = nullptr;
  prach_processor_spy*                        prach_proc_spy = nullptr;
  puxch_processor_spy*                        puxch_proc_spy = nullptr;
};

std::mt19937                                        LowerPhyUplinkProcessorFixture::rgen(0);
std::uniform_int_distribution<unsigned>             LowerPhyUplinkProcessorFixture::dist_sector_id(0, 16);
std::uniform_int_distribution<unsigned>             LowerPhyUplinkProcessorFixture::dist_initial_slot(0, 10);
std::uniform_int_distribution<unsigned>             LowerPhyUplinkProcessorFixture::dist_bandwidth_prb(1, MAX_RB);
std::uniform_real_distribution<double>              LowerPhyUplinkProcessorFixture::dist_center_freq_Hz(1e8, 6e9);
std::uniform_real_distribution<float>               LowerPhyUplinkProcessorFixture::dist_sample(-1, 1);
std::shared_ptr<prach_processor_factory_spy>        LowerPhyUplinkProcessorFixture::prach_proc_factory = nullptr;
std::shared_ptr<puxch_processor_factory_spy>        LowerPhyUplinkProcessorFixture::puxch_proc_factory = nullptr;
std::shared_ptr<lower_phy_uplink_processor_factory> LowerPhyUplinkProcessorFixture::ul_proc_factory    = nullptr;

} // namespace

TEST_P(LowerPhyUplinkProcessorFixture, PuxchConfiguration)
{
  puxch_processor_configuration expected_puxch_config;
  expected_puxch_config.cp                = config.cp;
  expected_puxch_config.scs               = config.scs;
  expected_puxch_config.srate             = config.rate;
  expected_puxch_config.bandwidth_rb      = config.bandwidth_prb;
  expected_puxch_config.dft_window_offset = 0.5;
  expected_puxch_config.center_freq_Hz    = config.center_frequency_Hz;
  expected_puxch_config.nof_rx_ports      = config.nof_rx_ports;
  ASSERT_EQ(expected_puxch_config, puxch_proc_spy->get_configuration());
}

TEST_P(LowerPhyUplinkProcessorFixture, Flow)
{
  unsigned           nof_rx_ports = std::get<0>(GetParam());
  sampling_rate      srate        = std::get<1>(GetParam());
  subcarrier_spacing scs          = std::get<2>(GetParam());
  cyclic_prefix      cp           = std::get<3>(GetParam());

  unsigned base_symbol_size = srate.get_dft_size(scs);

  baseband_gateway_buffer_dynamic buffer(nof_rx_ports, 2 * base_symbol_size);

  unsigned nof_symbols_per_slot   = get_nsymb_per_slot(cp);
  unsigned nof_slots_per_subframe = get_nof_slots_per_subframe(scs);

  // Create notifiers and connect.
  uplink_processor_notifier_spy uplink_proc_notifier_spy;
  prach_processor_notifier_spy  prach_proc_notifier_spy;
  puxch_processor_notifier_spy  puxch_proc_notifier_spy;
  ul_processor->connect(uplink_proc_notifier_spy, prach_proc_notifier_spy, puxch_proc_notifier_spy);

  uplink_processor_baseband& ul_proc_baseband = ul_processor->get_baseband();

  for (unsigned i_frame = 0, i_slot_frame = config.initial_slot_index; i_frame != nof_frames_test; ++i_frame) {
    for (unsigned i_subframe = 0; i_subframe != NOF_SUBFRAMES_PER_FRAME; ++i_subframe) {
      for (unsigned i_slot = 0, i_symbol_subframe = 0; i_slot != nof_slots_per_subframe; ++i_slot, ++i_slot_frame) {
        for (unsigned i_symbol = 0; i_symbol != nof_symbols_per_slot; ++i_symbol, ++i_symbol_subframe) {
          // Calculate cyclic prefix size in samples.
          unsigned cp_size = cp.get_length(i_symbol_subframe, scs).to_samples(srate.to_Hz());

          // Setup buffer.
          buffer.resize(cp_size + base_symbol_size);

          // Fill buffer.
          for (unsigned i_port = 0; i_port != nof_rx_ports; ++i_port) {
            span<cf_t> port_buffer = buffer.get_channel_buffer(i_port);
            std::generate(
                port_buffer.begin(), port_buffer.end(), []() { return cf_t(dist_sample(rgen), dist_sample(rgen)); });
          }

          // Clear spies.
          uplink_proc_notifier_spy.clear_notifications();
          prach_proc_notifier_spy.clear_notifications();
          puxch_proc_notifier_spy.clear_notifications();
          prach_proc_spy->clear();
          puxch_proc_spy->clear();

          // Process baseband.
          ul_proc_baseband.process(buffer);

          // Prepare expected PRACH baseband entry context.
          prach_processor_baseband::symbol_context prach_context;
          prach_context.slot   = slot_point(to_numerology_value(scs), i_slot_frame);
          prach_context.symbol = i_symbol;
          prach_context.sector = config.sector_id;
          prach_context.port   = 0;

          // Prepare expected PUxCH baseband entry context.
          lower_phy_rx_symbol_context puxch_context;
          puxch_context.slot        = slot_point(to_numerology_value(scs), i_slot_frame);
          puxch_context.sector      = config.sector_id;
          puxch_context.nof_symbols = i_symbol;

          // Assert PRACH processor call.
          auto& prach_proc_entries = prach_proc_spy->get_baseband_entries();
          ASSERT_EQ(prach_proc_entries.size(), 1);
          auto& prach_proc_entry = prach_proc_entries.back();
          ASSERT_EQ(prach_proc_entry.context, prach_context);
          ASSERT_EQ(span<const cf_t>(prach_proc_entry.samples), span<const cf_t>(buffer.get_channel_buffer(0)));

          // Assert PUxCH processor call.
          auto& puxch_proc_entries = puxch_proc_spy->get_baseband_entries();
          ASSERT_EQ(puxch_proc_entries.size(), 1);
          auto& puxch_proc_entry = puxch_proc_entries.back();
          ASSERT_EQ(puxch_proc_entry.context, puxch_context);

          // No PRACH or PUxCH notifications.
          ASSERT_EQ(prach_proc_notifier_spy.get_nof_notifications(), 0);
          ASSERT_EQ(puxch_proc_notifier_spy.get_nof_notifications(), 0);

          const auto& half_slot_entries = uplink_proc_notifier_spy.get_half_slots();
          if (i_symbol == nof_symbols_per_slot / 2 - 1) {
            ASSERT_EQ(half_slot_entries.size(), 1);
          } else {
            ASSERT_EQ(half_slot_entries.size(), 0);
          }

          const auto& full_slot_entries = uplink_proc_notifier_spy.get_full_slots();
          if (i_symbol == nof_symbols_per_slot - 1) {
            ASSERT_EQ(full_slot_entries.size(), 1);
          } else {
            ASSERT_EQ(full_slot_entries.size(), 0);
          }
        }
      }
    }
  }
}

// Creates test suite that combines all possible parameters.
INSTANTIATE_TEST_SUITE_P(LowerPhyUplinkProcessor,
                         LowerPhyUplinkProcessorFixture,
                         ::testing::Combine(::testing::Values(1, 2),
                                            ::testing::Values(sampling_rate::from_MHz(3.84)),
                                            ::testing::Values(subcarrier_spacing::kHz15, subcarrier_spacing::kHz30),
                                            ::testing::Values(cyclic_prefix::NORMAL, cyclic_prefix::EXTENDED)));
