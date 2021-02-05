//
// Created by hamburger on 05.02.21.
//

#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <set>
#include "PetriEngine/ReductionRules/ReductionRuleB.h"

namespace PetriEngine {
  bool ReductionRuleB::reduce(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      // Rule B - find place p that has exactly one transition in pre and exactly one in post and remove the place
      bool continueReductions = false;
      const size_t numberofplaces = parent->numberOfPlaces();
      for (uint32_t p = 0; p < numberofplaces; p++) {
          if (hasTimedout()) return false;
          Place &place = parent->_places[p];

          if (place.skip) continue;    // already removed
          // B5. dont mess up query
          if (placeInQuery[p] > 0)
              continue;

          // B2. Only one consumer/producer
          if (place.consumers.size() != 1 ||
              place.producers.size() < 1)
              continue; // no orphan removal

          auto tIn = place.consumers[0];

          // B1. producer is not consumer
          bool ok = true;
          for (auto &tOut : place.producers) {
              if (tOut == tIn) {
                  ok = false;
                  continue; // cannot remove this kind either
              }
          }
          if (!ok)
              continue;
          auto prod = place.producers;
          Transition &in = getTransition(tIn);
          for (auto tOut : prod) {
              Transition &out = getTransition(tOut);

              if (out.post.size() != 1 && in.pre.size() != 1)
                  continue; // at least one has to be singular for this to work

              if ((!remove_loops || !remove_consumers) && in.pre.size() != 1)
                  // the buffer can mean deadlocks and other interesting things
                  // also we can "hide" tokens, so we need to make sure not
                  // to remove consumers.
                  continue;

              if (parent->initMarking()[p] > 0 && in.pre.size() != 1)
                  continue;

              auto inArc = getInArc(p, in);
              auto outArc = getOutArc(out, p);

              // B3. Output is a multiple of input and nonzero.
              if (outArc->weight < inArc->weight)
                  continue;
              if ((outArc->weight % inArc->weight) != 0)
                  continue;

              size_t multiplier = outArc->weight / inArc->weight;

              // B4. Do inhibitor check, neither In, out or place can be involved with any inhibitor
              if (place.inhib || in.inhib || out.inhib)
                  continue;

              // B6. also, none of the places in the post-set of consuming transition can be participating in inhibitors.
              // B7. nor can they appear in the query.
              {
                  bool post_ok = false;
                  for (const Arc &a : in.post) {
                      post_ok |= parent->_places[a.place].inhib;
                      post_ok |= placeInQuery[a.place];
                      if (post_ok) break;
                  }
                  if (post_ok)
                      continue;
              }
              {
                  bool pre_ok = false;
                  for (const Arc &a : in.pre) {
                      pre_ok |= parent->_places[a.place].inhib;
                      pre_ok |= placeInQuery[a.place];
                      if (pre_ok) break;
                  }
                  if (pre_ok)
                      continue;
              }

              bool ok = true;
              if (in.pre.size() > 1)
                  for (const Arc &arc : out.pre)
                      ok &= placeInQuery[arc.place] == 0;
              if (!ok)
                  continue;

              // B2.a Check that there is no other place than p that gives to tPost,
              // tPre can give to other places
              auto &arcs = in.pre.size() < out.post.size() ? in.pre : out.post;
              for (auto &arc : arcs) {
                  if (arc.weight > 0 && arc.place != p) {
                      ok = false;
                      break;
                  }
              }

              if (!ok)
                  continue;

              // UB2. we need to remember initial marking
              uint initm = parent->initMarking()[p];
              initm /= inArc->weight; // integer-devision is floor by default

              if (reconstructTrace) {
                  // remember reduction for recreation of trace
                  std::string toutname = getTransitionName(tOut);
                  std::string tinname = getTransitionName(tIn);
                  std::string pname = getPlaceName(p);
                  Arc &a = *getInArc(p, in);
                  _extraconsume[tinname].emplace_back(pname, a.weight);
                  for (size_t i = 0; i < multiplier; ++i) {
                      _postfire[toutname].push_back(tinname);
                  }

                  for (size_t i = 0; initm > 0 && i < initm / inArc->weight; ++i) {
                      _initfire.push_back(tinname);
                  }
              }

              continueReductions = true;
              // TODO: _ruleB++;
              // UB1. Remove place p
              parent->initialMarking[p] = 0;
              // We need to remember that when tOut fires, tIn fires just after.
              // this should fix the trace

              // UB3. move arcs from t' to t
              for (auto &arc : in.post) { // remove tPost
                  auto _arc = getOutArc(out, arc.place);
                  // UB2. Update initial marking
                  parent->initialMarking[arc.place] += initm * arc.weight;
                  if (_arc != out.post.end()) {
                      _arc->weight += arc.weight * multiplier;
                  } else {
                      out.post.push_back(arc);
                      out.post.back().weight *= multiplier;
                      parent->_places[arc.place].producers.push_back(tOut);

                      std::sort(out.post.begin(), out.post.end());
                      std::sort(parent->_places[arc.place].producers.begin(),
                                parent->_places[arc.place].producers.end());
                  }
              }
              for (auto &arc : in.pre) { // remove tPost
                  if (arc.place == p)
                      continue;
                  auto _arc = getInArc(arc.place, out);
                  // UB2. Update initial marking
                  parent->initialMarking[arc.place] += initm * arc.weight;
                  if (_arc != out.pre.end()) {
                      _arc->weight += arc.weight * multiplier;
                  } else {
                      out.pre.push_back(arc);
                      out.pre.back().weight *= multiplier;
                      parent->_places[arc.place].consumers.push_back(tOut);

                      std::sort(out.pre.begin(), out.pre.end());
                      std::sort(parent->_places[arc.place].consumers.begin(),
                                parent->_places[arc.place].consumers.end());
                  }
              }

              for (auto it = out.post.begin(); it != out.post.end(); ++it) {
                  if (it->place == p) {
                      out.post.erase(it);
                      break;
                  }
              }
              for (auto it = place.producers.begin(); it != place.producers.end(); ++it) {
                  if (*it == tOut) {
                      place.producers.erase(it);
                      break;
                  }
              }
          }
          // UB1. remove transition
          if (place.producers.size() == 0) {
              skipPlace(p);
              skipTransition(tIn);
          }
      } // end of Rule B main for-loop
      assert(consistent());
      return continueReductions;
  }
}