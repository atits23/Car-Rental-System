#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <ctime>
using namespace std;

// Configuration: Use 86400 for production (1 day), 30 for testing (30 seconds)
#define SECONDS_PER_DAY 86400
// #define SECONDS_PER_DAY 30

// Utility function to convert string to integer
int convertStringToInt(const string& str) {
    stringstream stream(str);
    int value = 0;
    stream >> value;
    return value;
}

// Display due date in readable format
void showDueDate(time_t due_date) {
    if (due_date == 0) {
        cout << setw(15) << "Not rented\n";
        return;
    }
    
    auto current_time = time(0);
    if (current_time > due_date) {
        cout << setw(15) << "Overdue\n";
    } else {
        auto date_structure = localtime(&due_date);
        cout << date_structure->tm_mday << "/" 
             << (1 + date_structure->tm_mon) << "/" 
             << (1900 + date_structure->tm_year) << endl;
    }
}

//======================= VEHICLE CLASS =======================//

class Vehicle {
public:
    string vehicle_id;
    string brand_name;
    string vehicle_model;
    int daily_rent;
    int market_price;
    int seating_capacity;
    string vehicle_color;
    bool is_rented;
    time_t return_deadline;
    static int vehicle_counter;
    int vehicle_condition;
    
    friend class Administrator;

    Vehicle(string brand, string model, int rent, int price, int seats, 
            string color, int condition = 100, time_t deadline = 0) {
        this->vehicle_id = to_string(++vehicle_counter);
        this->brand_name = brand;
        this->vehicle_model = model;
        this->market_price = price;
        this->daily_rent = rent;
        this->seating_capacity = seats;
        this->vehicle_color = color;
        this->is_rented = false;
        this->vehicle_condition = condition;
        this->return_deadline = deadline;
    }
};

int Vehicle::vehicle_counter = 0;
vector<Vehicle> vehicle_inventory;

//======================= BASE USER CLASS =======================//

class BaseUser {
protected:
    string user_password;
    
public:
    int user_id;
    string username;
    static int customer_count;
    static int employee_count;
    
    void displayAllVehicles();
    bool validatePassword(const string& password);
};

int BaseUser::customer_count = 0;
int BaseUser::employee_count = 0;

void BaseUser::displayAllVehicles() {
    cout << setw(3) << "ID" << setw(15) << "Brand" << setw(15) << "Model" 
         << setw(7) << "Rent" << setw(12) << "Price" << setw(6) << "Seats" 
         << setw(10) << "Color" << setw(10) << "Available" << setw(10) 
         << "Condition" << setw(15) << "Due Date" << endl;
    
    for (const auto& vehicle : vehicle_inventory) {
        cout << setw(3) << vehicle.vehicle_id << setw(15) << vehicle.brand_name 
             << setw(15) << vehicle.vehicle_model << setw(7) << vehicle.daily_rent 
             << setw(12) << vehicle.market_price << setw(6) << vehicle.seating_capacity 
             << setw(10) << vehicle.vehicle_color << setw(10) << !vehicle.is_rented 
             << setw(10) << vehicle.vehicle_condition << setw(15);
        showDueDate(vehicle.return_deadline);
    }
}

bool BaseUser::validatePassword(const string& password) {
    return (this->user_password == password);
}

//======================= CUSTOMER CLASS =======================//

class Client : public BaseUser {
protected:
    vector<Vehicle> rented_vehicles;
    int outstanding_dues;
    int client_rating;

public:
    friend class Administrator;

    Client(string name, string password, int rating = 100, int dues = 0, 
           vector<Vehicle> vehicles = {}) {
        this->username = name;
        this->user_password = password;
        this->user_id = ++customer_count;
        this->outstanding_dues = dues;
        this->client_rating = rating;
        this->rented_vehicles = vehicles;
    }

    void accessClientPortal();
    void rentVehicle();
    void returnVehicle();
    void showRentedVehicles();
    void checkOutstandingDues();
    void payDues();
};

