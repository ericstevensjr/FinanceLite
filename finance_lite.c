#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include <sqlite3.h>

// Define constants
#define MAX_NAME_LENGTH 50       

// Structure to hold budget information
typedef struct {
    float income;
    int days_in_month;
} Budget;

// Function prototypes
void autoSetDaysInMonth(Budget *budget);
void initializeDatabase(sqlite3 **db, const char *db_name);
void insertIncome(sqlite3 *db, float amount, const char *date);
void insertExpense(sqlite3 *db, const char *category, float amount, const char *date);
void insertSavingsGoal(sqlite3 *db, const char *name, float target_amount);
void updateSavingsGoal(sqlite3 *db, int goal_id, float amount);
void fetchSavingsGoals(sqlite3 *db);
void insertRecurringEntry(sqlite3 *db, const char *type, const char *description, float amount);
void fetchRecurringEntries(sqlite3 *db);
void fetchExpenses(sqlite3 *db);
void calculateDailyBudget(sqlite3 *db, Budget *budget);
void showAnalytics(sqlite3 *db);
void saveBudgetToJSON(sqlite3 *db, const char *filename);

int main() {
    sqlite3 *db;
    initializeDatabase(&db, "finance_lite.db");

    Budget budget = {0, 30}; 
    int choice;
    char filename[] = "finance_lite_backup.json"; // Default filename for JSON export

    // Auto-set the number of days in the current month
    autoSetDaysInMonth(&budget);

    do {
        printf("\n=== Finance Lite ===\n");
        printf("1. Add Monthly Income\n");
        printf("2. Add Expense\n");
        printf("3. Add Savings Goal\n");
        printf("4. Update Savings Goal Progress\n");
        printf("5. Show Savings Progress\n");
        printf("6. Calculate Daily Budget\n");
        printf("7. Show Analytics\n");
        printf("8. Manage Recurring Entries\n");
        printf("9. Export Budget to JSON\n");
        printf("10. Save and Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                float income;
                char date[20];
                printf("Enter your monthly income: $");
                scanf("%f", &income);
                printf("Enter date (YYYY-MM-DD): ");
                scanf("%s", date);
                insertIncome(db, income, date);
                break;
            }
            case 2: {
                char category[MAX_NAME_LENGTH], date[20];
                float amount;
                printf("Enter expense category: ");
                scanf("%s", category);
                printf("Enter amount: $");
                scanf("%f", &amount);
                printf("Enter date (YYYY-MM-DD): ");
                scanf("%s", date);
                insertExpense(db, category, amount, date);
                break;
            }
            case 3: {
                char name[MAX_NAME_LENGTH];
                float target_amount;
                printf("Enter savings goal name: ");
                scanf("%s", name);
                printf("Enter target amount: $");
                scanf("%f", &target_amount);
                insertSavingsGoal(db, name, target_amount);
                break;
            }
            case 4: {
                int goal_id;
                float amount;
                fetchSavingsGoals(db);
                printf("Enter the ID of the savings goal to update: ");
                scanf("%d", &goal_id);
                printf("Enter the amount you saved: $");
                scanf("%f", &amount);
                updateSavingsGoal(db, goal_id, amount);
                break;
            }
            case 5:
                fetchSavingsGoals(db);
                break;
            case 6:
                calculateDailyBudget(db, &budget);
                break;
            case 7:
                showAnalytics(db);
                break;
            case 8:
                fetchRecurringEntries(db);
                break;
            case 9:
                saveBudgetToJSON(db, filename);
                break;
            case 10:
                printf("Budget saved. Goodbye!\n");
                sqlite3_close(db);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 10);

    return 0;
}


// Function to determine the number of days in the current month
void autoSetDaysInMonth(Budget *budget) {
    time_t t = time(NULL);
    struct tm *current_time = localtime(&t);
    int month = current_time->tm_mon + 1;
    int year = current_time->tm_year + 1900;

    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            budget->days_in_month = 31;
            break;
        case 4: case 6: case 9: case 11:
            budget->days_in_month = 30;
            break;
        case 2:
            budget->days_in_month = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
            break;
        default:
            budget->days_in_month = 30;
    }
    printf("Days in the current month (%d/%d): %d\n", month, year, budget->days_in_month);
}

