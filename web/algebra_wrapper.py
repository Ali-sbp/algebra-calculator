"""
Python wrapper for the C++ Algebra class
Uses ctypes to call the compiled shared library
"""

import ctypes
import os
from pathlib import Path

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '..', 'libalgebra.so')
if not os.path.exists(lib_path):
    raise RuntimeError(f"Shared library not found at {lib_path}. Please compile with: g++ -shared -fPIC -O3 algebra.cpp algebra_c_wrapper.cpp -o libalgebra.so -std=c++17")

_lib = ctypes.CDLL(lib_path)

# Define function signatures
_lib.algebra_create.argtypes = [ctypes.c_int]
_lib.algebra_create.restype = ctypes.c_void_p

_lib.algebra_destroy.argtypes = [ctypes.c_void_p]
_lib.algebra_destroy.restype = None

_lib.algebra_set_plus_one_rule.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
_lib.algebra_set_plus_one_rule.restype = None

_lib.algebra_set_bounded_mode.argtypes = [ctypes.c_void_p, ctypes.c_bool]
_lib.algebra_set_bounded_mode.restype = None

_lib.algebra_get_bounded_mode.argtypes = [ctypes.c_void_p]
_lib.algebra_get_bounded_mode.restype = ctypes.c_bool

_lib.algebra_add_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_subtract_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_multiply_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_divide_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_power_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_mod_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_gcd_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_lcm_arithmetic.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_format_result.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]

_lib.algebra_get_element_count.argtypes = [ctypes.c_void_p]
_lib.algebra_get_element_count.restype = ctypes.c_int

_lib.algebra_get_element.argtypes = [ctypes.c_void_p, ctypes.c_int]
_lib.algebra_get_element.restype = ctypes.c_char

_lib.algebra_get_element_position.argtypes = [ctypes.c_void_p, ctypes.c_char]
_lib.algebra_get_element_position.restype = ctypes.c_int

_lib.algebra_add_single.argtypes = [ctypes.c_void_p, ctypes.c_char, ctypes.c_char]
_lib.algebra_add_single.restype = ctypes.c_char

_lib.algebra_multiply_single.argtypes = [ctypes.c_void_p, ctypes.c_char, ctypes.c_char]
_lib.algebra_multiply_single.restype = ctypes.c_char

_lib.algebra_subtract_single.argtypes = [ctypes.c_void_p, ctypes.c_char, ctypes.c_char]
_lib.algebra_subtract_single.restype = ctypes.c_char

_lib.algebra_divide_single.argtypes = [ctypes.c_void_p, ctypes.c_char, ctypes.c_char]
_lib.algebra_divide_single.restype = ctypes.c_char

_lib.algebra_get_addition_carry.argtypes = [ctypes.c_void_p, ctypes.c_char, ctypes.c_char]
_lib.algebra_get_addition_carry.restype = ctypes.c_int

_lib.algebra_get_multiplication_carry.argtypes = [ctypes.c_void_p, ctypes.c_char, ctypes.c_char]
_lib.algebra_get_multiplication_carry.restype = ctypes.c_int

_lib.algebra_get_plus_one_rule_size.argtypes = [ctypes.c_void_p]
_lib.algebra_get_plus_one_rule_size.restype = ctypes.c_int

_lib.algebra_get_plus_one_rule_outputs.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_get_plus_one_rule_outputs.restype = ctypes.c_int

_lib.algebra_get_max_value.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_get_max_value.restype = None

_lib.algebra_get_min_value.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
_lib.algebra_get_min_value.restype = None