vector<Client> client_database;

void Client::rentVehicle() {
    cout << "Enter the vehicle ID you wish to rent: ";
    string vehicle_id;
    cin >> vehicle_id;
    
    for (auto& vehicle : vehicle_inventory) {
        if (vehicle.vehicle_id == vehicle_id) {
            if (vehicle.is_rented) {
                cout << "Vehicle is currently rented out\n";
                return;
            }
            
            vehicle.is_rented = true;
            this->outstanding_dues += vehicle.daily_rent;
            rented_vehicles.push_back(vehicle);
            vehicle.return_deadline = time(0) + SECONDS_PER_DAY * 7;
            cout << "Vehicle rented successfully\n";
            return;
        }
    }
    cout << "Vehicle with specified ID not found\n\n\n";
}

void Client::returnVehicle() {
    cout << "Enter the vehicle ID you wish to return: ";
    string vehicle_id;
    cin >> vehicle_id;
    
    for (auto iterator = rented_vehicles.begin(); iterator != rented_vehicles.end(); ++iterator) {
        if (iterator->vehicle_id == vehicle_id) {
            // Update vehicle availability in main inventory
            for (auto& main_vehicle : vehicle_inventory) {
                if (main_vehicle.vehicle_id == vehicle_id) {
                    main_vehicle.is_rented = false;
                    main_vehicle.return_deadline = 0;
                    break;
                }
            }
            
            auto current_time = time(0);
            if (current_time > iterator->return_deadline) {
                int penalty = (iterator->daily_rent * ((current_time - iterator->return_deadline) / SECONDS_PER_DAY)) / 5;
                this->outstanding_dues += penalty;
                this->client_rating -= 2 * ((current_time - iterator->return_deadline) / SECONDS_PER_DAY);
                cout << "Late return penalty applied: " << penalty << "\n";
            }
            
            rented_vehicles.erase(iterator);
            cout << "Vehicle returned successfully\n\n\n";
            return;
        }
    }
    cout << "Vehicle not found in your rentals\n";
}

void Client::showRentedVehicles() {
    if (rented_vehicles.empty()) {
        cout << "No vehicles currently rented\n\n\n";
        return;
    }
    
    cout << setw(3) << "ID" << setw(15) << "Brand" << setw(15) << "Model" 
         << setw(7) << "Rent" << setw(12) << "Price" << setw(6) << "Seats" 
         << setw(10) << "Color" << setw(10) << "Due Date" << setw(10) 
         << "Condition" << setw(15) << "Return By" << endl;
    
    for (const auto& vehicle : rented_vehicles) {
        cout << setw(3) << vehicle.vehicle_id << setw(15) << vehicle.brand_name 
             << setw(15) << vehicle.vehicle_model << setw(7) << vehicle.daily_rent 
             << setw(12) << vehicle.market_price << setw(6) << vehicle.seating_capacity 
             << setw(10) << vehicle.vehicle_color << setw(10) << vehicle.return_deadline 
             << setw(10) << vehicle.vehicle_condition;
        showDueDate(vehicle.return_deadline);
    }
    cout << "\n\n\n";
}

void Client::checkOutstandingDues() {
    cout << "Your current outstanding balance: " << this->outstanding_dues << "\n\n\n";
}

void Client::payDues() {
    if (this->outstanding_dues == 0) {
        cout << "No outstanding dues to pay\n\n\n";
        return;
    }
    
    cout << "Current outstanding balance: " << this->outstanding_dues << endl;
    cout << "Enter payment amount: ";
    string amount_str;
    cin >> amount_str;
    int payment_amount = convertStringToInt(amount_str);
    
    if (payment_amount > 0 && payment_amount <= this->outstanding_dues) {
        this->outstanding_dues -= payment_amount;
        cout << "Payment processed successfully\n";
        cout << "Remaining balance: " << this->outstanding_dues << "\n\n\n";
    } else {
        cout << "Invalid payment amount\n";
    }
}

