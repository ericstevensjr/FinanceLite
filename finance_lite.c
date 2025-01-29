#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include <sqlite3.h>

// Define constants
#define MAX_NAME_LENGTH 50       

// Budget structure to hold user budget information
typedef struct {
    float income;
    float expenses;
    float savings_goal;
    int days_in_month;
} Budget;

// Function prototypes
void autoSetDaysInMonth(Budget *budget);
void initializeDatabase(sqlite3 **db, const char *db_name);
void insertIncome(sqlite3 *db);
void insertExpense(sqlite3 *db);
void insertSavingsGoal(sqlite3 *db, const char *name, float target_amount, const char *due_date);
void updateSavingsGoal(sqlite3 *db, int goal_id, float amount);
void fetchSavingsGoals(sqlite3 *db);
void calculateDailyBudget(sqlite3 *db, Budget *budget);
void showAnalytics(sqlite3 *db);
void applyRecurringTransactions(sqlite3 *db);
void manageRecurringEntries(sqlite3 *db);
void removeSavingsGoal(sqlite3 *db);
void fetchRecurringEntries(sqlite3 *db);
void editRecurringEntry(sqlite3 *db);
void removeRecurringEntry(sqlite3 *db);
void saveBudgetToJSON(sqlite3 *db, const char *filename);

// Helper function to get a valid integer input from the user
int getValidIntInput() {
    int value;
    while (scanf("%d", &value) != 1) {
        printf("Error: Invalid input. Please enter a valid integer.\n");
        while (getchar() != '\n');  // Clear buffer
    }
    return value;
}

// Helper function to get a valid float input from the user
float getValidFloatInput() {
    float value;
    while (scanf("%f", &value) != 1 || value <= 0) {
        printf("Error: Invalid input. Please enter a valid positive number.\n");
        while (getchar() != '\n');  // Clear buffer
    }
    return value;
}

// Helper function to get a valid non-empty string input
void getValidStringInput(char *input, int max_len) {
    while (1) {
        fgets(input, max_len, stdin);
        input[strcspn(input, "\n")] = 0;  // Remove trailing newline
        if (strlen(input) == 0) {
            printf("Error: Input cannot be empty. Please enter a valid string.\n");
        } else {
            break;
        }
    }
}

// Helper function to validate date input (YYYY-MM-DD)
int getValidDateInput(char *date, int max_len) {
    while (1) {
        fgets(date, max_len, stdin);
        date[strcspn(date, "\n")] = 0;  // Remove trailing newline
        
        struct tm tm;
        if (strptime(date, "%Y-%m-%d", &tm) == NULL) {
            printf("Error: Invalid date format. Please enter a date in the format YYYY-MM-DD.\n");
        } else {
            return 1; // Valid date
        }
    }
}

