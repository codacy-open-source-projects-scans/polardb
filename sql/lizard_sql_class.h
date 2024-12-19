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

/** @file sql/lizard_sql_class.h

  lizard sql class.

  Created 2024-07-24 by Jiyang.zhang
 *******************************************************/

#ifndef SQL_LIZARD_SQL_CLASS_INCLUDED
#define SQL_LIZARD_SQL_CLASS_INCLUDED

#include "my_inttypes.h"

#include "sql/lizard/lizard_service.h"

class THD;

namespace lizard {
class GCN_context_backup {
 public:
  GCN_context_backup(THD *thd);

  ~GCN_context_backup();

 private:
  THD *m_thd;

  ulonglong m_innodb_snapshot_gcn_var;
  ulonglong m_innodb_commit_gcn_var;
  ulonglong m_innodb_current_snapshot_gcn_var;
  bool m_opt_query_via_flashback_area_var;

  MyGCN m_owned_commit_gcn;
  MyVisionGCN m_owned_vision_gcn;
  xa_branch_t m_owned_xa_branch;
  xa_addr_t m_owned_master_addr;
};
}  // namespace lizard

#endif
