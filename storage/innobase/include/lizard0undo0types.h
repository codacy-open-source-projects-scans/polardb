/*****************************************************************************

Copyright (c) 2013, 2020, Alibaba and/or its affiliates. All Rights Reserved.

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

/** @file include/lizard0undo0types.h
  Lizard transaction undo and purge types.

 Created 2020-04-02 by Jianwei.zhao
 *******************************************************/

#ifndef lizard0undo0types_h
#define lizard0undo0types_h

#include "sql/handler.h"

#include "trx0types.h"

#include "lizard0scn0types.h"
#include "lizard0txn.h"
#include "lizard0txn0service.h"

#include "sql/lizard/lizard_service.h"

struct trx_rseg_t;
struct trx_undo_t;

/**
  Lizard transaction system undo format:

  At the end of undo log header history node:

  8 bytes     SCN number
  8 bytes     UTC time

  Those two option will be included into all INSERT/UPDATE/TXN undo
  log header.


  Start from undo log old header, txn_undo will be different with trx_undo:

  1) txn undo : flag + reserved space

  2) trx undo : XA + GTID

  As the optional info, those will be controlled by TRX_UNDO_FLAGS.

     0x01 TRX_UNDO_FLAG_XID
     0x02 TRX_UNDO_FLAG_GTID
     0x80 TRX_UNDO_FLAG_TXN
*/

/** Those will exist all kinds of undo log header*/
/*-------------------------------------------------------------*/
/** Size of scn within undo log header */
#define TRX_UNDO_SCN_LEN 8

/** Size of UTC within undo log header */
#define TRX_UNDO_UTC_LEN 8

/** Size of UBA within undo log header */
#define TRX_UNDO_UBA_LEN 8
/*-------------------------------------------------------------*/

/** Flag determine that if it is active in UBA */
/*-------------------------------------------------------------*/
/**  */
#define UNDO_ADDR_T_ACTIVE 0
#define UNDO_ADDR_T_COMMITED 1
/*-------------------------------------------------------------*/

/**
 * Transaction slot address:
 */
struct slot_addr_t {
  /* undo tablespace id */
  space_id_t space_id;
  /* undo log header page */
  page_no_t page_no;
  /* offset of undo log header */
  ulint offset;

 public:
  slot_addr_t() : space_id(0), page_no(0), offset(0) {}

  slot_addr_t(space_id_t space_id_arg, page_no_t page_no_arg, ulint offset_arg)
      : space_id(space_id_arg), page_no(page_no_arg), offset(offset_arg) {}

  bool is_null() const;
  /** Normal txn undo allocated from txn undo space. */
  bool is_redo() const;
  /** Special fake address if didn't allocate txn undo */
  bool is_no_redo() const;

  bool equal_with(space_id_t space_id_arg, page_no_t page_no_arg,
                  ulint offset_arg) {
    return space_id == space_id_arg && page_no == page_no_arg &&
           offset == offset_arg;
  }
};

typedef struct slot_addr_t slot_addr_t;

/** Compare function */
inline bool operator==(const slot_addr_t &lhs, const slot_addr_t &rhs) {
  return (lhs.offset == rhs.offset && lhs.page_no == rhs.page_no &&
          lhs.space_id == rhs.space_id);
}

/**
  Format of transaction slot address:

   2  bit     has been used since of UBA.
   7  bit     reserved unused
   7  bit     undo space number (1-127)
   32 bit     page no (4 bytes)
   16 bit     Offset of undo log header (2 bytes)
*/

#define SLOT_POS_OFFSET 0
#define SLOT_WIDTH_OFFSET 16

#define SLOT_POS_PAGE_NO (SLOT_POS_OFFSET + SLOT_WIDTH_OFFSET)
#define SLOT_WIDTH_PAGE_NO 32

#define SLOT_POS_SPACE_ID (SLOT_POS_PAGE_NO + SLOT_WIDTH_PAGE_NO)
#define SLOT_WIDTH_SPACE_ID 7

#define SLOT_POS_UNUSED (SLOT_POS_SPACE_ID + SLOT_WIDTH_SPACE_ID)
#define SLOT_WIDTH_UNUSED 7