class Algebra:
    """Python interface to C++ Algebra calculator"""
    
    def __init__(self, bits=8):
        self.bits = bits
        self._handle = _lib.algebra_create(bits)
        self.initialized = False
    
    def __del__(self):
        if hasattr(self, '_handle') and self._handle:
            _lib.algebra_destroy(self._handle)
    
    def set_plus_one_rule(self, rule):
        """Set the +1 rule"""
        _lib.algebra_set_plus_one_rule(self._handle, rule.encode('utf-8'))
        self.initialized = True
    
    def set_bounded_mode(self, enabled):
        """Enable/disable bounded mode"""
        _lib.algebra_set_bounded_mode(self._handle, enabled)
    
    def get_bounded_mode(self):
        """Check if bounded mode is enabled"""
        return _lib.algebra_get_bounded_mode(self._handle)
    
    def add_arithmetic(self, a, b):
        """Add two multi-digit numbers"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_add_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def subtract_arithmetic(self, a, b):
        """Subtract two multi-digit numbers"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_subtract_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def multiply_arithmetic(self, a, b):
        """Multiply two multi-digit numbers"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_multiply_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def divide_arithmetic(self, a, b):
        """Divide two multi-digit numbers, return (quotient, remainder)"""
        quotient = ctypes.create_string_buffer(1024)
        remainder = ctypes.create_string_buffer(1024)
        _lib.algebra_divide_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), 
                                      quotient, 1024, remainder, 1024)
        return quotient.value.decode('utf-8'), remainder.value.decode('utf-8')
    
    def power_arithmetic(self, base, exp):
        """Calculate base^exp"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_power_arithmetic(self._handle, base.encode('utf-8'), exp.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def mod_arithmetic(self, a, b):
        """Calculate a mod b"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_mod_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def gcd_arithmetic(self, a, b):
        """Calculate GCD/NOD of a and b"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_gcd_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def lcm_arithmetic(self, a, b):
        """Calculate LCM/NOC of a and b"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_lcm_arithmetic(self._handle, a.encode('utf-8'), b.encode('utf-8'), result, 1024)
        return result.value.decode('utf-8')
    
    def format_result(self, input_str):
        """Format result with position notation"""
        result = ctypes.create_string_buffer(2048)
        _lib.algebra_format_result(self._handle, input_str.encode('utf-8'), result, 2048)
        return result.value.decode('utf-8')
    
    def get_elements(self):
        """Get list of all elements"""
        count = _lib.algebra_get_element_count(self._handle)
        elements = []
        for i in range(count):
            elem_byte = _lib.algebra_get_element(self._handle, i)
            # c_char returns bytes in Python 3
            if isinstance(elem_byte, bytes):
                elements.append(elem_byte.decode('utf-8'))
            else:
                elements.append(chr(elem_byte))
        return elements
    
    def get_element_position(self, element):
        """Get position of an element"""
        elem_byte = element.encode('utf-8')[0] if isinstance(element, str) else element
        return _lib.algebra_get_element_position(self._handle, elem_byte)
    
    def add_single(self, a, b):
        """Add two single elements"""
        a_byte = a.encode('utf-8')[0] if isinstance(a, str) else a
        b_byte = b.encode('utf-8')[0] if isinstance(b, str) else b
        result = _lib.algebra_add_single(self._handle, a_byte, b_byte)
        return result.decode('utf-8') if isinstance(result, bytes) else chr(result)
    
    def multiply_single(self, a, b):
        """Multiply two single elements"""
        a_byte = a.encode('utf-8')[0] if isinstance(a, str) else a
        b_byte = b.encode('utf-8')[0] if isinstance(b, str) else b
        result = _lib.algebra_multiply_single(self._handle, a_byte, b_byte)
        return result.decode('utf-8') if isinstance(result, bytes) else chr(result)
    
    def subtract_single(self, a, b):
        """Subtract two single elements"""
        a_byte = a.encode('utf-8')[0] if isinstance(a, str) else a
        b_byte = b.encode('utf-8')[0] if isinstance(b, str) else b
        result = _lib.algebra_subtract_single(self._handle, a_byte, b_byte)
        return result.decode('utf-8') if isinstance(result, bytes) else chr(result)
    
    def divide_single(self, a, b):
        """Divide two single elements"""
        a_byte = a.encode('utf-8')[0] if isinstance(a, str) else a
        b_byte = b.encode('utf-8')[0] if isinstance(b, str) else b
        result = _lib.algebra_divide_single(self._handle, a_byte, b_byte)
        return result.decode('utf-8') if isinstance(result, bytes) else chr(result)
    
    def get_addition_carry(self, a, b):
        """Get carry for addition of two elements"""
        a_byte = a.encode('utf-8')[0] if isinstance(a, str) else a
        b_byte = b.encode('utf-8')[0] if isinstance(b, str) else b
        return _lib.algebra_get_addition_carry(self._handle, a_byte, b_byte)
    
    def get_multiplication_carry(self, a, b):
        """Get carry for multiplication of two elements"""
        a_byte = a.encode('utf-8')[0] if isinstance(a, str) else a
        b_byte = b.encode('utf-8')[0] if isinstance(b, str) else b
        return _lib.algebra_get_multiplication_carry(self._handle, a_byte, b_byte)
    
    def get_max_value(self):
        """Get maximum value for bounded mode"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_get_max_value(self._handle, result, 1024)
        return result.value.decode('utf-8')
    
    def get_min_value(self):
        """Get minimum value for bounded mode"""
        result = ctypes.create_string_buffer(1024)
        _lib.algebra_get_min_value(self._handle, result, 1024)
        return result.value.decode('utf-8')
    
    def get_addition_table(self):
        """Generate addition table"""
        elements = self.get_elements()
        table = {}
        for a in elements:
            for b in elements:
                table[(a, b)] = self.add_single(a, b)
        return table
    
    def get_multiplication_table(self):
        """Generate multiplication table"""
        elements = self.get_elements()
        table = {}
        for a in elements:
            for b in elements:
                table[(a, b)] = self.multiply_single(a, b)
        return table
    
    def get_subtraction_table(self):
        """Generate subtraction table"""
        elements = self.get_elements()
        table = {}
        for a in elements:
            for b in elements:
                table[(a, b)] = self.subtract_single(a, b)
        return table
    
    def get_division_table(self):
        """Generate division table"""
        elements = self.get_elements()
        table = {}
        for a in elements:
            for b in elements:
                table[(a, b)] = self.divide_single(a, b)
        return table
    
    def get_addition_carry_table(self):
        """Generate addition carry table"""
        elements = self.get_elements()
        table = {}
        for a in elements:
            for b in elements:
                table[(a, b)] = self.get_addition_carry(a, b)
        return table
    
    def get_multiplication_carry_table(self):
        """Generate multiplication carry table"""
        elements = self.get_elements()
        table = {}
        for a in elements:
            for b in elements:
                table[(a, b)] = self.get_multiplication_carry(a, b)
        return table
    
    def get_hasse_diagram_data(self):
        """Get Hasse diagram structure with element positions and plus one rule"""
        elements = self.get_elements()
        positions = {}
        for elem in elements:
            pos = self.get_element_position(elem)
            if pos >= 0:
                positions[elem] = pos
        
        # Get plus one rule
        rule_size = _lib.algebra_get_plus_one_rule_size(self._handle)
        plus_one_rule = []
        
        for i in range(rule_size):
            outputs = ctypes.create_string_buffer(256)
            count = _lib.algebra_get_plus_one_rule_outputs(self._handle, i, outputs, 256)
            if count > 0:
                # outputs is a buffer, need to decode bytes properly
                output_list = []
                for j in range(count):
                    char_val = outputs[j]
                    if isinstance(char_val, bytes):
                        output_list.append(char_val.decode('utf-8'))
                    elif isinstance(char_val, int):
                        output_list.append(chr(char_val))
                    else:
                        output_list.append(str(char_val))
                plus_one_rule.append(output_list)
            else:
                plus_one_rule.append([])
        
        return {
            'element_positions': positions,
            'plus_one_rule': plus_one_rule,
            'elements': elements
        }

