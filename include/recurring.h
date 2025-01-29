#ifndef RECURRING_H
#define RECURRING_H

#include <sqlite3.h>

// Function prototypes for recurring entries
void manageRecurringEntries(sqlite3 *db);
void insertRecurringEntry(sqlite3 *db, const char *type);
void fetchRecurringEntries(sqlite3 *db);
void editRecurringEntries(sqlite3 *db);
void removeRecurringEntries(sqlite3 *db);
void removeSavingsGoal(sqlite3 *db);
#endif
