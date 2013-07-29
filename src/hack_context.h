class Hack_MDL_context {
public:
  MDL_wait m_wait;
  MDL_context::Ticket_list m_tickets[MDL_DURATION_END];
  MDL_context_owner *m_owner;
  bool m_needs_thr_lock_abort;
  mysql_prlock_t m_LOCK_waiting_for;
  MDL_wait_for_subgraph *m_waiting_for;
};
