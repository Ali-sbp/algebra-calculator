#include "algebra.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

Algebra::Algebra(int bits) : bits(bits), boundedMode(false) {
    // Generate elements dynamically based on bits
    // bits = 8 → elements = {a, b, c, d, e, f, g, h} (8 elements)
    //
    
    elements.clear();
    for (int i = 0; i < bits && i < 26; i++) { // Max 26 (a-z)
        elements.push_back('a' + i);
    }
    
    additiveIdentity = 'a';
    multiplicativeIdentity = 'b';
}

void Algebra::setPlusOneRule(const std::string& rule) {
    // Parse the rule string
    // Simple format: "b g f e c a h d"
    // Multi-element format: "bhg{e,c}afd" where {e,c} means e and c are at the same position
    
    plusOneRule.clear();
    plusOneRule.resize(elements.size());  // Initialize with empty vectors
    
    char minElem = elements[0];
    char maxElem = elements[elements.size() - 1];
    int position = 0;
    
    for (size_t i = 0; i < rule.length(); ) {
        if (rule[i] == '{') {
            // Multi-element: {d,f}
            i++; // skip '{'
            while (i < rule.length() && rule[i] != '}') {
                if (rule[i] >= minElem && rule[i] <= maxElem) {
                    plusOneRule[position].push_back(rule[i]);
                }
                i++;
            }
            i++; // skip '}'
            position++;
        } else if (rule[i] >= minElem && rule[i] <= maxElem) {
            // Single element
            plusOneRule[position].push_back(rule[i]);
            position++;
            i++;
        } else {
            i++; // skip spaces, dashes, etc.
        }
    }
    
    // Build the algebra structure
    BuildHasse();
    buildAdditionTable();
    buildMultiplicationTable();
    buildSubtractionTable();
    buildDivisionTable();
}
// TODO : prepolnenie : DONE , a/a = min - max : DONE
// TODO : DONE  
// n != Zn , always 8
/*
void Algebra::BuildHasse() {
    // Build the Hasse diagram positions by following the +b chain starting
    // from the additive identity. plusOneRule is interpreted as: for each
    // input element at index i (elements[i]), adding 'b' yields the list
    // plusOneRule[i] (we use the first element as the chain representative).
    // After walking the chain we propagate positions to any multi-element
    // groups so all members share the same step value.

    elementPosition.clear();

    // Step 0 is the additive identity
    elementPosition[additiveIdentity] = 0;

    char current = additiveIdentity;
    int steps = static_cast<int>(elements.size());
    for (int step = 1; step < steps; ++step) {
        // find index of current in elements
        auto it = std::find(elements.begin(), elements.end(), current);
        if (it == elements.end()) break; // unknown element
        int idx = static_cast<int>(std::distance(elements.begin(), it));

        // ensure plusOneRule has entry for this input
        if (idx < 0 || idx >= static_cast<int>(plusOneRule.size()) || plusOneRule[idx].empty()) {
            // cannot continue the chain
            break;
        }

        // Use the first element in the output list as the chain next element
        char next = plusOneRule[idx][0];

        // If we've already assigned a position to 'next', we've closed the cycle
        if (elementPosition.find(next) != elementPosition.end()) {
            break;
        }

        elementPosition[next] = step;
        current = next;
    }

    // Propagate positions to all members of multi-element outputs so they
    // share the same step as the representative (if any member has been
    // assigned a position during the chain walk).
    for (size_t inputIdx = 0; inputIdx < plusOneRule.size(); ++inputIdx) {
        const auto &outs = plusOneRule[inputIdx];
        if (outs.empty()) continue;

        // Check if any member already has a position
        int assignedPos = -1;
        for (char e : outs) {
            auto itp = elementPosition.find(e);
            if (itp != elementPosition.end()) {
                assignedPos = itp->second;
                break;
            }
        }

        // If we found an assigned position, assign it to the rest
        if (assignedPos != -1) {
            for (char e : outs) elementPosition[e] = assignedPos;
        }
    }
}
*/
void Algebra::BuildHasse() {
    // Build the Hasse diagram positions by following the +b chain starting
    // from the additive identity. plusOneRule is interpreted as: for each
    // input element at index i (elements[i]), adding 'b' yields the list
    // plusOneRule[i] (we use the first element as the chain representative).
    // After walking the chain we propagate positions to any multi-element
    // groups so all members share the same step value.

    elementPosition.clear();

    // Step 0 is the additive identity
    elementPosition[additiveIdentity] = 0;

    char current = additiveIdentity;
    int steps = static_cast<int>(elements.size());
    for (int step = 1; step < steps; ++step) {
        // find index of current in elements
        auto it = std::find(elements.begin(), elements.end(), current);
        if (it == elements.end()) break; // unknown element
        int idx = static_cast<int>(std::distance(elements.begin(), it));

        // ensure plusOneRule has entry for this input
        if (idx < 0 || idx >= static_cast<int>(plusOneRule.size()) || plusOneRule[idx].empty()) {
            // cannot continue the chain
            break;
        }

        // Use the first element in the output list as the chain next element
        char next = plusOneRule[idx][0];

        // If we've already assigned a position to 'next', we've closed the cycle
        if (elementPosition.find(next) != elementPosition.end()) {
            break;
        }

        elementPosition[next] = step;
        current = next;
    }

    // Propagate positions to all members of multi-element outputs so they
    // share the same step as the representative (if any member has been
    // assigned a position during the chain walk).
    for (size_t inputIdx = 0; inputIdx < plusOneRule.size(); ++inputIdx) {
        const auto &outs = plusOneRule[inputIdx];
        if (outs.empty()) continue;

        // Check if any member already has a position
        int assignedPos = -1;
        for (char e : outs) {
            auto itp = elementPosition.find(e);
            if (itp != elementPosition.end()) {
                assignedPos = itp->second;
                break;
            }
        }

        // If we found an assigned position, assign it to the rest
        if (assignedPos != -1) {
            for (char e : outs) elementPosition[e] = assignedPos;
        }
    }
    
    // **NEW: Map remaining unmapped elements using the +1 rule equations**
    // For any unmapped element X, if we know element Y where plusOneRule[Y] contains X,
    // and Y is mapped, then X should be at position (Y's position + 1) mod cycle_length
    bool changed = true;
    int maxIterations = elements.size() * 2; // Safety limit
    int iteration = 0;
    
    while (changed && iteration < maxIterations) {
        changed = false;
        iteration++;
        
        // Check all elements to see if any are unmapped
        for (char elem : elements) {
            if (elementPosition.find(elem) != elementPosition.end()) {
                continue; // Already mapped
            }
            
            // Element is unmapped - try to find it in plusOneRule outputs
            // If plusOneRule[inputIdx] contains elem, and elements[inputIdx] is mapped,
            // then elem should be at position (elementPosition[elements[inputIdx]] + multiplicativeIdentityPosition)
            for (size_t inputIdx = 0; inputIdx < plusOneRule.size(); ++inputIdx) {
                const auto &outs = plusOneRule[inputIdx];
                
                // Check if elem is in this output list
                bool found = false;
                for (char e : outs) {
                    if (e == elem) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) continue;
                
                // Found elem in plusOneRule[inputIdx]
                // This means: b + elements[inputIdx] = elem (and possibly others)
                // So: elem's position = elements[inputIdx]'s position + b's position
                
                char inputElem = elements[inputIdx];
                if (elementPosition.find(inputElem) != elementPosition.end() &&
                    elementPosition.find(multiplicativeIdentity) != elementPosition.end()) {
                    
                    int inputPos = elementPosition[inputElem];
                    int bPos = elementPosition[multiplicativeIdentity];
                    int newPos = inputPos + bPos;
                    
                    // Don't mod by cycle length yet - we're still building the cycle
                    // Just assign the position
                    elementPosition[elem] = newPos;
                    changed = true;
                    
                    // Also assign to all other elements in the same output group
                    for (char e : outs) {
                        if (elementPosition.find(e) == elementPosition.end()) {
                            elementPosition[e] = newPos;
                        }
                    }
                    
                    break; // Found mapping for elem, move to next element
                }
            }
        }
    }
}//last variant , g , derived elements , cant be result of + , * but can be result of - and / (ask how to handle)
void Algebra::buildAdditionTable() {
    additionTable.clear();
    additionCarryTable.clear();
    
    // Build addition table with carry tracking
    for (char x : elements) {
        for (char y : elements) {
            int carry = 0;
            additionTable[{x, y}] = addWithCarry(x, y, carry);
            additionCarryTable[{x, y}] = carry;
        }
    }
}

