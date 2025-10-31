### Purpose ###
#
# Check whether the specified table has flashback_area option.
#
### Parameters ###
#
# $SCHEMA_NAME
# $TABLE_NAME
# $PARTITION_NAME
# $CONTAIN_FBA        1 if the specified table should have fba, 0 otherwise
 
# Execute the query and store the result into a user variable
--disable_result_log
--disable_query_log

--let $options = query_get_value(SELECT options FROM information_schema.innodb_table_status WHERE SCHEMA_NAME = '$SCHEMA_NAME' AND TABLE_NAME = '$TABLE_NAME', options, 1)
if($PARTITION_NAME){
  --let $options = query_get_value(SELECT options FROM information_schema.innodb_table_status WHERE SCHEMA_NAME = '$SCHEMA_NAME' AND TABLE_NAME = '$TABLE_NAME' AND PARTITION_NAME = '$PARTITION_NAME', options, 1)
}

# Extract the fba value from the options
--let $fba_value=0
--let $fba_pos = `SELECT LOCATE('flashback_area=', '$options')`
if ($fba_pos > 0) {
  --let $fba_value = `SELECT CAST(SUBSTRING_INDEX(SUBSTRING_INDEX('$options', 'flashback_area=', -1), ';', 1) AS UNSIGNED)`
} 
 
# Assert that the fba option is equal to CONTAIN_FBA
--let $assert_text = The options for $SCHEMA_NAME.$TABLE_NAME.$PARTITION_NAME should match the expected FBA=$CONTAIN_FBA condition
--let $assert_cond = $fba_value = $CONTAIN_FBA
--source include/assert.inc
 
# Clean up
--enable_result_log
--enable_query_log