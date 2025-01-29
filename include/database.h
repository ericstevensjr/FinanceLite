#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>
#include <string.h>

// Function prototypes for database operations
void initializeDatabase(sqlite3 **db, const char *db_name);
void insertIncome(sqlite3 *db);
void insertExpense(sqlite3 *db);
void insertSavingsGoal(sqlite3 *db, const char *name, float target_amount, const char *due_date);
void updateSavingsGoal(sqlite3 *db, int goal_id, float amount);
void fetchSavingsGoals(sqlite3 *db);
void saveBudgetToJSON(sqlite3 *db, const char *filename);
void getLastProcessedMonth(sqlite3 *db, int *year, int *month);
void updateLastProcessedMonth(sqlite3 *db, int year, int month);
void applyRecurringTransactions(sqlite3 *db);

#endif