/** Undo block address (UBA) */
struct undo_addr_t {
  /* undo tablespace id */
  space_id_t space_id;
  /* undo log header page */
  page_no_t page_no;
  /* offset of undo log header */
  ulint offset;
  /* Active or Commit state */
  bool state;
  /** Commit number source for gcn */
  csr_t csr;
  /** Whether xa branch is slave */
  bool is_slave;

 public:
  undo_addr_t(const slot_addr_t &slot_addr, bool state_arg, csr_t csr_arg)
      : space_id(slot_addr.space_id),
        page_no(slot_addr.page_no),
        offset(slot_addr.offset),
        state(state_arg),
        csr(csr_arg),
        is_slave(false) {}

  undo_addr_t()
      : space_id(0),
        page_no(0),
        offset(0),
        state(false),
        csr(CSR_AUTOMATIC),
        is_slave(false) {}
};

typedef struct undo_addr_t undo_addr_t;

/**
  New record format will include SCN and UBA:

  1) Format of scn in record:

   64 bit     scn number (8 bytes);

  2) Format of undo log address in record:

   1  bit     active/commit state (0:active 1:commit)
   1  bit     commit source
   1  bit     share commit number
   6  bit     reserved unused
   7  bit     undo space number (1-127)
   32 bit     page no (4 bytes)
   16 bit     Offset of undo log header (2 bytes)
*/

#define UBA_POS_OFFSET 0
#define UBA_WIDTH_OFFSET 16

#define UBA_POS_PAGE_NO (UBA_POS_OFFSET + UBA_WIDTH_OFFSET)
#define UBA_WIDTH_PAGE_NO 32

#define UBA_POS_SPACE_ID (UBA_POS_PAGE_NO + UBA_WIDTH_PAGE_NO)
#define UBA_WIDTH_SPACE_ID 7

#define UBA_POS_UNUSED (UBA_POS_SPACE_ID + UBA_WIDTH_SPACE_ID)
#define UBA_WIDTH_UNUSED 6

#define UBA_POS_IS_SLAVE (UBA_POS_UNUSED + UBA_WIDTH_UNUSED)
#define UBA_WIDTH_IS_SLAVE 1

#define UBA_MASK_IS_SLAVE ((~(~0ULL << UBA_WIDTH_IS_SLAVE)) << UBA_POS_IS_SLAVE)

#define UBA_POS_CSR (UBA_POS_IS_SLAVE + UBA_WIDTH_IS_SLAVE)
#define UBA_WIDTH_CSR 1

#define UBA_MASK_CSR ((~(~0ULL << UBA_WIDTH_CSR)) << UBA_POS_CSR)

#define UBA_POS_STATE (UBA_POS_CSR + UBA_WIDTH_CSR)
#define UBA_WIDTH_STATE 1

#define UBA_MASK_STATE ((~(~0ULL << UBA_WIDTH_STATE)) << UBA_POS_STATE)

/** Address, include [offset, page_no, space_id] */
#define UBA_POS_ADDR 0
#define UBA_WIDTH_ADDR \
  (UBA_WIDTH_OFFSET + UBA_WIDTH_PAGE_NO + UBA_WIDTH_SPACE_ID)
#define UBA_MASK_ADDR ((~(~0ULL << UBA_WIDTH_ADDR)) << UBA_POS_ADDR)

static_assert((UBA_POS_STATE + UBA_WIDTH_STATE) == 64,
              "UBA length must be 8 bytes");

static_assert(UBA_POS_PAGE_NO == 16, "UBA page no from 16th bits");

static_assert(UBA_POS_SPACE_ID == 48, "UBA space id from 48th bits");

/** Undo log header address in record */
typedef ib_id_t undo_ptr_t;

/** NULL value of slot ptr  */
constexpr undo_ptr_t UNDO_PTR_NULL = std::numeric_limits<undo_ptr_t>::min();

inline bool undo_ptr_is_active(const undo_ptr_t &undo_ptr) {
  return !static_cast<bool>((undo_ptr & UBA_MASK_STATE) >> UBA_POS_STATE);
}

inline csr_t undo_ptr_get_csr(const undo_ptr_t &undo_ptr) {
  return static_cast<csr_t>((undo_ptr & UBA_MASK_CSR) >> UBA_POS_CSR);
}

