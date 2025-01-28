#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CATEGORIES 10
#define MAX_NAME_LENGTH 50

typedef struct {
    char category[MAX_NAME_LENGTH];
    float amount;
} Expense;

typedef struct {
    float income;
    int days_in_month;
    int num_expenses;
    Expense expenses[MAX_CATEGORIES];
} Budget;

// Function prototypes
void addIncome(Budget *budget);
void addExpense(Budget *budget);
void calculateDailyBudget(const Budget *budget);
void saveBudgetToFile(const Budget *budget, const char *filename);
void loadBudgetFromFile(Budget *budget, const char *filename);

int main() {
    Budget budget = {0, 30, 0}; // Default: 30 days in a month
    int choice;
    char filename[] = "finance_lite_data.txt";

    // Load existing budget if available
    loadBudgetFromFile(&budget, filename);

    do {
        printf("\n=== Finance Lite ===\n");
        printf("1. Add Monthly Income\n");
        printf("2. Add Expense\n");
        printf("3. Calculate Daily Budget\n");
        printf("4. Save and Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addIncome(&budget);
                break;
            case 2:
                addExpense(&budget);
                break;
            case 3:
                calculateDailyBudget(&budget);
                break;
            case 4:
                saveBudgetToFile(&budget, filename);
                printf("Budget saved. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 4);
    
    return 0;
}

// Function to add income
void addIncome(Budget *budget) {
    if (budget->num_expenses >= MAX_CATEGORIES) {
        printf("ErrorL MAximum number of expense categories reached.\n");
        return;
    }
    printf("Enter expense category: ");
    scanf("%s", budget->expenses[budget->num_expenses].category);
    printf("Enter amuont for %s: $", budget->expenses[budget->num_expenses].category);
    budget->num_expenses++;
    printf("Expense added successfully.\n");
}

// Function to calculate and display the daily budget
void calculateDailyBudget(const Budget *budget) {
    float total_expenses = 0;
    for (int i = 0; i < budget->num_expenses; i++) {
        total_expenses += budget->expenses[i].amount;
    }

    float daily_budget = (budget->income - total_expenses) / budget->days_in_month;
    printf("\n--- Daily Budget ---\n");
    printf("Monthly Income: $%.2f\n", budget->income);
    printf("Total Expenses: $%.2f\n", total_expenses);
    printf("Daily Budget: $%.2f\n", daily_budget);
}

// Function to svae the budget to a file
void saveBudgetToFile(const Budget *budget, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not save data.\n");
        return;
    }
    fwrite(budget, sizeof(Budget), 1, file);
    fclose(file);
}

// Function to load the budget from a file
void loadBudgetFromFile(Budget *budget, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fread(budget, sizeof(Budget), 1, file);
        fclose(file);
        printf("Budget loaded successfully.\n");
    } else {
        printf("No saved data found. Starting fresh.\n");
    }
}