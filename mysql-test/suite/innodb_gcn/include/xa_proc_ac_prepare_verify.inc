# ==== Purpose ====
# Check that the result of the query from dbms_xa.ac_prepare(...) is as expected.
# And also load result from ac_prepare into variables.

# ==== Usage ====
# --let $expect_uuid= 9738a7c0-44b5-11ef-ab06-b8cef6e45ffc
# --let $expect_gcn=10000
# --let $expect_csr= ASSIGNED_GCN
# --let $my_xid='1','',1
# --let $ac_args=1,1,$pre_commit_gcn
# --let $ac_conn=con1

# Parameters
# $expect_gcn
#   If set, gcn is checked to see if it is as expected.
# $expect_csr
#   if set, CSR(AUTOMATIC_GCN/ASSIGNED_GCN) is checked to see if it is as expected.
# $my_xid
#   XID.
# $ac_args
#   N_BRANCH, N_LOCAL_BRANCH, PRE_COMMIT_GCN

--echo call dbms_xa.ac_prepare(#gtrid, #bqual, #formatID, #N_BRANCH, #N_LOCAL_BRANCH, #gcn)
--disable_query_log
--output $MYSQLTEST_VARDIR/tmp/ac_prepare.log
--eval call dbms_xa.ac_prepare($my_xid, $ac_args)

--disable_connect_log
--connect(tmp_find_session, localhost, root,,)
connection tmp_find_session;

CREATE TEMPORARY TABLE temp_table (
    col_uuid VARCHAR(255),
    col_trx_id BIGINT,
    col_uba BIGINT,
    col_gcn BIGINT,
    col_csr VARCHAR(255)
);

eval LOAD DATA INFILE '$MYSQLTEST_VARDIR/tmp/ac_prepare.log'
INTO TABLE temp_table
FIELDS TERMINATED BY '\t'
LINES TERMINATED BY '\n'
IGNORE 1 LINES;

--let $ac_prepare_uuid = `select col_uuid from temp_table`
--let $ac_prepare_trx_id = `select col_trx_id from temp_table`
--let $ac_prepare_uba = `select col_uba from temp_table`
--let $ac_prepare_gcn = `select col_gcn from temp_table`
--let $ac_prepare_csr = `select col_csr from temp_table`
drop table temp_table;
--remove_file $MYSQLTEST_VARDIR/tmp/ac_prepare.log
--enable_query_log

if ($expect_uuid) {
  --let $actual_uuid = $ac_prepare_uuid
  --let $assert_text=expect_uuid == actual_uuid
  --let $assert_cond="$expect_uuid" = "$actual_uuid"
  --source include/assert.inc
}

if ($expect_gcn) {
  --let $actual_gcn = $ac_prepare_gcn
  --let $assert_text=expect_gcn == actual_gcn
  --let $assert_cond=$expect_gcn = $actual_gcn
  --source include/assert.inc
}

if ($expect_csr) {
  --let $actual_csr = $ac_prepare_csr
  --let $assert_text=expect_csr == actual_csr
  --let $assert_cond="$expect_csr" = "$actual_csr"
  --source include/assert.inc
}

connection $ac_conn;

# reset
--let $my_xid=
--let $ac_args=
--let $ac_conn=

--let $expect_uuid=
--let $expect_gcn=
--let $expect_csr=

--disconnect tmp_find_session

--enable_connect_log
