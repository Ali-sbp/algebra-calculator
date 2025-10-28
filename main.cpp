#include "algebra.h"
#include <iostream>
#include <string>

using namespace std;

void printMenu(int bits) {
    cout << "\n========================================\n";
    cout << "  Finite Algebra Calculator (Z" << bits << ")\n";
    cout << "========================================\n";
    cout << "SINGLE-DIGIT OPERATIONS:\n";
    cout << "1. Set +1 rule\n";
    cout << "2. Show Hasse diagram\n";
    cout << "3. Show addition table\n";
    cout << "4. Show multiplication table\n";
    cout << "5. Show subtraction table\n";
    cout << "6. Show division table\n";
    cout << "7. Calculate addition (x + y)\n";
    cout << "8. Calculate multiplication (x * y)\n";
    cout << "9. Calculate subtraction (x - y)\n";
    cout << "10. Calculate division (x / y)\n";
    cout << "11. Calculate power (x ^ y)\n";
    cout << "12. Calculate GCD/NOD (x, y)\n";
    cout << "13. Calculate LCM/NOC (x, y)\n";
    cout << "14. Show addition carry table\n";
    cout << "15. Show multiplication carry table\n";
    cout << "\nMULTI-DIGIT OPERATIONS:\n";
    cout << "17. Add multi-digit numbers (abc + def)\n";
    cout << "18. Subtract multi-digit numbers (abc - def)\n";
    cout << "19. Multiply multi-digit numbers (abc * def)\n";
    cout << "20. Divide multi-digit numbers (abc / def)\n";
    cout << "21. Modulo multi-digit numbers (abc % def)\n";
    cout << "22. GCD/NOD multi-digit (abc, def)\n";
    cout << "23. LCM/NOC multi-digit (abc, def)\n";
    cout << "\n16. Print map\n";
    cout << "0. Exit\n";
    cout << "========================================\n";
    cout << "Choose an option: ";
}

