#@ Session: validating members
|Session Members: 13|
|create_cluster: OK|
|delete_sandbox_instance: OK|
|deploy_sandbox_instance: OK|
|get_cluster: OK|
|help: OK|
|kill_sandbox_instance: OK|
|reset_session: OK|
|start_sandbox_instance: OK|
|check_instance_config: OK|
|stop_sandbox_instance: OK|
|drop_metadata_schema: OK|
|config_local_instance: OK|
|verbose: OK|

#@# Dba: create_cluster errors
||Invalid number of arguments in Dba.create_cluster, expected 1 to 2 but got 0
||Invalid number of arguments in Dba.create_cluster, expected 1 to 2 but got 4
||Dba.create_cluster: Argument #1 is expected to be a string
||Dba.create_cluster: The Cluster name cannot be empty
||Dba.create_cluster: Invalid values in the options: another, invalid

#@<OUT> Dba: create_cluster with interaction
A new InnoDB cluster will be created on instance 'root@localhost:<<<__mysql_sandbox_port1>>>'.

Creating InnoDB cluster 'devCluster' on 'root@localhost:<<<__mysql_sandbox_port1>>>'...
Adding Seed Instance...

Cluster successfully created. Use Cluster.add_instance() to add MySQL instances.
At least 3 instances are needed for the cluster to be able to withstand up to
one server failure.

#@ Dba: check_instance_config error
|Please provide a password for 'root@localhost:<<<__mysql_sandbox_port1>>>':|Dba.check_instance_config: The instance 'root@localhost:<<<__mysql_sandbox_port1>>>' is already part of an InnoDB Cluster

#@<OUT> Dba: check_instance_config ok 1
Please provide a password for 'root@localhost:<<<__mysql_sandbox_port2>>>': Validating instance...

The instance 'localhost:<<<__mysql_sandbox_port2>>>' is valid for Cluster usage
{
    "status": "ok"
}

#@<OUT> Dba: check_instance_config ok 2
Validating instance...

The instance 'localhost:<<<__mysql_sandbox_port2>>>' is valid for Cluster usage
{
    "status": "ok"
}


#@<OUT> Dba: check_instance_config report with errors
Please provide a password for 'root@localhost:<<<__mysql_sandbox_port1>>>': Validating instance...

The instance 'localhost:<<<__mysql_sandbox_port1>>>' is not valid for Cluster usage.

The following issues were encountered:

 - Some configuration options need to be fixed.

+----------------------------------+---------------+----------------------------------------+------------------------+
| Variable                         | Current Value | Required Value                         | Note                   |
+----------------------------------+---------------+----------------------------------------+------------------------+
| binlog_checksum                  | <no value>    | NONE                                   | Update the config file |
| binlog_format                    | <no value>    | ROW                                    | Update the config file |
| disabled_storage_engines         | <no value>    | MyISAM,BLACKHOLE,FEDERATED,CSV,ARCHIVE | Update the config file |
| enforce_gtid_consistency         | <no value>    | ON                                     | Update the config file |
| gtid_mode                        | OFF           | ON                                     | Update the config file |
| log_slave_updates                | <no value>    | ON                                     | Update the config file |
| master_info_repository           | <no value>    | TABLE                                  | Update the config file |
| relay_log_info_repository        | <no value>    | TABLE                                  | Update the config file |
| report_port                      | <no value>    | <<<__mysql_sandbox_port1>>>                                   | Update the config file |
| transaction_write_set_extraction | <no value>    | XXHASH64                               | Update the config file |
+----------------------------------+---------------+----------------------------------------+------------------------+


Please fix these issues and try again.

#@ Dba: config_local_instance error 1
||Dba.config_local_instance: This function only works with local instances

#@<OUT> Dba: config_local_instance error 2
Please provide a password for 'root@localhost:<<<__mysql_port>>>': Please specify the path to the MySQL configuration file: 
The path to the MySQL Configuration is required to verify and fix the InnoDB Cluster settings

#@<OUT> Dba: config_local_instance error 3
Please provide a password for 'root@localhost:<<<__mysql_sandbox_port1>>>': 
Detected as sandbox instance.

Validating MySQL configuration file at: /home/rennox/mysql-sandboxes/<<<__mysql_sandbox_port1>>>/my.cnf
Validating instance...

#@<ERR> Dba: config_local_instance error 3
RuntimeError: Dba.config_local_instance: The instance 'root@localhost:<<<__mysql_sandbox_port1>>>' is already part of an InnoDB Cluster

#@<OUT> Dba: config_local_instance updating config file
Please provide a password for 'root@localhost:<<<__mysql_sandbox_port2>>>': Validating instance...

The instance 'localhost:<<<__mysql_sandbox_port2>>>' is valid for Cluster usage
You can now add it to an InnoDB Cluster with the <Cluster>.add_instance() function.

#@# Dba: get_cluster errors
||ArgumentError: Dba.get_cluster: Invalid cluster name: Argument #1 is expected to be a string
||Dba.get_cluster: The Cluster name cannot be empty

#@<OUT> Dba: get_cluster with interaction
<Cluster:devCluster>

#@<OUT> Dba: get_cluster with interaction (default)
<Cluster:devCluster>
