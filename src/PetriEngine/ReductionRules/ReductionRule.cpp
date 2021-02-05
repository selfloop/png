//
// Created by hamburger on 04.02.21.
//

#include <PetriEngine/ReductionRules/ReductionRule.h>

namespace PetriEngine {
  std::string ReductionRule::getPlaceName(uint32_t place) {
      for (const auto &t : this->parent->_placenames) {
          if (t.second == place) return t.first;
      }
      assert(false);
      return "";
  }

  std::string ReductionRule::getTransitionName(uint32_t transition) {
      for (const auto &t : parent->_transitionnames) {
          if (t.second == transition) return t.first;
      }
      assert(false);
      return "";
  }

  Transition &ReductionRule::getTransition(uint32_t transition) {
      return parent->_transitions[transition];
  }

  ArcIter ReductionRule::getOutArc(Transition &trans, uint32_t place) {
      Arc a;
      a.place = place;
      auto ait = std::lower_bound(trans.post.begin(), trans.post.end(), a);
      if (ait != trans.post.end() && ait->place == place) {
          return ait;
      } else {
          return trans.post.end();
      }
  }

  void ReductionRule::skipTransition(uint32_t t) {
      ++(*_removedTransitions);
      Transition &trans = getTransition(t);
      assert(!trans.skip);
      for (auto p : trans.post) {
          eraseTransition(parent->_places[p.place].producers, t);
      }
      for (auto p : trans.pre) {
          eraseTransition(parent->_places[p.place].consumers, t);
      }
      trans.post.clear();
      trans.pre.clear();
      trans.skip = true;
      assert(consistent());
      _skipped_trans.push_back(t);
  }

  void ReductionRule::skipPlace(uint32_t place) {
      ++(*_removedPlaces);
      Place &pl = parent->_places[place];
      assert(!pl.skip);
      pl.skip = true;
      for (auto &t : pl.consumers) {
          Transition &trans = getTransition(t);
          auto ait = getInArc(place, trans);
          if (ait != trans.pre.end() && ait->place == place)
              trans.pre.erase(ait);
      }

      for (auto &t : pl.producers) {
          Transition &trans = getTransition(t);
          auto ait = getOutArc(trans, place);
          if (ait != trans.post.end() && ait->place == place)
              trans.post.erase(ait);
      }
      pl.consumers.clear();
      pl.producers.clear();
      assert(consistent());
  }

  bool ReductionRule::consistent() {
#ifndef NDEBUG
      size_t strans = 0;
      for (size_t i = 0; i < parent->numberOfTransitions(); ++i) {
          Transition &t = parent->_transitions[i];
          if (t.skip) ++strans;
          assert(std::is_sorted(t.pre.begin(), t.pre.end()));
          assert(std::is_sorted(t.post.end(), t.post.end()));
          assert(!t.skip || (t.pre.empty() && t.post.empty()));
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

      assert(strans == *_removedTransitions);

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
      assert(splaces == *_removedPlaces);
#endif
      return true;
  }

  void ReductionRule::eraseTransition(std::vector<uint32_t> &set, uint32_t el) {
      auto lb = std::lower_bound(set.begin(), set.end(), el);
      assert(lb != set.end());
      assert(*lb == el);
      set.erase(lb);
  }

  ArcIter ReductionRule::getInArc(uint32_t place, Transition &trans) {
      Arc a;
      a.place = place;
      auto ait = std::lower_bound(trans.pre.begin(), trans.pre.end(), a);
      if (ait != trans.pre.end() && ait->place == place) {
          return ait;
      } else {
          return trans.pre.end();
      }
  }
}