int main() {
    cout << "Enter number of elements (e.g., 8 for Z8, 16 for Z16): ";
    int bits;
    cin >> bits;
    
    if (bits < 2 || bits > 26) {
        cout << "⚠ Invalid number! Must be between 2 and 26.\n";
        return 1;
    }
    
    Algebra algebra(bits);
    string rule;
    bool ruleSet = false;
    
    char lastElement = 'a' + bits - 1;
    
    cout << "Welcome to Finite Algebra Calculator!\n";
    cout << "=====================================\n";
    
    while (true) {
        printMenu(bits);
        
        int choice;
        cin >> choice;
        
        if (choice == 0) {
            cout << "\nGoodbye!\n";
            break;
        }
        
        switch (choice) {
            case 1: {
                cout << "\nEnter the +1 rule:\n";
                cout << "  Simple format: b g f e c a h d\n";
                cout << "  Multi-element format: bhg{e,c}afd (elements in {} share same position)\n";
                cout << "Rule: ";
                cin.ignore();
                getline(cin, rule);
                
                algebra.setPlusOneRule(rule);
                ruleSet = true;
                cout << "\n✓ Rule set successfully!\n";
                break;
            }
            
            case 2: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printHasseDiagram();
                break;
            }
            
            case 3: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printAdditionTable();
                break;
            }
            
            case 4: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printMultiplicationTable();
                break;
            }
            
            case 5: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printSubtractionTable();
                break;
            }
            
            case 6: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printDivisionTable();
                break;
            }
            
            case 7: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter first element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter second element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.add(x, y);
                int carry = algebra.getAdditionCarry(x, y);
                
                if (carry > 0) {
                    cout << "\nResult: " << x << " + " << y << " = ";
                    // Print carry as element at that position
                    for (int c = carry; c > 0; c--) {
                        cout << "b";  // Each carry is represented as 'b' (position 1)
                    }
                    cout << result << " (carry: " << carry << ")\n";
                } else {
                    cout << "\nResult: " << x << " + " << y << " = " << result << "\n";
                }
                break;
            }
            
            case 8: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter first element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter second element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.multiply(x, y);
                int carry = algebra.getMultiplicationCarry(x, y);
                
                if (carry > 0) {
                    cout << "\nResult: " << x << " * " << y << " = ";
                    // Convert carry count to element representation
                    char carryElem = 'a' + carry;  // carry position mapped to element
                    if (carry < bits) {
                        cout << carryElem;
                    } else {
                        // If carry exceeds element range, show the numeric value
                        cout << "[" << carry << "]";
                    }
                    cout << result << " (carry: " << carry << ")\n";
                } else {
                    cout << "\nResult: " << x << " * " << y << " = " << result << "\n";
                }
                break;
            }
            
            case 9: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter first element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter second element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.subtract(x, y);
                cout << "\nResult: " << x << " - " << y << " = " << result << "\n";
                break;
            }
            
            case 10: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter first element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter second element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.divide(x, y);
                cout << "\nResult: " << x << " / " << y << " = " << result << "\n";
                break;
            }
            case 11: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter base element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter exponent element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.power(x, y);
                cout << "\nResult: " << x << " ^ " << y << " = " << result << "\n";
                break;
            }
            case 12: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter first element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter second element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.gcd(x, y);
                cout << "\nResult: GCD/NOD(" << x << ", " << y << ") = " << result << "\n";
                break;
            }
            case 13: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                char x, y;
                cout << "\nEnter first element (a-" << lastElement << "): ";
                cin >> x;
                cout << "Enter second element (a-" << lastElement << "): ";
                cin >> y;
                
                if (x < 'a' || x > lastElement || y < 'a' || y > lastElement) {
                    cout << "⚠ Invalid elements! Must be between a and " << lastElement << ".\n";
                    break;
                }
                
                char result = algebra.lcm(x, y);
                cout << "\nResult: LCM/NOC(" << x << ", " << y << ") = " << result << "\n";
                break;
            }
            case 14: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printAdditionCarryTable();
                break;
            }
            case 15: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                algebra.printMultiplicationCarryTable();
                break;
            }
            case 16: {
                if(!ruleSet){
                    cout <<"enter the +1 rule first" <<endl;
                    break;
                }
                algebra.printMap();
                break;
            }
            case 17: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter first multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter second multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;  // Allow minus at start
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;  // Allow minus at start
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string result = algebra.addArithmetic(num1, num2);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: " << num1 << " + " << num2 << " = " << formatted << "\n";
                break;
            }
            case 18: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter first multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter second multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string result = algebra.subtractArithmetic(num1, num2);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: " << num1 << " - " << num2 << " = " << formatted << "\n";
                break;
            }
            case 19: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter first multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter second multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string result = algebra.multiplyArithmetic(num1, num2);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: " << num1 << " * " << num2 << " = " << formatted << "\n";
                break;
            }
            case 20: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter dividend multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter divisor multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string remainder;
                string result = algebra.divideArithmetic(num1, num2, remainder);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: " << num1 << " / " << num2 << " = " << formatted;
                if (remainder != "a" && !remainder.empty()) {
                    string formattedRemainder = algebra.formatMultiDigitResult(remainder);
                    cout << " remainder " << formattedRemainder;
                }
                cout << "\n";
                break;
            }
            case 21: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter dividend multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter divisor multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string result = algebra.modArithmetic(num1, num2);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: " << num1 << " % " << num2 << " = " << formatted << "\n";
                break;
            }
            case 22: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter first multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter second multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string result = algebra.gcdArithmetic(num1, num2);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: GCD/NOD(" << num1 << ", " << num2 << ") = " << formatted << "\n";
                break;
            }
            case 23: {
                if (!ruleSet) {
                    cout << "\n⚠ Please set the +1 rule first (option 1)!\n";
                    break;
                }
                string num1, num2;
                cout << "\nEnter first multi-digit number (e.g., abc): ";
                cin >> num1;
                cout << "Enter second multi-digit number (e.g., def): ";
                cin >> num2;
                
                // Validate input (allow minus sign at the beginning)
                bool valid = true;
                for (size_t i = 0; i < num1.length(); i++) {
                    if (i == 0 && num1[i] == '-') continue;
                    if (num1[i] < 'a' || num1[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                for (size_t i = 0; i < num2.length(); i++) {
                    if (i == 0 && num2[i] == '-') continue;
                    if (num2[i] < 'a' || num2[i] > lastElement) {
                        valid = false;
                        break;
                    }
                }
                
                if (!valid) {
                    cout << "⚠ Invalid input! Characters must be between a and " << lastElement << " (minus sign allowed at start).\n";
                    break;
                }
                
                string result = algebra.lcmArithmetic(num1, num2);
                string formatted = algebra.formatMultiDigitResult(result);
                cout << "\nResult: LCM/NOC(" << num1 << ", " << num2 << ") = " << formatted << "\n";
                break;
            }
            default:
                cout << "\n⚠ Invalid option! Please choose 0-23.\n";
        }
    }
    
    return 0;
}
