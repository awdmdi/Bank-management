#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>   // For std::string and std::to_string

using namespace std;

const int max_accounts = 100;  // Maximum number of accounts allowed

// Arrays to hold account data
int* account_numbers = new int[max_accounts];
char** names = new char* [max_accounts];
int* pins = new int[max_accounts];
double* balances = new double[max_accounts];
char** cnics = new char* [max_accounts];

int total_accounts = 0;  // Tracks total number of accounts

// Get the current date and time as a string
const char* get_timestamp() {
    static char buffer[80];
    time_t now = time(0);
    tm local_time;

#ifdef _MSC_VER
    localtime_s(&local_time, &now);  // Microsoft secure version
#else
    tm* temp = localtime(&now);
    if (!temp) {
        buffer[0] = '\0';
        return buffer;
    }
    local_time = *temp;
#endif

    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &local_time);
    return buffer;
}

// Append a transaction to the corresponding account's log file
void log_transaction(int acc_no, const char* type, double amount, double balance) {
    ofstream fout;
    string filename = "transactions_" + to_string(acc_no) + ".txt";
    fout.open(filename, ios::app);
    fout << "[" << get_timestamp() << "] " << type
        << " of " << amount << " | Balance: " << balance << "\n";
    fout.close();
}

// Load account details from file into arrays
void load_accounts() {
    ifstream fin("accounts.txt");
    if (!fin) return;  // If file doesn't exist, skip loading

    while (fin >> account_numbers[total_accounts]) {
        fin.ignore();
        names[total_accounts] = new char[100];
        fin.getline(names[total_accounts], 100);

        fin >> pins[total_accounts];
        fin >> balances[total_accounts];
        fin.ignore();

        cnics[total_accounts] = new char[20];
        fin.getline(cnics[total_accounts], 20);

        total_accounts++;
        if (total_accounts >= max_accounts) break;
    }
    fin.close();
}

// Save current account details back to file
void save_accounts() {
    ofstream fout("accounts.txt");
    for (int i = 0; i < total_accounts; i++) {
        fout << account_numbers[i] << "\n"
            << names[i] << "\n"
            << pins[i] << "\n"
            << balances[i] << "\n"
            << cnics[i] << "\n";
    }
    fout.close();
}

// Find index of an account using its number
int find_account_index(int acc_no) {
    for (int i = 0; i < total_accounts; i++) {
        if (account_numbers[i] == acc_no) {
            return i;
        }
    }
    return -1;  // Not found
}

// Verify user's PIN
bool verify_pin(int index) {
    int input_pin;
    cout << "Enter 4-digit PIN: ";
    cin >> input_pin;

    if (pins[index] == input_pin) {
        return true;
    }
    else {
        cout << "Incorrect PIN.\n";
        return false;
    }
}

// Create a new bank account
void create_account() {
    if (total_accounts >= max_accounts) {
        cout << "Account limit reached.\n";
        return;
    }

    int acc_no, pin;
    double deposit;
    char temp_name[100];
    char temp_cnic[20];

    cout << "Enter account number: ";
    cin >> acc_no;

    if (find_account_index(acc_no) != -1) {
        cout << "Account already exists.\n";
        return;
    }

    cout << "Enter account holder name: ";
    cin.ignore();
    cin.getline(temp_name, 100);

    cout << "Set 4-digit PIN: ";
    cin >> pin;
    if (pin < 1000 || pin > 9999) {
        cout << "Invalid PIN. Must be 4 digits.\n";
        return;
    }

    cout << "Enter CNIC (xxxxx-xxxxxxx-x): ";
    cin.ignore();
    cin.getline(temp_cnic, 20);

    if (strlen(temp_cnic) != 15 || temp_cnic[5] != '-' || temp_cnic[13] != '-') {
        cout << "Invalid CNIC format.\n";
        return;
    }

    cout << "Enter initial deposit: ";
    cin >> deposit;

    // Store account information
    account_numbers[total_accounts] = acc_no;

    names[total_accounts] = new char[strlen(temp_name) + 1];
    strcpy_s(names[total_accounts], strlen(temp_name) + 1, temp_name);

    pins[total_accounts] = pin;
    balances[total_accounts] = deposit;

    cnics[total_accounts] = new char[strlen(temp_cnic) + 1];
    strcpy_s(cnics[total_accounts], strlen(temp_cnic) + 1, temp_cnic);

    log_transaction(acc_no, "Account created with deposit", deposit, deposit);

    total_accounts++;
    cout << "Account created successfully.\n";
    save_accounts();
}

