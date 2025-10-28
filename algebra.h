#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <string>
#include <vector>
#include <map>

class Algebra {
private:
    std::vector<char> elements;                      // All elements in order: a, b, c, d, e, f, g, h
    std::vector<std::vector<char>> plusOneRule;      // The +1 sequence (can have multiple elements per position)
    //for XL4/XL5 variants i can parse the rule as 3x3 matrix which lets the other type of elements exist also.
    std::map<char, int> elementPosition;             // Maps element to its position in Hasse diagram
    char additiveIdentity;                           // 'a'
    char multiplicativeIdentity;                     // 'b'
    bool boundedMode;                                // Enable/disable bounded arithmetic
    int bits;                                        // Number of elements (algebra size)
    
    // Operation tables
    std::map<std::pair<char, char>, char> additionTable;
    std::map<std::pair<char, char>, char> multiplicationTable;
    std::map<std::pair<char, char>, char> subtractionTable;
    std::map<std::pair<char, char>, char> divisionTable;
    
    // Carry tables (track how many times we overflow the cycle)
    //can easily change to map::<pair<char,char> char> (also count the cycles using the same algebra)
    std::map<std::pair<char, char>, int> additionCarryTable;
    std::map<std::pair<char, char>, int> multiplicationCarryTable;
    
    // Helper methods
    //char Add(char elem1, char elem2); 
    void BuildHasse();
    void buildAdditionTable();
    void buildMultiplicationTable();
    void buildSubtractionTable();
    void buildDivisionTable();
    char addMultipleTimes(char element, int times) const;
    char addMultipleTimesWithCarry(char element, int times, int& carry) const;
    std::string getElementsAtPosition(int position) const;  // Returns formatted string of elements at position
    int getCycleLength() const;  // Returns the number of distinct positions in the cycle
    std::string clampToBounds(const std::string& value) const;  // Clamp value to min/max bounds
    bool exceedsBounds(const std::string& value) const;  // Check if value exceeds bounds
    //void bitLimiter()
public:
    // Constructor
    Algebra(int bits = 8);
    
    void printMap();
    
    // Bounded mode helpers
    std::string getMaxValue() const;  // Get maximum value (e.g., "gggggggg" for 8-bit)
    std::string getMinValue() const;  // Get minimum value (e.g., "-gggggggg" for 8-bit)
    // Input the +1 rule
    void setPlusOneRule(const std::string& rule);
    
    // Display Hasse diagram
    void printHasseDiagram() const;
    
    // Operations
    char add(char a, char b) const;
    char multiply(char a, char b) const;
    char power(char base, char exponent) const;
    char subtract(char a, char b) const;
    char divide(char a, char b) const;
    char gcd(char a, char b) const;  // NOD - Greatest Common Divisor
    char lcm(char a, char b) const;  // NOC - Least Common Multiple
    
    // Operations with carry
    char addWithCarry(char a, char b, int& carry) const;
    char multiplyWithCarry(char a, char b, int& carry) const;
    int getAdditionCarry(char a, char b) const;
    int getMultiplicationCarry(char a, char b) const;
    
    // Multi-digit arithmetic operations
    std::string addArithmetic(const std::string& a, const std::string& b) const;
    std::string subtractArithmetic(const std::string& a, const std::string& b) const;
    std::string multiplyArithmetic(const std::string& a, const std::string& b) const;
    std::string divideArithmetic(const std::string& a, const std::string& b, std::string& remainder) const;
    std::string powerArithmetic(const std::string& base, const std::string& exponent) const;  // base ^ exponent
    std::string modArithmetic(const std::string& a, const std::string& b) const;  // a mod b
    std::string formatMultiDigitResult(const std::string& result) const;  // Format result with braces for equivalent elements
    std::string gcdArithmetic(const std::string& a, const std::string& b) const;  // Multi-digit GCD/NOD
    std::string lcmArithmetic(const std::string& a, const std::string& b) const;  // Multi-digit LCM/NOC
    
    // Print tables
    void printAdditionTable() const;
    void printMultiplicationTable() const;
    void printSubtractionTable() const;
    void printDivisionTable() const;
    void printAdditionCarryTable() const;
    void printMultiplicationCarryTable() const;
    
    // Getters
    const std::vector<char>& getElements() const { return elements; }
    const std::vector<std::vector<char>>& getPlusOneRule() const { return plusOneRule; }
    const std::map<char, int>& getElementPosition() const { return elementPosition; }
    
    // Bounded mode control
    void setBoundedMode(bool enabled) { boundedMode = enabled; }
    bool isBoundedMode() const { return boundedMode; }
};

#endif // ALGEBRA_H