int Algebra::getCycleLength() const {
    // Find the number of distinct positions in the Hasse diagram
    // This is the cycle length for modular arithmetic
    int maxPos = 0;
    for (const auto& [elem, pos] : elementPosition) {
        if (pos > maxPos) maxPos = pos;
    }
    return maxPos + 1;  // If max position is 6, we have 7 positions (0-6)
}

std::string Algebra::formatMultiDigitResult(const std::string& result) const {
    // Format a multi-digit result, replacing each character with its position representation
    // If multiple elements share a position, show them in braces: {d,f}
    
    // Handle division by zero result
    if (result == "∅") {
        return "∅";
    }
    
    // Handle negative numbers
    if (!result.empty() && result[0] == '-') {
        std::string formatted = "-";
        for (size_t i = 1; i < result.length(); i++) {
            // Skip non-element characters
            if (elementPosition.find(result[i]) == elementPosition.end()) {
                formatted += result[i];
                continue;
            }
            int pos = elementPosition.at(result[i]);
            std::string posStr = getElementsAtPosition(pos);
            formatted += posStr;
        }
        return formatted;
    }
    
    std::string formatted;
    for (char c : result) {
        // Skip non-element characters
        if (elementPosition.find(c) == elementPosition.end()) {
            formatted += c;
            continue;
        }
        int pos = elementPosition.at(c);
        std::string posStr = getElementsAtPosition(pos);
        formatted += posStr;
    }
    
    return formatted;
}
//can also change to addWithCarry(char a, char b, char& c) so it counts the carry in the same algebra
char Algebra::addWithCarry(char a, char b, int& carry) const {
    // To compute a + b with carry:
    // Start with a, then add 'b' (multiplicative identity) repeatedly
    // for elementPosition[b] times, tracking how many times we cycle
    
    char result = a;
    int steps = elementPosition.at(b);
    carry = 0;
    
    int cycleLength = getCycleLength();
    
    for (int i = 0; i < steps; i++) {
        // result = result + 'b' (the multiplicative identity element)
        // Find result in elements array to get its index
        auto it = std::find(elements.begin(), elements.end(), result);
        if (it != elements.end()) {
            int idx = std::distance(elements.begin(), it);
            // plusOneRule[idx] is a vector, take the first element
            if (!plusOneRule[idx].empty()) {
                char next = plusOneRule[idx][0];  // Apply +b using the rule
                
                // Check if we're wrapping around (carry occurs)
                int currentPos = elementPosition.at(result);
                int nextPos = elementPosition.at(next);
                
                // If next position is less than current, we've wrapped around
                if (nextPos < currentPos) {
                    carry++;
                }
                
                result = next;
            }
        }
    }
    
    return result;
}

int Algebra::getAdditionCarry(char a, char b) const {
    auto it = additionCarryTable.find({a, b});
    if (it != additionCarryTable.end()) {
        return it->second;
    }
    return 0;
}

char Algebra::addMultipleTimes(char element, int times) const {
    if (times == 0) return additiveIdentity;
    
    char result = additiveIdentity;
    for (int i = 0; i < times; i++) {
        result = add(result, element);
    }
    return result;
}

char Algebra::addMultipleTimesWithCarry(char element, int times, int& carry) const {
    if (times == 0) {
        carry = 0;
        return additiveIdentity;
    }
    
    char result = additiveIdentity;
    carry = 0;
    
    for (int i = 0; i < times; i++) {
        int stepCarry = 0;
        result = addWithCarry(result, element, stepCarry);
        carry += stepCarry;
    }
    
    return result;
}

