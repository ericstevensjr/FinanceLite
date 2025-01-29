#define _XOPEN_SOURCE 700
#include "budget.h"
#include <stdio.h>
#include <time.h>
#include <sqlite3.h>
#include <string.h>

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

// Function to calculate the daily budget
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
        // Instead of directly passing sqlite3_column_text to cJSON
        const char *goal_name = (const char*) sqlite3_column_text(stmt, 0);
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