inline bool undo_ptr_is_slave(const undo_ptr_t &undo_ptr) {
  return static_cast<bool>((undo_ptr & UBA_MASK_IS_SLAVE) >> UBA_POS_IS_SLAVE);
}

inline void undo_ptr_set_commit(undo_ptr_t *undo_ptr, unsigned int csr,
                                bool is_slave) {
  *undo_ptr |= ((undo_ptr_t)1 << UBA_POS_STATE);

  undo_ptr_t value = static_cast<undo_ptr_t>(csr);
  *undo_ptr |= (value << UBA_POS_CSR);

  value = static_cast<undo_ptr_t>(is_slave);
  *undo_ptr |= (value << UBA_POS_IS_SLAVE);
}

/** Retrieve slot address from undo address */
inline slot_ptr_t undo_ptr_get_slot(const undo_ptr_t &undo_ptr) {
  return ((undo_ptr & UBA_MASK_ADDR) >> UBA_POS_ADDR);
}

inline bool undo_ptr_is_slot(const undo_ptr_t &undo_ptr) {
  return !(undo_ptr >> UBA_WIDTH_ADDR);
}


/** Scn in record */
typedef scn_t scn_id_t;

/**
  XA branch info structure:
  {n_globals, n_locals}
*/

struct xes_tags_t {
  bool is_rollback;
  csr_t csr;
};

/**
  The transaction description:

  It will be inited when allocate the first txn undo log
  header, and never change until transaction commit or rollback.
*/
struct txn_desc_t {
 public:
  /** undo log header address */
  undo_ptr_t undo_ptr;
  /** scn number */
  commit_mark_t cmmt;
  /** proposal commit number. */
  proposal_mark_t pmmt;
  /** branch info */
  xa_branch_t branch;
  /** Master txn address for async commit. */
  xa_addr_t maddr;

 public:
  txn_desc_t() : undo_ptr(UNDO_PTR_NULL), cmmt(), pmmt(), branch(), maddr() {}

  void reset() {
    undo_ptr = UNDO_PTR_NULL;
    cmmt.reset();
    pmmt.reset();
    branch.reset();
    maddr.reset();
  }


  /** assemble cmmt and undo ptr */
  void assemble(const commit_mark_t &mark, const slot_addr_t &slot_addr);

  /** assemble undo ptr */
  void assemble_undo_ptr(const slot_addr_t &slot_addr);

  void resurrect_xa(const proposal_mark_t &pmmt, const xa_branch_t &branch,
                    const xa_addr_t &maddr);

  void copy_xa_when_prepare(const MyGCN &xa_gcn, const xa_branch_t &xa_branch);

  void copy_xa_when_commit(const MyGCN &xa_gcn, const xa_addr_t &xa_maddr);
};

/**
  Lizard transaction attributes in record (used by Vision)
   1) trx_id
   2) scn
   3) undo_ptr
*/
struct txn_rec_t {
 public:
  /* trx id */
  trx_id_t trx_id;
  /** scn number */
  scn_id_t scn;
  /** undo log header address */
  undo_ptr_t undo_ptr;

  /**
    Although gcn isn't saved on record, but Global query still use gcn as
    visible judgement, and it can be retrieved by txn undo header, so defined
    gcn as txn record attribute.
  */
  /** Revision: Persist gcn into record */
  gcn_t gcn;

 public:
  txn_rec_t()
      : trx_id(0), scn(SCN_NULL), undo_ptr(UNDO_PTR_NULL), gcn(GCN_NULL) {}

  txn_rec_t(trx_id_t trx_id_arg, scn_id_t scn_arg, undo_ptr_t undo_ptr_arg,
            gcn_t gcn_arg)
      : trx_id(trx_id_arg),
        scn(scn_arg),
        undo_ptr(undo_ptr_arg),
        gcn(gcn_arg) {}

  bool is_committed() const {
    if (!undo_ptr_is_active(undo_ptr)) {
      ut_ad(scn != SCN_NULL && gcn != GCN_NULL && trx_id != 0);
      return true;
    } else {
      /** Active trx didn't known Commit Info. */
      ut_ad(scn == SCN_NULL && gcn == GCN_NULL &&
            undo_ptr_get_csr(undo_ptr) == CSR_AUTOMATIC);
      return false;
    }
  }