std::string Algebra::getElementsAtPosition(int position) const {
    // Find all elements at this position
    std::vector<char> elemsAtPos;
    for (const auto& [elem, pos] : elementPosition) {
        if (pos == position) {
            elemsAtPos.push_back(elem);
        }
    }
    
    // Sort for consistent display
    std::sort(elemsAtPos.begin(), elemsAtPos.end());
    
    // Format output
    if (elemsAtPos.empty()) {
        return "?";
    } else if (elemsAtPos.size() == 1) {
        return std::string(1, elemsAtPos[0]);
    } else {
        std::string result = "{";
        for (size_t i = 0; i < elemsAtPos.size(); i++) {
            result += elemsAtPos[i];
            if (i < elemsAtPos.size() - 1) result += ",";
        }
        result += "}";
        return result;
    }
}

void Algebra::buildMultiplicationTable() {
    multiplicationTable.clear();
    multiplicationCarryTable.clear();
    
    // Build multiplication table with carry tracking
    for (char x : elements) {
        for (char y : elements) {
            int carry = 0;
            multiplicationTable[{x, y}] = multiplyWithCarry(x, y, carry);
            multiplicationCarryTable[{x, y}] = carry;
        }
    }
}

char Algebra::multiplyWithCarry(char a, char b, int& carry) const {
    // Handle special cases with identities
    if (a == additiveIdentity || b == additiveIdentity) {
        // x * a = a for all x (given property)
        carry = 0;
        return additiveIdentity;
    }
    
    if (b == multiplicativeIdentity) {
        // x * b = x (b is multiplicative identity)
        carry = 0;
        return a;
    }
    
    if (a == multiplicativeIdentity) {
        // b * y = y (b is multiplicative identity, commutative)
        carry = 0;
        return b;
    }
    
    // General case: a * b = add a to itself b times, tracking carries
    int bPos = elementPosition.at(b);
    return addMultipleTimesWithCarry(a, bPos, carry);
}

int Algebra::getMultiplicationCarry(char a, char b) const {
    auto it = multiplicationCarryTable.find({a, b});
    if (it != multiplicationCarryTable.end()) {
        return it->second;
    }
    return 0;
}

void Algebra::buildSubtractionTable() {
    subtractionTable.clear();
    
    // Build subtraction table using the subtract function
    for (char x : elements) {
        for (char y : elements) {
            subtractionTable[{x, y}] = subtract(x, y);
        }
    }
}

void Algebra::buildDivisionTable() {
    divisionTable.clear();
    
    // Build division table using the divide function
    for (char x : elements) {
        for (char y : elements) {
            divisionTable[{x, y}] = divide(x, y);
        }
    }
}

void Algebra::printHasseDiagram() const {
    std::cout << "\nHasse Diagram (ordering by +1 steps from '" << additiveIdentity << "'):\n";
    std::cout << "=======================================================\n";
    
    // Group elements by their computed position
    std::map<int, std::vector<char>> positionGroups;
    for (const auto& [elem, pos] : elementPosition) {
        positionGroups[pos].push_back(elem);
    }
    
    // Print the diagram in position order
    bool first = true;
    for (const auto& [pos, elems] : positionGroups) {
        if (!first) {
            std::cout << " → ";
        }
        first = false;
        
        // Sort elements for consistent display
        std::vector<char> sortedElems = elems;
        std::sort(sortedElems.begin(), sortedElems.end());
        
        // Print elements at this position
        if (sortedElems.size() == 1) {
            std::cout << sortedElems[0] << " (" << pos << ")";
        } else {
            std::cout << "{";
            for (size_t i = 0; i < sortedElems.size(); i++) {
                std::cout << sortedElems[i];
                if (i < sortedElems.size() - 1) std::cout << ",";
            }
            std::cout << "} (" << pos << ")";
        }
    }
    std::cout << " → (back to " << additiveIdentity << ")\n\n";
}

char Algebra::add(char a, char b) const {
    // To compute a + b:
    // Start with a, then add 'b' (multiplicative identity) repeatedly
    // for elementPosition[b] times
    
    char result = a;
    int steps = elementPosition.at(b);
    
    for (int i = 0; i < steps; i++) {
        // result = result + 'b' (the multiplicative identity element)
        // Find result in elements array to get its index
        auto it = std::find(elements.begin(), elements.end(), result);
        if (it != elements.end()) {
            int idx = std::distance(elements.begin(), it);
            // plusOneRule[idx] is a vector, take the first element
            if (!plusOneRule[idx].empty()) {
                result = plusOneRule[idx][0];  // Apply +b using the rule
            }
        }
    }
    
    return result;
}

char Algebra::multiply(char a, char b) const {
    // Handle special cases with identities
    if (a == additiveIdentity || b == additiveIdentity) {
        // x * a = a for all x (given property)
        return additiveIdentity;
    }
    
    if (b == multiplicativeIdentity) {
        // x * b = x (b is multiplicative identity)
        return a;
    }
    
    if (a == multiplicativeIdentity) {
        // b * y = y (b is multiplicative identity, commutative)
        return b;
    }
    
    // General case: a * b = add a to itself b times
    int bPos = elementPosition.at(b);
    return addMultipleTimes(a, bPos);
}

char Algebra::power(char base, char exponent) const {
    // Calculate base^exponent
    // exponent's position determines how many times to multiply
    
    // Handle special cases
    if (exponent == additiveIdentity) {
        // x^0 = multiplicative identity (b)
        return multiplicativeIdentity;
    }
    
    if (base == additiveIdentity) {
        // 0^x = 0 (additive identity)
        return additiveIdentity;
    }
    
    if (exponent == multiplicativeIdentity) {
        // x^1 = x
        return base;
    }
    
    // General case: multiply base by itself exponent times
    // exponent's position determines the number of multiplications
    int expPos = elementPosition.at(exponent);
    
    char result = multiplicativeIdentity; // Start with 1
    for (int i = 0; i < expPos; i++) {
        result = multiply(result, base);
    }
    
    return result;
}