void Client::accessClientPortal() {
    bool session_active = true;
    cout << "Welcome, " << this->username << endl;
    
    while (session_active) {
        cout << "Select an option:\n";
        cout << "1. View all vehicles\n2. View your rentals\n3. Rent a vehicle\n"
             << "4. Return a vehicle\n5. Check dues\n6. Pay dues\n0. Logout\n"
             << "Enter your choice: ";
        
        char user_choice;
        cin >> user_choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (user_choice) {
            case '1': displayAllVehicles(); break;
            case '2': showRentedVehicles(); break;
            case '3': rentVehicle(); break;
            case '4': returnVehicle(); break;
            case '5': checkOutstandingDues(); break;
            case '6': payDues(); break;
            case '0': session_active = false; break;
            default: cout << "Invalid selection\n"; break;
        }
    }
}

void handleClientLogin() {
    bool login_session = true;
    
    while (login_session) {
        cout << "Client Portal Options:\n";
        cout << "1. Login\n2. Register\n0. Exit\nEnter your choice: ";
        char choice;
        cin >> choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            case '1': {
                cout << "Enter your username:\n";
                string name;
                getline(cin, name);
                cout << "Enter your password:\n";
                string password;
                cin >> password;
                
                bool login_success = false;
                for (auto& client : client_database) {
                    if (client.username == name && client.validatePassword(password)) {
                        client.accessClientPortal();
                        login_success = true;
                        break;
                    }
                }
                
                if (!login_success) {
                    cout << "Authentication failed: Invalid credentials\n";
                }
                break;
            }
            case '2': {
                cout << "Enter desired username:\n";
                string name;
                getline(cin, name);
                cout << "Enter password:\n";
                string password;
                cin >> password;
                cout << "Confirm password:\n";
                string confirm_password;
                cin >> confirm_password;
                
                if (password != confirm_password) {
                    cout << "Registration failed: Passwords do not match\n";
                    break;
                }
                
                bool username_available = true;
                for (const auto& client : client_database) {
                    if (client.username == name) {
                        cout << "Registration failed: Username already exists\n";
                        username_available = false;
                        break;
                    }
                }
                
                if (username_available) {
                    Client new_client(name, password);
                    client_database.push_back(new_client);
                    cout << "Registration successful\nPlease login to access your account\n\n\n";
                }
                break;
            }
            case '0': login_session = false; break;
            default: cout << "Invalid selection\n"; break;
        }
    }
}

//======================= EMPLOYEE CLASS =======================//

class StaffMember : public BaseUser {
protected:
    vector<Vehicle> rented_vehicles;
    int outstanding_dues;
    double performance_rating;

public:
    friend class Administrator;

    StaffMember(string name, string password, double rating = 1.00, int dues = 0, 
                vector<Vehicle> vehicles = {}) {
        this->username = name;
        this->user_password = password;
        this->user_id = ++employee_count;
        this->outstanding_dues = dues;
        this->performance_rating = rating;
        this->rented_vehicles = vehicles;
    }

    void accessStaffPortal();
    void rentVehicle();
    void returnVehicle();
    void showRentedVehicles();
    void checkOutstandingDues();
    void payDues();
};

vector<StaffMember> staff_database;

void StaffMember::rentVehicle() {
    cout << "Enter the vehicle ID you wish to rent: ";
    string vehicle_id;
    cin >> vehicle_id;
    
    for (auto& vehicle : vehicle_inventory) {
        if (vehicle.vehicle_id == vehicle_id) {
            if (vehicle.is_rented) {
                cout << "Vehicle is currently rented out\n";
                return;
            }
            
            vehicle.is_rented = true;
            this->outstanding_dues += int(vehicle.daily_rent * 0.85); // Employee discount
            rented_vehicles.push_back(vehicle);
            cout << "Vehicle rented successfully (Employee discount applied)\n";
            return;
        }
    }
    cout << "Vehicle with specified ID not found\n\n\n";
}