// Function to initialize SQLite database
void initializeDatabase(sqlite3 **db, const char *db_name) {
    if (sqlite3_open(db_name, db) != SQLITE_OK) {
        printf("Error: Unable to open database: %s\n", sqlite3_errmsg(*db));
        exit(1);
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS income (id INTEGER PRIMARY KEY, amount REAL, date TEXT);"
        "CREATE TABLE IF NOT EXISTS expenses (id INTEGER PRIMARY KEY, category TEXT, amount REAL, date TEXT);"
        "CREATE TABLE IF NOT EXISTS savings_goals (id INTEGER PRIMARY KEY, name TEXT, target_amount REAL, saved_amount REAL);"
        "CREATE TABLE IF NOT EXISTS recurring (id INTEGER PRIMARY KEY, type TEXT, description TEXT, amount REAL);";

    char *err_msg = NULL;
    if (sqlite3_exec(*db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        printf("Error: Failed to create tables: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(*db);
        exit(1);
    }

    printf("Database initialized successfully.\n");
}

// Function to insert income
void insertIncome(sqlite3 *db, float amount, const char *date) {
    const char *sql = "INSERT INTO income (amount, date) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    printf("Income added successfully.\n");
}

// Function to insert expense
void insertExpense(sqlite3 *db, const char *category, float amount, const char *date) {
    const char *sql = "INSERT INTO expenses (category, amount, date) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, category, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, amount);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    printf("Expense added successfully.\n");
}

// Function to insert savings goal
void insertSavingsGoal(sqlite3 *db, const char *name, float target_amount) {
    const char *sql = "INSERT INTO savings_goals (name, target_amount, saved_amount) VALUES (?, ?, 0);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, target_amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    printf("Savings goal added successfully.\n");
}

// Function to update savings goal
void updateSavingsGoal(sqlite3 *db, int goal_id, float amount) {
    const char *sql = "UPDATE savings_goals SET saved_amount = saved_amount + ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, goal_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    printf("Savings goal updated successfully.\n");
}

// Function to fetch and display savings goals
void fetchSavingsGoals(sqlite3 *db) {
    const char *sql = "SELECT name, target_amount, saved_amount FROM savings_goals;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    printf("\n--- Savings Goals ---\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Goal: %s, Target: $%.2f, Saved: $%.2f\n", 
               sqlite3_column_text(stmt, 0), 
               sqlite3_column_double(stmt, 1), 
               sqlite3_column_double(stmt, 2));
    }
    sqlite3_finalize(stmt);
}

// Function to insert recurring entry
void insertRecurringEntry(sqlite3 *db, const char *type, const char *description, float amount) {
    const char *sql = "INSERT INTO recurring (type, description, amount) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, type, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    printf("Recurring entry added successfully.\n");
}

// Function to fetch recurring entries
void fetchRecurringEntries(sqlite3 *db) {
    const char *sql = "SELECT type, description, amount FROM recurring;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    printf("\n--- Recurring Entries ---\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Type: %s, Description: %s, Amount: $%.2f\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_double(stmt, 2));
    }
    sqlite3_finalize(stmt);
}

// Function to fetch expenses
void fetchExpenses(sqlite3 *db) {
    const char *sql = "SELECT category, amount, date FROM expenses;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    printf("\n--- Expenses ---\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Category: %s, Amount: $%.2f, Date: %s\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_double(stmt, 1),
               sqlite3_column_text(stmt, 2));
    }
    sqlite3_finalize(stmt);
}

// Function to calculate daily budget
void calculateDailyBudget(sqlite3 *db, Budget *budget) {
    float total_expenses = 0;
    fetchExpenses(db);
    float daily_budget = (budget->income - total_expenses) / budget->days_in_month;
    printf("\nDaily Budget: $%.2f\n", daily_budget);
}