char Algebra::subtract(char a, char b) const {
    // To compute a - b:
    // Find x such that b + x = a
    // We search through all elements to find the appropriate x
    
    for (char x : elements) {
        if (add(b, x) == a) {
            return x;
        }
    }
    
    // If no solution found, return additive identity as fallback
    return additiveIdentity;
}

char Algebra::divide(char a, char b) const {
    // To compute a / b:
    // Find x such that b * x = a
    // We search through all elements to find the appropriate x
    
    // Special case: division by additive identity (zero) is undefined
    // Return additive identity as convention
    if (b == additiveIdentity) {
        return additiveIdentity;
    }
    
    for (char x : elements) {
        if (multiply(b, x) == a) {
            return x;
        }
    }
    
    // If no solution found, return additive identity as fallback
    return additiveIdentity;
}

char Algebra::gcd(char a, char b) const {
    // GCD (NOD) in finite algebra: largest element d such that
    // d divides both a and b (i.e., exists x, y where d*x = a and d*y = b)
    // We search from the highest position down to find the greatest common divisor
    
    // Special case: gcd(a, a) = a (identity is always 0)
    if (a == additiveIdentity) return b;
    if (b == additiveIdentity) return a;
    
    // Find the maximum position to start from
    int maxPos = 0;
    for (const auto& [elem, pos] : elementPosition) {
        if (pos > maxPos) maxPos = pos;
    }
    
    // Search from highest position down
    for (int pos = maxPos; pos >= 0; pos--) {
        // Get all elements at this position
        for (const auto& [elem, p] : elementPosition) {
            if (p == pos) {
                // Check if elem divides both a and b
                bool dividesA = false;
                bool dividesB = false;
                
                for (char x : elements) {
                    if (multiply(elem, x) == a) dividesA = true;
                    if (multiply(elem, x) == b) dividesB = true;
                }
                
                if (dividesA && dividesB) {
                    return elem;
                }
            }
        }
    }
    
    // If no common divisor found, return multiplicative identity
    return multiplicativeIdentity;
}

char Algebra::lcm(char a, char b) const {
    // LCM (NOC) in finite algebra: smallest element m such that
    // both a and b divide m (i.e., exists x, y where a*x = m and b*y = m)
    
    // Special case: lcm with additive identity
    if (a == additiveIdentity || b == additiveIdentity) {
        return additiveIdentity;
    }
    
    // Search from lowest position up
    for (int pos = 0; pos < static_cast<int>(elements.size()); pos++) {
        // Get all elements at this position
        for (const auto& [elem, p] : elementPosition) {
            if (p == pos) {
                // Check if both a and b divide elem
                bool aDivides = false;
                bool bDivides = false;
                
                for (char x : elements) {
                    if (multiply(a, x) == elem) aDivides = true;
                    if (multiply(b, x) == elem) bDivides = true;
                }
                
                if (aDivides && bDivides) {
                    return elem;
                }
            }
        }
    }
    
    // If no common multiple found, return the last element
    return elements.back();
}

void Algebra::printAdditionTable() const {
    std::cout << "\nAddition Table (+):\n";
    std::cout << "===================\n";
    
    // Print header
    std::cout << "+    |";
    for (char e : elements) {
        std::cout << "  " << e << "    ";
    }
    std::cout << "\n-----|";
    for (size_t i = 0; i < elements.size(); i++) {
        std::cout << "-------";
    }
    std::cout << "\n";
    
    // Print rows
    for (char row : elements) {
        std::cout << row << "    |";
        for (char col : elements) {
            char resultElem = add(row, col);
            int resultPos = elementPosition.at(resultElem);
            std::string display = getElementsAtPosition(resultPos);
            
            // Center display in 7-char cell
            int padding = (7 - display.length()) / 2;
            for (int i = 0; i < padding; i++) std::cout << " ";
            std::cout << display;
            for (int i = 0; i < 7 - padding - display.length(); i++) std::cout << " ";
        }
        std::cout << "\n\n";  // Extra vertical space
    }
    std::cout << "\n";
}

void Algebra::printMultiplicationTable() const {
    std::cout << "\nMultiplication Table (*):\n";
    std::cout << "=========================\n";
    
    // Print header
    std::cout << "*    |";
    for (char e : elements) {
        std::cout << "  " << e << "    ";
    }
    std::cout << "\n-----|";
    for (size_t i = 0; i < elements.size(); i++) {
        std::cout << "-------";
    }
    std::cout << "\n";
    
    // Print rows
    for (char row : elements) {
        std::cout << row << "    |";
        for (char col : elements) {
            char resultElem = multiply(row, col);
            int resultPos = elementPosition.at(resultElem);
            std::string display = getElementsAtPosition(resultPos);
            
            // Center display in 7-char cell
            int padding = (7 - display.length()) / 2;
            for (int i = 0; i < padding; i++) std::cout << " ";
            std::cout << display;
            for (int i = 0; i < 7 - padding - display.length(); i++) std::cout << " ";
        }
        std::cout << "\n\n";  // Extra vertical space
    }
    std::cout << "\n";
}

void Algebra::printSubtractionTable() const {
    std::cout << "\nSubtraction Table (-):\n";
    std::cout << "======================\n";
    
    // Print header
    std::cout << "-    |";
    for (char e : elements) {
        std::cout << "  " << e << "    ";
    }
    std::cout << "\n-----|";
    for (size_t i = 0; i < elements.size(); i++) {
        std::cout << "-------";
    }
    std::cout << "\n";
    
    // Print rows
    for (char row : elements) {
        std::cout << row << "    |";
        for (char col : elements) {
            char resultElem = subtract(row, col);
            int resultPos = elementPosition.at(resultElem);
            std::string display = getElementsAtPosition(resultPos);
            
            // Center display in 7-char cell
            int padding = (7 - display.length()) / 2;
            for (int i = 0; i < padding; i++) std::cout << " ";
            std::cout << display;
            for (int i = 0; i < 7 - padding - display.length(); i++) std::cout << " ";
        }
        std::cout << "\n\n";  // Extra vertical space
    }
    std::cout << "\n";
}

