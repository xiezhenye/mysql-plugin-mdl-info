cmake_minimum_required(VERSION 2.6)

MYSQL_ADD_PLUGIN(mdl_locks mdl_locks.cc
  MODULE_ONLY MODULE_OUTPUT_NAME "mdl_locks")

