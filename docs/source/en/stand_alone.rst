.. Copyright 2023 The Elastic AI Search Authors.

.. _stand_alone_section:

=======================================
Stand Alone deploy
=======================================

Now, we start using `EA`, starting from a single machine and single node raft node.The purpose of this
part is to become familiar with the functions of `EA meta` and the use of `eacli`. Make sure that the `EA`
components are properly installed via any way and that they are placed in the system's search path. if you
have notinstall `EA`, see :ref:`build_and_install_section` install it first. Two components will be used
here: `eameta` and `eacli`. here we go!

.. note::
    eameta will create files in the running directory and read and write files. Make sure user has read,
    write and run permissions on the current directory.

config setting
======================================

The default stand-alone configuration file of eameta is as follows, gflags::

    -defer_close_second=300
    -meta_snapshot_interval_s=600
    -meta_election_timeout_ms=10000
    -meta_raft_max_election_delay_ms=5000
    -meta_log_uri=local://./meta/raft_log/
    -meta_stable_uri=local://./meta/raft_data/stable
    -meta_snapshot_uri=local://./meta/raft_data/snapshot
    -meta_db_path=./meta/rocks_db
    -meta_replica_number=1
    -meta_server_peers=127.0.0.1:8010
    -meta_listen=127.0.0.1:8010
    -bthread_concurrency=100
    -bvar_dump
    -bvar_dump_file=./monitor/bvar.eameta.data
    -ea_log_base_name=meta_log.txt

at the directory you want to run eameta, create directory `conf` and copy the config above, write to file
`meta_gflags.conf`.

.. note::
    The meaning of gflags configuration will be explained in detail in the management and deployment section
    later. Ignore them now, Detailed configuration explanation is here  :ref:`gflags_section`, But it is not
    recommended to look at the configuration now, because it will affect our wonderful time.

start eameta
========================================

start eameta::

    >eameta &

`eameta` fixedly reads `conf/meta_gflags.conf` files as configuration startup. `eameta` success start up,
corresponding to the above configuration, eameta will listen to port 8010.

check the listening port::

    >netstat -ntl
    tcp        0      0 127.0.0.1:8010          0.0.0.0:*               LISTEN

`eameta` will create dirs for data store, after start up `meta` `monitor` `logs` dir will be create automic.

check directories::

    > ls
    conf logs monitor meta


now we run `eacli` to check `eameta` and `eacli` can interact correctly::

    >eacli meta config list

if return this means work well::

    +---------+-----------------------------------------------------------------------------------------+
    | phase   |                                          status                                         |
    +---------+-----------------------------------------------------------------------------------------+
    | prepare |                                            ok                                           |
    +---------+-----------------------------------------------------------------------------------------+
    | rpc     |                                            ok                                           |
    +---------+-----------------------------------------------------------------------------------------+
    | result  | +---------+----------------+---------+-------------------+------------+---------------+ |
    |         | | status  | meta leader    | op code | op string         | error code | error message | |
    |         | +---------+----------------+---------+-------------------+------------+---------------+ |
    |         | | success | 127.0.0.1:8010 | 19      | QUERY_LIST_CONFIG | 0          | success       | |
    |         | +---------+----------------+---------+-------------------+------------+---------------+ |
    +---------+-----------------------------------------------------------------------------------------+
    | summary | +-------------+--------+                                                                |
    |         | | config size | 0      |                                                                |
    |         | +-------------+--------+                                                                |
    |         | | number      | config |                                                                |
    |         | +-------------+--------+                                                                |
    +---------+-----------------------------------------------------------------------------------------+

and this means some thing wrong::

    +---------+--------------------------------------------------------------------------------------------------+
    | phase   |                                              status                                              |
    +---------+--------------------------------------------------------------------------------------------------+
    | prepare |                                                ok                                                |
    +---------+--------------------------------------------------------------------------------------------------+
    | rpc     | +--------+---------+-------------------+------------+------------------------------------------+ |
    |         | | status | op code | op string         | error code | error message                            | |
    |         | +--------+---------+-------------------+------------+------------------------------------------+ |
    |         | | fail   | 19      | QUERY_LIST_CONFIG | 14         | can not connect server after 3 times try | |
    |         | +--------+---------+-------------------+------------+------------------------------------------+ |
    +---------+--------------------------------------------------------------------------------------------------+