void StaffMember::returnVehicle() {
    cout << "Enter the vehicle ID you wish to return: ";
    string vehicle_id;
    cin >> vehicle_id;
    
    for (auto iterator = rented_vehicles.begin(); iterator != rented_vehicles.end(); ++iterator) {
        if (iterator->vehicle_id == vehicle_id) {
            // Update vehicle availability in main inventory
            for (auto& main_vehicle : vehicle_inventory) {
                if (main_vehicle.vehicle_id == vehicle_id) {
                    main_vehicle.is_rented = false;
                    break;
                }
            }
            
            auto current_time = time(0);
            if (current_time > iterator->return_deadline) {
                int penalty = 0.17 * (iterator->daily_rent * ((current_time - iterator->return_deadline) / SECONDS_PER_DAY));
                this->outstanding_dues += penalty;
                this->performance_rating -= 2 * ((current_time - iterator->return_deadline) / SECONDS_PER_DAY);
                cout << "Late return penalty applied: " << penalty << "\n";
            }
            
            rented_vehicles.erase(iterator);
            cout << "Vehicle returned successfully\n\n\n";
            return;
        }
    }
    cout << "Vehicle not found in your rentals\n";
}

void StaffMember::showRentedVehicles() {
    if (rented_vehicles.empty()) {
        cout << "No vehicles currently rented\n\n\n";
        return;
    }
    
    cout << setw(3) << "ID" << setw(15) << "Brand" << setw(15) << "Model" 
         << setw(7) << "Rent" << setw(12) << "Price" << setw(6) << "Seats" 
         << setw(10) << "Color" << setw(10) << "Due Date" << setw(10) 
         << "Condition" << setw(15) << "Return By" << endl;
    
    for (const auto& vehicle : rented_vehicles) {
        cout << setw(3) << vehicle.vehicle_id << setw(15) << vehicle.brand_name 
             << setw(15) << vehicle.vehicle_model << setw(7) << vehicle.daily_rent 
             << setw(12) << vehicle.market_price << setw(6) << vehicle.seating_capacity 
             << setw(10) << vehicle.vehicle_color << setw(10) << vehicle.return_deadline 
             << setw(10) << vehicle.vehicle_condition;
        showDueDate(vehicle.return_deadline);
    }
    cout << "\n\n\n";
}

void StaffMember::checkOutstandingDues() {
    cout << "Your current outstanding balance: " << this->outstanding_dues << "\n\n\n";
}

void StaffMember::payDues() {
    if (this->outstanding_dues == 0) {
        cout << "No outstanding dues to pay\n\n\n";
        return;
    }
    
    cout << "Current outstanding balance: " << this->outstanding_dues << endl;
    cout << "Enter payment amount: ";
    string amount_str;
    cin >> amount_str;
    int payment_amount = convertStringToInt(amount_str);
    
    if (payment_amount > 0 && payment_amount <= this->outstanding_dues) {
        this->outstanding_dues -= payment_amount;
        cout << "Payment processed successfully\n";
        cout << "Remaining balance: " << this->outstanding_dues << "\n\n\n";
    } else {
        cout << "Invalid payment amount\n";
    }
}

void StaffMember::accessStaffPortal() {
    bool session_active = true;
    cout << "Welcome, " << this->username << endl;
    
    while (session_active) {
        cout << "Select an option:\n";
        cout << "1. View all vehicles\n2. View your rentals\n3. Rent a vehicle\n"
             << "4. Return a vehicle\n5. Check dues\n6. Pay dues\n0. Logout\n"
             << "Enter your choice: ";
        
        char user_choice;
        cin >> user_choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (user_choice) {
            case '1': displayAllVehicles(); break;
            case '2': showRentedVehicles(); break;
            case '3': rentVehicle(); break;
            case '4': returnVehicle(); break;
            case '5': checkOutstandingDues(); break;
            case '6': payDues(); break;
            case '0': session_active = false; break;
            default: cout << "Invalid selection\n"; break;
        }
    }
}

