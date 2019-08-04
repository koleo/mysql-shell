/*
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms, as
 * designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MYSQLSHDK_LIBS_MYSQL_GROUP_REPLICATION_H_
#define MYSQLSHDK_LIBS_MYSQL_GROUP_REPLICATION_H_

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "mysql/instance.h"
#include "mysqlshdk/libs/config/config.h"
#include "mysqlshdk/libs/utils/nullable.h"
#include "mysqlshdk/libs/utils/utils_general.h"

#ifdef _WIN32
#undef ERROR
#endif

namespace mysqlshdk {
// TODO(.) namespace gr should probably be renamed to mysql
namespace gr {

static constexpr const char k_gr_plugin_name[] = "group_replication";
static constexpr const char k_group_recovery_user_prefix[] =
    "mysql_innodb_cluster_";
static constexpr const char k_group_recovery_old_user_prefix[] =
    "mysql_innodb_cluster_r";
static constexpr const char k_gr_applier_channel[] =
    "group_replication_applier";
static constexpr const char k_gr_recovery_channel[] =
    "group_replication_recovery";

/**
 * Enumeration of the supported states for Group Replication members.
 */
enum class Member_state {
  ONLINE,
  RECOVERING,
  OFFLINE,
  ERROR,
  UNREACHABLE,
  MISSING
};

enum class Member_role { PRIMARY, SECONDARY };

enum class Topology_mode { SINGLE_PRIMARY, MULTI_PRIMARY };

std::string to_string(const Member_state state);
Member_state to_member_state(const std::string &state);

std::string to_string(const Member_role role);
Member_role to_member_role(const std::string &role);

/**
 * Convert Topology_mode enumeration values to string.
 *
 * @param mode Topology_mode value to convert to string.
 * @return string representing the Topology_mode value.
 */
std::string to_string(const Topology_mode mode);

/**
 * Convert string to Topology_mode enumeration value.
 *
 * @param mode string to convert to Topology_mode value.
 * @return Topology_mode value resulting from the string conversion.
 */
Topology_mode to_topology_mode(const std::string &mode);

/**
 * Data structure representing a Group Replication member.
 */
struct Member {
  // Address of the member
  std::string host;
  // port of the member
  int port = 0;
  // member_id aka server_uuid of the member
  std::string uuid;
  // State of the member
  Member_state state = Member_state::MISSING;
  // Role of the member (primary vs secondary)
  Member_role role = Member_role::SECONDARY;
  // Version of the member
  std::string version;
};

enum class Gr_seeds_change_type {
  ADD,
  REMOVE,
  OVERRIDE,
};

// Function to check membership and state.
bool is_member(const mysqlshdk::mysql::IInstance &instance);
bool is_member(const mysqlshdk::mysql::IInstance &instance,
               const std::string &group_name);
Member_state get_member_state(const mysqlshdk::mysql::IInstance &instance);
std::vector<Member> get_members(const mysqlshdk::mysql::IInstance &instance,
                                bool *out_single_primary_mode = nullptr,
                                bool *out_has_quorum = nullptr,
                                std::string *out_group_view_id = nullptr);

bool is_primary(const mysqlshdk::mysql::IInstance &instance);

bool has_quorum(const mysqlshdk::mysql::IInstance &instance,
                int *out_unreachable, int *out_total);

// Fetch various basic info bits from the group the given instance is member of
bool get_group_information(const mysqlshdk::mysql::IInstance &instance,
                           Member_state *out_member_state,
                           std::string *out_member_id = nullptr,
                           std::string *out_group_name = nullptr,
                           bool *out_single_primary_mode = nullptr,
                           bool *out_has_quorum = nullptr,
                           bool *out_is_primary = nullptr);

std::string get_group_primary_uuid(const mysqlshdk::mysql::IInstance &instance,
                                   bool *out_single_primary_mode);

