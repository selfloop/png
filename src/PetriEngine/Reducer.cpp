/* 
 * File:   Reducer.cpp
 * Author: srba
 *
 * Created on 15 February 2014, 10:50
 */

#include "PetriEngine/Reducer.h"
#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"
#include <queue>
#include <set>
#include <algorithm>
#include <PetriEngine/ReductionRules/ReductionRuleA.h>
#include <PetriEngine/ReductionRules/ReductionRuleB.h>
#include <PetriEngine/ReductionRules/ReductionRuleC.h>
#include <PetriEngine/ReductionRules/ReductionRuleD.h>
#include <PetriEngine/ReductionRules/ReductionRuleE.h>
#include <PetriEngine/ReductionRules/ReductionRuleI.h>
#include <PetriEngine/ReductionRules/ReductionRuleF.h>
#include <PetriEngine/ReductionRules/ReductionRuleG.h>
#include <PetriEngine/ReductionRules/ReductionRuleJ.h>
#include <PetriEngine/ReductionRules/ReductionRuleH.h>

namespace PetriEngine {

  Reducer::Reducer(PetriNetBuilder *p)
      : parent(p) {
  }

  Reducer::~Reducer() {

  }

  void Reducer::Print(QueryPlaceAnalysisContext &context) {
      std::cout << "\nNET INFO:\n"
                << "Number of places: " << parent->numberOfPlaces() << std::endl
                << "Number of transitions: " << parent->numberOfTransitions()
                << std::endl << std::endl;
      for (uint32_t t = 0; t < parent->numberOfTransitions(); t++) {
          std::cout << "Transition " << t << " :\n";
          if (parent->_transitions[t].skip) {
              std::cout << "\tSKIPPED" << std::endl;
          }
          for (auto &arc : parent->_transitions[t].pre) {
              if (arc.weight > 0)
                  std::cout << "\tInput place " << arc.place
                            << " (" << getPlaceName(arc.place) << ")"
                            << " with arc-weight " << arc.weight << std::endl;
          }
          for (auto &arc : parent->_transitions[t].post) {
              if (arc.weight > 0)
                  std::cout << "\tOutput place " << arc.place
                            << " (" << getPlaceName(arc.place) << ")"
                            << " with arc-weight " << arc.weight << std::endl;
          }
          std::cout << std::endl;
      }
      for (uint32_t i = 0; i < parent->numberOfPlaces(); i++) {
          std::cout << "Marking at place " << i <<
                    " is: " << parent->initMarking()[i] << std::endl;
      }
      for (uint32_t i = 0; i < parent->numberOfPlaces(); i++) {
          std::cout << "Query count for place " << i
                    << " is: " << context.getQueryPlaceCount()[i] << std::endl;
      }
  }

  std::string Reducer::getPlaceName(uint32_t place) {
      for (auto t : parent->_placenames) {
          if (t.second == place) return t.first;
      }
      assert(false);
      return "";
  }

  ArcIter Reducer::getOutArc(Transition &trans, uint32_t place) {
      Arc a;
      a.place = place;
      auto ait = std::lower_bound(trans.post.begin(), trans.post.end(), a);
      if (ait != trans.post.end() && ait->place == place) {
          return ait;
      } else {
          return trans.post.end();
      }
  }

  ArcIter Reducer::getInArc(uint32_t place, Transition &trans) {
      Arc a;
      a.place = place;
      auto ait = std::lower_bound(trans.pre.begin(), trans.pre.end(), a);
      if (ait != trans.pre.end() && ait->place == place) {
          return ait;
      } else {
          return trans.pre.end();
      }
  }

  bool Reducer::consistent() {
#ifndef NDEBUG
      size_t strans = 0;
      for (size_t i = 0; i < parent->numberOfTransitions(); ++i) {
          Transition &t = parent->_transitions[i];
          if (t.skip) ++strans;
          assert(std::is_sorted(t.pre.begin(), t.pre.end()));
          assert(std::is_sorted(t.post.end(), t.post.end()));
          assert(!t.skip || (t.pre.size() == 0 && t.post.size() == 0));
          for (Arc &a : t.pre) {
              assert(a.weight > 0);
              Place &p = parent->_places[a.place];
              assert(!p.skip);
              assert(std::find(p.consumers.begin(), p.consumers.end(), i) != p.consumers.end());
          }
          for (Arc &a : t.post) {
              assert(a.weight > 0);
              Place &p = parent->_places[a.place];
              assert(!p.skip);
              assert(std::find(p.producers.begin(), p.producers.end(), i) != p.producers.end());
          }
      }

      assert(strans == _removedTransitions);

      size_t splaces = 0;
      for (size_t i = 0; i < parent->numberOfPlaces(); ++i) {
          Place &p = parent->_places[i];
          if (p.skip) ++splaces;
          assert(std::is_sorted(p.consumers.begin(), p.consumers.end()));
          assert(std::is_sorted(p.producers.begin(), p.producers.end()));
          assert(!p.skip || (p.consumers.size() == 0 && p.producers.size() == 0));

          for (uint c : p.consumers) {
              Transition &t = parent->_transitions[c];
              assert(!t.skip);
              auto a = getInArc(i, t);
              assert(a != t.pre.end());
              assert(a->place == i);
          }

          for (uint prod : p.producers) {
              Transition &t = parent->_transitions[prod];
              assert(!t.skip);
              auto a = getOutArc(t, i);
              assert(a != t.post.end());
              assert(a->place == i);
          }
      }
      assert(splaces == _removedPlaces);
#endif
      return true;
  }

