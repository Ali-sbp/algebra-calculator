/*
 * C wrapper for Algebra class to expose to Python via ctypes
 * This creates a C-style API that Python can call
 */

#include "algebra.h"
#include <cstring>

extern "C" {
    // Opaque pointer type for Algebra object
    typedef void* AlgebraHandle;
    
    // Create new Algebra instance
    AlgebraHandle algebra_create(int bits) {
        return new Algebra(bits);
    }
    
    // Destroy Algebra instance
    void algebra_destroy(AlgebraHandle handle) {
        delete static_cast<Algebra*>(handle);
    }
    
    // Set plus one rule
    void algebra_set_plus_one_rule(AlgebraHandle handle, const char* rule) {
        static_cast<Algebra*>(handle)->setPlusOneRule(std::string(rule));
    }
    
    // Set bounded mode
    void algebra_set_bounded_mode(AlgebraHandle handle, bool enabled) {
        static_cast<Algebra*>(handle)->setBoundedMode(enabled);
    }
    
    // Get bounded mode
    bool algebra_get_bounded_mode(AlgebraHandle handle) {
        return static_cast<Algebra*>(handle)->isBoundedMode();
    }
    
    // Add arithmetic (multi-digit)
    void algebra_add_arithmetic(AlgebraHandle handle, const char* a, const char* b, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->addArithmetic(std::string(a), std::string(b));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // Subtract arithmetic
    void algebra_subtract_arithmetic(AlgebraHandle handle, const char* a, const char* b, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->subtractArithmetic(std::string(a), std::string(b));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // Multiply arithmetic
    void algebra_multiply_arithmetic(AlgebraHandle handle, const char* a, const char* b, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->multiplyArithmetic(std::string(a), std::string(b));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // Divide arithmetic
    void algebra_divide_arithmetic(AlgebraHandle handle, const char* a, const char* b, 
                                   char* quotient, int q_size, char* remainder, int r_size) {
        std::string rem;
        std::string quot = static_cast<Algebra*>(handle)->divideArithmetic(std::string(a), std::string(b), rem);
        strncpy(quotient, quot.c_str(), q_size - 1);
        quotient[q_size - 1] = '\0';
        strncpy(remainder, rem.c_str(), r_size - 1);
        remainder[r_size - 1] = '\0';
    }
    
    // Power arithmetic
    void algebra_power_arithmetic(AlgebraHandle handle, const char* base, const char* exp, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->powerArithmetic(std::string(base), std::string(exp));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // Mod arithmetic
    void algebra_mod_arithmetic(AlgebraHandle handle, const char* a, const char* b, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->modArithmetic(std::string(a), std::string(b));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // GCD arithmetic
    void algebra_gcd_arithmetic(AlgebraHandle handle, const char* a, const char* b, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->gcdArithmetic(std::string(a), std::string(b));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // LCM arithmetic
    void algebra_lcm_arithmetic(AlgebraHandle handle, const char* a, const char* b, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->lcmArithmetic(std::string(a), std::string(b));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // Format result
    void algebra_format_result(AlgebraHandle handle, const char* input, char* result, int result_size) {
        std::string res = static_cast<Algebra*>(handle)->formatMultiDigitResult(std::string(input));
        strncpy(result, res.c_str(), result_size - 1);
        result[result_size - 1] = '\0';
    }
    
    // Get element count
    int algebra_get_element_count(AlgebraHandle handle) {
        return static_cast<Algebra*>(handle)->getElements().size();
    }
    
    // Get element at index
    char algebra_get_element(AlgebraHandle handle, int index) {
        const auto& elements = static_cast<Algebra*>(handle)->getElements();
        if (index >= 0 && index < elements.size()) {
            return elements[index];
        }
        return 'a';
    }
    
    // Get element position
    int algebra_get_element_position(AlgebraHandle handle, char element) {
        const auto& positions = static_cast<Algebra*>(handle)->getElementPosition();
        auto it = positions.find(element);
        if (it != positions.end()) {
            return it->second;
        }
        return -1;
    }
    
    // Single element operations
    char algebra_add_single(AlgebraHandle handle, char a, char b) {
        return static_cast<Algebra*>(handle)->add(a, b);
    }
    
    char algebra_multiply_single(AlgebraHandle handle, char a, char b) {
        return static_cast<Algebra*>(handle)->multiply(a, b);
    }
    
    char algebra_subtract_single(AlgebraHandle handle, char a, char b) {
        return static_cast<Algebra*>(handle)->subtract(a, b);
    }
    
    char algebra_divide_single(AlgebraHandle handle, char a, char b) {
        return static_cast<Algebra*>(handle)->divide(a, b);
    }
    
    // Get addition carry
    int algebra_get_addition_carry(AlgebraHandle handle, char a, char b) {
        return static_cast<Algebra*>(handle)->getAdditionCarry(a, b);
    }
    
    // Get multiplication carry
    int algebra_get_multiplication_carry(AlgebraHandle handle, char a, char b) {
        return static_cast<Algebra*>(handle)->getMultiplicationCarry(a, b);
    }
    
    // Get plus one rule size
    int algebra_get_plus_one_rule_size(AlgebraHandle handle) {
        const auto& rule = static_cast<Algebra*>(handle)->getPlusOneRule();
        return rule.size();
    }
    
    // Get plus one rule for a specific input index (returns number of output elements)
    int algebra_get_plus_one_rule_outputs(AlgebraHandle handle, int inputIndex, char* outputs, int maxSize) {
        const auto& rule = static_cast<Algebra*>(handle)->getPlusOneRule();
        if (inputIndex < 0 || inputIndex >= (int)rule.size()) {
            return 0;
        }
        
        const auto& outputElements = rule[inputIndex];
        int count = std::min((int)outputElements.size(), maxSize);
        
        for (int i = 0; i < count; i++) {
            outputs[i] = outputElements[i];
        }
        
        return count;
    }
    
    // Get max value (for bounded mode)
    void algebra_get_max_value(AlgebraHandle handle, char* result, int maxSize) {
        std::string maxVal = static_cast<Algebra*>(handle)->getMaxValue();
        strncpy(result, maxVal.c_str(), maxSize - 1);
        result[maxSize - 1] = '\0';
    }
    
    // Get min value (for bounded mode)
    void algebra_get_min_value(AlgebraHandle handle, char* result, int maxSize) {
        std::string minVal = static_cast<Algebra*>(handle)->getMinValue();
        strncpy(result, minVal.c_str(), maxSize - 1);
        result[maxSize - 1] = '\0';
    }
}
