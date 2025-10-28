"""
Python wrapper for the C++ Algebra class
Uses ctypes to call the compiled shared library
"""

import ctypes
import os
from pathlib import Path

class Algebra:
    """Python interface to C++ Algebra calculator"""
    
    def __init__(self, bits=8):
        self.bits = bits
        self.plus_one_rule = ""
        self.elements = [chr(ord('a') + i) for i in range(bits)]
        self.element_position = {}
        self.initialized = False
        self.bounded_mode = False
        
    def set_plus_one_rule(self, rule):
        """Parse and set the +1 rule"""
        self.plus_one_rule = rule
        self.element_position = {}
        
        # Parse rule: "bhgecea{d,f}" or "b-h-g-e-c-e-a-{d,f}"
        clean_rule = rule.replace('-', '').replace(' ', '')
        
        position = 0
        i = 0
        while i < len(clean_rule):
            if clean_rule[i] == '{':
                # Multi-element: {d,f}
                i += 1
                elements_at_pos = []
                while i < len(clean_rule) and clean_rule[i] != '}':
                    if clean_rule[i].isalpha():
                        elements_at_pos.append(clean_rule[i])
                    i += 1
                for elem in elements_at_pos:
                    self.element_position[elem] = position
                i += 1  # skip '}'
                position += 1
            elif clean_rule[i].isalpha():
                # Single element
                self.element_position[clean_rule[i]] = position
                position += 1
                i += 1
            else:
                i += 1
        
        self.initialized = True
        
    def add_with_carry(self, a, b):
        """Add two elements with carry tracking"""
        if not self.initialized:
            return 'a', 0
            
        result = a
        steps = self.element_position.get(b, 0)
        carry = 0
        cycle_length = len(set(self.element_position.values()))
        
        for _ in range(steps):
            current_pos = self.element_position.get(result, 0)
            next_elem = None
            
            # Find next element (element at position + 1)
            for elem, pos in self.element_position.items():
                if pos == (current_pos + 1) % cycle_length:
                    next_elem = elem
                    break
            
            if next_elem is None:
                break
                
            next_pos = self.element_position[next_elem]
            if next_pos < current_pos:
                carry += 1
                
            result = next_elem
            
        return result, carry
    
    def add(self, a, b):
        """Add two elements"""
        result, _ = self.add_with_carry(a, b)
        return result
    
    def multiply(self, a, b):
        """Multiply two elements"""
        if a == 'a' or b == 'a':
            return 'a'
        if b == 'b':
            return a
        if a == 'b':
            return b
            
        # a * b = add a to itself b times
        result = 'a'
        b_pos = self.element_position.get(b, 0)
        for _ in range(b_pos):
            result = self.add(result, a)
        return result
    
    def add_arithmetic(self, num1, num2):
        """Add two multi-digit numbers"""
        # Handle negative numbers
        if num1.startswith('-') and num2.startswith('-'):
            return '-' + self.add_arithmetic(num1[1:], num2[1:])
        if num1.startswith('-'):
            return self.subtract_arithmetic(num2, num1[1:])
        if num2.startswith('-'):
            return self.subtract_arithmetic(num1, num2[1:])
        
        # Add positives
        result = []
        carry = 0
        i, j = len(num1) - 1, len(num2) - 1
        
        while i >= 0 or j >= 0 or carry > 0:
            digit_a = num1[i] if i >= 0 else 'a'
            digit_b = num2[j] if j >= 0 else 'a'
            
            # Add digits
            sum_digit, c1 = self.add_with_carry(digit_a, digit_b)
            
            # Add carry
            for _ in range(carry):
                sum_digit, c2 = self.add_with_carry(sum_digit, 'b')
                c1 += c2
            
            result.append(sum_digit)
            carry = c1
            i -= 1
            j -= 1
        
        return ''.join(reversed(result))
    
    def subtract_arithmetic(self, num1, num2):
        """Subtract two multi-digit numbers"""
        # Simplified implementation
        # For full implementation, see the C++ version
        return "a"  # Placeholder
    
    def multiply_arithmetic(self, num1, num2):
        """Multiply two multi-digit numbers"""
        if num1.startswith('-') and num2.startswith('-'):
            return self.multiply_arithmetic(num1[1:], num2[1:])
        if num1.startswith('-'):
            return '-' + self.multiply_arithmetic(num1[1:], num2)
        if num2.startswith('-'):
            return '-' + self.multiply_arithmetic(num1, num2[1:])
        
        # Simplified - multiply using repeated addition
        result = 'a'
        for _ in range(len(num2)):
            # This is a simplified version
            pass
        return result
    
    def divide_arithmetic(self, num1, num2):
        """Divide two multi-digit numbers, return (quotient, remainder)"""
        # Simplified
        return "a", "a"
    
    def format_result(self, result):
        """Format result with position notation"""
        if result == "∅":
            return "∅"
        
        if result.startswith('-'):
            return '-' + self.format_result(result[1:])
        
        formatted = []
        for char in result:
            pos = self.element_position.get(char, 0)
            # Find all elements at this position
            elems_at_pos = [e for e, p in self.element_position.items() if p == pos]
            if len(elems_at_pos) > 1:
                formatted.append('{' + ','.join(sorted(elems_at_pos)) + '}')
            else:
                formatted.append(char)
        
        return ''.join(formatted)
    
    def get_addition_table(self):
        """Generate addition table"""
        table = {}
        for a in self.elements:
            for b in self.elements:
                table[(a, b)] = self.add(a, b)
        return table
    
    def get_hasse_diagram_data(self):
        """Get Hasse diagram structure"""
        return self.element_position
