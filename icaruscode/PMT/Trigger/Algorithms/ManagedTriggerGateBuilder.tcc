/**
 * @file   icaruscode/PMT/Trigger/Algorithms/ManagedTriggerGateBuilder.tcc
 * @brief  Algorithm to produce trigger gates out of optical readout waveforms.
 * @author Gianluca Petrillo (petrillo@slac.stanford.edu)
 * @date   April 1, 2019
 * @see    `icaruscode/PMT/Trigger/Algorithms/ManagedTriggerGateBuilder.h`
 * 
 */


#ifndef ICARUSCODE_PMT_TRIGGER_ALGORITHMS_MANAGEDTRIGGERGATEBUILDER_TCC
#define ICARUSCODE_PMT_TRIGGER_ALGORITHMS_MANAGEDTRIGGERGATEBUILDER_TCC

#ifndef ICARUSCODE_PMT_TRIGGER_ALGORITHMS_MANAGEDTRIGGERGATEBUILDER_H
# error "ManagedTriggerGateBuilder.tcc must not be included directly."\
        " #include \"icaruscode/PMT/Trigger/Algorithms/ManagedTriggerGateBuilder.h\" instead."
#endif // ICARUSCODE_PMT_TRIGGER_ALGORITHMS_MANAGEDTRIGGERGATEBUILDER_H


// ICARUS libraries
#include "icaruscode/PMT/Trigger/Algorithms/TriggerTypes.h" // icarus::trigger::ADCCounts_t
#include "icaruscode/PMT/Trigger/Data/SingleChannelOpticalTriggerGate.h"
#include "icaruscode/Utilities/WaveformOperations.h"

// LArSoft libraries
#include "lardataobj/RawData/OpDetWaveform.h"
#include "larcorealg/CoreUtils/counter.h"
// #include "larcorealg/CoreUtils/StdUtils.h" // util::to_string()

// framework libraries
#include "messagefacility/MessageLogger/MessageLogger.h" // MF_LOG_TRACE()

// range library
#include "range/v3/view/group_by.hpp"

// C/C++ standard libraries
#include <optional>
#include <iterator> // std::next(), std::prev()
#include <cstddef> // std::ptrdiff_t


//------------------------------------------------------------------------------
//--- icarus::trigger::ManagedTriggerGateBuilder
//------------------------------------------------------------------------------
template <typename GateMgr>
auto icarus::trigger::ManagedTriggerGateBuilder::unifiedBuild
  (GateMgr&& gateManager, std::vector<raw::OpDetWaveform> const& waveforms)
  const -> std::vector<TriggerGates>
{
  using GateManager_t = GateMgr;
  using GateInfo_t = typename GateManager_t::GateInfo_t;
  
  /*
   * This is the simple algorithm where each channel is treated independently,
   * and we have as many trigger gates as we have channels.
   */
  
  // create an empty TriggerGates object for each threshold;
  // thresholds are kept relative
  std::vector<TriggerGates> allGates = prepareAllGates();
  
  raw::Channel_t channel = raw::InvalidChannel;
  
  // now group the waveforms by channel (must be already sorted!)
  // and process waveforms channel by channel
  auto sameChannel
    = [](raw::OpDetWaveform const& a, raw::OpDetWaveform const& b)
      { return a.ChannelNumber() == b.ChannelNumber(); }
    ;
  
  auto byChannel = waveforms | ranges::view::group_by(sameChannel);
  for (auto const& channelWaveforms: byChannel) {
    
    auto const& firstWaveform = channelWaveforms.front();
    
    // assert that the waveforms are sorted by channel and then by time
    // and not overlapping
    assert(
      !raw::isValidChannel(channel)
      || (firstWaveform.ChannelNumber() >= channel)
      );
    if (firstWaveform.ChannelNumber() != channel)
      channel = firstWaveform.ChannelNumber();
    
    // we don't know how many...
    MF_LOG_TRACE(details::TriggerGateDebugLog)
      << "Building trigger gates from waveforms on channel " << channel;
    
    std::vector<GateInfo_t> channelGates;
    channelGates.reserve(nChannelThresholds());
    for (TriggerGates& thrGates: allGates) {
      channelGates.push_back 
        (gateManager.create(thrGates.gateFor(firstWaveform)));
    }
    
    // this method will update the channel gates referenced in `channelGates`,
    // which are owned by `allGates`
    buildChannelGates(channelGates, channelWaveforms);
    
  } // for channels
  
  return allGates;
} // icarus::trigger::ManagedTriggerGateBuilder::unifiedBuild()