void handleStaffLogin() {
    bool login_session = true;
    
    while (login_session) {
        cout << "Staff Portal Options:\n";
        cout << "1. Login\n0. Exit\nEnter your choice: ";
        char choice;
        cin >> choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            case '1': {
                cout << "Enter your username:\n";
                string name;
                getline(cin, name);
                cout << "Enter your password:\n";
                string password;
                cin >> password;
                
                bool login_success = false;
                for (auto& staff : staff_database) {
                    if (staff.username == name && staff.validatePassword(password)) {
                        staff.accessStaffPortal();
                        login_success = true;
                        break;
                    }
                }
                
                if (!login_success) {
                    cout << "Authentication failed: Invalid credentials\n";
                }
                break;
            }
            case '0': login_session = false; break;
            default: cout << "Invalid selection\n"; break;
        }
    }
}

//======================= ADMINISTRATOR CLASS =======================//

class Administrator : public BaseUser {
public:
    void accessAdminPortal();
    void addNewVehicle();
    void modifyVehicle();
    void removeVehicle();
    void viewAllClients();
    void addNewClient();
    void modifyClient();
    void removeClient();
    void viewAllStaff();
    void addNewStaff();
    void modifyStaff();
    void removeStaff();
};

Administrator system_admin;

void Administrator::addNewVehicle() {
    cout << "Enter vehicle brand: ";
    string brand;
    cin >> brand;
    cout << "Enter vehicle model: ";
    string model;
    cin >> model;
    cout << "Enter daily rent: ";
    int rent;
    cin >> rent;
    cout << "Enter market price: ";
    int price;
    cin >> price;
    cout << "Enter seating capacity: ";
    int seats;
    cin >> seats;
    cout << "Enter vehicle color: ";
    string color;
    cin >> color;
    
    vehicle_inventory.push_back(Vehicle(brand, model, rent, price, seats, color));
    cout << "Vehicle added successfully\n";
}

void Administrator::addNewClient() {
    cout << "Enter client name: ";
    string name;
    cin >> name;
    cout << "Enter client password: ";
    string password;
    cin >> password;
    
    client_database.push_back(Client(name, password));
    cout << "Client added successfully\n";
}

void Administrator::addNewStaff() {
    cout << "Enter staff name: ";
    string name;
    cin >> name;
    cout << "Enter staff password: ";
    string password;
    cin >> password;
    
    staff_database.push_back(StaffMember(name, password));
    cout << "Staff member added successfully\n";
}

void Administrator::viewAllClients() {
    cout << "ID\tName\tDues\tRating" << endl;
    for (const auto& client : client_database) {
        cout << client.user_id << "\t" << client.username << "\t" 
             << client.outstanding_dues << "\t" << client.client_rating << endl;
        cout << "\tRented Vehicles:\n";
        for (const auto& vehicle : client.rented_vehicles) {
            cout << "\t" << vehicle.brand_name << " " << vehicle.vehicle_model << endl;
        }
    }
}

void Administrator::viewAllStaff() {
    cout << "ID\tName" << endl;
    for (const auto& staff : staff_database) {
        cout << staff.user_id << "\t" << staff.username << endl;
        cout << "\tRented Vehicles:\n";
        for (const auto& vehicle : staff.rented_vehicles) {
            cout << "\t" << vehicle.brand_name << " " << vehicle.vehicle_model << endl;
        }
    }
}

void Administrator::removeVehicle() {
    cout << "Enter vehicle ID to remove: ";
    string vehicle_id;
    cin >> vehicle_id;
    
    for (auto iterator = vehicle_inventory.begin(); iterator != vehicle_inventory.end(); ++iterator) {
        if (iterator->vehicle_id == vehicle_id) {
            if (iterator->is_rented) {
                cout << "Cannot remove: Vehicle is currently rented\n";
                return;
            }
            vehicle_inventory.erase(iterator);
            cout << "Vehicle removed successfully\n";
            return;
        }
    }
    cout << "Vehicle not found\n";
}

