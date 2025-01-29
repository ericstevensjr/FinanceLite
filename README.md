# Finance Lite

Finance Lite is a simple personal budgeting tool designed to help you manage your finances. It allows you to track your income, expenses, and savings goals, as well as manage recurring transactions such as monthly subscriptions or salaries. The tool is built with a focus on modularity and simplicity, and is powered by an SQLite database for persistent storage.

## Features

- **Income Tracking**: Easily add and track both one-time and recurring income sources.
- **Expense Tracking**: Add expenses and categorize them. Recurring expenses are also supported.
- **Savings Goals**: Set and track savings goals, view progress, and update as you save money.
- **Recurring Transactions**: Manage recurring income and expenses such as subscriptions or automatic deposits.
- **Analytics**: Get insights into your finances, including daily budget calculations, savings progress, and overall financial health.
- **JSON Export**: Export your budget and financial data to a JSON file for backup or external analysis.
- **Database**: Uses an SQLite database to store data persistently.
- **Modular Code Structure**: The code has been split into modular components for better maintainability and scalability.

## Modules

The codebase is structured into the following modules:

- **Budget Module (`budget.c`, `budget.h`)**: Handles budget-related operations, such as calculating the daily budget and managing savings goals.
- **Database Module (`database.c`, `database.h`)**: Manages interactions with the SQLite database, including initializing the database, and inserting, updating, and fetching records.
- **Recurring Module (`recurring.c`, `recurring.h`)**: Manages recurring income and expenses, allowing you to add, edit, and remove them.
- **Utility Functions (`utils.c`, `utils.h`)**: Contains helper functions for user input validation, date handling, and other utility tasks.
- **Main Application (`main.c`)**: Contains the core logic for running the finance application, including the user interface and the flow of operations.

## Installation

To use Finance Lite, you'll need to have the following installed:

- **SQLite3**: A lightweight database engine used to store your financial data.
- **C Compiler (e.g., GCC)**: To compile the project.

### Clone the repository

```bash
git clone https://github.com/yourusername/FinanceLite.git
cd FinanceLite
```

### Install dependencies

Make sure you have SQLite3 and the necessary libraries installed on your system. You can install SQLite3 using the package manager on your system:

- On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install sqlite3 libsqlite3-dev
```

- On macOS (using Homebrew):

```bash
brew install sqlite
```

### Compile the project

You can compile the project using the provided `Makefile`.

```bash
make
```

### Run the application

Once compiled, you can run the application:

```bash
./finance_lite
```

## Usage

### Main Menu

After running the program, you'll be greeted with the following menu:

```
=== Finance Lite ===
1. Add Income
2. Add Expense
3. Add Savings Goal
4. Update Savings Goal Progress
5. Show Savings Progress
6. Calculate Daily Budget
7. Show Analytics
8. Manage Recurring Entries and Savings
9. Export Budget to JSON
10. Save and Exit
```

Here’s a breakdown of each option:

### 1. Add Income

Add a new income source. You can specify if it's recurring or one-time. For recurring income, you can specify the start date (e.g., 1st of the month, the date entered, or a custom date).

### 2. Add Expense

Add a new expense. Similar to income, you can specify if it's recurring or one-time. Recurring expenses can be set to start on a specific date.

### 3. Add Savings Goal

Set a new savings goal. You’ll need to provide the name, target amount, and due date. As you save, you can update the progress on this goal.

### 4. Update Savings Goal Progress

Track and update the amount saved toward a specific savings goal.

### 5. Show Savings Progress

Display all savings goals and their progress, including how much you’ve saved towards each goal and how much remains.

### 6. Calculate Daily Budget

Based on your income, expenses, and savings goals, the program will calculate the daily budget required to meet your financial targets.

### 7. Show Analytics

Shows an overview of your financial health, including:
- Total income
- Total expenses
- Expense breakdown by category
- Savings progress
- Recurring expenses total
- Recommendations based on budget

### 8. Manage Recurring Entries and Savings

This option allows you to:
- Add, edit, or remove recurring income and expenses.
- View and remove savings goals.

### 9. Export Budget to JSON

Export your financial data, including income, expenses, savings goals, and recurring entries, to a JSON file for backup or external analysis.

### 10. Save and Exit

Exit the application and save all changes to the database.

## Database Schema

Finance Lite uses an SQLite database with the following tables:

### 1. `income`
Tracks one-time and recurring income.

| Column    | Type     |
|-----------|----------|
| id        | INTEGER  |
| amount    | REAL     |
| date      | TEXT     |
| is_recurring | INTEGER |

### 2. `expenses`
Tracks one-time and recurring expenses.

| Column    | Type     |
|-----------|----------|
| id        | INTEGER  |
| category  | TEXT     |
| amount    | REAL     |
| date      | TEXT     |
| is_recurring | INTEGER |

### 3. `savings_goals`
Tracks user-defined savings goals.

| Column      | Type    |
|-------------|---------|
| id          | INTEGER |
| name        | TEXT    |
| target_amount | REAL  |
| saved_amount | REAL  |
| due_date    | TEXT    |

### 4. `recurring`
Tracks recurring income and expenses.

| Column      | Type    |
|-------------|---------|
| id          | INTEGER |
| type        | TEXT    |
| description | TEXT    |
| amount      | REAL    |
| date        | TEXT    |

### 5. `last_processed_month`
Tracks the last month when recurring transactions were processed.

| Column      | Type    |
|-------------|---------|
| id          | INTEGER |
| year        | INTEGER |
| month       | INTEGER |

## Contributing

Contributions are welcome! If you’d like to contribute to Finance Lite, please fork the repository and submit a pull request.

### Bug Reports

If you encounter any issues, please report them on the [GitHub Issues page](https://github.com/yourusername/FinanceLite/issues).

### Feature Requests

Feel free to submit any new features you'd like to see in Finance Lite. We aim to continually improve the project.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