// Function to show analytics
void showAnalytics(sqlite3 *db) {
    printf("\n=== Budget Analytics ===\n");

    // 1. Calculate Total Income
    const char *income_sql = "SELECT SUM(amount) FROM income;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, income_sql, -1, &stmt, NULL);
    float total_income = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total_income = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    printf("Total Monthly Income: $%.2f\n", total_income);

    // 2. Calculate Total Expenses
    const char *expense_sql = "SELECT SUM(amount) FROM expenses;";
    sqlite3_prepare_v2(db, expense_sql, -1, &stmt, NULL);
    float total_expenses = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total_expenses = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    printf("Total Expenses: $%.2f\n", total_expenses);

    // 3. Breakdown of Expenses by Category
    printf("\nExpense Breakdown by Category:\n");
    const char *category_sql = "SELECT category, SUM(amount) FROM expenses GROUP BY category ORDER BY SUM(amount) DESC;";
    sqlite3_prepare_v2(db, category_sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf(" - %s: $%.2f\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_double(stmt, 1));
    }
    sqlite3_finalize(stmt);

    // 4. Recurring Expenses Total
    const char *recurring_expense_sql = "SELECT SUM(amount) FROM recurring WHERE type='expense';";
    sqlite3_prepare_v2(db, recurring_expense_sql, -1, &stmt, NULL);
    float recurring_expenses = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        recurring_expenses = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    printf("\nTotal Recurring Expenses: $%.2f\n", recurring_expenses);

    // 5. Show Savings Progress
    printf("\nSavings Goals Progress:\n");
    const char *savings_sql = "SELECT name, target_amount, saved_amount FROM savings_goals;";
    sqlite3_prepare_v2(db, savings_sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *goal_name = sqlite3_column_text(stmt, 0);
        float target_amount = sqlite3_column_double(stmt, 1);
        float saved_amount = sqlite3_column_double(stmt, 2);
        float progress = (saved_amount / target_amount) * 100;
        printf(" - %s: $%.2f / $%.2f (%.2f%% complete)\n",
               goal_name, saved_amount, target_amount, progress);
    }
    sqlite3_finalize(stmt);

    // 6. Calculate Remaining Budget
    float remaining_budget = total_income - (total_expenses + recurring_expenses);
    printf("\nRemaining Budget After Expenses: $%.2f\n", remaining_budget);

    // 7. Recommendations Based on Budget
    if (remaining_budget > 0) {
        printf("\n✔ You have a positive balance. Consider saving or investing.\n");
    } else {
        printf("\n⚠ Warning: Your expenses exceed your income. Consider adjusting spending.\n");
    }

    printf("\n=== End of Analytics ===\n");
}

// Function to save budget into a JSON file
void saveBudgetToJSON(sqlite3 *db, const char *filename) {
    // Create a JSON object
    cJSON *json_budget = cJSON_CreateObject();

    // Export income
    const char *income_sql = "SELECT SUM(amount) FROM income;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, income_sql, -1, &stmt, NULL);
    float total_income = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total_income = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    cJSON_AddNumberToObject(json_budget, "income", total_income);

    // Export expenses
    cJSON *json_expenses = cJSON_CreateArray();
    const char *expense_sql = "SELECT category, amount FROM expenses;";
    sqlite3_prepare_v2(db, expense_sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cJSON *expense = cJSON_CreateObject();
        cJSON_AddStringToObject(expense, "category", sqlite3_column_text(stmt, 0));
        cJSON_AddNumberToObject(expense, "amount", sqlite3_column_double(stmt, 1));
        cJSON_AddItemToArray(json_expenses, expense);
    }
    sqlite3_finalize(stmt);
    cJSON_AddItemToObject(json_budget, "expenses", json_expenses);

    // Export savings goals
    cJSON *json_savings = cJSON_CreateArray();
    const char *savings_sql = "SELECT name, target_amount, saved_amount FROM savings_goals;";
    sqlite3_prepare_v2(db, savings_sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cJSON *goal = cJSON_CreateObject();
        cJSON_AddStringToObject(goal, "name", sqlite3_column_text(stmt, 0));
        cJSON_AddNumberToObject(goal, "target_amount", sqlite3_column_double(stmt, 1));
        cJSON_AddNumberToObject(goal, "saved_amount", sqlite3_column_double(stmt, 2));
        cJSON_AddItemToArray(json_savings, goal);
    }
    sqlite3_finalize(stmt);
    cJSON_AddItemToObject(json_budget, "savings_goals", json_savings);

    // Export recurring transactions
    cJSON *json_recurring = cJSON_CreateArray();
    const char *recurring_sql = "SELECT type, description, amount FROM recurring;";
    sqlite3_prepare_v2(db, recurring_sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "type", sqlite3_column_text(stmt, 0));
        cJSON_AddStringToObject(entry, "description", sqlite3_column_text(stmt, 1));
        cJSON_AddNumberToObject(entry, "amount", sqlite3_column_double(stmt, 2));
        cJSON_AddItemToArray(json_recurring, entry);
    }
    sqlite3_finalize(stmt);
    cJSON_AddItemToObject(json_budget, "recurring_entries", json_recurring);

    // Write JSON to file
    FILE *file = fopen(filename, "w");
    if (file) {
        char *json_string = cJSON_Print(json_budget);
        fprintf(file, "%s", json_string);
        fclose(file);
        printf("Budget exported to %s.\n", filename);
        free(json_string);
    } else {
        printf("Error: Could not save to file.\n");
    }

    // Free memory
    cJSON_Delete(json_budget);
}