void Administrator::removeClient() {
    cout << "Enter client ID to remove: ";
    int client_id;
    cin >> client_id;
    
    for (auto iterator = client_database.begin(); iterator != client_database.end(); ++iterator) {
        if (iterator->user_id == client_id) {
            if (iterator->outstanding_dues > 0) {
                cout << "Cannot remove: Client has outstanding dues\n";
                return;
            }
            if (!iterator->rented_vehicles.empty()) {
                cout << "Cannot remove: Client has rented vehicles\n";
                return;
            }
            client_database.erase(iterator);
            cout << "Client removed successfully\n";
            return;
        }
    }
    cout << "Client not found\n";
}

void Administrator::removeStaff() {
    cout << "Enter staff ID to remove: ";
    string staff_str;
    cin >> staff_str;
    int staff_id = convertStringToInt(staff_str);
    
    for (auto iterator = staff_database.begin(); iterator != staff_database.end(); ++iterator) {
        if (iterator->user_id == staff_id) {
            if (iterator->outstanding_dues > 0) {
                cout << "Cannot remove: Staff has outstanding dues\n";
                return;
            }
            if (!iterator->rented_vehicles.empty()) {
                cout << "Cannot remove: Staff has rented vehicles\n";
                return;
            }
            staff_database.erase(iterator);
            cout << "Staff removed successfully\n";
            return;
        }
    }
    cout << "Staff not found\n";
}

void Administrator::modifyVehicle() {
    cout << "Enter vehicle ID to modify: ";
    string vehicle_id;
    cin >> vehicle_id;
    
    for (auto& vehicle : vehicle_inventory) {
        if (vehicle.vehicle_id == vehicle_id) {
            bool updating = true;
            while (updating) {
                cout << "Select field to update:\n";
                cout << "1. Brand\n2. Model\n3. Rent\n4. Price\n5. Seats\n6. Color\n"
                     << "7. Rental Status\n8. Condition\n0. Finish\nEnter choice: ";
                
                char choice;
                cin >> choice;
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                switch (choice) {
                    case '1':
                        cout << "Current brand: " << vehicle.brand_name << "\n";
                        cout << "Enter new brand: ";
                        cin >> vehicle.brand_name;
                        break;
                    case '2':
                        cout << "Current model: " << vehicle.vehicle_model << "\n";
                        cout << "Enter new model: ";
                        cin >> vehicle.vehicle_model;
                        break;
                    case '3':
                        cout << "Current rent: " << vehicle.daily_rent << "\n";
                        cout << "Enter new rent: ";
                        cin >> vehicle.daily_rent;
                        break;
                    case '4':
                        cout << "Current price: " << vehicle.market_price << "\n";
                        cout << "Enter new price: ";
                        cin >> vehicle.market_price;
                        break;
                    case '5':
                        cout << "Current seats: " << vehicle.seating_capacity << "\n";
                        cout << "Enter new seats: ";
                        cin >> vehicle.seating_capacity;
                        break;
                    case '6':
                        cout << "Current color: " << vehicle.vehicle_color << "\n";
                        cout << "Enter new color: ";
                        cin >> vehicle.vehicle_color;
                        break;
                    case '7':
                        cout << "Currently " << (vehicle.is_rented ? "rented" : "not rented") << "\n";
                        cout << "Enter new status: ";
                        cin >> vehicle.is_rented;
                        break;
                    case '8':
                        cout << "Current condition: " << vehicle.vehicle_condition << "\n";
                        cout << "Enter new condition: ";
                        cin >> vehicle.vehicle_condition;
                        break;
                    case '0':
                        updating = false;
                        break;
                    default:
                        cout << "Invalid choice\n";
                        break;
                }
            }
            return;
        }
    }
    cout << "Vehicle not found\n";
}

