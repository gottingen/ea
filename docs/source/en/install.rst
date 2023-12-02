.. Copyright 2023 The Elastic AI Search Authors.

.. _build_and_install_section:

=================================================
Build and Install
=================================================

Build Prepare
====================

EA uses cmake as the build system, and EA servers need to run on the Linux platform.


..   list-table:: EA requirements
    :widths: 15 10 10 10 10
    :header-rows: 1

    * - Name
      - Require
      - Level
      - Link
      - Manage
    * - gcc/g++
      - >=9.4
      - build
      - NONE
      - NONE
    * - cmake
      - >= 3.20
      - build
      - NONE
      - NONE
    * - openssl
      - >=1.1.1*
      - RT
      - DYNAMIC
      - NONE
    * - python
      - >= 3.8
      - build
      - NONE
      - NONE
    * - carbin
      - >= 0.3.0
      - build
      - NONE
      - NONE
    * - turbo
      - >= 0.9.34
      - RT
      - STATIC
      - carbin
    * - gflags
      - 2.2.2
      - RT
      - STATIC
      - carbin
    * - zlib
      - 1.2.8
      - RT
      - STATIC
      - carbin
    * - snappy
      - 1.1.7
      - RT
      - STATIC
      - carbin
    * - rocksdb
      - 6.29.3
      - RT
      - STATIC
      - carbin
    * - protobuf
      - 3.18
      - RT
      - STATIC
      - carbin
    * - leveldb
      - >= 1.22
      - RT
      - STATIC
      - carbin
    * - brpc
      - 1.3.0
      - RT
      - STATIC
      - carbin
    * - braft
      - ea_dev
      - RT
      - STATIC
      - carbin
    * - eapi
      - dev
      - RT
      - STATIC
      - carbin



Looking at the table above, RT level dependencies need to be installed in any environment,
and build level dependencies only need to be installed on the compilation machine. Dependent
projects managed by carbin do not need to be installed manually. Dependencies of NONE managed
need to be installed manually. Next, the first step is to install the NONE managed dependent
projects, and the second step is to install the dependent projects managed by carbin with one
click. If you want to learn more about the use of caibin, please see: `carbin on github <github.com/gottingen/carbin>`_
and the docs `carbin docs <carbin.readthedocs.io/>`_.


Ubuntu Dependencies
----------------------------------

install dependencies::

    >sudo apt-get install -y git g++ make libssl-dev
    >pip install carbin


centos Dependencies
-----------------------------------

install dependencies::

    >sudo yum install epel-release
    >sudo yum install git gcc-c++ make openssl-devel
    >pip install carbin


install cmake
-------------------------------------

EA relies on some external projects, and some external projects have higher requirements for the cmake
version. It is a good choice to try to install a newer version of cmake.

install EA Dependencies
-------------------------------------

install dependencies::

    >git clone https://github.com/gottingen/ea.git
    >cd ea
    >carbin install

then the dependencies will be install to `ea/carbin`. the  dependencies descriptions ara in the ea/carbin_deps.txt.

.. note::
    gottingen/turbo@v0.9.34 --ignore-requirements -DCARBIN_BUILD_TEST=OFF -DCARBIN_BUILD_BENCHMARK=OFF -DCARBIN_BUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBRARY=OFF -DBUILD_STATIC_LIBRARY=ON -DCMAKE_BUILD_TYPE=release


    in *gottingen/turbo@v0.9.34* `gottingen/turbo` descript which repo, and `v0.9.34` can be a tag or branch.
    `--ignore-requirements` means that, do not download turbo's requirements recursively. and then, the other's is build
    arguments for cmake.

Build And Install
==================================

build::

    > mkdir build
    > cd build
    > cmake .. -DCMAKE_INSTALL_PREFIX=you_install_dir
    > make -j 6


Test Running
=================================

run cmd::

    >./ea/eameta
    >./ea/eacli meta config list
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


if the command returns the as above, means that ea build and run normal on system. we have build and install it success full.
then we go to next part to config and deploy `EA`.