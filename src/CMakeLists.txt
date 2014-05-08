cmake_minimum_required(VERSION 2.6)

MYSQL_ADD_PLUGIN(mdl_info mdl_info.cc
  MODULE_ONLY MODULE_OUTPUT_NAME "mdl_info")