void Algebra::printDivisionTable() const {
    std::cout << "\nDivision Table (/):\n";
    std::cout << "===================\n";
    
    // Print header
    std::cout << "/    |";
    for (char e : elements) {
        std::cout << "  " << e << "    ";
    }
    std::cout << "\n-----|";
    for (size_t i = 0; i < elements.size(); i++) {
        std::cout << "-------";
    }
    std::cout << "\n";
    
    // Print rows
    for (char row : elements) {
        std::cout << row << "    |";
        for (char col : elements) {
            char resultElem = divide(row, col);
            int resultPos = elementPosition.at(resultElem);
            std::string display = getElementsAtPosition(resultPos);
            
            // Center display in 7-char cell
            int padding = (7 - display.length()) / 2;
            for (int i = 0; i < padding; i++) std::cout << " ";
            std::cout << display;
            for (int i = 0; i < 7 - padding - display.length(); i++) std::cout << " ";
        }
        std::cout << "\n\n";  // Extra vertical space
    }
    std::cout << "\n";
}

void Algebra::printMap (){
    for (const auto& pair : elementPosition) {
        std::cout << pair.first << " -> " << pair.second << std::endl;
    }
}

void Algebra::printAdditionCarryTable() const {
    std::cout << "\nAddition Carry Table:\n";
    std::cout << "=====================\n";
    
    // Print header
    std::cout << "+    |";
    for (char e : elements) {
        std::cout << "  " << e << "    ";
    }
    std::cout << "\n-----|";
    for (size_t i = 0; i < elements.size(); i++) {
        std::cout << "-------";
    }
    std::cout << "\n";
    
    // Print rows
    for (char row : elements) {
        std::cout << row << "    |";
        for (char col : elements) {
            int carry = getAdditionCarry(row, col);
            
            std::string display;
            if (carry > 0) {
                // Convert carry count to element at that position
                display = getElementsAtPosition(carry);
            } else {
                display = getElementsAtPosition(0);  // 'a' for 0 carries
            }
            
            // Center display in 7-char cell
            int padding = (7 - display.length()) / 2;
            for (int i = 0; i < padding; i++) std::cout << " ";
            std::cout << display;
            for (int i = 0; i < 7 - padding - display.length(); i++) std::cout << " ";
        }
        std::cout << "\n\n";  // Extra vertical space
    }
    std::cout << "\n";
}

void Algebra::printMultiplicationCarryTable() const {
    std::cout << "\nMultiplication Carry Table:\n";
    std::cout << "===========================\n";
    
    // Print header
    std::cout << "*    |";
    for (char e : elements) {
        std::cout << "  " << e << "    ";
    }
    std::cout << "\n-----|";
    for (size_t i = 0; i < elements.size(); i++) {
        std::cout << "-------";
    }
    std::cout << "\n";
    
    // Print rows
    for (char row : elements) {
        std::cout << row << "    |";
        for (char col : elements) {
            int carry = getMultiplicationCarry(row, col);
            
            std::string display;
            if (carry > 0) {
                // Convert carry count to element at that position
                display = getElementsAtPosition(carry);
            } else {
                display = getElementsAtPosition(0);  // 'a' for 0 carries
            }
            
            // Center display in 7-char cell
            int padding = (7 - display.length()) / 2;
            for (int i = 0; i < padding; i++) std::cout << " ";
            std::cout << display;
            for (int i = 0; i < 7 - padding - display.length(); i++) std::cout << " ";
        }
        std::cout << "\n\n";  // Extra vertical space
    }
    std::cout << "\n";
}

std::string Algebra::addArithmetic(const std::string& a, const std::string& b) const {
    // Add two multi-digit numbers from right to left with carry propagation
    // Handle negative numbers: -a + b = b - a, a + (-b) = a - b, -a + (-b) = -(a + b)
    
    bool aNeg = !a.empty() && a[0] == '-';
    bool bNeg = !b.empty() && b[0] == '-';
    
    std::string aAbs = aNeg ? a.substr(1) : a;
    std::string bAbs = bNeg ? b.substr(1) : b;
    
    // Case 1: -a + (-b) = -(a + b)
    if (aNeg && bNeg) {
        std::string result = addArithmetic(aAbs, bAbs);
        return "-" + result;
    }
    
    // Case 2: -a + b = b - a
    if (aNeg && !bNeg) {
        return subtractArithmetic(bAbs, aAbs);
    }
    
    // Case 3: a + (-b) = a - b
    if (!aNeg && bNeg) {
        return subtractArithmetic(aAbs, bAbs);
    }
    
    // Case 4: a + b (both positive)
    std::string result;
    int carryValue = 0;
    
    // Process from right to left
    int i = aAbs.length() - 1;
    int j = bAbs.length() - 1;
    
    while (i >= 0 || j >= 0 || carryValue > 0) {
        char digitA = (i >= 0) ? aAbs[i] : additiveIdentity;
        char digitB = (j >= 0) ? bAbs[j] : additiveIdentity;
        
        // Add the two digits
        int carry1 = 0;
        char sum = addWithCarry(digitA, digitB, carry1);
        
        // Add the carry from previous position
        int carry2 = 0;
        for (int c = 0; c < carryValue; c++) {
            sum = addWithCarry(sum, multiplicativeIdentity, carry2);
            carry1 += carry2;
            carry2 = 0;
        }
        
        result = sum + result;  // Prepend to result
        carryValue = carry1;
        
        i--;
        j--;
    }
    
    return clampToBounds(result);
}

