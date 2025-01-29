#define _XOPEN_SOURCE 700
#include "budget.h"
#include "utils.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>

// Function to initialize database
void initializeDatabase(sqlite3 **db, const char *db_name) {
    if (sqlite3_open(db_name, db) != SQLITE_OK) {
        printf("Error: Unable to open database: %s\n", sqlite3_errmsg(*db));
        exit(1);
    }

    // Create savings_goals table
    const char *sql_savings_goals =
        "CREATE TABLE IF NOT EXISTS savings_goals ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "target_amount REAL, "
        "saved_amount REAL DEFAULT 0, "
        "due_date TEXT);";
        
    // Create income table
    const char *sql_income =
        "CREATE TABLE IF NOT EXISTS income ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "amount REAL, "
        "date TEXT);";
    
    // Create expenses table
    const char *sql_expenses =
        "CREATE TABLE IF NOT EXISTS expenses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "category TEXT, "
        "amount REAL, "
        "date TEXT);";
    
    char *err_msg = NULL;

    // Execute all the table creation queries
    if (sqlite3_exec(*db, sql_savings_goals, 0, 0, &err_msg) != SQLITE_OK ||
        sqlite3_exec(*db, sql_income, 0, 0, &err_msg) != SQLITE_OK ||
        sqlite3_exec(*db, sql_expenses, 0, 0, &err_msg) != SQLITE_OK) {
        printf("Error: Failed to create tables: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(*db);
        exit(1);
    }

    printf("Database initialized successfully.\n");
}



// Function to insert income
void insertIncome(sqlite3 *db) {
    float amount;
    int is_recurring;
    char date[20];

    printf("Enter income amount: $");
    amount = getValidFloatInput();  // Ensure valid positive amount

    printf("Is this recurring income? (1 = Yes, 0 = No): ");
    is_recurring = getValidIntInput();  // Ensure valid input for is_recurring

    // Automatically set today's date unless the user chooses to enter a past date
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);

    if (is_recurring) {
        int recurring_start_date_choice;
        printf("Is the recurring income to start on:\n");
        printf("1. The 1st of the month\n");
        printf("2. The date entered above\n");
        printf("3. A custom date\n");
        printf("Enter your choice: ");
        recurring_start_date_choice = getValidIntInput();  // Validate input for the date choice

        switch (recurring_start_date_choice) {
            case 1:
                // Set the recurring income to the 1st of the current month
                struct tm first_of_month_tm = {0};
                first_of_month_tm.tm_year = tm_info->tm_year;
                first_of_month_tm.tm_mon = tm_info->tm_mon;
                first_of_month_tm.tm_mday = 1;
                strftime(date, sizeof(date), "%Y-%m-%d", &first_of_month_tm);
                break;

            case 2:
                // Set to the date entered
                break;

            case 3:
                // Allow custom date input
                printf("Enter custom date (YYYY-MM-DD): ");
                fgets(date, sizeof(date), stdin);
                date[strcspn(date, "\n")] = 0;  // Remove trailing newline
                break;

            default:
                printf("Invalid choice. Setting to the date entered.\n");
                break;
        }
    }

    // Insert income into the database
    const char *sql = "INSERT INTO income (amount, date) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, amount);
        sqlite3_bind_text(stmt, 2, date, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Income added: $%.2f on %s\n", amount, date);
        } else {
            printf("Error: Failed to add income: %s\n", sqlite3_errmsg(db));
        }
    }
    sqlite3_finalize(stmt);
}