  bool is_active() const { return !is_committed(); }

  bool is_null() const {
    if (trx_id == 0) {
      ut_ad(scn == SCN_NULL && gcn == GCN_NULL && undo_ptr == UNDO_PTR_NULL);
      return true;
    }
    return false;
  }

  void reset() {
    trx_id = 0;
    scn = SCN_NULL;
    undo_ptr = UNDO_PTR_NULL;
    gcn = GCN_NULL;
  }

  slot_ptr_t slot() const { return undo_ptr_get_slot(undo_ptr); }
  csr_t csr() const { return undo_ptr_get_csr(undo_ptr); }
};

/**
  Lizard transaction attributes in undo log record
   1) scn
   2) undo_ptr
   3) gcn
*/
struct txn_info_t {
  /** scn number */
  scn_id_t scn;
  /** undo log header address */
  undo_ptr_t undo_ptr;
  /** gcn number */
  gcn_t gcn;
};

/**
  Lizard transaction attributes in index (used by Vision)
   1) scn
   2) undo_ptr
   3) gcn
*/
struct txn_index_t {
  /** undo log header address */
  std::atomic<undo_ptr_t> uba;
  /** scn number */
  std::atomic<scn_id_t> scn;
  /** gcn number */
  std::atomic<gcn_t> gcn;
};

/** The struct of transaction undo for UBA */
struct txn_undo_ptr_t {
  // XID will be actively and explicitly initialized
  txn_undo_ptr_t() : rseg(nullptr), txn_undo(nullptr), xid_for_hash() {}
  /** Rollback segment in txn space */
  trx_rseg_t *rseg;
  /* transaction undo log segment */
  trx_undo_t *txn_undo;
  /** XID that is used to map rseg, and also will be persisted in TXN undo */
  XID xid_for_hash;
};

struct txn_slot_t {
 public:
  txn_slot_t()
      : image(),
        slot_ptr(0),
        trx_id(0),
        magic_n(0),
        prev_image(),
        state(0),
        xes_storage(0),
        tags(0),
        is_2pp(false),
        pmmt(),
        branch(),
        maddr() {}

  txn_slot_t(commit_mark_t image_arg, slot_ptr_t slot_ptr_arg,
             trx_id_t trx_id_arg, ulint magic_n_arg,
             commit_mark_t prev_image_arg, ulint state_arg,
             ulint xes_storage_arg, ulint tags_arg, bool is_2pp_arg,
             proposal_mark_t pmmt_arg, xa_branch_t branch_arg,
             xa_addr_t addr_arg)
      : image(image_arg),
        slot_ptr(slot_ptr_arg),
        trx_id(trx_id_arg),
        magic_n(magic_n_arg),
        prev_image(prev_image_arg),
        state(state_arg),
        xes_storage(xes_storage_arg),
        tags(tags_arg),
        is_2pp(is_2pp_arg),
        pmmt(pmmt_arg),
        branch(branch_arg),
        maddr(addr_arg) {}

  /** commit image in txn undo header */
  commit_mark_t image;
  /** slot address */
  slot_ptr_t slot_ptr;
  /** current trx who own the txn header */
  trx_id_t trx_id;
  /** A magic number, check if the page is corrupt */
  ulint magic_n;
  /* Previous scn/utc of the trx who used the same TXN */
  commit_mark_t prev_image;
  /** txn undo header state: TXN_UNDO_LOG_ACTIVE, TXN_UNDO_LOG_COMMITED,
  or TXN_UNDO_LOG_PURGED */
  ulint state;
  /** A flag determining how to explain the txn extension */
  ulint xes_storage;
  /** flags of the TXN. For example: 0x01 means rollback. */
  ulint tags;
  /** true if the TXN is two phase purge. */
  bool is_2pp;
  /** AC XA PMMT info */
  proposal_mark_t pmmt;
  /** XA branch count info */
  xa_branch_t branch;
  /** XA master branch info */
  xa_addr_t maddr;
  /** Return true if the transaction was eventually rolled back. */
  bool is_rollback() const;
  /** Return true if the txn has new_flags. */
  bool tags_allocated() const;
  bool ac_prepare_allocated() const;
  bool ac_commit_allocated() const;
};