void Administrator::modifyClient() {
    cout << "Enter client ID to modify: ";
    int client_id;
    cin >> client_id;
    
    for (auto& client : client_database) {
        if (client.user_id == client_id) {
            bool updating = true;
            while (updating) {
                cout << "Select field to update:\n";
                cout << "1. Name\n2. Password\n3. Dues\n4. Rating\n0. Finish\nEnter choice: ";
                
                char choice;
                cin >> choice;
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                switch (choice) {
                    case '1':
                        cout << "Current name: " << client.username << "\n";
                        cout << "Enter new name: ";
                        cin >> client.username;
                        break;
                    case '2':
                        cout << "Enter new password: ";
                        cin >> client.user_password;
                        break;
                    case '3':
                        cout << "Current dues: " << client.outstanding_dues << "\n";
                        cout << "Enter new dues: ";
                        cin >> client.outstanding_dues;
                        break;
                    case '4':
                        cout << "Current rating: " << client.client_rating << "\n";
                        cout << "Enter new rating: ";
                        cin >> client.client_rating;
                        break;
                    case '0':
                        updating = false;
                        break;
                    default:
                        cout << "Invalid choice\n";
                        break;
                }
            }
            return;
        }
    }
    cout << "Client not found\n";
}

void Administrator::modifyStaff() {
    cout << "Enter staff ID to modify: ";
    string staff_str;
    cin >> staff_str;
    int staff_id = convertStringToInt(staff_str);
    
    for (auto& staff : staff_database) {
        if (staff.user_id == staff_id) {
            bool updating = true;
            while (updating) {
                cout << "Select field to update:\n";
                cout << "1. Name\n2. Password\n3. Dues\n4. Rating\n0. Finish\nEnter choice: ";
                
                char choice;
                cin >> choice;
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                switch (choice) {
                    case '1':
                        cout << "Current name: " << staff.username << "\n";
                        cout << "Enter new name: ";
                        cin >> staff.username;
                        cout << "Name updated to " << staff.username << endl;
                        break;
                    case '2':
                        cout << "Enter new password: ";
                        cin >> staff.user_password;
                        cout << "Password updated successfully\n";
                        break;
                    case '3':
                        cout << "Current dues: " << staff.outstanding_dues << "\n";
                        cout << "Enter new dues: ";
                        cin >> staff.outstanding_dues;
                        cout << "Dues updated to " << staff.outstanding_dues << endl;
                        break;
                    case '4':
                        cout << "Current rating: " << staff.performance_rating << "\n";
                        cout << "Enter new rating: ";
                        cin >> staff.performance_rating;
                        cout << "Rating updated to " << staff.performance_rating << endl;
                        break;
                    case '0':
                        updating = false;
                        break;
                    default:
                        cout << "Invalid choice\n";
                        break;
                }
            }
            return;
        }
    }
    cout << "Staff not found\n";
}

void Administrator::accessAdminPortal() {
    bool session_active = true;
    cout << "Welcome, " << this->username << endl;
    
    while (session_active) {
        cout << "Administrator Panel Options:\n";
        cout << "1. View all vehicles\n2. Add vehicle\n3. Modify vehicle\n4. Remove vehicle\n"
             << "5. View all clients\n6. Add client\n7. Modify client\n8. Remove client\n"
             << "9. View all staff\na. Add staff\nb. Modify staff\nc. Remove staff\n"
             << "0. Logout\nEnter your choice: ";
        
        char choice;
        cin >> choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            case '1': displayAllVehicles(); break;
            case '2': addNewVehicle(); break;
            case '3': modifyVehicle(); break;
            case '4': removeVehicle(); break;
            case '5': viewAllClients(); break;
            case '6': addNewClient(); break;
            case '7': modifyClient(); break;
            case '8': removeClient(); break;
            case '9': viewAllStaff(); break;
            case 'a': addNewStaff(); break;
            case 'b': modifyStaff(); break;
            case 'c': removeStaff(); break;
            case '0': session_active = false; break;
            default: cout << "Invalid selection\n"; break;
        }
    }
}