int main() {
    sqlite3 *db;
    initializeDatabase(&db, "finance_lite.db");

    Budget budget = {0, 0, 0, 30}; 
    int choice;
    char filename[] = "finance_lite_backup.json"; // Default filename for JSON export

    autoSetDaysInMonth(&budget);
    applyRecurringTransactions(db);

    do {
        printf("\n=== Finance Lite ===\n");
        printf("1. Add Income\n");
        printf("2. Add Expense\n");
        printf("3. Add Savings Goal\n");
        printf("4. Update Savings Goal Progress\n");
        printf("5. Show Savings Progress\n");
        printf("6. Calculate Daily Budget\n");
        printf("7. Show Analytics\n");
        printf("8. Manage Recurring Entries and Savings\n");
        printf("9. Export Budget to JSON\n");
        printf("10. Save and Exit\n");
        printf("Enter your choice: ");
        choice = getValidIntInput();
        getchar(); // Consume newline left in buffer

        switch (choice) {
            case 1:
                insertIncome(db);
                break;
            case 2:
                insertExpense(db);
                break;
            case 3: {
                char name[MAX_NAME_LENGTH], due_date[11];
                float target_amount;

                printf("Enter savings goal name: ");
                getValidStringInput(name, MAX_NAME_LENGTH);

                printf("Enter target amount: $");
                target_amount = getValidFloatInput();  // Use the helper function for validation

                printf("Enter due date (YYYY-MM-DD): ");
                scanf("%10s", due_date);

                insertSavingsGoal(db, name, target_amount, due_date);
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
                manageRecurringEntries(db);
                break;
            case 9:
                saveBudgetToJSON(db, filename);
                break;
            case 10: {
                char confirm_exit;
                printf("\nAre you sure you want to exit? (Y/N): ");
                scanf(" %c", &confirm_exit); // Notice the space before %c to catch newline character

                if (confirm_exit == 'Y' || confirm_exit == 'y') {
                    printf("Budget saved. Goodbye!\n");
                    sqlite3_close(db);
                    exit(0); // Exits only if user confirms
                }
                choice = -1;
                printf("Returning to menu...\n");
                break; // Return to the main menu if not exiting
            }
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 10); // Exit loop when choice is 10 (Save and Exit)

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

// Function to initialize database and ensure schema is up to date
void initializeDatabase(sqlite3 **db, const char *db_name) {
    if (sqlite3_open(db_name, db) != SQLITE_OK) {
        printf("Error: Unable to open database: %s\n", sqlite3_errmsg(*db));
        exit(1);
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS savings_goals ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "target_amount REAL, "
        "saved_amount REAL DEFAULT 0, "
        "due_date TEXT);";

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


// Function to insert recurring entry with start date validation
void insertRecurringEntry(sqlite3 *db, const char *type) {
    char description[MAX_NAME_LENGTH];
    float amount;
    char date[20];

    printf("Enter description for recurring %s: ", type);
    getValidStringInput(description, MAX_NAME_LENGTH);  // Ensure valid description

    printf("Enter amount: $");
    amount = getValidFloatInput();  // Ensure valid positive amount

    printf("Choose the start date for the recurring transaction:\n");
    printf("1. Set to the 1st of the month\n");
    printf("2. Set to the date entered\n");
    printf("3. Set to a custom date\n");
    int choice = getValidIntInput();

    switch (choice) {
        case 1:
            // Set date to the 1st of the current month
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            tm_info->tm_mday = 1;  // Set day to the 1st
            strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
            break;
        case 2:
            // Set to the date entered when the transaction was added
            strftime(date, sizeof(date), "%Y-%m-%d", localtime(&t));
            break;
        case 3:
            // Ask for custom date
            printf("Enter custom date (YYYY-MM-DD): ");
            getValidDateInput(date, sizeof(date));  // Validate custom date input
            break;
        default:
            printf("Error: Invalid choice.\n");
            return;
    }

    const char *sql = "INSERT INTO recurring (type, description, amount, date) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, type, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, description, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, amount);
        sqlite3_bind_text(stmt, 4, date, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Recurring %s added: %s - $%.2f, Start Date: %s\n", type, description, amount, date);
        } else {
            printf("Error: Failed to add recurring %s: %s\n", type, sqlite3_errmsg(db));
        }
    }
    sqlite3_finalize(stmt);
}

// Function to manage recurring entries and savings goals
void manageRecurringEntries(sqlite3 *db) {
    int choice;
    do {
        printf("\n--- Manage Recurring Entries & Savings Goals ---\n");
        printf("1. Add Recurring Income\n");
        printf("2. Add Recurring Expense\n");
        printf("3. View Recurring Entries\n");
        printf("4. Edit Recurring Entry\n");
        printf("5. Remove Recurring Entry\n");
        printf("6. View Savings Goals\n");
        printf("7. Remove Savings Goal\n");  // Added option for removing savings goals
        printf("8. Return to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                insertRecurringEntry(db, "income");
                break;
            case 2:
                insertRecurringEntry(db, "expense");
                break;
            case 3:
                fetchRecurringEntries(db);
                break;
            case 4:
                editRecurringEntry(db);
                break;
            case 5:
                removeRecurringEntry(db);
                break;
            case 6:
                fetchSavingsGoals(db);
                break;
            case 7:
                removeSavingsGoal(db);  // Handle remove savings goal
                break;
            case 8:
                return;  // Return to the main menu
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 8);
}

// Function to remove savings goal by ID or Name
// Function to remove savings goal by ID or Name after displaying the list
void removeSavingsGoal(sqlite3 *db) {
    int choice;
    int goal_id;
    char goal_name[MAX_NAME_LENGTH];

    // Display all savings goals first
    fetchSavingsGoals(db);

    // Ask user to choose removal method
    printf("\nEnter 1 to remove by ID or 2 to remove by Name: ");
    scanf("%d", &choice);

    if (choice == 1) {
        // Remove by ID
        printf("Enter the ID of the savings goal you want to remove: ");
        goal_id = getValidIntInput();

        const char *sql = "DELETE FROM savings_goals WHERE id = ?;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, goal_id);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf("Savings goal removed successfully!\n");
            } else {
                printf("Error: Failed to remove savings goal: %s\n", sqlite3_errmsg(db));
            }
        }
        sqlite3_finalize(stmt);

    } else if (choice == 2) {
        // Remove by Name
        printf("Enter the name of the savings goal you want to remove: ");
        getchar(); // Consume the newline character left by previous scanf
        getValidStringInput(goal_name, MAX_NAME_LENGTH);

        const char *sql = "DELETE FROM savings_goals WHERE name = ?;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, goal_name, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf("Savings goal removed successfully!\n");
            } else {
                printf("Error: Failed to remove savings goal: %s\n", sqlite3_errmsg(db));
            }
        }
        sqlite3_finalize(stmt);

    } else {
        printf("Invalid choice.\n");
    }
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
    float total_income = 0;
    float total_expenses = 0;
    float total_savings_today = 0;

    // Fetch total income (recurring + one-time)
    const char *income_sql = "SELECT IFNULL(SUM(amount), 0) FROM income;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, income_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_income = sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    // Fetch recurring income (if any)
    const char *recurring_income_sql = "SELECT IFNULL(SUM(amount), 0) FROM recurring WHERE type = 'income';";
    if (sqlite3_prepare_v2(db, recurring_income_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_income += sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    // Fetch total expenses (recurring + one-time)
    const char *expense_sql = "SELECT IFNULL(SUM(amount), 0) FROM expenses;";
    if (sqlite3_prepare_v2(db, expense_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_expenses = sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    // Fetch recurring expenses (if any)
    const char *recurring_expense_sql = "SELECT IFNULL(SUM(amount), 0) FROM recurring WHERE type = 'expense';";
    if (sqlite3_prepare_v2(db, recurring_expense_sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_expenses += sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    // Fetch savings goal data and calculate daily savings goal
    const char *savings_sql = "SELECT target_amount, saved_amount, due_date FROM savings_goals;";
    if (sqlite3_prepare_v2(db, savings_sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            float target_amount = sqlite3_column_double(stmt, 0);
            float saved_amount = sqlite3_column_double(stmt, 1);
            const char *due_date = (const char *)sqlite3_column_text(stmt, 2);

            if (due_date == NULL || strlen(due_date) == 0) {
                printf("Warning: Invalid due date for savings goal. Skipping...\n");
                continue;
            }

            // Parse the due date to calculate how many days left
            struct tm due_date_tm = {0};
            time_t now = time(NULL);
            struct tm *current_time = localtime(&now);

            if (strptime(due_date, "%Y-%m-%d", &due_date_tm) == NULL) {
                printf("Warning: Invalid due date format for savings goal, skipping calculation.\n");
                continue;
            }

            time_t due_date_time = mktime(&due_date_tm);
            int days_left = (due_date_time - now) / (60 * 60 * 24);

            // Ensure we don't divide by 0
            if (days_left <= 0) {
                days_left = 1;
            }

            // Calculate how much needs to be saved per day
            float daily_savings = (target_amount - saved_amount) / days_left;
            total_savings_today += (daily_savings > 0) ? daily_savings : 0;
        }
    }
    sqlite3_finalize(stmt);

    // Calculate the total daily budget
    float daily_budget = (total_income - total_expenses - total_savings_today) / budget->days_in_month;

    // Display the result
    printf("\n--- Daily Budget ---\n");
    printf("Total Income: $%.2f\n", total_income);
    printf("Total Expenses: $%.2f\n", total_expenses);
    printf("Total Savings Needed for Today: $%.2f\n", total_savings_today);
    printf("Daily Budget (after savings): $%.2f\n", daily_budget);
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

// Function to retrieve the last month processed
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
            const char *description = (const char *)sqlite3_column_text(stmt, 0);
            float amount = sqlite3_column_double(stmt, 1);

            // Insert income into the income table
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
            const char *description = (const char *)sqlite3_column_text(stmt, 0);
            float amount = sqlite3_column_double(stmt, 1);

            // Insert expense into the expenses table
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


// Function to fetch recurring entries
void fetchRecurringEntries(sqlite3 *db) {
    const char *sql = "SELECT id, type, description, amount FROM recurring ORDER BY id ASC;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("SQLite Error: %s\n", sqlite3_errmsg(db));  // More detailed error message
        return;
    }


    printf("\n--- Recurring Income & Expenses ---\n");
    int found = 0;  // Track if there are any recurring entries

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *type = sqlite3_column_text(stmt, 1);
        const char *description = sqlite3_column_text(stmt, 2);
        float amount = sqlite3_column_double(stmt, 3);

        printf("[%d] %s - %s: $%.2f\n", id, type, description, amount);
        found = 1;
    }

    sqlite3_finalize(stmt);

    if (!found) {
        printf("No recurring entries found.\n");
    }
}

// Function to edit recurring entries
void editRecurringEntry(sqlite3 *db) {
    int id;
    char new_description[MAX_NAME_LENGTH];
    float new_amount;

    // Show existing recurring entries first
    fetchRecurringEntries(db);

    printf("\nEnter the ID of the recurring entry you want to edit: ");
    if (scanf("%d", &id) != 1) {
        printf("Error: Invalid ID.\n");
        return;
    }

    printf("Enter new description: ");
    scanf("%s", new_description);

    printf("Enter new amount: $");
    if (scanf("%f", &new_amount) != 1 || new_amount <= 0) {
        printf("Error: Invalid amount. Must be a positive number.\n");
        return;
    }

    const char *sql = "UPDATE recurring SET description = ?, amount = ? WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, new_description, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, new_amount);
        sqlite3_bind_int(stmt, 3, id);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Recurring entry updated successfully!\n");
        } else {
            printf("Error: Failed to update recurring entry: %s\n", sqlite3_errmsg(db));
        }
    }
    sqlite3_finalize(stmt);
}


// Function to remove recurring entries
void removeRecurringEntry(sqlite3 *db) {
    int id;

    // Show existing recurring entries first
    fetchRecurringEntries(db);

    printf("\nEnter the ID of the recurring entry you want to remove: ");
    if (scanf("%d", &id) != 1) {
        printf("Error: Invalid ID.\n");
        return;
    }

    const char *sql = "DELETE FROM recurring WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Recurring entry removed successfully!\n");
        } else {
            printf("Error: Failed to remove recurring entry: %s\n", sqlite3_errmsg(db));
        }
    }
    sqlite3_finalize(stmt);
}
