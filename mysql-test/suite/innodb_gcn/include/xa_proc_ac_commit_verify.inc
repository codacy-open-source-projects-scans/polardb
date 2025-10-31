# ==== Purpose ====
# Check that the result of the query from dbms_xa.ac_commit(...) is as expected.
# And also load and return result after ac_commit into variables.

# ==== Usage ====
# --let $my_xid='1','',1
# --let $m_phyid= 32412, 123204923908
# --let $expect_gcn= 62490
# --let $expect_csr= AUTOMATIC_GCN
# --let $server_uuid= 9738a7c0-44b5-11ef-ab06-b8cef6e45ffc
# --let $expect_m_trx_id= 32412
# --let $expect_m_uba= 123204923908

echo call dbms_xa.ac_commit(#gtrid, #bqual, #formatID, #server_uuid,
  #master_trx_id, #master_uba, #gcn);
--disable_query_log
--eval call dbms_xa.ac_commit($my_xid, $expect_gcn, "$server_uuid", $m_phyid)
--enable_query_log

--let $expect_status= COMMIT
--source suite/innodb_gcn/include/xa_info_verify.inc