void insertExpense(sqlite3 *db) {
    char category[MAX_NAME_LENGTH];
    float amount;
    int is_recurring;
    char date[20];

    printf("Enter expense category: ");
    scanf("%s", category);

    printf("Enter amount: $");
    amount = getValidFloatInput();  // Ensure valid positive amount

    printf("Is this a recurring expense? (1 = Yes, 0 = No): ");
    is_recurring = getValidIntInput();  // Ensure valid input for is_recurring

    // Automatically set today's date unless the user wants to enter a past date
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);

    if (is_recurring) {
        int recurring_start_date_choice;
        printf("Is the recurring expense to start on:\n");
        printf("1. The 1st of the month\n");
        printf("2. The date entered above\n");
        printf("3. A custom date\n");
        printf("Enter your choice: ");
        recurring_start_date_choice = getValidIntInput();  // Validate input for the date choice

        switch (recurring_start_date_choice) {
            case 1:
                // Set the recurring expense to the 1st of the current month
                struct tm first_of_month_tm = {0};
                first_of_month_tm.tm_year = tm_info->tm_year;
                first_of_month_tm.tm_mon = tm_info->tm_mon;
                first_of_month_tm.tm_mday = 1;
                strftime(date, sizeof(date), "%Y-%m-%d", &first_of_month_tm);
                break;

            case 2:
                // Set to the date entered
                break;

            case 3:
                // Allow custom date input
                printf("Enter custom date (YYYY-MM-DD): ");
                fgets(date, sizeof(date), stdin);
                date[strcspn(date, "\n")] = 0;  // Remove trailing newline
                break;

            default:
                printf("Invalid choice. Setting to the date entered.\n");
                break;
        }
    }

    // Insert expense into the database
    const char *sql = "INSERT INTO expenses (category, amount, date) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, category, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, amount);
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Expense added: %s - $%.2f on %s\n", category, amount, date);
        } else {
            printf("Error: Failed to add expense: %s\n", sqlite3_errmsg(db));
        }
    }
    sqlite3_finalize(stmt);
}

// Function to insert a savings goal
void insertSavingsGoal(sqlite3 *db, const char *name, float target_amount, const char *due_date) {
    const char *sql = "INSERT INTO savings_goals (name, target_amount, saved_amount, due_date) VALUES (?, ?, 0, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, target_amount);
        sqlite3_bind_text(stmt, 3, due_date, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Savings goal added: %s - Target: $%.2f, Due: %s\n", name, target_amount, due_date);
        } else {
            printf("Error: Failed to add savings goal: %s\n", sqlite3_errmsg(db));
        }
    }
    sqlite3_finalize(stmt);
}

// Function to update savings goal
void updateSavingsGoal(sqlite3 *db, int goal_id, float amount) {
    const char *sql = "UPDATE savings_goals SET saved_amount = saved_amount + ? WHERE id = ? RETURNING id;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, goal_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Savings goal updated successfully.\n");
    } else {
        printf("Error: Goal ID not found.\n");
    }   
}

// Function to fetch and display savings goals
void fetchSavingsGoals(sqlite3 *db) {
    const char *sql = "SELECT id, name, target_amount, saved_amount, due_date FROM savings_goals;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    printf("\n--- Savings Goals ---\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("[ID: %d] Goal: %s, Target: $%.2f, Saved: $%.2f, Due: %s\n",
            sqlite3_column_int(stmt, 0),  // ID
            sqlite3_column_text(stmt, 1),  // Name
            sqlite3_column_double(stmt, 2), // Target amount
            sqlite3_column_double(stmt, 3), // Saved amount
            sqlite3_column_text(stmt, 4)); // Due date
    }
    sqlite3_finalize(stmt);
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
        cJSON_AddStringToObject(expense, "category", (const char*)sqlite3_column_text(stmt, 0));
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
        cJSON_AddStringToObject(goal, "name", (const char*)sqlite3_column_text(stmt, 0));
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
        cJSON_AddStringToObject(entry, "type", (const char*)sqlite3_column_text(stmt, 0));
        cJSON_AddStringToObject(entry, "description", (const char*)sqlite3_column_text(stmt, 1));
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

#include "database.h"
#include <sqlite3.h>
#include <stdio.h>
#include <time.h>

// Function to get the last processed month
void getLastProcessedMonth(sqlite3 *db, int *year, int *month) {
    const char *sql = "SELECT year, month FROM last_processed_month WHERE id = 1;";
    sqlite3_stmt *stmt;

    *year = 0;
    *month = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            *year = sqlite3_column_int(stmt, 0);
            *month = sqlite3_column_int(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);
}

