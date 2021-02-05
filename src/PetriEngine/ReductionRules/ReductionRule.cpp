//
// Created by hamburger on 04.02.21.
//

#include <PetriEngine/ReductionRules/ReductionRule.h>

namespace PetriEngine {
  std::string ReductionRule::getTransitionName(uint32_t transition) {
      for (const auto &t : reducer->parent->_transitionnames) {
          if (t.second == transition) return t.first;
      }
      assert(false);
      return "";
  }

  Transition &ReductionRule::getTransition(uint32_t transition) {
      return reducer->parent->_transitions[transition];
  }

  void ReductionRule::skipTransition(uint32_t t) {
      ++reducer->_removedTransitions;
      Transition &trans = getTransition(t);
      assert(!trans.skip);
      for (auto p : trans.post) {
          eraseTransition(reducer->parent->_places[p.place].producers, t);
      }
      for (auto p : trans.pre) {
          eraseTransition(reducer->parent->_places[p.place].consumers, t);
      }
      trans.post.clear();
      trans.pre.clear();
      trans.skip = true;
      assert(reducer->consistent());
      reducer->_skipped_trans.push_back(t);
  }

  void ReductionRule::skipPlace(uint32_t place) {
      ++reducer->_removedPlaces;
      Place &pl = reducer->parent->_places[place];
      assert(!pl.skip);
      pl.skip = true;
      for (auto &t : pl.consumers) {
          Transition &trans = getTransition(t);
          auto ait = reducer->getInArc(place, trans);
          if (ait != trans.pre.end() && ait->place == place)
              trans.pre.erase(ait);
      }

      for (auto &t : pl.producers) {
          Transition &trans = getTransition(t);
          auto ait = reducer->getOutArc(trans, place);
          if (ait != trans.post.end() && ait->place == place)
              trans.post.erase(ait);
      }
      pl.consumers.clear();
      pl.producers.clear();
      assert(reducer->consistent());
  }

  void ReductionRule::eraseTransition(std::vector<uint32_t> &set, uint32_t el) {
      auto lb = std::lower_bound(set.begin(), set.end(), el);
      assert(lb != set.end());
      assert(*lb == el);
      set.erase(lb);
  }
}