#define _XOPEN_SOURCE 700
#include "budget.h"
#include "database.h"
#include "utils.h"
#include "recurring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include <sqlite3.h>



int main() {
    sqlite3 *db;
    initializeDatabase(&db, "finance_lite.db");

    Budget budget = {0, 0, 0, 30};
    autoSetDaysInMonth(&budget);
    applyRecurringTransactions(db);

    int choice;
    char filename[] = "finance_lite_backup.json";

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
                applyRecurringTransactions(db);
                break;
            case 2:
                insertExpense(db);
                applyRecurringTransactions(db);
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

    sqlite3_close(db);
    return 0;
}
