#ifndef BUDGET_H
#define BUDGET_H
#include <sqlite3.h>

// Budget structure to hold user budget information
typedef struct {
    float income;
    float expenses;
    float savings_goal;
    int days_in_month;
} Budget;

// Function prototypes
void autoSetDaysInMonth(Budget *budget);
void calculateDailyBudget(sqlite3 *db, Budget *budget);
void showAnalytics(sqlite3 *db);

#endif
