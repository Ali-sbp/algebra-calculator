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

// Display Hasse diagram with SVG visualization (like Qt version)
function displayHasse(hasseData) {
    const display = document.getElementById('tableDisplay');
    
    if (!hasseData || !hasseData.element_positions) {
        display.innerHTML = '<div class="hasse-empty">No Hasse Diagram<br>Enter +1 rule to display</div>';
        return;
    }
    
    const positions = hasseData.element_positions;
    const plusOneRule = hasseData.plus_one_rule || [];
    const elements = hasseData.elements || [];
    
    // Group elements by position
    const groups = {};
    let maxPosition = 0;
    for (const [elem, pos] of Object.entries(positions)) {
        if (!groups[pos]) groups[pos] = [];
        groups[pos].push(elem);
        maxPosition = Math.max(maxPosition, parseInt(pos));
    }
    
    // Sort elements in each group by their source index in the rule
    const elementSourceIndex = {};
    plusOneRule.forEach((outputs, inputIdx) => {
        outputs.forEach(outputElem => {
            if (!elementSourceIndex.hasOwnProperty(outputElem)) {
                elementSourceIndex[outputElem] = inputIdx;
            }
        });
    });
    
    for (const pos in groups) {
        groups[pos].sort((a, b) => {
            const sourceA = elementSourceIndex[a] !== undefined ? elementSourceIndex[a] : 999;
            const sourceB = elementSourceIndex[b] !== undefined ? elementSourceIndex[b] : 999;
            return sourceA - sourceB;
        });
    }
    
    // Sort positions
    const sortedPos = Object.keys(groups).sort((a, b) => parseInt(a) - parseInt(b));
    
    // SVG dimensions
    const width = 400;
    const height = Math.max(500, sortedPos.length * 100);
    const nodeRadius = 25;
    const horizontalSpacing = 60;
    const centerX = width / 2;
    const marginTop = 30;
    const marginBottom = 30;
    const verticalStep = (height - marginTop - marginBottom) / (sortedPos.length + 1);
    
    // Calculate node positions
    const nodePositions = {};
    sortedPos.forEach((pos, idx) => {
        const y = height - marginBottom - (idx + 1) * verticalStep;
        const elems = groups[pos];
        
        elems.forEach((elem, j) => {
            const offsetX = (j - (elems.length - 1) / 2) * horizontalSpacing;
            nodePositions[elem] = { x: centerX + offsetX, y: y, pos: parseInt(pos) };
        });
    });
    
    // Build SVG
    let svg = `<svg width="${width}" height="${height}" style="background: #1e1e1e; border-radius: 8px;">`;
    
    // Draw edges based on +1 rule
    plusOneRule.forEach((outputs, inputIdx) => {
        const inputElem = String.fromCharCode(97 + inputIdx); // 'a' + inputIdx
        
        if (!nodePositions[inputElem]) return;
        
        const fromPos = nodePositions[inputElem];
        const inputPosition = fromPos.pos;
        
        outputs.forEach(outputElem => {
            if (!nodePositions[outputElem]) return;
            
            const toPos = nodePositions[outputElem];
            const outputPosition = toPos.pos;
            
            // Skip cycle edges (from top to bottom)
            if (inputPosition === maxPosition && outputPosition === 0) {
                return;
            }
            
            // Calculate line endpoints on circle edges
            const dx = toPos.x - fromPos.x;
            const dy = toPos.y - fromPos.y;
            const angle = Math.atan2(dy, dx);
            
            const startX = fromPos.x + nodeRadius * Math.cos(angle);
            const startY = fromPos.y + nodeRadius * Math.sin(angle);
            const endX = toPos.x - nodeRadius * Math.cos(angle);
            const endY = toPos.y - nodeRadius * Math.sin(angle);
            
            svg += `<line x1="${startX}" y1="${startY}" x2="${endX}" y2="${endY}" stroke="white" stroke-width="2"/>`;
        });
    });
    
    // Draw cycle indicator (dashed arrow from top back to bottom)
    if (sortedPos.length > 1) {
        const topElems = groups[sortedPos[sortedPos.length - 1]];
        const bottomElems = groups[sortedPos[0]];
        
        const topElem = topElems[topElems.length - 1];
        const bottomElem = bottomElems[bottomElems.length - 1];
        
        const topPos = nodePositions[topElem];
        const bottomPos = nodePositions[bottomElem];
        
        // Draw horizontal line connecting all top elements if multiple
        if (topElems.length > 1) {
            const leftElem = topElems[0];
            const rightElem = topElems[topElems.length - 1];
            const leftPos = nodePositions[leftElem];
            const rightPos = nodePositions[rightElem];
            
            svg += `<line x1="${leftPos.x}" y1="${leftPos.y}" x2="${rightPos.x}" y2="${rightPos.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        }
        
        const curveOffset = 50;
        const topRight = { x: topPos.x + curveOffset, y: topPos.y };
        const bottomRight = { x: bottomPos.x + curveOffset, y: bottomPos.y };
        
        svg += `<line x1="${topPos.x}" y1="${topPos.y}" x2="${topRight.x}" y2="${topRight.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        svg += `<line x1="${topRight.x}" y1="${topRight.y}" x2="${bottomRight.x}" y2="${bottomRight.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        svg += `<line x1="${bottomRight.x}" y1="${bottomRight.y}" x2="${bottomPos.x}" y2="${bottomPos.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        
        // Arrow head
        svg += `<polygon points="${bottomPos.x + 10},${bottomPos.y} ${bottomPos.x + 5},${bottomPos.y - 5} ${bottomPos.x + 5},${bottomPos.y + 5}" fill="white"/>`;
    }
    
    // Draw nodes
    for (const [elem, nodePos] of Object.entries(nodePositions)) {
        const { x, y, pos } = nodePos;
        
        // Circle
        svg += `<circle cx="${x}" cy="${y}" r="${nodeRadius}" fill="#6496c8" stroke="white" stroke-width="2"/>`;
        
        // Element label
        svg += `<text x="${x}" y="${y + 5}" text-anchor="middle" fill="white" font-size="16" font-weight="bold">${elem}</text>`;
        
        // Position number (to the left, only show once per level)
        const groupElems = groups[pos];
        if (elem === groupElems[0]) {
            svg += `<text x="${x - nodeRadius - 35}" y="${y + 5}" text-anchor="end" fill="lightgray" font-size="12">(${pos})</text>`;
        }
    }
    
    svg += '</svg>';
    
    display.innerHTML = `<div class="hasse-container">${svg}</div>`;
}

// Bounded mode toggle
document.getElementById('boundedMode').addEventListener('change', async (e) => {
    try {
        const response = await fetch(`${API_URL}/api/bounded`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ enabled: e.target.checked })
        });
        
        const data = await response.json();
        
        if (data.success && e.target.checked && data.max_value && data.min_value) {
            alert(`Bounded Mode Enabled\n\nArithmetic results will be clamped to:\nMaximum: ${data.max_value}\nMinimum: ${data.min_value}`);
        }
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
