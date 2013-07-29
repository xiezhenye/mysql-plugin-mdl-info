mysql-plugin-mdl-locks
=======================

show all mdl locks

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

    mysql> INSTALL PLUGIN MDL_LOCKS SONAME 'proc_vars.so';
    
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
