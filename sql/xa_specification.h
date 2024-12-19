/*****************************************************************************

Copyright (c) 2013, 2023, Alibaba and/or its affiliates. All Rights Reserved.

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

/** @file sql/xa_specification.h

  Transaction coordinator log structure specification.

  Created 2023-06-14 by Jianwei.zhao
 *******************************************************/

#ifndef SQL_XA_SPECIFICATION_H
#define SQL_XA_SPECIFICATION_H

#include "sql/lizard/lizard_service.h"  // MyGCN...
#include "sql/xa.h"

/**
 * Transaction coordinator should write log to complete XA,
 * except of xid for all TC_LOG category,  Other specifications are maybe
 * record according to different implemention.
 */

/**
 * transaction coordinator log specification, except of xid.
 */
class XA_specification {
 public:
  XA_specification() : m_gcn(), m_xa_branch(), m_xa_maddr() {}

  virtual ~XA_specification() {}

  virtual std::string print() { return m_gcn.print(); }

  virtual void clear() { m_gcn.reset(); }

  XA_specification(const XA_specification &other) {
    m_gcn = other.gcn();
    m_xa_branch = other.xa_branch();
    m_xa_maddr = other.xa_maddr();
  }

  XA_specification &operator=(const XA_specification &other) {
    if (this != &other) {
      m_gcn = other.gcn();
      m_xa_branch = other.xa_branch();
      m_xa_maddr = other.xa_maddr();
    }
    return *this;
  }

  const MyGCN &gcn() const { return m_gcn; }

  const xa_addr_t xa_maddr() const { return m_xa_maddr; }

  const xa_branch_t xa_branch() const { return m_xa_branch; }

  void set_when_recovery(const MyGCN &gcn, const xa_branch_t &xa_branch) {
    m_gcn = gcn;
    m_xa_branch = xa_branch;
    m_xa_maddr.reset();
  }

  void set_when_commit(const MyGCN &gcn, const xa_addr_t &maddr) {
    m_gcn = gcn;
    m_xa_branch.reset();
    m_xa_maddr = maddr;
  }

  [[nodiscard]] virtual XA_specification *clone() const {
    return new XA_specification(*this);
  }

 private:
  /** All TC_LOGs can have GCN. */
  MyGCN m_gcn;
  /** XA branch info of a XA transaction */
  xa_branch_t m_xa_branch;
  /** Master address of a XA transaction */
  xa_addr_t m_xa_maddr;
};

#endif
