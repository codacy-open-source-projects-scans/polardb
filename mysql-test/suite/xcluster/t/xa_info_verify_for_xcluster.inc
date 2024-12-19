
--let $save_expect_gcn= $expect_gcn
--let $save_expect_status= $expect_status
--let $save_expect_csr= $expect_csr
--let $save_n_branch= $expect_n_branch
--let $save_n_local_branch= $expect_n_local_branch
--let $save_xid= $my_xid

if ($skip_leader) {
  --connection conn1
# --source include/wait_follower_catchup.inc
  select ROLE from information_schema.ALISQL_CLUSTER_LOCAL;
  --let $expect_gcn= $save_expect_gcn
  --let $expect_status= $save_expect_status
  --let $expect_csr= $save_expect_csr
  --let $expect_n_branch= $save_n_branch
  --let $expect_n_local_branch= $save_n_local_branch
  --let $my_xid= $save_xid
  --source mysql-test/suite/innodb_gcn/include/xa_info_verify.inc
}

--connection conn2
select ROLE from information_schema.ALISQL_CLUSTER_LOCAL;
--source include/wait_apply_catchup.inc
--let $expect_gcn= $save_expect_gcn
--let $expect_status= $save_expect_status
--let $expect_csr= $save_expect_csr
--let $expect_n_branch= $save_n_branch
--let $expect_n_local_branch= $save_n_local_branch
--let $my_xid= $save_xid
--source suite/innodb_gcn/include/xa_info_verify.inc

--connection conn3
select ROLE from information_schema.ALISQL_CLUSTER_LOCAL;
--source include/wait_apply_catchup.inc
--let $expect_gcn= $save_expect_gcn
--let $expect_status= $save_expect_status
--let $expect_csr= $save_expect_csr
--let $expect_n_branch= $save_n_branch
--let $expect_n_local_branch= $save_n_local_branch
--let $my_xid= $save_xid
--source suite/innodb_gcn/include/xa_info_verify.inc