/**
 * Gets the Group Replication protocol version.
 *
 * This function retrieves the Group Replication protocol version currently in
 * use on the ReplicaSet
 *
 * @param instance Instance object that points to the server that will be used
 * to retrieve the protocol version.
 *
 * @return a mysqlshdk::utils::Version object containing the Group Replication
 * protocol version.
 */
mysqlshdk::utils::Version get_group_protocol_version(
    const mysqlshdk::mysql::IInstance &instance);

/**
 * Set the Group Replication protocol version.
 *
 * This function reconfigures the Group Replication protocol version currently
 * in use on the ReplicaSet, in runtime
 *
 * @param instance Instance object that points to the server that will be used
 * to reconfigure the protocol version.
 * @param version mysqlshdk::utils::Version object the Group Replication
 * protocol version to be set.
 */
void set_group_protocol_version(const mysqlshdk::mysql::IInstance &instance,
                                mysqlshdk::utils::Version version);

/**
 * Verify if a Group Replication protocol version downgrade is required
 *
 * This function verifies if a Group Replication protocol version downgrade is
 * required. This is necessary to verify if an instance that is joining a group
 * does not support the GR protocol version in use on the group (because it is
 * an older version) so the group protocol version must be downgraded
 *
 * @param version mysqlshdk::utils::Version object with the current GR protocol
 * version in used in the group
 * @param instance Instance object that points to the server that is joining the
 * cluster.
 */
bool is_protocol_downgrade_required(
    mysqlshdk::utils::Version current_group_version,
    const mysqlshdk::mysql::IInstance &instance);

/**
 * Verify if a Group Replication protocol version upgrade is required
 *
 * This function verifies if a Group Replication protocol version upgrade is
 * required. This is necessary to verify if a group needs to upgrade its
 * protocol version after the removal of the target instance.
 *
 * @param instance Instance object that points to the server that is leaving the
 * cluster or a member of the cluster
 * @param server_uuid nullable string containing the instance server_uuid to be
 * skipped.
 * @param out_protocol_version mysqlshdk::utils::Version object with the version
 * value that the group should be upgraded to.
 */
bool is_protocol_upgrade_required(
    const mysqlshdk::mysql::IInstance &instance,
    mysqlshdk::utils::nullable<std::string> server_uuid,
    mysqlshdk::utils::Version *out_protocol_version);

/**
 *
 * @param instance
 * @return map with all the group_replication variables and respective values
 * found on the provided instance.
 */
std::map<std::string, utils::nullable<std::string>> get_all_configurations(
    const mysqlshdk::mysql::IInstance &instance);

// Function to do a change master (set the GR recovery user)
void change_recovery_credentials(const mysqlshdk::mysql::IInstance &instance,
                                 const std::string &rpl_user,
                                 const std::string &rpl_pwd);

// Functions to manage the GR plugin
bool install_group_replication_plugin(
    const mysqlshdk::mysql::IInstance &instance,
    mysqlshdk::config::Config *config);
bool uninstall_group_replication_plugin(
    const mysqlshdk::mysql::IInstance &instance,
    mysqlshdk::config::Config *config);

void start_group_replication(const mysqlshdk::mysql::IInstance &instance,
                             const bool bootstrap,
                             const uint16_t read_only_timeout = 900);
void stop_group_replication(const mysqlshdk::mysql::IInstance &instance);

/**
 * Generate a UUID to use for the group name.
 *
 * The UUID is generated from the target instance (MySQL server) using the
 * UUID() SQL function.
 *
 * @param instance target instance used to generate the UUID.
 *
 * @return A string with a new UUID to be used for the group name.
 */
std::string generate_group_name(const mysqlshdk::mysql::IInstance &instance);

// Function to manage the replication (recovery) user for GR.
mysql::User_privileges_result check_replication_user(
    const mysqlshdk::mysql::IInstance &instance, const std::string &user,
    const std::string &host);