std::string Algebra::subtractArithmetic(const std::string& a, const std::string& b) const {
    // Subtract two multi-digit numbers using base-n positional arithmetic
    // Handle negative numbers: a - (-b) = a + b, (-a) - b = -(a + b), (-a) - (-b) = b - a
    
    bool aNeg = !a.empty() && a[0] == '-';
    bool bNeg = !b.empty() && b[0] == '-';
    
    std::string aAbs = aNeg ? a.substr(1) : a;
    std::string bAbs = bNeg ? b.substr(1) : b;
    
    // Case 1: a - (-b) = a + b
    if (!aNeg && bNeg) {
        return addArithmetic(aAbs, bAbs);
    }
    
    // Case 2: (-a) - b = -(a + b)
    if (aNeg && !bNeg) {
        std::string result = addArithmetic(aAbs, bAbs);
        return "-" + result;
    }
    
    // Case 3: (-a) - (-b) = b - a
    if (aNeg && bNeg) {
        return subtractArithmetic(bAbs, aAbs);
    }
    
    // Case 4: a - b (both positive)
    int base = getCycleLength();  // Use actual cycle length, not number of elements
    
    // Helper to compare two numbers (returns: 1 if a > b, -1 if a < b, 0 if equal)
    auto compare = [this](const std::string& a, const std::string& b) -> int {
        if (a.length() > b.length()) return 1;
        if (a.length() < b.length()) return -1;
        
        for (size_t i = 0; i < a.length(); i++) {
            int posA = elementPosition.at(a[i]);
            int posB = elementPosition.at(b[i]);
            if (posA > posB) return 1;
            if (posA < posB) return -1;
        }
        return 0;
    };
    
    int cmp = compare(aAbs, bAbs);
    
    // If a == b, return zero
    if (cmp == 0) {
        return std::string(1, additiveIdentity);
    }
    
    bool isNegative = false;
    std::string larger, smaller;
    
    // If a < b, result will be negative, swap them
    if (cmp < 0) {
        isNegative = true;
        larger = bAbs;
        smaller = aAbs;
    } else {
        larger = aAbs;
        smaller = bAbs;
    }
    
    // Do subtraction: larger - smaller
    std::string result;
    int borrow = 0;
    
    int i = larger.length() - 1;
    int j = smaller.length() - 1;
    
    while (i >= 0 || j >= 0) {
        int posLarger = (i >= 0) ? elementPosition.at(larger[i]) : 0;
        int posSmaller = (j >= 0) ? elementPosition.at(smaller[j]) : 0;
        
        int diff = posLarger - posSmaller - borrow;
        
        if (diff < 0) {
            diff += base;
            borrow = 1;
        } else {
            borrow = 0;
        }
        
        char digitChar = additiveIdentity;
        for (const auto& pair : elementPosition) {
            if (pair.second == diff) {
                digitChar = pair.first;
                break;
            }
        }
        
        result = digitChar + result;
        i--;
        j--;
    }
    
    // Remove leading zeros
    while (result.length() > 1 && result[0] == additiveIdentity) {
        result = result.substr(1);
    }
    
    // Add minus sign if negative
    if (isNegative) {
        result = "-" + result;
    }
    
    return clampToBounds(result);
}

std::string Algebra::multiplyArithmetic(const std::string& a, const std::string& b) const {
    // Multiply two multi-digit numbers using the standard algorithm
    // Handle negative numbers: (-a) * (-b) = a * b, (-a) * b = -(a * b), a * (-b) = -(a * b)
    
    if (a.empty() || b.empty()) return std::string(1, additiveIdentity);
    
    bool aNeg = !a.empty() && a[0] == '-';
    bool bNeg = !b.empty() && b[0] == '-';
    
    std::string aAbs = aNeg ? a.substr(1) : a;
    std::string bAbs = bNeg ? b.substr(1) : b;
    
    // Check if either is zero
    bool aIsZero = true, bIsZero = true;
    for (char c : aAbs) if (c != additiveIdentity) aIsZero = false;
    for (char c : bAbs) if (c != additiveIdentity) bIsZero = false;
    if (aIsZero || bIsZero) return std::string(1, additiveIdentity);
    
    std::string result(1, additiveIdentity);
    
    // Multiply absolute values
    // Multiply aAbs by each digit of bAbs
    for (int j = bAbs.length() - 1; j >= 0; j--) {
        std::string partialProduct;
        int carryValue = 0;
        
        // Multiply aAbs by bAbs[j]
        for (int i = aAbs.length() - 1; i >= 0; i--) {
            int carry = 0;
            char product = multiplyWithCarry(aAbs[i], bAbs[j], carry);
            
            // Add previous carry
            int addCarry = 0;
            for (int c = 0; c < carryValue; c++) {
                product = addWithCarry(product, multiplicativeIdentity, addCarry);
                carry += addCarry;
                addCarry = 0;
            }
            
            partialProduct = product + partialProduct;
            carryValue = carry;
        }
        
        // Add remaining carries
        while (carryValue > 0) {
            int pos = carryValue % getCycleLength();
            // Find an element at this position (use the first one)
            char carryChar = additiveIdentity;
            for (const auto& [elem, p] : elementPosition) {
                if (p == pos) {
                    carryChar = elem;
                    break;
                }
            }
            partialProduct = carryChar + partialProduct;
            carryValue /= getCycleLength();
        }
        
        // Add zeros for position (shift left)
        for (int k = j; k < (int)bAbs.length() - 1; k++) {
            partialProduct += additiveIdentity;
        }
        
        // Add to result
        result = addArithmetic(result, partialProduct);
    }
    
    // Apply sign: negative if signs differ
    if (aNeg != bNeg) {
        result = "-" + result;
    }
    
    return clampToBounds(result);
}

