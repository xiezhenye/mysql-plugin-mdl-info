mysql-plugin-mdl-locks
=======================

This plugin is used to show all MDL locks of mysql 5.5+.
The behavior of mdl locks has changed in mysql 5.5 . DDL statements may be blocked by MDL locks in some unobvious way like `begin; select * from some table;` and has no way to find out by whom the table is locked. This plugin used a trick way to fetch the MDL lock information and now, you can know who lock the tables through a information schema table.

usage
-----

first, compile the plugin and install in to plugin dir

    cp -r src /path/to/mysql-src/plugin/mdl_locks
    cd /path/to/mysql-src
    cmake .
    cd plugin/mdl_locks
    make
    make install
    
then, load the plugin into mysql

    mysql> INSTALL PLUGIN MDL_LOCKS SONAME 'mdl_Locks.so';
    
thus, you can found `MDL_LOCKS` table in `information_schema` database

try `select * from information_schema.MDL_LOCKS;` and you may got something like

    mysql> select * from information_schema.MDL_LOCKS;
    +-----------+----------+-------+-------------+-------------+
    | THREAD_ID | DATABASE | TABLE | TYPE        | DURATION    |
    +-----------+----------+-------+-------------+-------------+
    |         4 | test     | test  | SHARED_READ | EXPLICIT    |
    |         5 | mysql    | user  | SHARED_READ | TRANSACTION |
    +-----------+----------+-------+-------------+-------------+
    2 rows in set (0.00 sec)
