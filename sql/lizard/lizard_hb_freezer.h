/* Copyright (c) 2008, 2024, Alibaba and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef LIZARD_HB_FREEZER_H_INCLUDED
#define LIZARD_HB_FREEZER_H_INCLUDED

#include <stdint.h> // uint32_t...
#include <ctime> // std::time_t...
#include <mutex>

class THD;
class LEX;

namespace lizard {

extern bool opt_no_heartbeat_freeze;

extern bool no_heartbeat_freeze;

extern uint64_t opt_no_heartbeat_freeze_timeout;

class Simple_timer {
 public:
  Simple_timer() : m_ts(0) {}

  void update() { m_ts = std::time(0); }

  uint64_t since_last_update() const {
    std::time_t now = std::time(0);
    return now - m_ts;
  }

 private:
  std::time_t m_ts;
};

class Lazy_printer {
 public:
  Lazy_printer(const uint32_t interval_secs)
      : m_internal_secs(interval_secs), m_timer(), m_first(true) {}

  bool print(const char *msg);
  void reset();

 private:
  /** log printing interval */
  const uint32_t m_internal_secs;

  /** Timer */
  Simple_timer m_timer;

  /** First time to print, no take interval into consideration. */
  bool m_first;
};

class Heartbeat_freezer {
 public:
  Heartbeat_freezer() : m_is_freeze(false) {}

  bool is_freeze() { return m_is_freeze; }

  void heartbeat();

  bool determine_freeze();

 private:
  /** Timer for check timeout. */
  Simple_timer m_timer;

  /* No need to use std::atomic because no need to read the newest value
  immediately. */
  bool m_is_freeze;

  /* Mutex modification of m_is_freeze, m_timer. */
  std::mutex m_mutex;
};

/** Check if should be blocked all update DML becase the heartbeat timeout. */
extern bool cn_heartbeat_timeout_freeze_updating(LEX *const lex);

/** Check if should be blocked all slave apply becase the heartbeat timeout. */
extern bool cn_heartbeat_timeout_freeze_applying_event(THD *);

/** Generte a heartbeat. */
extern void hb_freezer_heartbeat();

/** Determine if frozen all update, purge and apply. */
extern bool hb_freezer_determine_freeze();
}

#endif