// Function to update the last processed month
void updateLastProcessedMonth(sqlite3 *db, int year, int month) {
    const char *sql = "INSERT INTO last_processed_month (id, year, month) VALUES (1, ?, ?) "
                      "ON CONFLICT(id) DO UPDATE SET year = excluded.year, month = excluded.month;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, year);
        sqlite3_bind_int(stmt, 2, month);
        sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
}

// Function to apply recurring income and expenses automatically at the start of a new month
void applyRecurringTransactions(sqlite3 *db) {
    // Get current date
    time_t t = time(NULL);
    struct tm *current_time = localtime(&t);
    int current_year = current_time->tm_year + 1900;
    int current_month = current_time->tm_mon + 1;

    // Get last processed month from the database
    int last_year = 0, last_month = 0;
    getLastProcessedMonth(db, &last_year, &last_month);

    // Check if we've already processed this month
    if (last_year == current_year && last_month == current_month) {
        printf("Recurring transactions already applied for %d/%d.\n", current_month, current_year);
        return; // Exit, no need to process again
    }

    printf("\nApplying recurring income and expenses for %d/%d...\n", current_month, current_year);

    // Generate the current date in YYYY-MM-DD format
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d", localtime(&t));

    // Process recurring income
    const char *recurring_income_sql = "SELECT description, amount FROM recurring WHERE type = 'income';";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, recurring_income_sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *description = (const char*)sqlite3_column_text(stmt, 0);
            float amount = sqlite3_column_double(stmt, 1);

            // Insert recurring income into the income table
            const char *insert_income_sql = "INSERT INTO income (amount, date) VALUES (?, ?);";
            sqlite3_stmt *income_stmt;

            if (sqlite3_prepare_v2(db, insert_income_sql, -1, &income_stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_double(income_stmt, 1, amount);
                sqlite3_bind_text(income_stmt, 2, date, -1, SQLITE_STATIC);
                
                if (sqlite3_step(income_stmt) == SQLITE_DONE) {
                    printf("Added recurring income: %s - $%.2f on %s\n", description, amount, date);
                } else {
                    printf("Error: Failed to insert recurring income: %s\n", sqlite3_errmsg(db));
                }
            }
            sqlite3_finalize(income_stmt);
        }
    }
    sqlite3_finalize(stmt);

    // Process recurring expenses
    const char *recurring_expense_sql = "SELECT description, amount FROM recurring WHERE type = 'expense';";

    if (sqlite3_prepare_v2(db, recurring_expense_sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *description = (const char*)sqlite3_column_text(stmt, 0);
            float amount = sqlite3_column_double(stmt, 1);

            // Insert recurring expenses into the expenses table
            const char *insert_expense_sql = "INSERT INTO expenses (category, amount, date) VALUES (?, ?, ?);";
            sqlite3_stmt *expense_stmt;

            if (sqlite3_prepare_v2(db, insert_expense_sql, -1, &expense_stmt, NULL) == SQLITE_OK) {
                sqlite3_bind_text(expense_stmt, 1, description, -1, SQLITE_STATIC);
                sqlite3_bind_double(expense_stmt, 2, amount);
                sqlite3_bind_text(expense_stmt, 3, date, -1, SQLITE_STATIC);
                
                if (sqlite3_step(expense_stmt) == SQLITE_DONE) {
                    printf("Added recurring expense: %s - $%.2f on %s\n", description, amount, date);
                } else {
                    printf("Error: Failed to insert recurring expense: %s\n", sqlite3_errmsg(db));
                }
            }
            sqlite3_finalize(expense_stmt);
        }
    }
    sqlite3_finalize(stmt);

    // Update last processed month so transactions aren't duplicated
    updateLastProcessedMonth(db, current_year, current_month);

    printf("Recurring transactions applied successfully.\n");
}
