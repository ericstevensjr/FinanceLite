#define _XOPEN_SOURCE 700
#include "utils.h"
#include "recurring.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

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
                editRecurringEntries(db);
                break;
            case 5:
                removeRecurringEntries(db);
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

// Fetch recurring entries
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
        const char *type = (const char*)sqlite3_column_text(stmt, 1);  // Cast to const char*
        const char *description = (const char*)sqlite3_column_text(stmt, 2);  // Cast to const char*
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
void editRecurringEntries(sqlite3 *db) {
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
void removeRecurringEntries(sqlite3 *db) {
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
