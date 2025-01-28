#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CATEGORIES 10       
#define MAX_NAME_LENGTH 50
#define MAX_GOALS 5             
#define MAX_RECURRING 10        

// Data structure for individual expenses
typedef struct {
    char category[MAX_NAME_LENGTH]; 
    float amount;                   
} Expense;

// Data structure for recurring income/expense entries
typedef struct {
    char description[MAX_NAME_LENGTH]; 
    float amount;                      
} RecurringEntry;

// Data structure for savings goals
typedef struct {
    char name[MAX_NAME_LENGTH];   
    float target_amount;          
    float saved_amount;           
} SavingsGoal;

// Main budget structure
typedef struct {
    float income;                 
    int days_in_month;            
    int num_expenses;             
    Expense expenses[MAX_CATEGORIES]; 
    int num_goals;                    
    SavingsGoal goals[MAX_GOALS];     
    int num_recurring_income;         
    int num_recurring_expenses;       
    RecurringEntry recurring_income[MAX_RECURRING];   
    RecurringEntry recurring_expenses[MAX_RECURRING]; 
} Budget;

// Function prototypes
void addIncome(Budget *budget);
void addExpense(Budget *budget);
void addSavingsGoal(Budget *budget);
void updateSavingsGoal(Budget *budget);
void showSavingsProgress(const Budget *budget);
void calculateDailyBudget(const Budget *budget);
void showAnalytics(const Budget *budget);
void manageRecurringEntries(Budget *budget);
void addRecurringIncome(Budget *budget);
void addRecurringExpense(Budget *budget);
void viewRecurringEntries(const Budget *budget);
void saveBudgetToFile(const Budget *budget, const char *filename);
void loadBudgetFromFile(Budget *budget, const char *filename);
void autoSetDaysInMonth(Budget *budget);