check log::

    >tail logs/ea_log_2023-12-02.txt
    [2023-12-02 08:16:21.425] [ea-logger] [warning] [meta_server.cc:82] _tso_state_machine init success
    [2023-12-02 08:16:21.426] [ea-logger] [info] [base_state_machine.cc:152] new conf committed, new peer: 127.0.0.1:8010:0,
    [2023-12-02 08:16:21.426] [ea-logger] [info] [base_state_machine.cc:127] leader start at term: 3
    [2023-12-02 08:16:21.426] [ea-logger] [warning] [tso_state_machine.cc:363] tso leader start
    [2023-12-02 08:16:21.426] [ea-logger] [warning] [tso_state_machine.cc:374] leader_start current(phy:123668180368,log:0) save:123668184426
    [2023-12-02 08:16:21.428] [ea-logger] [warning] [tso_state_machine.cc:380] sync timestamp ok


Try Config Component
====================

In this part, I use `eacli` to interact with `eameta` to demonstrate the functions of `eameta`. actually,
our previous test was one of the config command -- view the config list.

dump a example
-------------------------------

First, we create a simple config through eacli::

    >eacli meta config dump -e 123.json
    +--------------+----------------------------------+
    | phase        |              status              |
    +--------------+----------------------------------+
    | prepare      |                ok                |
    +--------------+----------------------------------+
    | prepare file |                ok                |
    +--------------+----------------------------------+
    | convert      |                ok                |
    +--------------+----------------------------------+
    | write        |                ok                |
    +--------------+----------------------------------+
    | summary      | success write to  file: 123.json |
    +--------------+----------------------------------+

..  code:: json

    {
      "name": "example",
      "version": {
        "major": 1,
        "minor": 2,
        "patch": 3
      },
      "content": "{\"servlet\":\"sug\",\"zone\":{\"instance\":[\"192.168.1.2\",\"192.168.1.3\",\"192.168.1.3\"],\"name\":\"ea_search\",\"user\":\"jeff\"}}",
      "type": "CF_JSON",
      "time": 1701477509
    }

In the above json fragment, the `content` field is our real `configuration content`. and other field are attributes of
the config. you can put any thing as string to the `content` field as you wander.

..  note::
    config uses a combination of the two fields name and version to determine whether it is the same
    configuration. Configurations with the same name can only be set in order and cannot be modified.
    If you want to update the configuration, you must update the version field to ensure that the
    version is monotonically increasing.

    new.version.major>=old.version.major,
    new.version.minor>=old.version.minor
    new.version.patch>=old.version.patch,

    and among these three fields, the value of the new field is at least greater than the value of old.

create config
---------------------------

Using the configuration we just generated, we create the first configuration.

create config::

    >eacli meta config create -j 123.json
    +---------+----------------------------------------------------------------------------------------+
    | phase   |                                         status                                         |
    +---------+----------------------------------------------------------------------------------------+
    | prepare |                                           ok                                           |
    +---------+----------------------------------------------------------------------------------------+
    | rpc     |                                           ok                                           |
    +---------+----------------------------------------------------------------------------------------+
    | result  | +---------+----------------+---------+------------------+------------+---------------+ |
    |         | | status  | meta leader    | op code | op string        | error code | error message | |
    |         | +---------+----------------+---------+------------------+------------+---------------+ |
    |         | | success | 127.0.0.1:8010 | 38      | OP_CREATE_CONFIG | 0          | success       | |
    |         | +---------+----------------+---------+------------------+------------+---------------+ |
    +---------+----------------------------------------------------------------------------------------+


if we run the command again, means create it twice, even if we modify except the name and version fields.
it will act fail.

try again::

    >eacli meta config create -j 123.json
    +---------+----------------------------------------------------------------------------------------------+
    | phase   |                                            status                                            |
    +---------+----------------------------------------------------------------------------------------------+
    | prepare |                                              ok                                              |
    +---------+----------------------------------------------------------------------------------------------+
    | rpc     |                                              ok                                              |
    +---------+----------------------------------------------------------------------------------------------+
    | result  | +--------+----------------+---------+------------------+------------+----------------------+ |
    |         | | status | meta leader    | op code | op string        | error code | error message        | |
    |         | +--------+----------------+---------+------------------+------------+----------------------+ |
    |         | | fail   | 127.0.0.1:8010 | 38      | OP_CREATE_CONFIG | 21         | config already exist | |
    |         | +--------+----------------+---------+------------------+------------+----------------------+ |
    +---------+----------------------------------------------------------------------------------------------+

download config
-----------------------------

Now that we have created the config in `eameta`, download it through the command line and save it to my_config.json