std::string Algebra::divideArithmetic(const std::string& a, const std::string& b, std::string& remainder) const {
    // Division with remainder using repeated subtraction
    // quotient = how many times we can subtract b from a
    // remainder = what's left after all subtractions
    
    // Handle negative numbers
    bool aNeg = !a.empty() && a[0] == '-';
    bool bNeg = !b.empty() && b[0] == '-';
    
    std::string aAbs = aNeg ? a.substr(1) : a;
    std::string bAbs = bNeg ? b.substr(1) : b;
    
    // Helper to check if number is zero
    auto isZero = [this](const std::string& s) {
        for (char c : s) {
            if (c != additiveIdentity) return false;
        }
        return true;
    };
    
    // Helper to compare two numbers (returns true if a >= b)
    auto isGreaterOrEqual = [this](const std::string& a, const std::string& b) {
        if (a.empty() || b.empty()) return a.length() >= b.length();
        if (a.length() > b.length()) return true;
        if (a.length() < b.length()) return false;
        
        for (size_t i = 0; i < a.length(); i++) {
            // Check if character exists in elementPosition
            if (elementPosition.find(a[i]) == elementPosition.end() ||
                elementPosition.find(b[i]) == elementPosition.end()) {
                return false;
            }
            int posA = elementPosition.at(a[i]);
            int posB = elementPosition.at(b[i]);
            if (posA > posB) return true;
            if (posA < posB) return false;
        }
        return true; // Equal
    };
    
    // Special case: a/a (0/0) should return range [min - max]
    if (isZero(aAbs) && isZero(bAbs)) {
        std::string minVal = getMinValue();
        std::string maxVal = getMaxValue();
        remainder = std::string(1, additiveIdentity);  // No remainder (zero)
        return "[" + minVal + " - " + maxVal + "]";
    }
    
    // Check for division by zero (any non-zero number / 0)
    if (isZero(bAbs)) {
        remainder = "∅";  // Empty set symbol for undefined
        return "∅";  // Return empty set symbol for division by zero
    }
    
    // Special case: x/x, -x/x, or x/-x (same non-zero values) should return range [min - max]
    if (aAbs == bAbs) {
        std::string minVal = getMinValue();
        std::string maxVal = getMaxValue();
        remainder = std::string(1, additiveIdentity);  // No remainder
        return "[" + minVal + " - " + maxVal + "]";
    }
    
    // If a < b, quotient is 0, remainder is a
    if (!isGreaterOrEqual(aAbs, bAbs)) {
        remainder = aAbs;
        return std::string(1, additiveIdentity);
    }
    
    // Repeatedly subtract b from a and count
    std::string current = aAbs;
    std::string quotient(1, additiveIdentity); // Start with 0
    
    int maxIterations = 100000;
    int iterations = 0;
    
    while (isGreaterOrEqual(current, bAbs) && !isZero(current) && iterations < maxIterations) {
        std::string newCurrent = subtractArithmetic(current, bAbs);
        
        // Check if subtraction resulted in negative (shouldn't happen with isGreaterOrEqual check)
        if (!newCurrent.empty() && newCurrent[0] == '-') {
            break;
        }
        
        current = newCurrent;
        
        // Increment quotient by 1
        quotient = addArithmetic(quotient, std::string(1, multiplicativeIdentity));
        
        iterations++;
        
        if (iterations >= maxIterations) {
            // Hit iteration limit - return what we have
            break;
        }
    }
    
    remainder = current;
    //TODO : DONE
    // Apply sign: quotient is negative if signs differ
    if (aNeg != bNeg) {
        // For negative dividend with non-zero remainder, adjust for Euclidean division
        if (aNeg && !isZero(remainder)) {
            // Quotient becomes more negative: -(|quotient| + 1)
            quotient = addArithmetic(quotient, std::string(1, multiplicativeIdentity));
            // Remainder becomes: divisor - remainder
            remainder = subtractArithmetic(bAbs, remainder);
        }
        quotient = "-" + quotient;
    }
    
    return quotient;
}

std::string Algebra::modArithmetic(const std::string& a, const std::string& b) const {
    // Calculate a mod b using repeated subtraction
    // a mod b = remainder when a is divided by b
    // For negative numbers, modulo should return positive result
    
    // Work with absolute values
    bool aNeg = !a.empty() && a[0] == '-';
    std::string aAbs = aNeg ? a.substr(1) : a;
    std::string bAbs = (!b.empty() && b[0] == '-') ? b.substr(1) : b;
    
    // Helper to check if number is zero
    auto isZero = [this](const std::string& s) {
        for (char c : s) {
            if (c != additiveIdentity) return false;
        }
        return true;
    };
    
    // Helper to compare two numbers (returns true if a >= b)
    auto isGreaterOrEqual = [this](const std::string& a, const std::string& b) {
        if (a.empty() || b.empty()) return a.length() >= b.length();
        if (a.length() > b.length()) return true;
        if (a.length() < b.length()) return false;
        
        // Same length, compare digit by digit
        for (size_t i = 0; i < a.length(); i++) {
            // Check if character exists in elementPosition
            if (elementPosition.find(a[i]) == elementPosition.end() ||
                elementPosition.find(b[i]) == elementPosition.end()) {
                return false;
            }
            int posA = elementPosition.at(a[i]);
            int posB = elementPosition.at(b[i]);
            if (posA > posB) return true;
            if (posA < posB) return false;
        }
        return true; // Equal
    };
    
    if (isZero(bAbs)) {
        // Modulo by zero is undefined, return a
        return a;
    }
    
    std::string remainder = aAbs;
    
    // Repeatedly subtract bAbs from remainder while remainder >= bAbs
    int maxIterations = 10000; // Safety limit to prevent infinite loops
    int iterations = 0;
    
    while (isGreaterOrEqual(remainder, bAbs) && !isZero(remainder) && iterations < maxIterations) {
        std::string newRemainder = subtractArithmetic(remainder, bAbs);
        
        // If subtraction resulted in negative or didn't change, we're done
        if (!newRemainder.empty() && newRemainder[0] == '-') {
            break;
        }
        if (newRemainder == remainder) {
            break;
        }
        
        remainder = newRemainder;
        iterations++;
    }
    
    return remainder;
}

