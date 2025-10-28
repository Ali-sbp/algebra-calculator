// Global state
let history = [];
const API_URL = '';  // Same origin

// Initialize algebra
async function initAlgebra() {
    const bits = document.getElementById('bits').value;
    const rule = document.getElementById('rule').value;
    
    if (!rule) {
        showError('Please enter a +1 rule');
        return;
    }
    
    try {
        const response = await fetch(`${API_URL}/api/init`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ bits: parseInt(bits), rule })
        });
        
        const data = await response.json();
        
        if (data.success) {
            showSuccess(`Algebra Z${bits} initialized successfully!`);
        } else {
            showError(data.error);
        }
    } catch (error) {
        showError('Failed to initialize algebra: ' + error);
    }
}

// Perform calculation
async function calculate(operation) {
    const num1 = document.getElementById('num1').value;
    const num2 = document.getElementById('num2').value;
    
    if (!num1 || !num2) {
        showError('Please enter both numbers');
        return;
    }
    
    try {
        const response = await fetch(`${API_URL}/api/calculate`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ num1, num2, operation })
        });
        
        const data = await response.json();
        
        if (data.success) {
            let result = data.result;
            if (data.remainder) {
                result += ` (remainder: ${data.remainder})`;
            }
            document.getElementById('result').value = result;
            
            // Add to history
            addToHistory(num1, num2, operation, result);
        } else {
            showError(data.error);
        }
    } catch (error) {
        showError('Calculation failed: ' + error);
    }
}

// Add to history
function addToHistory(num1, num2, operation, result) {
    const opSymbol = {
        'add': '+',
        'subtract': '−',
        'multiply': '×',
        'divide': '÷',
        'power': '^',
        'mod': 'mod',
        'gcd': 'NOD',
        'lcm': 'NOC'
    }[operation] || operation;
    
    const entry = { num1, num2, operation, result };
    history.unshift(entry);
    
    // Limit to 20 entries
    if (history.length > 20) {
        history.pop();
    }
    
    updateHistoryDisplay();
}

// Update history display
function updateHistoryDisplay() {
    const historyDiv = document.getElementById('history');
    historyDiv.innerHTML = '';
    
    history.forEach((entry, index) => {
        const opSymbol = {
            'add': '+',
            'subtract': '−',
            'multiply': '×',
            'divide': '÷',
            'power': '^',
            'mod': 'mod',
            'gcd': 'NOD',
            'lcm': 'NOC'
        }[entry.operation] || entry.operation;
        
        const div = document.createElement('div');
        div.className = 'history-item';
        div.textContent = `[${index + 1}] ${entry.num1} ${opSymbol} ${entry.num2} = ${entry.result}`;
        div.onclick = () => loadFromHistory(entry);
        historyDiv.appendChild(div);
    });
}

// Load entry from history
function loadFromHistory(entry) {
    document.getElementById('num1').value = entry.num1;
    document.getElementById('num2').value = entry.num2;
    document.getElementById('result').value = entry.result;
}

// Clear history
function clearHistory() {
    history = [];
    updateHistoryDisplay();
}

// Clear all inputs
function clearAll() {
    document.getElementById('bits').value = '8';
    document.getElementById('rule').value = '';
    document.getElementById('num1').value = '';
    document.getElementById('num2').value = '';
    document.getElementById('result').value = '';
    document.getElementById('tableDisplay').innerHTML = '';
}

// Show table
async function showTable() {
    const tableType = document.getElementById('tableType').value;
    
    try {
        if (tableType === 'hasse') {
            await showHasse();
            return;
        }
        
        const response = await fetch(`${API_URL}/api/table/${tableType}`);
        const data = await response.json();
        
        if (data.success) {
            displayTable(data.elements, data.table, tableType);
        } else {
            showError(data.error);
        }
    } catch (error) {
        showError('Failed to load table: ' + error);
    }
}

// Display table
function displayTable(elements, tableData, type) {
    const display = document.getElementById('tableDisplay');
    
    let html = '<table>';
    html += '<tr><th>' + type + '</th>';
    elements.forEach(e => html += `<th>${e}</th>`);
    html += '</tr>';
    
    elements.forEach(row => {
        html += `<tr><th>${row}</th>`;
        elements.forEach(col => {
            const entry = tableData.find(t => t.a === row && t.b === col);
            html += `<td>${entry ? entry.result : '-'}</td>`;
        });
        html += '</tr>';
    });
    
    html += '</table>';
    display.innerHTML = html;
}

// Show Hasse diagram
async function showHasse() {
    try {
        const response = await fetch(`${API_URL}/api/hasse`);
        const data = await response.json();
        
        if (data.success) {
            displayHasse(data.positions);
        } else {
            showError(data.error);
        }
    } catch (error) {
        showError('Failed to load Hasse diagram: ' + error);
    }
}

// Display Hasse diagram
function displayHasse(positions) {
    const display = document.getElementById('tableDisplay');
    
    // Group elements by position
    const groups = {};
    for (const [elem, pos] of Object.entries(positions)) {
        if (!groups[pos]) groups[pos] = [];
        groups[pos].push(elem);
    }
    
    // Sort positions
    const sortedPos = Object.keys(groups).sort((a, b) => parseInt(a) - parseInt(b));
    
    let html = '<div class="hasse-diagram">';
    html += sortedPos.map(pos => {
        const elems = groups[pos].sort();
        const display = elems.length > 1 ? `{${elems.join(',')}}` : elems[0];
        return `${display} (${pos})`;
    }).join(' → ');
    html += ' → (back to a)</div>';
    
    display.innerHTML = html;
}

// Bounded mode toggle
document.getElementById('boundedMode').addEventListener('change', async (e) => {
    try {
        await fetch(`${API_URL}/api/bounded`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ enabled: e.target.checked })
        });
    } catch (error) {
        console.error('Failed to set bounded mode:', error);
    }
});

// Show error message
function showError(message) {
    const result = document.getElementById('result');
    result.value = `Error: ${message}`;
    result.style.color = '#d32f2f';
    setTimeout(() => { result.style.color = ''; }, 3000);
}

// Show success message
function showSuccess(message) {
    const result = document.getElementById('result');
    result.value = message;
    result.style.color = '#388e3c';
    setTimeout(() => { result.value = ''; result.style.color = ''; }, 3000);
}