get config::

    >eacli meta config get -n example -o my_config.json
    +-----------+----------------------------------------------------------------------------------------+
    | operation |                                     get config info                                    |
    +-----------+----------------------------------------------------------------------------------------+
    | phase     |                                         status                                         |
    +-----------+----------------------------------------------------------------------------------------+
    | prepare   |                                         +----+                                         |
    |           |                                         | ok |                                         |
    |           |                                         +----+                                         |
    +-----------+----------------------------------------------------------------------------------------+
    | prepare   |                                           ok                                           |
    +-----------+----------------------------------------------------------------------------------------+
    | rpc       |                                           ok                                           |
    +-----------+----------------------------------------------------------------------------------------+
    | result    | +---------+----------------+---------+------------------+------------+---------------+ |
    |           | | status  | meta leader    | op code | op string        | error code | error message | |
    |           | +---------+----------------+---------+------------------+------------+---------------+ |
    |           | | success | 127.0.0.1:8010 | 17      | QUERY_GET_CONFIG | 0          | success       | |
    |           | +---------+----------------+---------+------------------+------------+---------------+ |
    +-----------+----------------------------------------------------------------------------------------+
    | summary   | +---------+---------------------------+                                                |
    |           | | version | 1.2.3                     |                                                |
    |           | +---------+---------------------------+                                                |
    |           | | type    | json                      |                                                |
    |           | +---------+---------------------------+                                                |
    |           | | size    | 114                       |                                                |
    |           | +---------+---------------------------+                                                |
    |           | | time    | 2023-12-02T08:38:29+08:00 |                                                |
    |           | +---------+---------------------------+                                                |
    |           | | file    | my_config.json            |                                                |
    |           | +---------+---------------------------+                                                |
    |           | | status  | ok                        |                                                |
    |           | +---------+---------------------------+                                                |
    +-----------+----------------------------------------------------------------------------------------+

We have successfully saved the configuration to the specified file, let's take a look at the content.

..  code:: json

    {
      "servlet": "sug",
      "zone": {
        "instance": [
          "192.168.1.2",
          "192.168.1.3",
          "192.168.1.3"
        ],
        "name": "ea_search",
        "user": "jeff"
      }
    }

it is same with we upload before.

create config more version
---------------------------------------

Now, we create `example` config more versions and another names `jeff`

        >eacli meta config create -n example -f my_config.json -v 1.2.4
        ...
        >eacli meta config create -n example -f my_config.json -v 1.2.8
        >eacli meta config create -n jeff -f my_config.json -v 0.1.0

list config
-------------------------------------------

let us see config list, the command that we always using it for a check.

config list::

    >eacli meta config list
    +---------+-----------------------------------------------------------------------------------------+
    | phase   |                                          status                                         |
    +---------+-----------------------------------------------------------------------------------------+
    | prepare |                                            ok                                           |
    +---------+-----------------------------------------------------------------------------------------+
    | rpc     |                                            ok                                           |
    +---------+-----------------------------------------------------------------------------------------+
    | result  | +---------+----------------+---------+-------------------+------------+---------------+ |
    |         | | status  | meta leader    | op code | op string         | error code | error message | |
    |         | +---------+----------------+---------+-------------------+------------+---------------+ |
    |         | | success | 127.0.0.1:8010 | 19      | QUERY_LIST_CONFIG | 0          | success       | |
    |         | +---------+----------------+---------+-------------------+------------+---------------+ |
    +---------+-----------------------------------------------------------------------------------------+
    | summary | +-------------+---------+                                                               |
    |         | | config size | 2       |                                                               |
    |         | +-------------+---------+                                                               |
    |         | | number      | config  |                                                               |
    |         | +-------------+---------+                                                               |
    |         | | 0           | example |                                                               |
    |         | +-------------+---------+                                                               |
    |         | | 1           | jeff    |                                                               |
    |         | +-------------+---------+                                                               |
    +---------+-----------------------------------------------------------------------------------------+

If we specify the name field, it will list the config version that we have create.

list version::

    >eacli meta config list -n example
    +---------+-------------------------------------------------------------------------------------------------+
    | phase   |                                              status                                             |
    +---------+-------------------------------------------------------------------------------------------------+
    | prepare |                                                ok                                               |
    +---------+-------------------------------------------------------------------------------------------------+
    | rpc     |                                                ok                                               |
    +---------+-------------------------------------------------------------------------------------------------+
    | result  | +---------+----------------+---------+---------------------------+------------+---------------+ |
    |         | | status  | meta leader    | op code | op string                 | error code | error message | |
    |         | +---------+----------------+---------+---------------------------+------------+---------------+ |
    |         | | success | 127.0.0.1:8010 | 18      | QUERY_LIST_CONFIG_VERSION | 0          | success       | |
    |         | +---------+----------------+---------+---------------------------+------------+---------------+ |
    +---------+-------------------------------------------------------------------------------------------------+
    | summary | +--------------+---------+                                                                      |
    |         | | version size | 6       |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | number       | version |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | 0            | 1.2.3   |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | 1            | 1.2.4   |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | 2            | 1.2.5   |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | 3            | 1.2.6   |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | 4            | 1.2.7   |                                                                      |
    |         | +--------------+---------+                                                                      |
    |         | | 5            | 1.2.8   |                                                                      |
    |         | +--------------+---------+                                                                      |
    +---------+-------------------------------------------------------------------------------------------------+

