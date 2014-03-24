#define MYSQL_SERVER

#include "sql_class.h"
#include "table.h"
#include "set_var.h"

#include "hack_context.h"
#include "mdl.cc"

static struct st_mysql_information_schema is_mdl_locks =
{ MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

static ST_FIELD_INFO mdl_locks_table_fields[] =
{
  {"THREAD_ID",    11,   MYSQL_TYPE_LONG,   0, MY_I_S_UNSIGNED, 0, 0},
  {"DURATION", 30,  MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"TYPE",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"NAMESPACE", 30, MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"DATABASE", 255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"NAME",  255,   MYSQL_TYPE_STRING, 0, 0, 0, 0},
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
  "EXCLUSIVE",
  "NULL"  //MDL_TYPE_END
};

static const char *ns_desc[] = 
{
  "GLOBAL",
  "SCHEMA",
  "TABLE",
  "FUNCTION",
  "PROCEDURE",
  "TRIGGER",
  "EVENT",
  "COMMIT",
  "NULL"  //NAMESPACE_END
};

struct MDL_lock;
typedef MDL_context::Ticket_iterator Ticket_iterator;
bool schema_table_store_record(THD *thd, TABLE *table);
static int mdl_locks_init(void *ptr);
static int mdl_locks_fill_table(THD *thd, TABLE_LIST *tables, Item *cond);
static void ticket_fill_table(THD *thd, THD *cur_thd, TABLE *table, Item *cond, Ticket_iterator itr, const char *duration);

static int mdl_locks_fill_table(THD *thd, TABLE_LIST *tables, Item *cond)
{
  TABLE *table = tables->table;
  if (thd->killed != 0) {
    return 1;
  }

  mysql_mutex_lock(&LOCK_thread_count);
  I_List_iterator<THD> it(threads);

  THD *cur_thd;
  while ((cur_thd= it++))
  {
    if (cur_thd == NULL)
    {
      continue;
    }
    if (! cur_thd->mdl_context.has_locks()) {
      continue;
    }
    Hack_MDL_context *hmc;
    hmc = (Hack_MDL_context *)(&(cur_thd->mdl_context));
    Ticket_iterator stmt_tks = MDL_context::Ticket_iterator(hmc->m_tickets[MDL_STATEMENT]);
    ticket_fill_table(thd, cur_thd, table, cond, stmt_tks, "STATEMENT");
    Ticket_iterator tran_tks = MDL_context::Ticket_iterator(hmc->m_tickets[MDL_TRANSACTION]);
    ticket_fill_table(thd, cur_thd, table, cond, tran_tks, "TRANSACTION");
    Ticket_iterator expl_tks = MDL_context::Ticket_iterator(hmc->m_tickets[MDL_EXPLICIT]);
    ticket_fill_table(thd, cur_thd, table, cond, expl_tks, "EXPLICIT");
  }
  mysql_mutex_unlock(&LOCK_thread_count);
  return 0;
}

static void ticket_fill_table(THD *thd, THD *cur_thd, TABLE *table, Item *cond, Ticket_iterator itr, const char *duration)
{
  MDL_key *key;
  MDL_lock *lock;
  MDL_ticket *ticket;
  const char *type;
  const char *ns;

  while ((ticket = itr++)) {
    type = type_desc[(int) ticket->get_type()];
    lock = ticket->get_lock();
    if (!lock) {
      continue;
    } 
    key = &(lock->key);
    if (!key) {
      continue;
    }
    ns = ns_desc[(int) key->mdl_namespace()];
    table->field[0]->store(cur_thd->thread_id);
    table->field[1]->store(duration, strlen(duration), system_charset_info);
    table->field[2]->store(type, strlen(type), system_charset_info);
    table->field[3]->store(ns, strlen(ns), system_charset_info);
    table->field[4]->store(key->db_name(), key->db_name_length(), system_charset_info);
    table->field[5]->store(key->name(), key->name_length(), system_charset_info);
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

mysql_declare_plugin(mdl_locks)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &is_mdl_locks,
  "MDL_LOCKS",
  "Xie Zhenye",
  "MDL Locks",
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