//------------------------------------------------------------------------------
template <typename GateInfo, typename Waveforms>
void icarus::trigger::ManagedTriggerGateBuilder::buildChannelGates(
  std::vector<GateInfo>& channelGates,
  Waveforms const& channelWaveforms
) const
{
  using ops
    = icarus::waveform_operations::NegativePolarityOperations<ADCCounts_t>;
  
  if (channelWaveforms.empty()) return;
  
  //
  // extract all thresholds from each waveform in one pass
  //
  
  raw::OpDetWaveform const& firstWaveform = channelWaveforms.front();
  raw::Channel_t const channel = firstWaveform.ChannelNumber();
  
  // used only in debug mode:
  optical_tick lastWaveformTick [[gnu::unused]]
    = timeStampToOpticalTick(firstWaveform.TimeStamp());
  
  /*
   * The algorithm finds gate openings and closing.
   * The actual actions on opening and closing depends on the gate info class.
   * For example, while a dynamic gate duration algorithm will perform open and
   * close operations directly, a fixed gate duration algorithm may perform
   * both opening and closing at open time, and nothing at all at closing time.
   */
  unsigned int nWaveforms = 0U;
  for (raw::OpDetWaveform const& waveform: channelWaveforms) {
    ++nWaveforms;
    assert(waveform.ChannelNumber() == channel);
    
    // start of the waveform (tick #0) in electronics time scale
    // and in optical tick units
    optical_tick const waveformTickStart
      = timeStampToOpticalTick(waveform.TimeStamp());
    optical_tick const waveformTickEnd
      = waveformTickStart + optical_time_ticks::castFrom(waveform.size());
    
    MF_LOG_TRACE(details::TriggerGateDebugLog)
      << "Waveform with " << waveform.size() << " ticks"
      <<  ": [ " << waveformTickStart << " -- " << waveformTickEnd << " ]";
    
    // assert that the waveforms are sorted by channel and then by time
    // and not overlapping
    assert(lastWaveformTick <= waveformTickStart);
    lastWaveformTick = waveformTickEnd;
    
    auto const tbegin = absoluteThresholds().begin();
    auto const tend = absoluteThresholds().end();
    
    // register this waveform with the gates (this feature is unused here)
    for (auto& gateInfo: channelGates) gateInfo.gate().add(waveform);
    
    // all gates start closed; this gate is not necessarily closed, but the
    // waveform is not above the gate threshold any more.
    auto nextGateToOpen = channelGates.begin();
    
    // we keep track of whether we have no lower or higher thresholds available
    // to simplify the checks;
    // we name them "pp" because they behave (almost) like pointers to pointers
    using ThresholdIterPtr_t
      = std::optional<std::vector<ADCCounts_t>::const_iterator>;
    ThresholdIterPtr_t ppLowerThreshold; // start at bottom with no lower threshold
    ThresholdIterPtr_t ppUpperThreshold;
    if (!absoluteThresholds().empty())
      ppUpperThreshold = absoluteThresholds().begin(); // std::optional behavior
    
    for (auto iSample: util::counter<std::ptrdiff_t>(waveform.size())) {
      
      ADCCounts_t const sample { waveform[iSample] };
      /*
      MF_LOG_TRACE(details::TriggerGateDebugLog)
        << "Sample #" << iSample << ": " << sample
        << "; thresholds lower: "
          << (ppLowerThreshold? util::to_string(**ppLowerThreshold): "none")
        << ", upper: "
          << (ppUpperThreshold? util::to_string(**ppUpperThreshold): "none")
        ;
      */
      //
      // if this sample is lower than the current lower threshold,
      // we are just tracking the thresholds: gate closing has already happened
      //
      if (ppLowerThreshold && (ops::lessThan(sample, **ppLowerThreshold))) {
        
        do { // we keep opening gates at increasing thresholds
          
          // if there is an lower threshold, there must also be a open gate!
          assert(nextGateToOpen != channelGates.begin());
          
          
          (--nextGateToOpen)->belowThresholdAt
            (waveformTickStart + optical_time_ticks{ iSample });
          
//           MF_LOG_TRACE(details::TriggerGateDebugLog)
//              << "  => decreasing threshold " << **ppLowerThreshold;
          
          if (*ppLowerThreshold == tbegin) { // was this the bottom threshold?
            ppLowerThreshold = std::nullopt;
            break;
          }
          --*ppLowerThreshold; // point to previous threshold
          
        } while (ops::lessThan(sample, **ppLowerThreshold));
        
        // we can't be at the top since we just closed a gate
        ppUpperThreshold
          = ppLowerThreshold? std::next(*ppLowerThreshold): tbegin;
        
      } // if closing gate
      
      //
      // if this sample is greater or matching the next threshold,
      // we *are* opening gate(s)
      //
      else if (ppUpperThreshold && ops::noLessThan(sample, **ppUpperThreshold))
      {
        
        do { // we keep opening gates at increasing thresholds
          
          // if there is an upper threshold, there must also be a closed gate!
          assert(nextGateToOpen != channelGates.end());
          
          MF_LOG_TRACE(details::TriggerGateDebugLog)
            << "Opening thr=" << (**ppUpperThreshold)
            << " at " << waveformTickStart
            << " + " << optical_time_ticks{ iSample };
          
          
          (nextGateToOpen++)->aboveThresholdAt
            (waveformTickStart + optical_time_ticks{ iSample });
          
//           MF_LOG_TRACE(details::TriggerGateDebugLog)
//             << "  => opening threshold " << **ppUpperThreshold;
          
          if (++*ppUpperThreshold == tend) { // was this the top threshold?
            ppUpperThreshold = std::nullopt;
            break;
          }
          
        } while (ops::noLessThan(sample, **ppUpperThreshold));
        
        // we can't be at the bottom since we just opened a gate
        ppLowerThreshold = std::prev(ppUpperThreshold? *ppUpperThreshold: tend);
        
      } // if opening gate
      
    } // for threshold
    
  } // for waveforms
  
  MF_LOG_TRACE(details::TriggerGateDebugLog)
    << "Trigger gates from " << nWaveforms << " waveforms on channel "
    << channel << " completed.";
  
} // icarus::trigger::ManagedTriggerGateBuilder::buildChannelGates()


//------------------------------------------------------------------------------

#endif // ICARUSCODE_PMT_TRIGGER_ALGORITHMS_MANAGEDTRIGGERGATEBUILDER_TCC