std::string Algebra::powerArithmetic(const std::string& base, const std::string& exponent) const {
    // Calculate base ^ exponent using repeated multiplication
    // Handle negative base: (-a)^n = a^n if n is even, -(a^n) if n is odd
    // Negative exponents not supported (would require division/fractions)
    
    // Check if exponent is negative
    if (!exponent.empty() && exponent[0] == '-') {
        // Negative exponent not supported
        return std::string(1, additiveIdentity);  // Return 0
    }
    
    // Helper to check if number is zero
    auto isZero = [this](const std::string& s) {
        for (char c : s) {
            if (c != additiveIdentity) return false;
        }
        return true;
    };
    
    // Check if exponent is zero
    if (isZero(exponent)) {
        // Any number to the power of 0 is 1
        return std::string(1, multiplicativeIdentity);
    }
    
    // Check if base is zero
    if (isZero(base)) {
        return std::string(1, additiveIdentity);
    }
    
    // Handle negative base
    bool baseNeg = !base.empty() && base[0] == '-';
    std::string baseAbs = baseNeg ? base.substr(1) : base;
    
    // Check if exponent is 1
    if (exponent.length() == 1 && exponent[0] == multiplicativeIdentity) {
        return base;
    }
    
    // Use repeated multiplication: base^exp = base * base * ... * base (exp times)
    std::string result(1, multiplicativeIdentity);  // Start with 1
    std::string currentPower = baseAbs;
    std::string remainingExp = exponent;
    
    // We'll use a simple approach: multiply result by base, exp times
    // For efficiency, we could use binary exponentiation, but keep it simple
    
    int maxIterations = 10000;  // Safety limit
    int iterations = 0;
    
    // Count down from exponent to 0, multiplying each time
    while (!isZero(remainingExp) && iterations < maxIterations) {
        result = multiplyArithmetic(result, baseAbs);
        
        // Decrement remainingExp by 1
        remainingExp = subtractArithmetic(remainingExp, std::string(1, multiplicativeIdentity));
        
        // Check if subtraction resulted in negative (means we're done)
        if (!remainingExp.empty() && remainingExp[0] == '-') {
            break;
        }
        
        iterations++;
    }
    
    // Apply sign if base was negative and exponent is odd
    if (baseNeg) {
        // Check if exponent is odd by checking the last digit
        int expLastPos = elementPosition.at(exponent[exponent.length() - 1]);
        if (expLastPos % 2 == 1) {
            result = "-" + result;
        }
    }
    
    return clampToBounds(result);
}

std::string Algebra::gcdArithmetic(const std::string& a, const std::string& b) const {
    // Euclidean algorithm: GCD(a, b) = GCD(b, a mod b)
    // GCD works with absolute values
    
    std::string aAbs = (!a.empty() && a[0] == '-') ? a.substr(1) : a;
    std::string bAbs = (!b.empty() && b[0] == '-') ? b.substr(1) : b;
    
    // Helper to check if number is zero
    auto isZero = [this](const std::string& s) {
        for (char c : s) {
            if (c != additiveIdentity) return false;
        }
        return true;
    };
    
    std::string num1 = aAbs;
    std::string num2 = bAbs;
    
    if (isZero(num1)) return num2;
    if (isZero(num2)) return num1;
    
    // Euclidean algorithm using modulo
    while (!isZero(num2)) {
        std::string temp = num2;
        num2 = modArithmetic(num1, num2);
        num1 = temp;
    }
    
    return num1;
}

std::string Algebra::lcmArithmetic(const std::string& a, const std::string& b) const {
    // LCM(a, b) = (a * b) / GCD(a, b)
    // LCM works with absolute values
    
    std::string aAbs = (!a.empty() && a[0] == '-') ? a.substr(1) : a;
    std::string bAbs = (!b.empty() && b[0] == '-') ? b.substr(1) : b;
    
    // Check if either is zero
    auto isZero = [this](const std::string& s) {
        for (char c : s) {
            if (c != additiveIdentity) return false;
        }
        return true;
    };
    
    if (isZero(aAbs) || isZero(bAbs)) {
        return std::string(1, additiveIdentity);
    }
    
    // Calculate aAbs * bAbs
    std::string product = multiplyArithmetic(aAbs, bAbs);
    
    // Get GCD
    std::string gcdResult = gcdArithmetic(aAbs, bAbs);
    
    // Divide product by GCD
    std::string remainder;
    std::string lcmResult = divideArithmetic(product, gcdResult, remainder);
    
    return lcmResult;
}

std::string Algebra::getMaxValue() const {
    // Maximum value is limit repetitions of the element with highest position
    // Find element with maximum position in elementPosition map
    if (elements.empty() || elementPosition.empty()) return "";
    // TODO : DONE, hard coded boundry 8 bits
    const int limit = 8;  // Hardcoded limit for bounded mode
    
    char maxElem = additiveIdentity;
    int maxPos = -1;
    
    for (const auto& [elem, pos] : elementPosition) {
        if (pos > maxPos) {
            maxPos = pos;
            maxElem = elem;
        }
    }
    
    return std::string(limit, maxElem);
}

std::string Algebra::getMinValue() const {
    // Minimum value is negative of maximum
    return "-" + getMaxValue();
}

bool Algebra::exceedsBounds(const std::string& value) const {
    if (!boundedMode) return false;
    
    // Helper to check if number is zero
    auto isZero = [this](const std::string& s) {
        for (char c : s) {
            if (c != additiveIdentity) return false;
        }
        return true;
    };
    
    // Zero is always within bounds
    if (isZero(value)) return false;
    
    bool isNeg = !value.empty() && value[0] == '-';
    std::string absValue = isNeg ? value.substr(1) : value;
    std::string maxAbs = getMaxValue();
    
    // Check length first
    if (absValue.length() > maxAbs.length()) return true;
    if (absValue.length() < maxAbs.length()) return false;
    
    // Same length - compare element by element
    for (size_t i = 0; i < absValue.length(); i++) {
        if (elementPosition.find(absValue[i]) == elementPosition.end() ||
            elementPosition.find(maxAbs[i]) == elementPosition.end()) {
            return false;
        }
        int posVal = elementPosition.at(absValue[i]);
        int posMax = elementPosition.at(maxAbs[i]);
        if (posVal > posMax) return true;
        if (posVal < posMax) return false;
    }
    
    return false; // Equal to max, which is OK
}

std::string Algebra::clampToBounds(const std::string& value) const {
    if (!boundedMode) return value;
    
    // Check if value exceeds bounds
    if (!exceedsBounds(value)) return value;
    
    // Return overflow message instead of clamping to max/min
    return "преполнение";
}



