mysql-plugin-mdl-locks
=======================

This plugin is used to show all MDL locks of mysql 5.5+.
The behavior of mdl locks has changed in mysql 5.5 . DDL statements may be blocked by MDL locks in some unobvious way like `begin; select * from some table;` and has no way to find out by whom the table is locked. This plugin used a trick way to fetch the MDL lock information and now, you can know who lock the tables through a information schema table.


download
--------
You can find some compiled binary files at the [download page](https://github.com/xiezhenye/mysql-plugin-mdl-locks/wiki/downloads), or you can compile it from source by yourself.

usage
-----

first, compile the plugin and install in to plugin dir

    cp -r src /path/to/mysql-src/plugin/mdl_locks
    cd /path/to/mysql-src
    cmake . -DBUILD_CONFIG=mysql_release
    cd plugin/mdl_locks
    make
    make install
    
then, load the plugin into mysql

    mysql> INSTALL PLUGIN MDL_LOCKS SONAME 'mdl_Locks.so';
    
thus, you can found `MDL_LOCKS` table in `information_schema` database
    
    mysql> lock tables plugin read;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> select * from information_schema.MDL_LOCKS;
    +-----------+----------+-------------+-----------+----------+--------+
    | THREAD_ID | DURATION | TYPE        | NAMESPACE | DATABASE | NAME   |
    +-----------+----------+-------------+-----------+----------+--------+
    |         4 | EXPLICIT | SHARED_READ | TABLE     | mysql    | plugin |
    +-----------+----------+-------------+-----------+----------+--------+
    1 row in set (0.00 sec)
    
    mysql> unlock tables;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> lock tables plugin write;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> select * from information_schema.MDL_LOCKS;
    +-----------+----------+----------------------+-----------+----------+--------+
    | THREAD_ID | DURATION | TYPE                 | NAMESPACE | DATABASE | NAME   |
    +-----------+----------+----------------------+-----------+----------+--------+
    |         4 | EXPLICIT | INTENTION_EXCLUSIVE  | GLOBAL    |          |        |
    |         4 | EXPLICIT | SHARED_NO_READ_WRITE | TABLE     | mysql    | plugin |
    |         4 | EXPLICIT | INTENTION_EXCLUSIVE  | SCHEMA    | mysql    |        |
    +-----------+----------+----------------------+-----------+----------+--------+
    3 rows in set (0.00 sec)
    
    mysql> unlock tables;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> begin;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> select * from plugin;
    +-----------+--------------+
    | name      | dl           |
    +-----------+--------------+
    | MDL_LOCKS | mdl_locks.so |
    +-----------+--------------+
    1 row in set (0.01 sec)
    
    mysql> select * from information_schema.MDL_LOCKS;
    +-----------+-------------+-------------+-----------+----------+--------+
    | THREAD_ID | DURATION    | TYPE        | NAMESPACE | DATABASE | NAME   |
    +-----------+-------------+-------------+-----------+----------+--------+
    |         4 | TRANSACTION | SHARED_READ | TABLE     | mysql    | plugin |
    +-----------+-------------+-------------+-----------+----------+--------+
    1 row in set (0.01 sec)
    
    mysql> rollback;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> flush tables with read lock;
    Query OK, 0 rows affected (0.00 sec)
    
    mysql> select * from information_schema.MDL_LOCKS;
    +-----------+----------+--------+-----------+----------+------+
    | THREAD_ID | DURATION | TYPE   | NAMESPACE | DATABASE | NAME |
    +-----------+----------+--------+-----------+----------+------+
    |         4 | EXPLICIT | SHARED | COMMIT    |          |      |
    |         4 | EXPLICIT | SHARED | GLOBAL    |          |      |
    +-----------+----------+--------+-----------+----------+------+
    2 rows in set (0.00 sec)
    
    mysql> unlock tables;
    Query OK, 0 rows affected (0.00 sec)
    

    