/**
 * Create a replication (recovery) user with the given name and required
 * privileges for Group Replication and a randomly generated password. NOTE: The
 * replication (recovery) user is always created at the primary instance with
 * disabled password expiration (see: BUG#28855764)
 *
 * @param username The name of the replication user we want to create
 * @param primary instance where the account will be created
 * @param hosts list of hosts that will be used for the user creation.
 * @param password password to be used for the account.
 *        If null a random password is assigned.
 * @return a Auth_options object with information about the created user
 * @throw std::runtime error if the replication user already exists on the
 * target instance with any of the given hosts or if an error occurs with the
 * user creation.
 */
mysqlshdk::mysql::Auth_options create_recovery_user(
    const std::string &username, const mysqlshdk::mysql::IInstance &primary,
    const std::vector<std::string> &hosts,
    const mysqlshdk::utils::nullable<std::string> &password,
    bool clone_supported = false);

std::string get_recovery_user(const mysqlshdk::mysql::IInstance &instance);

/**
 * Checks if the thread for a delayed initialization of the group replication is
 * currently running on the given instance.
 *
 * @param instance Instance to be checked.
 *
 * @return True if group replication is currently being initialized.
 */
bool is_group_replication_delayed_starting(
    const mysqlshdk::mysql::IInstance &instance);

/**
 * Check if the specified instance address corresponds to an active member from
 * the perspective of the given instance.
 *
 * The instance is considered active if it belongs to the group and it is not
 * OFFLINE or UNREACHABLE.
 *
 * @param instance Instance to use to perform the check.
 * @param host string with the host of the instance to check.
 * @param port int with the port of the instance to check.
 *
 * @return a boolean value indicating if the instance is an active member of the
 *         Group Replication group (true) or not (false).
 */
bool is_active_member(const mysqlshdk::mysql::IInstance &instance,
                      const std::string &host, const int port);

/**
 * Update auto-increment setting based on the GR mode.
 *
 * IMPORTANT NOTE: It is assumed that the Config object used as parameter
 * contains one and only one configuration handler for each server in the GR
 * group.
 *
 * @param config Config object used to set the auto-increment settings on all
 *               servers.
 * @param topology_mode target topology mode to determine how auto-increment
 *                      settings should be set.
 * @param group_size Size of the group to consider when setting auto-increment
 *                   values for multi-primary topologies. Only used if different
 *                   from 0 (by default 0), otherwise the group size is
 *                   determined based on the number of handlers in the Config
 *                   object.
 */
void update_auto_increment(mysqlshdk::config::Config *config,
                           const Topology_mode &topology_mode,
                           uint64_t group_size = 0);

/**
 * Update Group Replication group seeds on the group members.
 *
 * IMPORTANT NOTE: It is assumed that the Config object used as parameter
 * contains one and only one configuration handler for each member in the GR
 * group.
 *
 * @param config Config object used to set the GR group_seeds
 *               on all servers.
 * @param gr_address string with the input GR address used to update the current
 *                   group_seeds value.
 * @param change_type Enumeration with the type of change that will be
 *                    performed to the group_seeds (ADD, REMOVE, or OVERRIDE).
 */
void update_group_seeds(mysqlshdk::config::Config *config,
                        const std::string &gr_address,
                        Gr_seeds_change_type change_type);

/**
 * Configure which member of a single-primary replication group is the primary
 *
 * @param instance Instance to run the operations.
 * @param uuid server_uuid of the member that shall become the new primary.
 */
void set_as_primary(const mysqlshdk::mysql::IInstance &instance,
                    const std::string &uuid);

/**
 * Changes a group running in single-primary mode to multi-primary mode
 *
 * @param instance Instance to run the operations.
 */
void switch_to_multi_primary_mode(const mysqlshdk::mysql::IInstance &instance);

/**
 * Changes a group running in multi-primary mode to single-primary mode
 *
 * @param instance Instance to run the operations.
 * @param A string containing the UUID of a member of the group which should
 * become the new single primary
 */
void switch_to_single_primary_mode(const mysqlshdk::mysql::IInstance &instance,
                                   const std::string &uuid = "");