struct txn_lookup_t {
 public:
  /**
    Unlike normal UNDOs (insert undo / update undo), there are 5 kinds of states
    of TXN. Among them, Status::ACTIVE, Status::COMMITTED and Status::PURGED
    are specified by TXN_UNDO_LOG_STATE flag (respectively, TXN_UNDO_LOG_ACTIVE,
    TXN_UNDO_LOG_COMMITED and TXN_UNDO_LOG_PURGED) in TXN header. And also, that's
    mean these TXN headers are existing.

    By contrast, Status::REUSE / Status::UNDO_CORRUPTED mean that the TXN
    headers are non-existing.

    * State::ACTIVE: A txn header is initialized as Status::ACTIVE when the
    transaction begins.

    * Status::COMMITTED: The state of txn header is set as Status::COMMITTED
    at the moment that the transaction commits.

    * Status::PURGED: At the moment that the purge sys start purging it. Notes
    that: Access to the binding normal UNDOs (insert undo / update undo) is not
    safe from then on.

    * Status::ERASED: At the moment that the erase sys start erasing it. Notes
    that: Access to the binding normal UNDOs (insert undo / update undo) is not
    safe for two phase purge tables from then on.

    * Status::REUSE: At the moment that the TXN headers are reused by another
    transactions. These TXN headers are reinited as Status::ACTIVE, but for
    those UBAs who also pointed at them, are supposed to be Status::REUSE.

    * Status::UNDO_CORRUPTED: In fact, Status::REUSE also lost their TXN
    headers, but Status::UNDO_CORRUPTED is a abnormal state for some special
    cases, for example, page corrupt or TXN file unexpectedly removed.

    So the life cycle of TXN hedaer:

    Status::ACTIVE (Trx_A) ==> Status::COMMITTED (Trx_A) ==>
      Status::PURGED (Trx_A) ==> { (Status::ERASED) (Trx_A) ==> }
        * Status::REUSE  (from Trx_A's point of view)
        * Status::ACTIVE (from Trx_B's point of view)
  */
  enum Status : char {
    ACTIVE,
    COMMITTED,
    PURGED,
    ERASED,
    REUSE,
    UNDO_CORRUPTED,
  };

 public:
  txn_lookup_t()
      : txn_slot(),
        real_image(),
        real_status(Status::ACTIVE) {}
  /** The raw data in txn slot. */
  txn_slot_t txn_slot;
  /**
    If the txn is still existing:
      * real_state: [Status::ACTIVE, Status::COMMITTED, Status::PURGED]
      * real_image == txn_slot.image

    If the txn is non-existing:
      * real_state: [Status::REUSE]
      * real_image == txn_slot.prev_image

    If the txn is unexpectedly lost:
      * real_state: [Status::UNDO_CORRUPTED]
      * real_image == {SCN_UNDO_CORRUPTED, US_UNDO_CORRUPTED}
  */
  commit_mark_t real_image;
  Status real_status;
};

typedef txn_lookup_t::Status txn_status_t;

namespace lizard {

/**
  The element of minimum heap for the purge.
*/
class TxnUndoRsegs {
 public:
  explicit TxnUndoRsegs(scn_t scn) : m_scn(scn) {
    for (auto &rseg : m_rsegs) {
      rseg = nullptr;
    }
  }

  /** Default constructor */
  TxnUndoRsegs() : TxnUndoRsegs(0) {}

  void set_scn(scn_t scn) { m_scn = scn; }

  scn_t get_scn() const { return m_scn; }

  /** Add rollback segment.
  @param rseg rollback segment to add. */
  void insert(trx_rseg_t *rseg) {
    for (size_t i = 0; i < m_rsegs_n; ++i) {
      if (m_rsegs[i] == rseg) {
        return;
      }
    }
    // ut_a(m_rsegs_n < 2);
    /* Lizard: one more txn rseg. */
    ut_a(m_rsegs_n < 2 + 1);
    m_rsegs[m_rsegs_n++] = rseg;
  }

