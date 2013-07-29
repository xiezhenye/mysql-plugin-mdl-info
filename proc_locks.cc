#define MYSQL_SERVER

#include "sql_class.h"
#include "table.h"
#include "global_threads.h"
#include "set_var.h"

static struct st_mysql_information_schema proc_locks =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

static ST_FIELD_INFO proc_locks_table_fields[] =
{
  {"THREAD_ID",    11,   MYSQL_TYPE_LONG,   0, MY_I_S_UNSIGNED, 0, 0},
  {"DATABASE", 255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"TABLE",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"LOCKED", 11, MYSQL_TYPE_LONG,   0, MY_I_S_UNSIGNED, 0, 0},
  {"TYPE",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"DESCRIPTION",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"QUERY", 1055,  MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0}
};

static const char *lock_descriptions[] =
{
  /* TL_UNLOCK                  */  "No lock",
  /* TL_READ_DEFAULT            */  NULL,
  /* TL_READ                    */  "Low priority read lock",
  /* TL_READ_WITH_SHARED_LOCKS  */  "Shared read lock",
  /* TL_READ_HIGH_PRIORITY      */  "High priority read lock",
  /* TL_READ_NO_INSERT          */  "Read lock without concurrent inserts",
  /* TL_WRITE_ALLOW_WRITE       */  "Write lock that allows other writers",
  /* TL_WRITE_CONCURRENT_INSERT */  "Concurrent insert lock",
  /* TL_WRITE_DELAYED           */  "Lock used by delayed insert",
  /* TL_WRITE_DEFAULT           */  NULL,
  /* TL_WRITE_LOW_PRIORITY      */  "Low priority write lock",
  /* TL_WRITE                   */  "High priority write lock",
  /* TL_WRITE_ONLY              */  "Highest priority write lock"
};

bool schema_table_store_record(THD *thd, TABLE *table);
static int proc_locks_init(void *ptr);
static int proc_locks_fill_table(THD *thd, TABLE_LIST *tables, Item *cond);
static void fill_fields(TABLE *table, THR_LOCK_DATA *data, bool write, bool locked);

static int proc_locks_fill_table(THD *thd, TABLE_LIST *tables, Item *cond)
{
  TABLE *table = tables->table;
  LIST *list;
  THR_LOCK *lock;

  if (thd->killed != 0) {
    return 1;
  }

  mysql_mutex_lock(&THR_LOCK_lock);
  for (list= thr_lock_thread_list; list; list = list_rest(list))
  {
    lock=(THR_LOCK*) list->data;
    mysql_mutex_lock(&lock->mutex);

    fill_fields(table, lock->write.data, true, true);
    if (schema_table_store_record(thd, table)) {
       goto err;
    }
    fill_fields(table, lock->read.data, false, true);
    if (schema_table_store_record(thd, table)) {
       goto err;
    }
    fill_fields(table, lock->write_wait.data, true, false);
    if (schema_table_store_record(thd, table)) {
       goto err;
    }
    fill_fields(table, lock->read_wait.data, false, false);
    if (schema_table_store_record(thd, table)) {
       goto err;
    }

    mysql_mutex_unlock(&lock->mutex);
  }
  mysql_mutex_unlock(&THR_LOCK_lock);
  return 0;
err:
  mysql_mutex_unlock(&lock->mutex);
  mysql_mutex_unlock(&THR_LOCK_lock);
  return 1;  
}

static void fill_fields(TABLE *table, THR_LOCK_DATA *data, bool write, bool locked)
{
    if (!data) {
        return;
    }
    TABLE *entry = (TABLE *)data->debug_print_param;
    THD *thd = entry->in_use;
    table->field[0]->store(thd ? thd->thread_id : 0);//thread_id
    table->field[1]->store(entry->s->db.str, strlen(entry->s->db.str), system_charset_info); //database
    table->field[2]->store(entry->s->table_name.str, strlen(entry->s->table_name.str), system_charset_info); //table
    table->field[3]->store(locked ? 1 : 0);
    if (write) {
        table->field[4]->store("write", 5, system_charset_info);
    } else {
        table->field[4]->store("read", 4, system_charset_info);
    }
    const char *desc = thd ? lock_descriptions[(int)entry->reginfo.lock_type] : "NULL";
    table->field[5]->store(desc, strlen(desc), system_charset_info);
    if (entry->in_use) {
        table->field[6]->store(thd->query(), thd->query_length(), thd->query_charset());
    } else {
        table->field[6]->store("NULL", 4, system_charset_info);
    }
}



static int proc_locks_init(void *ptr)
{
  ST_SCHEMA_TABLE *schema_table = (ST_SCHEMA_TABLE *)ptr;
  schema_table->fields_info = proc_locks_table_fields;
  schema_table->fill_table = proc_locks_fill_table;
  return 0;
}

mysql_declare_plugin(proc_vars)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &proc_locks,
  "PROCESS_LOCKS",
  "Xie Zhenye",
  "Locks",
  PLUGIN_LICENSE_GPL,
  proc_locks_init,
  NULL,
  0x0100,
  NULL,
  NULL,
  NULL,
  0
}
mysql_declare_plugin_end;