/**
 * Checks if the thread for the group-replication auto-rejoin procedure is
 * currently running on the given instance.
 *
 * @param instance Instance to use to perform the check.
 *
 * @return a boolean value indicating if the instance is running auto-rejoin
 * (true) or not (false).
 */
bool is_running_gr_auto_rejoin(const mysqlshdk::mysql::IInstance &instance);

/**
 * Check the instance version compatibility to join Group Replication.
 *
 *  The compatibility of the instance version is determined based on the
 *  instance group_replication_allow_local_lower_version_join value and its
 *  version compared to the lowest version of the members in the cluster.
 *
 *  For more information see:
 *  WL#13084 (AdminAPI: Handling of cross-version policies)
 *
 * @param instance Instance to use to perform the check.
 * @param lowest_cluster_version Version object with the lowest server version
 *                               in the cluster to join.
 */
void check_instance_version_compatibility(
    const mysqlshdk::mysql::IInstance &instance,
    mysqlshdk::utils::Version lowest_cluster_version);

/**
 * Check if the instance is only read compatible.
 *
 * Instance that are exclusively read compatible can join the cluster but only
 * in read-only mode, even if joining a multi-primary cluster. This can be
 * determined by comparing the instance version with the lowest version of the
 * members in the cluster.
 *
 *  For more information see:
 *  WL#13084 (AdminAPI: Handling of cross-version policies)
 *
 * @param instance Instance to use to perform the check.
 * @param lowest_cluster_version Version object with the lowest server version
 *                               in the cluster to join.
 * @return boolean indicating if the instance can only join the cluster in
 *         read-only mode (true).
 */
bool is_instance_only_read_compatible(
    const mysqlshdk::mysql::IInstance &instance,
    mysqlshdk::utils::Version lowest_cluster_version);

enum class Group_member_recovery_status {
  DONE_ONLINE,        // not in recovery (ONLINE)
  DONE_OFFLINE,       // not in recovery (OFFLINE)
  DISTRIBUTED,        // distributed recovery (binlog)
  DISTRIBUTED_ERROR,  // error during distributed (ERROR state)
  CLONE,              // clone recovery
  CLONE_ERROR,        // error during clone (ERROR state)
  UNKNOWN,            // couldn't detect
  UNKNOWN_ERROR       // couldn't detect (ERROR state)
};

/**
 * Given a member in RECOVERING state, tries to detect the type of recovery in
 * use.
 *
 * @param instance connection options to the instance being checked
 * @param start_time timestamp of a point in time before the recovery is known
 * to have started
 * @param out_recovering if not null, will be set to true if RECOVERING
 * @returns current status of the recovery, as detected
 *
 * Note: The only way to tell apart different executions of clone is
 * through the time it started, so the start_time arg should be set to the
 * timestamp of the server right before the START GROUP_REPLICATION is executed,
 * in that same server.
 */
Group_member_recovery_status detect_recovery_status(
    const mysqlshdk::mysql::IInstance &instance, const std::string &start_time,
    bool *out_recovering = nullptr);

/**
 * Validate if the endpoint (host + port) is supported by Group replication.
 * Performs "syntactic" validation of the host part when it is
 * an IP address (either IPv4 or IPv6). Hostnames are not validated (the method
 * returns true in these cases) because even if we could resolve them we cannot
 * assume that name resolution is the same across all the cluster instances and
 * the machine where the ngshell is running.
 *
 * // Note cannot be used for ipWhitelist validation as ipWhitelist values
 * // don't have a port, just IP/hostname and also accept netmasks.
 * @param endpoint The endpoint we want to check is supported by the instance to
 * be used as either localAddress or groupSeed.
 * @param version The version of the instance
 * @return true if endpoint is supported with the given version or an hostname
 * is used on the host part, false otherwise.
 */

bool is_endpoint_supported_by_gr(const std::string &endpoint,
                                 const mysqlshdk::utils::Version &version);

}  // namespace gr
}  // namespace mysqlshdk

#endif  // MYSQLSHDK_LIBS_MYSQL_GROUP_REPLICATION_H_
