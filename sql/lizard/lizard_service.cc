/*****************************************************************************

Copyright (c) 2013, 2024, Alibaba and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

#include "sql/lizard/lizard_service.h"
#include "sql/mysqld.h"
#include "sql/handler.h"

/*-----------------------------------------------------------------------------*/
/** The lizard innodb handlerton service */
/*-----------------------------------------------------------------------------*/
inline gcn_t innodb_gcn_load() { return innodb_hton->ext.load_gcn(); }

void innodb_set_gcn_if_bigger(gcn_t gcn) {
  innodb_hton->ext.set_gcn_if_bigger(gcn);
}

/*-----------------------------------------------------------------------------*/
/** MyGCN structure that represent user behavior */
/*-----------------------------------------------------------------------------*/
void MyGCN::copy_pmmt(const gcn_tuple_t &gcn_tuple) {
  m_gtuple = gcn_tuple;
  is_proposal = true;
  has_decided = true;
  has_pushed_up = true;
}

void MyGCN::copy_cmmt(const gcn_tuple_t &gcn_tuple) {
  m_gtuple = gcn_tuple;
  is_proposal = false;
  has_decided = true;
  has_pushed_up = true;
}

void MyGCN::assign_from_ac_prepare(gcn_t gcn) {
  m_gtuple = {gcn, csr_t::CSR_ASSIGNED};
  is_proposal = true;
  has_decided = false;
  has_pushed_up = false;
}

void MyGCN::assign_from_ac_commit(gcn_t gcn) {
  m_gtuple = {gcn, csr_t::CSR_ASSIGNED};
  is_proposal = false;
  has_decided = false;
  has_pushed_up = false;
}

/**
  Decide from external assigned GCN.
  @param[in]    gcn_tuple     {assigned_gcn, csr}
*/
void MyGCN::assign_from_var(gcn_t gcn) {
  m_gtuple = {gcn, CSR_ASSIGNED};
  is_proposal = false;
  has_pushed_up = false;
  has_decided = true;
}

/**
  Decide from Gcn_log_event. Might be proposal or non-proposal.
*/
void MyGCN::assign_from_binlog(const gcn_tuple_t &gcn_tuple, bool proposal) {
  m_gtuple = gcn_tuple;
  is_proposal = proposal;
  has_pushed_up = false;
  has_decided = true;
}

/**
  Decide by loading local SYS_GCN.
*/
void MyGCN::decide_if_null() {
  assert(is_null());
  assert(!is_proposal);

  m_gtuple = {innodb_gcn_load(), csr_t::CSR_AUTOMATIC};
  has_decided = true;

  /** Load from local SYS_GCN, so no need to push up. */
  has_pushed_up = true;
}

/**
  Decide proposal_gcn if ac prepare
  @param[in]    proposal
*/
void MyGCN::decide_if_ac_prepare(const gcn_tuple_t &proposal) {
  assert(!is_null());
  assert(is_proposal);

  assert(!has_pushed_up);
  if (proposal.csr == CSR_AUTOMATIC) {
    has_pushed_up = true;
  }
  m_gtuple = proposal;
  has_decided = true;
}

/**
  Decide commit_gcn if ac commit. Only allowed to negotiate csr
  @param[in]    csr
  @param[in]    external_automatic
*/
void MyGCN::decide_if_ac_commit(const csr_t csr, bool external_automatic) {
  assert(!is_null());
  assert(!is_proposal);

  assert(!has_pushed_up);
  if (!external_automatic) {
    has_pushed_up = true;
  }

  m_gtuple.csr = csr;
  has_decided = true;
}

void MyGCN::push_up_sys_gcn() {
  if (!has_pushed_up) {
    innodb_set_gcn_if_bigger(m_gtuple.gcn);
    has_pushed_up = true;
  }
}
