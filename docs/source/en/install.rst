.. Copyright 2023 The Elastic AI Search Authors.

Build
=====

EA uses cmake as the build system, and EA servers need to run on the Linux platform.

Dependencies
---------------------------------

..  |ea_requirements| list-table:: EA requirements
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
click. If you want to learn more about the use of caibin, please see: |carbin_github|


Ubuntu Dependencies
____________________________________

install dependencies::

    sudo apt-get install -y git g++ make libssl-dev
    pip install carbin


centos Dependencies
___________________________________

install dependencies::

    sudo yum install epel-release
    sudo yum install git gcc-c++ make openssl-devel
    pip install carbin


install cmake
___________________________________

EA relies on some external projects, and some external projects have higher requirements for the cmake
version. It is a good choice to try to install a newer version of cmake.

install EA Dependencies
__________________________________

install dependencies::

    $cd your_EA_root
    carbin install


Install
=======