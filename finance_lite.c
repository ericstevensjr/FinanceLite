#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CATEGORIES 10
#define MAX_NAME_LENGTH 50
#define MAX_GOALS 5

typedef struct {
    char category[MAX_NAME_LENGTH];
    float amount;
} Expense;

typedef struct {
    char name[MAX_NAME_LENGTH];
    float target_amount;
    float saved_amount;
} SavingsGoal;

typedef struct {
    float income;
    int days_in_month;
    int num_expenses;
    Expense expenses[MAX_CATEGORIES];
    int num_goals;
    SavingsGoal goals[MAX_GOALS];
} Budget;

// Function prototypes
void addIncome(Budget *budget);
void addExpense(Budget *budget);
void addSavingsGoal(Budget *budget);
void updateSavingsGoal(Budget *budget);
void calculateDailyBudget(const Budget *budget);
void saveBudgetToFile(const Budget *budget, const char *filename);
void loadBudgetFromFile(Budget *budget, const char *filename);
void showSavingsProgress(const Budget *budget);

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
        printf("3. Add Savings Goal\n");
        printf("4. Update Savings Goal Proress\n");
        printf("5. Show Savings Progress\n");
        printf("6. Calculate Daily Budget\n");
        printf("7. Save and Exit\n");
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
                addSavingsGoal(&budget);
                break;
            case 4:
                updateSavingsGoal(&budget);
                break;
            case 5:
                showSavingsProgress(&budget);
                break;
            case 6:
                calculateDailyBudget(&budget);
                break;
            case 7:
                saveBudgetToFile(&budget, filename);
                printf("Budget saved. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 7);
    
    return 0;
}

// Function to add income
void addIncome(Budget *budget) {
    float income;
    printf("Enter your monthly income: $");
    if (scanf("%f", &income) != 1 || income < 0) {
        printf("Error: Invalid income amount.\n");
        return;
    }
    budget->income = income;
    printf("Income updated: $%.2f\n", budget->income);
}

// Function to add an expense
void addExpense(Budget *budget) {
    if (budget->num_expenses >= MAX_CATEGORIES) {
        printf("Error: Maximum number of expense categories reached.\n");
        return;
    }
    printf("Enter expense category: ");
    scanf("%s", budget->expenses[budget->num_expenses].category);
    printf("Enter amount for %s: $", budget->expenses[budget->num_expenses].category);
    if (scanf("%f", &budget->expenses[budget->num_expenses].amount) != 1 || budget->expenses[budget->num_expenses].amount < 0) {
        printf("Error: Invalid expense amount.\n");
        return;
    }
    budget->num_expenses++;
    printf("Expense added successfully.\n");
}

// Function to add a savings goal
void addSavingsGoal(Budget *budget) {
    if (budget->num_goals >= MAX_GOALS) {
        printf("Error: Maximum number of savings goals reached.\n");
        return;
    }
    printf("Enter the name of your savings goal: ");
    scanf("%s", budget->goals[budget->num_goals].name);
    printf("Enter the target amount for %s: $", budget->goals[budget->num_goals].name);
    if (scanf("%f", &budget->goals[budget->num_goals].target_amount) != 1 || budget->goals[budget->num_goals].target_amount <= 0) {
        printf("Error: Invalid target amount.\n");
        return;
    }
    budget->goals[budget->num_goals].saved_amount = 0; // Initialize saved amount
    budget->num_goals++;
    printf("Savings goal added successfully.\n");
}

// Function to update savings progress
void updateSavingsGoal(Budget *budget) {
    if (budget->num_goals == 0) {
        printf("No savings goals to update.\n");
        return;
    }
    for (int i = 0; i < budget->num_goals; i++) {
        printf("%d. %s (TargetL $%.2f, Saved: $%.2f)\n", i + 1, budget->goals[i].name, budget->goals[i].target_amount, budget->goals[i].saved_amount);
    }
    printf("Select a goal to update (1-%d): ", budget->num_goals);
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > budget->num_goals) {
        printf("Error: Invalid choice.\n");
        return;
    }
    choice--; // Convert to 0-based index
    float amount;
    printf("Enter the amount you have saved for %s: $", budget->goals[choice].name);
    if (scanf("%f", &amount) != 1 || amount < 0) {
        printf("Error: Invalid amount.\n");
        return;
    }
    budget->goals[choice].saved_amount += amount;
    if (budget->goals[choice].saved_amount > budget->goals[choice].target_amount) {
        budget->goals[choice].saved_amount > budget->goals[choice].target_amount;
    }
    printf("Progress updated for %s. Saved: $%.2f / $%.2f\n", budget->goals[choice].name, budget->goals[choice].saved_amount, budget->goals[choice].target_amount);
}


// Function to show savings progress
void showSavingsProgress(const Budget *budget) {
    if (budget->num_goals == 0) {
        printf("No savings goals to display.\n");
        return;
    }
    printf("\n--- Savings Progress ---\n");
    for (int i = 0; i < budget->num_goals; i++) {
        float progress = (budget->goals[i].saved_amount / budget->goals[i].target_amount) * 100;
        printf("%s: $%.2f / $%.2f (%.2f%% complete)\n", budget->goals[i].name, budget->goals[i].saved_amount, budget->goals[i].target_amount, progress);
    }
}

// Function to calculate and display the daily budget
void calculateDailyBudget(const Budget *budget) {
    float total_expenses = 0;
    float total_savings = 0;

    for (int i = 0; i < budget->num_expenses; i++) {
        total_expenses += budget->expenses[i].amount;
    }

    for (int i = 0; i < budget->num_goals; i++) {
        total_savings += (budget->goals[i].target_amount - budget->goals[i].saved_amount) / budget->days_in_month;
    }

    float daily_budget = (budget->income - total_expenses - total_savings) / budget->days_in_month;
    printf("\n--- Daily Budget ---\n");
    printf("Monthly Income: $%.2f\n", budget->income);
    printf("Total Expenses: $%.2f\n", total_expenses);
    printf("Savings Allocation (Daily): %.2f\n", total_savings);
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