// Deposit money into an account
void deposit() {
    int acc_no;
    cout << "Enter account number: ";
    cin >> acc_no;

    int index = find_account_index(acc_no);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    if (!verify_pin(index)) return;

    double amount;
    cout << "Enter deposit amount: ";
    cin >> amount;

    balances[index] += amount;
    log_transaction(account_numbers[index], "Deposit", amount, balances[index]);

    cout << "Deposit successful. New balance: " << balances[index] << "\n";
    save_accounts();
}

// Withdraw money from an account
void withdraw() {
    int acc_no;
    cout << "Enter account number: ";
    cin >> acc_no;

    int index = find_account_index(acc_no);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    if (!verify_pin(index)) return;

    double amount;
    cout << "Enter withdrawal amount: ";
    cin >> amount;

    if (amount > balances[index]) {
        cout << "Insufficient funds.\n";
        return;
    }

    balances[index] -= amount;
    log_transaction(account_numbers[index], "Withdrawal", amount, balances[index]);

    cout << "Withdrawal successful. New balance: " << balances[index] << "\n";
    save_accounts();
}

// Show balance of a specific account
void check_balance() {
    int acc_no;
    cout << "Enter account number: ";
    cin >> acc_no;

    int index = find_account_index(acc_no);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    if (!verify_pin(index)) return;

    cout << "Account holder: " << names[index] << "\n";
    cout << "CNIC: " << cnics[index] << "\n";
    cout << "Balance: " << balances[index] << "\n";
}

// Show past transactions for an account
void show_history() {
    int acc_no;
    cout << "Enter account number: ";
    cin >> acc_no;

    int index = find_account_index(acc_no);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    if (!verify_pin(index)) return;

    string filename = "transactions_" + to_string(acc_no) + ".txt";
    ifstream fin(filename);
    if (!fin) {
        cout << "No transaction history found.\n";
        return;
    }

    cout << "\nTransaction history for account #" << acc_no << ":\n";
    cout << "----------------------------------------\n";

    string line;
    while (getline(fin, line)) {
        cout << line << "\n";
    }
    fin.close();
}

// Transfer money from one account to another
void send_money() {
    int sender_acc, receiver_acc;
    cout << "Enter your account number (sender): ";
    cin >> sender_acc;

    int sender_index = find_account_index(sender_acc);
    if (sender_index == -1) {
        cout << "Sender account not found.\n";
        return;
    }

    if (!verify_pin(sender_index)) return;

    cout << "Enter receiver account number: ";
    cin >> receiver_acc;

    int receiver_index = find_account_index(receiver_acc);
    if (receiver_index == -1) {
        cout << "Receiver account not found.\n";
        return;
    }

    double amount;
    cout << "Enter amount to send: ";
    cin >> amount;

    if (amount <= 0) {
        cout << "Invalid amount.\n";
        return;
    }

    if (amount > balances[sender_index]) {
        cout << "Insufficient funds.\n";
        return;
    }

    // Update balances
    balances[sender_index] -= amount;
    balances[receiver_index] += amount;

    // Log transactions for both accounts
    log_transaction(sender_acc, "Sent money", amount, balances[sender_index]);
    log_transaction(receiver_acc, "Received money", amount, balances[receiver_index]);

    cout << "Transfer successful.\n";
    cout << "Your new balance: " << balances[sender_index] << "\n";
    save_accounts();
}

// Main menu loop
void menu() {
    int choice;
    do {
        cout << "\n=== Welcome to Bank-Al-Bahria ===\n";
        cout << "1. Create Account\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Check Balance\n";
        cout << "5. View Transaction History\n";
        cout << "6. Send Money\n";
        cout << "7. Exit\n";
        cout << "Select an option: ";
        cin >> choice;

        switch (choice) {
        case 1: create_account(); break;
        case 2: deposit(); break;
        case 3: withdraw(); break;
        case 4: check_balance(); break;
        case 5: show_history(); break;
        case 6: send_money(); break;
        case 7: cout << "Exiting program...\n"; break;
        default: cout << "Invalid choice.\n"; break;
        }

    } while (choice != 7);
}

// Entry point
int main() {
    load_accounts();  // Load saved account data if available
    menu();           // Start main user interaction

    // Clean up dynamically allocated memory
    for (int i = 0; i < total_accounts; i++) {
        delete[] names[i];
        delete[] cnics[i];
    }

    delete[] names;
    delete[] cnics;
    delete[] account_numbers;
    delete[] pins;
    delete[] balances;

    return 0;
}
