"""
Flask web server for Algebra Calculator
"""

from flask import Flask, render_template, request, jsonify
from flask_cors import CORS
from algebra_wrapper import Algebra

app = Flask(__name__)
CORS(app)

# Global algebra instance
algebra = None

@app.route('/')
def index():
    """Serve the main page"""
    return render_template('index.html')

@app.route('/api/init', methods=['POST'])
def init_algebra():
    """Initialize algebra with bits and +1 rule"""
    global algebra
    data = request.json
    bits = int(data.get('bits', 8))
    rule = data.get('rule', '')
    
    try:
        algebra = Algebra(bits)
        algebra.set_plus_one_rule(rule)
        
        return jsonify({
            'success': True,
            'message': f'Algebra Z{bits} initialized',
            'elements': algebra.elements,
            'hasse': algebra.get_hasse_diagram_data()
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

@app.route('/api/calculate', methods=['POST'])
def calculate():
    """Perform calculation"""
    global algebra
    if not algebra or not algebra.initialized:
        return jsonify({'success': False, 'error': 'Algebra not initialized'}), 400
    
    data = request.json
    num1 = data.get('num1', '')
    num2 = data.get('num2', '')
    operation = data.get('operation', '')
    
    try:
        result = None
        remainder = None
        
        if operation == 'add':
            result = algebra.add_arithmetic(num1, num2)
        elif operation == 'subtract':
            result = algebra.subtract_arithmetic(num1, num2)
        elif operation == 'multiply':
            result = algebra.multiply_arithmetic(num1, num2)
        elif operation == 'divide':
            result, remainder = algebra.divide_arithmetic(num1, num2)
        else:
            return jsonify({'success': False, 'error': 'Unknown operation'}), 400
        
        formatted_result = algebra.format_result(result)
        
        response = {
            'success': True,
            'result': formatted_result
        }
        
        if remainder is not None:
            response['remainder'] = algebra.format_result(remainder)
        
        return jsonify(response)
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

@app.route('/api/table/<table_type>', methods=['GET'])
def get_table(table_type):
    """Get operation table"""
    global algebra
    if not algebra or not algebra.initialized:
        return jsonify({'success': False, 'error': 'Algebra not initialized'}), 400
    
    try:
        if table_type == 'addition':
            table = algebra.get_addition_table()
        elif table_type == 'multiplication':
            table = algebra.get_multiplication_table()
        elif table_type == 'subtraction':
            table = algebra.get_subtraction_table()
        elif table_type == 'division':
            table = algebra.get_division_table()
        elif table_type == 'addition_carry':
            table = algebra.get_addition_carry_table()
        elif table_type == 'multiplication_carry':
            table = algebra.get_multiplication_carry_table()
        else:
            return jsonify({'success': False, 'error': 'Unknown table type'}), 400
        
        # Convert to serializable format
        table_data = []
        for (a, b), result in table.items():
            table_data.append({
                'a': a,
                'b': b,
                'result': result
            })
        
        return jsonify({
            'success': True,
            'elements': algebra.elements,
            'table': table_data
        })
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 400

@app.route('/api/hasse', methods=['GET'])
def get_hasse():
    """Get Hasse diagram data"""
    global algebra
    if not algebra or not algebra.initialized:
        return jsonify({'success': False, 'error': 'Algebra not initialized'}), 400
    
    return jsonify({
        'success': True,
        'positions': algebra.get_hasse_diagram_data()
    })

@app.route('/api/bounded', methods=['POST'])
def set_bounded():
    """Enable/disable bounded mode"""
    global algebra
    if not algebra:
        return jsonify({'success': False, 'error': 'Algebra not initialized'}), 400
    
    data = request.json
    enabled = data.get('enabled', False)
    algebra.set_bounded_mode(enabled)
    
    response = {'success': True, 'bounded': enabled}
    
    # If enabling bounded mode and algebra is initialized, return max/min values
    if enabled and algebra.initialized:
        try:
            max_val = algebra.format_result(algebra.get_max_value())
            min_val = algebra.format_result(algebra.get_min_value())
            response['max_value'] = max_val
            response['min_value'] = min_val
        except:
            pass
    
    return jsonify(response)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