  bool Reducer::ReducebyRuleA(uint32_t *placeInQuery) {
      return ReductionRuleA(
          parent,
          &_timer,
          _timeout,
          reconstructTrace,
          _initfire,
          _postfire,
          _extraconsume,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  bool Reducer::ReducebyRuleB(uint32_t *placeInQuery, bool remove_deadlocks, bool remove_consumers) {
      return ReductionRuleB(
          parent,
          &_timer,
          _timeout,
          reconstructTrace,
          _initfire,
          _postfire,
          _extraconsume,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, remove_deadlocks, remove_consumers);
  }

  bool Reducer::ReducebyRuleC(uint32_t *placeInQuery) {
      return ReductionRuleC(
          parent,
          &_timer,
          _timeout,
          reconstructTrace,
          _extraconsume,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  bool Reducer::ReducebyRuleD(uint32_t *placeInQuery) {
      return ReductionRuleD(
          parent,
          &_timer,
          _timeout,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  bool Reducer::ReducebyRuleE(uint32_t *placeInQuery) {
      return ReductionRuleE(
          parent,
          &_timer,
          _timeout,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  bool Reducer::ReducebyRuleF(uint32_t *placeInQuery) {
      return ReductionRuleF(
          parent,
          &_timer,
          _timeout,
          reconstructTrace,
          _extraconsume,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  bool Reducer::ReducebyRuleG(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      return ReductionRuleG(
          parent,
          &_timer,
          _timeout,
          reconstructTrace,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, remove_loops, remove_consumers);
  }

  bool Reducer::ReducebyRuleH(uint32_t *placeInQuery) {
      return ReductionRuleH(
          parent,
          &_timer,
          _timeout,
          reconstructTrace,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  bool Reducer::ReducebyRuleI(uint32_t *placeInQuery, bool remove_loops, bool remove_consumers) {
      return ReductionRuleI(
          parent,
          &_timer,
          _timeout,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, remove_loops, remove_consumers);
  }

  bool Reducer::ReducebyRuleJ(uint32_t *placeInQuery) {
      return ReductionRuleJ(
          parent,
          &_timer,
          _timeout,
          _skipped_trans,
          &_removedTransitions,
          &_removedPlaces
      ).reduce(placeInQuery, false, false);
  }

  void Reducer::Reduce(QueryPlaceAnalysisContext &context,
                       int enablereduction,
                       bool reconstructTrace,
                       int timeout,
                       bool remove_loops,
                       bool remove_consumers,
                       bool next_safe,
                       std::vector<uint32_t> &reduction) {
      this->_timeout = timeout;
      _timer = std::chrono::high_resolution_clock::now();
      assert(consistent());
      this->reconstructTrace = reconstructTrace;
      if (reconstructTrace && enablereduction >= 1 && enablereduction <= 2)
          std::cout << "Rule H disabled when a trace is requested." << std::endl;
      if (enablereduction == 2) { // for k-boundedness checking only rules A, D and H are applicable
          bool changed = true;
          while (changed && !hasTimedout()) {
              changed = false;
              if (!next_safe) {
                  while (ReducebyRuleA(context.getQueryPlaceCount())) changed = true;
                  while (ReducebyRuleD(context.getQueryPlaceCount())) changed = true;
                  while (ReducebyRuleH(context.getQueryPlaceCount())) changed = true;
              }
          }
      } else if (enablereduction
          == 1) { // in the aggressive reduction all four rules are used as long as they remove something
          bool changed = false;
          do {
              if (remove_loops && !next_safe)
                  while (ReducebyRuleI(context.getQueryPlaceCount(), remove_loops, remove_consumers)) changed = true;
              do {
                  do { // start by rules that do not move tokens
                      changed = false;
                      while (ReducebyRuleE(context.getQueryPlaceCount())) changed = true;
                      while (ReducebyRuleC(context.getQueryPlaceCount())) changed = true;
                      if (!next_safe) {
                          while (ReducebyRuleF(context.getQueryPlaceCount())) changed = true;
                          while (ReducebyRuleG(context.getQueryPlaceCount(), remove_loops, remove_consumers))
                              changed = true;
                          if (!remove_loops)
                              while (ReducebyRuleI(context.getQueryPlaceCount(), remove_loops, remove_consumers))
                                  changed = true;
                          while (ReducebyRuleD(context.getQueryPlaceCount())) changed = true;
                      }
                  } while (changed && !hasTimedout());
                  if (!next_safe) { // then apply tokens moving rules
                      //while(ReducebyRuleJ(context.getQueryPlaceCount())) changed = true;
                      while (ReducebyRuleB(context.getQueryPlaceCount(), remove_loops, remove_consumers))
                          changed = true;
                      while (ReducebyRuleA(context.getQueryPlaceCount())) changed = true;
                  }
              } while (changed && !hasTimedout());
              if (!next_safe && !changed) {
                  // Only try RuleH last. It can reduce applicability of other rules.
                  while (ReducebyRuleH(context.getQueryPlaceCount())) changed = true;
              }
          } while (!hasTimedout() && changed);

      } else {
          const char *rnames = "ABCDEFGHIJ";
          for (int i = reduction.size() - 1; i > 0; --i) {
              if (next_safe) {
                  if (reduction[i] != 2 && reduction[i] != 4) {
                      std::cerr << "Skipping Rule" << rnames[reduction[i]] << " due to NEXT operator in proposition"
                                << std::endl;
                      reduction.erase(reduction.begin() + i);
                      continue;
                  }
              }
              if (!remove_loops && reduction[i] == 5) {
                  std::cerr << "Skipping Rule" << rnames[reduction[i]] << " as proposition is loop sensitive"
                            << std::endl;
                  reduction.erase(reduction.begin() + i);
              }
          }
          bool changed = true;
          while (changed && !hasTimedout()) {
              changed = false;
              for (auto r : reduction) {
#ifndef NDEBUG
                  auto c = std::chrono::high_resolution_clock::now();
                  auto op = _removedPlaces;
                  auto ot = _removedTransitions;
#endif
                  switch (r) {
                      case 0:while (ReducebyRuleA(context.getQueryPlaceCount())) changed = true;
                          break;
                      case 1:
                          while (ReducebyRuleB(context.getQueryPlaceCount(), remove_loops, remove_consumers))
                              changed = true;
                          break;
                      case 2:while (ReducebyRuleC(context.getQueryPlaceCount())) changed = true;
                          break;
                      case 3:while (ReducebyRuleD(context.getQueryPlaceCount())) changed = true;
                          break;
                      case 4:while (ReducebyRuleE(context.getQueryPlaceCount())) changed = true;
                          break;
                      case 5:while (ReducebyRuleF(context.getQueryPlaceCount())) changed = true;
                          break;
                      case 6:
                          while (ReducebyRuleG(context.getQueryPlaceCount(), remove_loops, remove_consumers))
                              changed = true;
                          break;
                      case 7:while (ReducebyRuleH(context.getQueryPlaceCount())) changed = true;
                          break;
                      case 8:
                          while (ReducebyRuleI(context.getQueryPlaceCount(), remove_loops, remove_consumers))
                              changed = true;
                          break;
                      case 9:while (ReducebyRuleJ(context.getQueryPlaceCount())) changed = true;
                          break;
                  }
#ifndef NDEBUG
                  auto end = std::chrono::high_resolution_clock::now();
                  auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - c);
                  std::cout << "SPEND " << diff.count() << " ON " << rnames[r] << std::endl;
                  std::cout << "REM " << ((int) _removedPlaces - (int) op) << " "
                            << ((int) _removedTransitions - (int) ot) << std::endl;
#endif
                  if (hasTimedout())
                      break;
              }
          }
      }
  }

  void Reducer::postFire(std::ostream &out, const std::string &transition) {
      if (_postfire.count(transition) > 0) {
          std::queue<std::string> tofire;

          for (auto &el : _postfire[transition]) tofire.push(el);

          for (auto &el : _postfire[transition]) {
              tofire.pop();
              out << "\t<transition id=\"" << el << "\">\n";
              extraConsume(out, el);
              out << "\t</transition>\n";
              postFire(out, el);
          }
      }
  }

  void Reducer::initFire(std::ostream &out) {
      for (std::string &init : _initfire) {
          out << "\t<transition id=\"" << init << "\">\n";
          extraConsume(out, init);
          out << "\t</transition>\n";
          postFire(out, init);
      }
  }

  void Reducer::extraConsume(std::ostream &out, const std::string &transition) {
      if (_extraconsume.count(transition) > 0) {
          for (auto &ec : _extraconsume[transition]) {
              out << ec;
          }
      }
  }

} //PetriNet namespace
