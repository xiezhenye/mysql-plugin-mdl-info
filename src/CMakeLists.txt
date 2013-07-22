cmake_minimum_required(VERSION 2.8)

MYSQL_ADD_PLUGIN(proc_locks proc_locks.cc
  MODULE_ONLY MODULE_OUTPUT_NAME "proc_locks")

