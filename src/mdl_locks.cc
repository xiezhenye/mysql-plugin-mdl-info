#define MYSQL_SERVER

#include "sql_class.h"
#include "table.h"
#include "global_threads.h"
#include "set_var.h"

#include "hack_context.h"
#include "mdl.cc"

static struct st_mysql_information_schema is_mdl_locks =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

static ST_FIELD_INFO mdl_locks_table_fields[] =
{
  {"THREAD_ID",    11,   MYSQL_TYPE_LONG,   0, MY_I_S_UNSIGNED, 0, 0},
  {"DATABASE", 255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"TABLE",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"TYPE",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"DURATION", 30,  MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0}
};

static const char *type_desc[] =
{
  "INTENTION_EXCLUSIVE",
  "SHARED",
  "SHARED_HIGH_PRIO",
  "SHARED_READ",
  "SHARED_WRITE",
  "SHARED_NO_WRITE",
  "SHARED_NO_READ_WRITE",
  "EXCLUSIVE"
  //MDL_TYPE_END
};
struct MDL_lock;
bool schema_table_store_record(THD *thd, TABLE *table);
static int mdl_locks_init(void *ptr);
static int mdl_locks_fill_table(THD *thd, TABLE_LIST *tables, Item *cond);
static void fill_table(THD *thd, THD *cur_thd, TABLE *table, Item *cond);
static int mdl_locks_fill_table(THD *thd, TABLE_LIST *tables, Item *cond)
{
  TABLE *table = tables->table;
  if (thd->killed != 0) {
    return 1;
  }

  mysql_mutex_lock(&LOCK_thread_count);
  Thread_iterator it= global_thread_list_begin();
  Thread_iterator end= global_thread_list_end();
  for (; it != end; ++it)
  {
    THD *cur_thd = *it;
    if (cur_thd == NULL)
    {
      continue;
    }
    fill_table(thd, cur_thd, table, cond);
  }
  mysql_mutex_unlock(&LOCK_thread_count);
  return 0;
}

static void fill_table(THD *thd, THD *cur_thd, TABLE *table, Item *cond)
{
  MDL_ticket *ticket;
  Hack_MDL_context *hmc;
  hmc = (Hack_MDL_context *)(&(cur_thd->mdl_context));
  const char *type;
  MDL_key *key;
  MDL_context::Ticket_iterator stmt_tks = MDL_context::Ticket_iterator(hmc->m_tickets[MDL_STATEMENT]);
  while ((ticket = stmt_tks++)) {
    if (ticket) {
      type = type_desc[(int) ticket->get_type()];
    } else {
      type= "NULL";
    }
    key = &(ticket->get_lock()->key);
    table->field[0]->store(cur_thd->thread_id);
    table->field[1]->store(key->db_name(), key->db_name_length(), system_charset_info);
    table->field[2]->store(key->name(), key->name_length(), system_charset_info);
    table->field[3]->store(type, strlen(type), system_charset_info);
    table->field[4]->store("STATEMENT", strlen("STATEMENT"), system_charset_info);
    schema_table_store_record(thd, table);        
  } 
  MDL_context::Ticket_iterator tran_tks = MDL_context::Ticket_iterator(hmc->m_tickets[MDL_TRANSACTION]);
  while ((ticket = tran_tks++)) {
    if (ticket) {
      type = type_desc[(int) ticket->get_type()];
    } else {
      type= "NULL";
    }
    key = &(ticket->get_lock()->key);
    table->field[0]->store(cur_thd->thread_id);
    table->field[1]->store(key->db_name(), key->db_name_length(), system_charset_info);
    table->field[2]->store(key->name(), key->name_length(), system_charset_info);
    table->field[3]->store(type, strlen(type), system_charset_info);
    table->field[4]->store("TRANSACTION", strlen("TRANSACTION"), system_charset_info);
    schema_table_store_record(thd, table);        
  } 
  MDL_context::Ticket_iterator expl_tks = MDL_context::Ticket_iterator(hmc->m_tickets[MDL_EXPLICIT]);
  while ((ticket = expl_tks++)) {
    if (ticket) {
      type = type_desc[(int) ticket->get_type()];
    } else {
      type= "NULL";
    }
    key = &(ticket->get_lock()->key);
    table->field[0]->store(cur_thd->thread_id);
    table->field[1]->store(key->db_name(), key->db_name_length(), system_charset_info);
    table->field[2]->store(key->name(), key->name_length(), system_charset_info);
    table->field[3]->store(type, strlen(type), system_charset_info);
    table->field[4]->store("EXPLICIT", strlen("EXPLICIT"), system_charset_info);
    schema_table_store_record(thd, table);        
  }
  return; 
}

static int mdl_locks_init(void *ptr)
{
  ST_SCHEMA_TABLE *schema_table = (ST_SCHEMA_TABLE *)ptr;
  schema_table->fields_info = mdl_locks_table_fields;
  schema_table->fill_table = mdl_locks_fill_table;
  return 0;
}

mysql_declare_plugin(proc_vars)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &is_mdl_locks,
  "MDL_LOCKS",
  "Xie Zhenye",
  "Locks",
  PLUGIN_LICENSE_GPL,
  mdl_locks_init,
  NULL,
  0x0100,
  NULL,
  NULL,
  NULL,
  0
}
mysql_declare_plugin_end;

