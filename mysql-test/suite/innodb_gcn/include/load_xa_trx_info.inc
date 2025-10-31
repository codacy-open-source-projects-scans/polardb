# ==== Purpose ====
# Load xa trx info by dbms_xa.find_by_xid(...)

# ==== Usage ====
# --let $my_xid='1','',1

--disable_ps_protocol
--let $actual_status= query_get_value("call dbms_xa.find_by_xid($my_xid)", STATUS, 1)

--let $actual_gcn= query_get_value("call dbms_xa.find_by_xid($my_xid)", GCN, 1)

--let $actual_csr= query_get_value("call dbms_xa.find_by_xid($my_xid)", CSR, 1)

--let $actual_trx_id= query_get_value("call dbms_xa.find_by_xid($my_xid)", TRX_ID, 1)

--let $actual_uba= query_get_value("call dbms_xa.find_by_xid($my_xid)", UBA, 1)

--let $actual_m_trx_id= query_get_value("call dbms_xa.find_by_xid($my_xid)", MASTER_TRX_ID, 1)

--let $actual_m_uba= query_get_value("call dbms_xa.find_by_xid($my_xid)", MASTER_UBA, 1)

--let $actual_n_branch= query_get_value("call dbms_xa.find_by_xid($my_xid)", N_BRANCH, 1)

--let $actual_n_local_branch= query_get_value("call dbms_xa.find_by_xid($my_xid)", N_LOCAL_BRANCH, 1)
--enable_ps_protocol