  Rsegs_array<3>::iterator arrange_txn_first() {
    ut_ad(m_rsegs.size() > 0);

    auto iter = begin();
    while (iter != end()) {
      bool is_txn_rseg = fsp_is_txn_tablespace_by_id((*iter)->space_id);
      if (is_txn_rseg) {
        if (iter != begin()) {
          /* Move txn rseg to position 0 */
          std::swap(*iter, m_rsegs.front());
        }
        break;
      }
      ++iter;
    }

    /* If no txn rseg, then only one temp rseg */
    ut_ad(iter != end() || size() == 1);

    return begin();
  }

  /** Number of registered rsegs.
  @return size of rseg list. */
  size_t size() const { return (m_rsegs_n); }

  /**
  @return an iterator to the first element */
  typename Rsegs_array<3>::iterator begin() { return m_rsegs.begin(); }

  /**
  @return an iterator to the end */
  typename Rsegs_array<3>::iterator end() {
    return m_rsegs.begin() + m_rsegs_n;
  }

  /** Append rollback segments from referred instance to current
  instance. */
  void insert(const TxnUndoRsegs &append_from) {
    ut_ad(get_scn() == append_from.get_scn());
    for (size_t i = 0; i < append_from.m_rsegs_n; ++i) {
      insert(append_from.m_rsegs[i]);
    }
  }

  /** Compare two TxnUndoRsegs based on scn.
  @param lhs first element to compare
  @param rhs second element to compare
  @return true if elem1 > elem2 else false.*/
  bool operator()(const TxnUndoRsegs &lhs, const TxnUndoRsegs &rhs) {
    return (lhs.m_scn > rhs.m_scn);
  }

  /** Compiler defined copy-constructor/assignment operator
  should be fine given that there is no reference to a memory
  object outside scope of class object.*/

 private:
  scn_t m_scn;

  size_t m_rsegs_n{};

  /** Rollback segments of a transaction, scheduled for purge. */
  // Rsegs_array<2> m_rsegs;
  /* Lizard: one more txn rseg. */
  Rsegs_array<3> m_rsegs;
};

/**
  Use priority_queue as the minimum heap structure
  which is order by scn number */
typedef std::priority_queue<
    TxnUndoRsegs, std::vector<TxnUndoRsegs, ut::allocator<TxnUndoRsegs>>,
    TxnUndoRsegs>
    purge_heap_t;

/**
  The element of minimum heap for the erase sys.
*/
class UpdateUndoRseg {
 public:
  explicit UpdateUndoRseg(scn_t scn, trx_rseg_t *rseg)
      : m_scn(scn), m_rseg(rseg) {
    ut_ad(!m_rseg || !m_rseg->is_txn);
  }

  /** Default constructor */
  UpdateUndoRseg() : UpdateUndoRseg(0, nullptr) {}

  void set_scn(scn_t scn) { m_scn = scn; }

  scn_t get_scn() const { return m_scn; }

  void set_rseg(trx_rseg_t *rseg) { m_rseg = rseg; }

  trx_rseg_t *get_rseg() const { return m_rseg; }

  /** Compare two UpdateUndoRseg based on scn.
  @param lhs first element to compare
  @param rhs second element to compare
  @return true if elem1 > elem2 else false.*/
  bool operator()(const UpdateUndoRseg &lhs, const UpdateUndoRseg &rhs) {
    return (lhs.m_scn > rhs.m_scn);
  }

 private:
  scn_t m_scn;

  /** Rollback segments of update undo. */
  trx_rseg_t *m_rseg;
};

/**
  Use priority_queue as the minimum heap structure
  which is order by scn number */
typedef std::priority_queue<
    UpdateUndoRseg, std::vector<UpdateUndoRseg, ut::allocator<UpdateUndoRseg>>,
    UpdateUndoRseg>
    erase_heap_t;

} /* namespace lizard */

struct txn_space_rseg_slot_t {
  ulint space_slot;
  ulint rseg_slot;
};

struct txn_cursor_t {
  /** trx_id that is used to check if the TXN is valid. */
  trx_id_t trx_id;

  /** TXN address */
  slot_addr_t txn_addr;

  txn_cursor_t() : trx_id(0), txn_addr() {}
};

#endif  // lizard0undo0types_h