int main() {
    Budget budget = {0, 30, 0, 0, 0, 0, 0, 0}; 
    int choice;
    char filename[] = "finance_lite_data.txt";

    // Auto-set the number of days in the current month
    autoSetDaysInMonth(&budget);

    // Load existing budget from file if available
    loadBudgetFromFile(&budget, filename);

    do {
        // Display the main menu
        printf("\n=== Finance Lite ===\n");
        printf("1. Add Monthly Income\n");
        printf("2. Add Expense\n");
        printf("3. Add Savings Goal\n");
        printf("4. Update Savings Goal Progress\n");
        printf("5. Show Savings Progress\n");
        printf("6. Calculate Daily Budget\n");
        printf("7. Show Analytics\n");
        printf("8. Manage Recurring Entries\n");
        printf("9. Save and Exit\n");
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
                showAnalytics(&budget);
                break;
            case 8:
                manageRecurringEntries(&budget);
                break;
            case 9:
                saveBudgetToFile(&budget, filename);
                printf("Budget saved. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 9);

    return 0;
}

// Function to add income to the budget
void addIncome(Budget *budget) {
    float income;
    printf("Enter your monthly income: $");
    if (scanf("%f", &income) != 1 || income < 0) {
        printf("Error: Invalid income amount.\n");
        return;
    }
    budget->income += income;
    printf("Income updated: $%.2f\n", budget->income);
}

// Function to add a one-time expense to the budget
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

// Function to add a savings goal to the budget
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
    budget->goals[budget->num_goals].saved_amount = 0;
    budget->num_goals++;
    printf("Savings goal added successfully.\n");
}

// Function to update the progress of a savings goal
void updateSavingsGoal(Budget *budget) {
    if (budget->num_goals == 0) {
        printf("No savings goals to update.\n");
        return;
    }
    for (int i = 0; i < budget->num_goals; i++) {
        printf("%d. %s (Target: $%.2f, Saved: $%.2f)\n", i + 1, budget->goals[i].name, budget->goals[i].target_amount, budget->goals[i].saved_amount);
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
        budget->goals[choice].saved_amount = budget->goals[choice].target_amount;
    }
    printf("Progress updated for %s. Saved: $%.2f / $%.2f\n", budget->goals[choice].name, budget->goals[choice].saved_amount, budget->goals[choice].target_amount);
}

// Function to show the progress of all savings goals
void showSavingsProgress(const Budget *budget) {
    if (budget->num_goals == 0) {
        printf("No savings goals to display.\n");
        return;
    }
    printf("\n--- Savings Progress ---\n");
    for (int i = 0; i < budget->num_goals; i++) {
        float progress = (budget->goals[i].saved_amount / budget->goals[i].target_amount) * 100;
        printf("%s: $%.2f / $%.2f (%.2f%% complete)\n", 
               budget->goals[i].name, 
               budget->goals[i].saved_amount, 
               budget->goals[i].target_amount, 
               progress);
    }
}

// Function to calculate and display the daily budget
void calculateDailyBudget(const Budget *budget) {
    float total_expenses = 0, total_income = budget->income;

    // Calculate total one-time expenses
    for (int i = 0; i < budget->num_expenses; i++) {
        total_expenses += budget->expenses[i].amount;
    }

    // Include recurring income and expenses
    for (int i = 0; i < budget->num_recurring_income; i++) {
        total_income += budget->recurring_income[i].amount;
    }
    for (int i = 0; i < budget->num_recurring_expenses; i++) {
        total_expenses += budget->recurring_expenses[i].amount;
    }

    // Calculate daily budget
    float daily_budget = (total_income - total_expenses) / budget->days_in_month;
    printf("\n--- Daily Budget ---\n");
    printf("Total Income: $%.2f\n", total_income);
    printf("Total Expenses: $%.2f\n", total_expenses);
    printf("Daily Budget: $%.2f\n", daily_budget);
}

// Function to show analytics (spending trends and savings progress)
void showAnalytics(const Budget *budget) {
    printf("\n--- Analytics ---\n");

    // Total income and expenses
    printf("Monthly Income: $%.2f\n", budget->income);

    float total_expenses = 0;
    for (int i = 0; i < budget->num_expenses; i++) {
        total_expenses += budget->expenses[i].amount;
    }
    printf("Total Expenses: $%.2f\n", total_expenses);

    // Expenses by category
    printf("\nExpenses by Category:\n");
    for (int i = 0; i < budget->num_expenses; i++) {
        printf("%s: $%.2f\n", budget->expenses[i].category, budget->expenses[i].amount);
    }

    // Savings progress
    printf("\n--- Savings Progress ---\n");
    float total_saved = 0;
    for (int i = 0; i < budget->num_goals; i++) {
        float progress = (budget->goals[i].saved_amount / budget->goals[i].target_amount) * 100;
        printf("%s: $%.2f / $%.2f (%.2f%% complete)\n", 
               budget->goals[i].name, 
               budget->goals[i].saved_amount, 
               budget->goals[i].target_amount, 
               progress);
        total_saved += budget->goals[i].saved_amount;
    }
    printf("Total Savings: $%.2f\n", total_saved);

    // Remaining budget
    float remaining_budget = budget->income - total_expenses - total_saved;
    printf("\nRemaining Budget: $%.2f\n", remaining_budget);
}

// Function to manage recurring income and expense entries
void manageRecurringEntries(Budget *budget) {
    int choice;
    do {
        printf("\n--- Manage Recurring Entries ---\n");
        printf("1. Add Recurring Income\n");
        printf("2. Add Recurring Expense\n");
        printf("3. View Recurring Entries\n");
        printf("4. Return to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addRecurringIncome(budget);
                break;
            case 2:
                addRecurringExpense(budget);
                break;
            case 3:
                viewRecurringEntries(budget);
                break;
            case 4:
                printf("Returning to main menu...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 4);
}

// Function to add a recurring income entry
void addRecurringIncome(Budget *budget) {
    if (budget->num_recurring_income >= MAX_RECURRING) {
        printf("Error: Maximum number of recurring income entries reached.\n");
        return;
    }
    printf("Enter the description of recurring income: ");
    scanf("%s", budget->recurring_income[budget->num_recurring_income].description);
    printf("Enter the amount for %s: $", budget->recurring_income[budget->num_recurring_income].description);
    if (scanf("%f", &budget->recurring_income[budget->num_recurring_income].amount) != 1 || budget->recurring_income[budget->num_recurring_income].amount < 0) {
        printf("Error: Invalid income amount.\n");
        return;
    }
    budget->num_recurring_income++;
    printf("Recurring income added successfully.\n");
}

// Function to add a recurring expense entry
void addRecurringExpense(Budget *budget) {
    if (budget->num_recurring_expenses >= MAX_RECURRING) {
        printf("Error: Maximum number of recurring expense entries reached.\n");
        return;
    }
    printf("Enter the description of recurring expense: ");
    scanf("%s", budget->recurring_expenses[budget->num_recurring_expenses].description);
    printf("Enter the amount for %s: $", budget->recurring_expenses[budget->num_recurring_expenses].description);
    if (scanf("%f", &budget->recurring_expenses[budget->num_recurring_expenses].amount) != 1 || budget->recurring_expenses[budget->num_recurring_expenses].amount < 0) {
        printf("Error: Invalid expense amount.\n");
        return;
    }
    budget->num_recurring_expenses++;
    printf("Recurring expense added successfully.\n");
}

// Function to view all recurring income and expenses
void viewRecurringEntries(const Budget *budget) {
    printf("\n--- Recurring Income ---\n");
    for (int i = 0; i < budget->num_recurring_income; i++) {
        printf("%s: $%.2f\n", budget->recurring_income[i].description, budget->recurring_income[i].amount);
    }
    printf("\n--- Recurring Expenses ---\n");
    for (int i = 0; i < budget->num_recurring_expenses; i++) {
        printf("%s: $%.2f\n", budget->recurring_expenses[i].description, budget->recurring_expenses[i].amount);
    }
}

// Function to save the budget to a file
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

// Function to calculate the number of days in the current month
void autoSetDaysInMonth(Budget *budget) {
    // Get the current time
    time_t t = time(NULL);
    // Convert to local time
    struct tm *current_time = localtime(&t);
    //tm_mon is 0-based (Jan = 0)
    int month = current_time->tm_mon + 1;
    // tm_year is years since 1900
    int year = current_time->tm_year + 1900;

    // Determine the number of days in the month
    switch(month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            budget->days_in_month = 31;
            break;
        case 4: case 6: case 9: case 11:
            budget->days_in_month = 30;
            break;
        case 2:
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
                budget->days_in_month = 29; // Leap year
            } else {
                budget->days_in_month = 28;
            }
            break;
        default:
            printf("Error: Invalid month.\n");
            budget-> days_in_month = 30; // Fallback default
    }

    printf("Days in the current month (%d/%d): %d\n", month, year, budget->days_in_month);
}   
