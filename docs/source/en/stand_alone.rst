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
the config.