At this point, we have demonstrated the use of eameta by using parts of  config functions. For detailed config
functions, see the config module. Next we will focus our attention on how to build an eameta cluster.First build a
cluster on a single machine.

Deploy Clust Stand Alone
==========================================

deploy a cluster on a node, need three step.

* prepare work directories for every peer.
* prepare configuration for every peer.
* start all peers.

follow show how to deploy a cluster have three node for raft cluster. the nodes we just call it
node-1,node-2, node-3.

prepare directory
-------------------------------------

every node need it's own work director. for the simple, we

create directory::

    >mkdir -p node-1/conf
    >mkdir -p node-2/conf
    >mkdir -p node-3/conf

prepare configration
-------------------------------------
we designed the three node listening to ports `127.0.0.1:8011`, `127.0.0.1:8012`, and `127.0.0.1:8013` respectively.

* node-1 -- `127.0.0.1:8011`
* node-2 -- `127.0.0.1:8012`
* node-3 -- `127.0.0.1:8013`

node-1 configration::

    -defer_close_second=300
    -meta_snapshot_interval_s=600
    -meta_election_timeout_ms=10000
    -meta_raft_max_election_delay_ms=5000
    -meta_log_uri=local://./meta/raft_log/
    -meta_stable_uri=local://./meta/raft_data/stable
    -meta_snapshot_uri=local://./meta/raft_data/snapshot
    -meta_db_path=./meta/rocks_db
    -meta_replica_number=1
    -meta_server_peers=127.0.0.1:8011,127.0.0.1:8012,127.0.0.1:8013
    -meta_listen=127.0.0.1:8011
    -bthread_concurrency=100
    -bvar_dump
    -bvar_dump_file=./monitor/bvar.eameta.data
    -ea_log_base_name=meta_log.txt

node-2 configration::

    -defer_close_second=300
    -meta_snapshot_interval_s=600
    -meta_election_timeout_ms=10000
    -meta_raft_max_election_delay_ms=5000
    -meta_log_uri=local://./meta/raft_log/
    -meta_stable_uri=local://./meta/raft_data/stable
    -meta_snapshot_uri=local://./meta/raft_data/snapshot
    -meta_db_path=./meta/rocks_db
    -meta_replica_number=1
    -meta_server_peers=127.0.0.1:8011,127.0.0.1:8012,127.0.0.1:8013
    -meta_listen=127.0.0.1:8012
    -bthread_concurrency=100
    -bvar_dump
    -bvar_dump_file=./monitor/bvar.eameta.data
    -ea_log_base_name=meta_log.txt


node-3 configration::

    -defer_close_second=300
    -meta_snapshot_interval_s=600
    -meta_election_timeout_ms=10000
    -meta_raft_max_election_delay_ms=5000
    -meta_log_uri=local://./meta/raft_log/
    -meta_stable_uri=local://./meta/raft_data/stable
    -meta_snapshot_uri=local://./meta/raft_data/snapshot
    -meta_db_path=./meta/rocks_db
    -meta_replica_number=1
    -meta_server_peers=127.0.0.1:8011,127.0.0.1:8012,127.0.0.1:8013
    -meta_listen=127.0.0.1:8013
    -bthread_concurrency=100
    -bvar_dump
    -bvar_dump_file=./monitor/bvar.eameta.data
    -ea_log_base_name=meta_log.txt

as deploy signal peer, write the **meta_gflags.conf** configration for every node to
respectively directory(node-[1|2|3]/conf/meta_gflags.conf).

start up cluster
------------------------------------

start up::

    > cd node-1
    > eameta
    > cd node-2
    > eameta
    > cd node-3
    > eameta

check cluster
---------------------------------------

now we can check the cluster running statusas the signal peers above. more information see
administrate(:ref:`administrate_section`) and develop(:ref:`develop_section`) parts, Of course, there are also FAQs(:ref:`faq_section`) that can help you use `EA` smartly.