void handleAdminLogin() {
    bool login_session = true;
    
    while (login_session) {
        cout << "Administrator Portal Options:\n";
        cout << "1. Login\n0. Exit\nEnter your choice: ";
        char choice;
        cin >> choice;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (choice) {
            case '1': {
                cout << "Enter username:\n";
                string name;
                getline(cin, name);
                cout << "Enter password:\n";
                string password;
                cin >> password;
                
                if (name == "admin" && password == "admin") {
                    cout << "Authentication successful\n";
                    system_admin.accessAdminPortal();
                    break;
                }
                cout << "Authentication failed: Invalid credentials\n";
                break;
            }
            case '0': login_session = false; break;
            default: cout << "Invalid selection\n"; break;
        }
    }
}

//======================= MAIN FUNCTION =======================//

int main() {
    bool system_running = true;
    
    // Initialize sample vehicle inventory
    vehicle_inventory.push_back(Vehicle("Toyota", "Corolla", 1000, 2000000, 5, "White", 100, time(0) + SECONDS_PER_DAY));
    vehicle_inventory.push_back(Vehicle("Honda", "Civic", 1500, 2500000, 5, "Black", 100, time(0) + SECONDS_PER_DAY));
    vehicle_inventory.push_back(Vehicle("Suzuki", "Cultus", 800, 1000000, 5, "Grey"));
    vehicle_inventory.push_back(Vehicle("Suzuki", "Mehran", 500, 500000, 5, "White"));
    vehicle_inventory.push_back(Vehicle("Toyota", "Vitz", 1200, 1500000, 5, "Red"));
    vehicle_inventory.push_back(Vehicle("Rolls", "Royce", 50000, 20000000, 4, "White", 90));
    vehicle_inventory.push_back(Vehicle("Ferrari", "Laferrari", 15000, 30000000, 2, "Red", 100, time(0) + 600));
    vehicle_inventory.push_back(Vehicle("Lamborghini", "Aventador", 20000, 40000000, 2, "Black", 100, time(0) + 1200));
    vehicle_inventory.push_back(Vehicle("Bugatti", "Veyron", 25000, 50000000, 2, "Blue"));
    vehicle_inventory.push_back(Vehicle("Audi", "A8", 5000, 10000000, 5, "Black"));

    // Initialize sample client database
    client_database.push_back(Client("ali", "asd", 100, 5000, {vehicle_inventory[0], vehicle_inventory[1]}));
    client_database.push_back(Client("ahmed", "123"));
    client_database.push_back(Client("asad", "123"));
    client_database.push_back(Client("ahsan", "123"));
    client_database.push_back(Client("adeel", "123"));
    vehicle_inventory[0].is_rented = true;
    vehicle_inventory[1].is_rented = true;

    // Initialize sample staff database
    staff_database.push_back(StaffMember("dev", "123", 1.00, 0));
    staff_database.push_back(StaffMember("daksh", "123"));
    staff_database.push_back(StaffMember("sway", "123"));
    staff_database.push_back(StaffMember("himan", "123", 0.99, 5000, {vehicle_inventory[6], vehicle_inventory[7]}));
    staff_database.push_back(StaffMember("kum", "123"));
    vehicle_inventory[6].is_rented = true;
    vehicle_inventory[7].is_rented = true;

    while (system_running) {
        cout << "####################### Digital Vehicle Rental Management System #######################\n";
        cout << "1. Client Portal\n2. Staff Portal\n3. Administrator Portal\n0. Exit System\n"
             << "Enter your selection: ";
        
        char main_choice;
        cin >> main_choice;
        
        switch (main_choice) {
            case '0': system_running = false; break;
            case '1': handleClientLogin(); break;
            case '2': handleStaffLogin(); break;
            case '3': handleAdminLogin(); break;
            default: cout << "Invalid selection\n"; break;
        }
        
        system("cls"); // Use "clear" for Linux/Mac, "cls" for Windows
    }
    
    cin.get();
    return 0;
}
