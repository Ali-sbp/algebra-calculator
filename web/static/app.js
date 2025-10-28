const API_URL = window.location.origin;
let calculationHistory = [];
const MAX_HISTORY = 20;

// Dark mode toggle
document.addEventListener('DOMContentLoaded', () => {
    const darkModeToggle = document.getElementById('darkModeToggle');
    const savedMode = localStorage.getItem('darkMode');
    
    if (savedMode === 'enabled') {
        document.body.classList.add('dark-mode');
        darkModeToggle.checked = true;
    }
    
    darkModeToggle.addEventListener('change', () => {
        if (darkModeToggle.checked) {
            document.body.classList.add('dark-mode');
            localStorage.setItem('darkMode', 'enabled');
        } else {
            document.body.classList.remove('dark-mode');
            localStorage.setItem('darkMode', 'disabled');
        }
    });
});

// Initialize algebra
async function initAlgebra() {
    const bits = document.getElementById('bits').value;
    const rule = document.getElementById('rule').value;
    
    try {
        const response = await fetch(`${API_URL}/api/init`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({ bits, rule })
        });
        
        const data = await response.json();
        
        if (data.success) {
            showSuccess(data.message);
            // Update both table displays and Hasse diagram
            updateTable1();
            updateTable2();
            updateHasse();
        } else {
            showError(data.error);
        }
    } catch (error) {
        showError('Failed to initialize: ' + error);
    }
}

// Calculate operation
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
            let resultText = data.result;
            if (data.remainder !== undefined) {
                resultText += ` (remainder: ${data.remainder})`;
            }
            document.getElementById('result').value = resultText;
            document.getElementById('result').style.color = '';
            
            // Add to history
            const opSymbols = {
                'add': '+', 'subtract': '-', 'multiply': '×', 
                'divide': '÷', 'power': '^', 'mod': 'mod',
                'gcd': 'NOD', 'lcm': 'NOC'
            };
            addToHistory(num1, num2, opSymbols[operation] || operation, resultText);
        } else {
            showError(data.error);
        }
    } catch (error) {
        showError('Calculation failed: ' + error);
    }
}

// Add to history
function addToHistory(num1, num2, operation, result) {
    const entry = { num1, num2, operation, result };
    calculationHistory.unshift(entry);
    
    if (calculationHistory.length > MAX_HISTORY) {
        calculationHistory.pop();
    }
    
    updateHistoryDisplay();
}

// Update history display
function updateHistoryDisplay() {
    const historyDiv = document.getElementById('history');
    historyDiv.innerHTML = calculationHistory.map((entry, index) => 
        `<div class="history-item" onclick="loadFromHistory(${index})">
            ${entry.num1} ${entry.operation} ${entry.num2} = ${entry.result}
        </div>`
    ).join('');
}

// Load from history
function loadFromHistory(index) {
    const entry = calculationHistory[index];
    document.getElementById('num1').value = entry.num1;
    document.getElementById('num2').value = entry.num2;
    document.getElementById('result').value = entry.result;
}

// Clear history
function clearHistory() {
    calculationHistory = [];
    updateHistoryDisplay();
}

// Clear all
function clearAll() {
    document.getElementById('bits').value = '8';
    document.getElementById('rule').value = '';
    document.getElementById('num1').value = '';
    document.getElementById('num2').value = '';
    document.getElementById('result').value = '';
    document.getElementById('boundedMode').checked = false;
    clearHistory();
    document.getElementById('table1Display').innerHTML = '';
    document.getElementById('table2Display').innerHTML = '';
    document.getElementById('hasseDisplay').innerHTML = '';
}

// Update table 1
async function updateTable1() {
    const tableType = document.getElementById('table1Type').value;
    await updateTableDisplay('table1Display', tableType);
}

// Update table 2
async function updateTable2() {
    const tableType = document.getElementById('table2Type').value;
    await updateTableDisplay('table2Display', tableType);
}

// Update table display
async function updateTableDisplay(displayId, tableType) {
    const display = document.getElementById(displayId);
    
    try {
        if (tableType === 'hasse') {
            await displayHasseInTable(display);
            return;
        }
        
        const response = await fetch(`${API_URL}/api/table/${tableType}`);
        const data = await response.json();
        
        if (data.success) {
            displayTable(display, data.elements, data.table, tableType);
        } else {
            display.innerHTML = `<div style="color: red;">${data.error}</div>`;
        }
    } catch (error) {
        display.innerHTML = `<div style="color: red;">Failed to load: ${error}</div>`;
    }
}

// Display table
function displayTable(display, elements, tableData, tableType) {
    let html = '<table>';
    
    // Header row
    html += '<tr><th>' + tableType.replace('_', ' ').toUpperCase() + '</th>';
    elements.forEach(elem => {
        html += `<th>${elem}</th>`;
    });
    html += '</tr>';
    
    // Data rows
    elements.forEach(a => {
        html += `<tr><th>${a}</th>`;
        elements.forEach(b => {
            const entry = tableData.find(item => item.a === a && item.b === b);
            const result = entry ? entry.result : '';
            html += `<td>${result}</td>`;
        });
        html += '</tr>';
    });
    
    html += '</table>';
    display.innerHTML = html;
}

// Display Hasse in table display
async function displayHasseInTable(display) {
    try {
        const response = await fetch(`${API_URL}/api/hasse`);
        const data = await response.json();
        
        if (data.success) {
            const hasseData = data.positions;
            if (!hasseData || !hasseData.element_positions) {
                display.innerHTML = '<div style="text-align: center; padding: 20px; color: #999;">No Hasse Diagram<br>Initialize algebra first</div>';
                return;
            }
            
            // Show text representation for table displays
            const positions = hasseData.element_positions;
            const groups = {};
            for (const [elem, pos] of Object.entries(positions)) {
                if (!groups[pos]) groups[pos] = [];
                groups[pos].push(elem);
            }
            
            const sortedPos = Object.keys(groups).sort((a, b) => parseInt(a) - parseInt(b));
            
            let html = '<div style="padding: 20px; font-family: Courier;">';
            sortedPos.forEach(pos => {
                const elems = groups[pos].sort();
                const displayText = elems.length > 1 ? `{${elems.join(',')}}` : elems[0];
                html += `<div style="margin: 10px 0;"><strong>(${pos})</strong> ${displayText}</div>`;
            });
            html += '<div style="margin: 10px 0;">→ (cycle back to a)</div>';
            html += '</div>';
            
            display.innerHTML = html;
        } else {
            display.innerHTML = `<div style="color: red;">${data.error}</div>`;
        }
    } catch (error) {
        display.innerHTML = `<div style="color: red;">Failed to load: ${error}</div>`;
    }
}

// Update Hasse widget
async function updateHasse() {
    try {
        const response = await fetch(`${API_URL}/api/hasse`);
        const data = await response.json();
        
        if (data.success) {
            displayHasseWidget(data.positions);
        }
    } catch (error) {
        console.error('Failed to load Hasse diagram:', error);
    }
}

// Display Hasse diagram in widget
function displayHasseWidget(hasseData) {
    const display = document.getElementById('hasseDisplay');
    
    if (!hasseData || !hasseData.element_positions) {
        display.innerHTML = '<div style="text-align: center; padding: 20px; color: #999; font-size: 12px;">No Hasse Diagram<br><br>Enter +1 rule to display</div>';
        return;
    }
    
    const positions = hasseData.element_positions;
    const plusOneRule = hasseData.plus_one_rule || [];
    
    // Group elements by position
    const groups = {};
    let maxPosition = 0;
    for (const [elem, pos] of Object.entries(positions)) {
        if (!groups[pos]) groups[pos] = [];
        groups[pos].push(elem);
        maxPosition = Math.max(maxPosition, parseInt(pos));
    }
    
    // Sort elements in each group
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
    
    const sortedPos = Object.keys(groups).sort((a, b) => parseInt(a) - parseInt(b));
    
    // SVG dimensions
    const width = 280;
    const height = Math.max(400, sortedPos.length * 80);
    const nodeRadius = 20;
    const horizontalSpacing = 50;
    const centerX = width / 2;
    const marginTop = 20;
    const marginBottom = 20;
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
    
    // Draw edges
    plusOneRule.forEach((outputs, inputIdx) => {
        const inputElem = String.fromCharCode(97 + inputIdx);
        
        if (!nodePositions[inputElem]) return;
        
        const fromPos = nodePositions[inputElem];
        const inputPosition = fromPos.pos;
        
        outputs.forEach(outputElem => {
            if (!nodePositions[outputElem]) return;
            
            const toPos = nodePositions[outputElem];
            const outputPosition = toPos.pos;
            
            if (inputPosition === maxPosition && outputPosition === 0) {
                return;
            }
            
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
    
    // Draw cycle indicator
    if (sortedPos.length > 1) {
        const topElems = groups[sortedPos[sortedPos.length - 1]];
        const bottomElems = groups[sortedPos[0]];
        
        const topElem = topElems[topElems.length - 1];
        const bottomElem = bottomElems[bottomElems.length - 1];
        
        const topPos = nodePositions[topElem];
        const bottomPos = nodePositions[bottomElem];
        
        const curveOffset = 40;
        const topRight = { x: topPos.x + curveOffset, y: topPos.y };
        const bottomRight = { x: bottomPos.x + curveOffset, y: bottomPos.y };
        
        svg += `<line x1="${topPos.x}" y1="${topPos.y}" x2="${topRight.x}" y2="${topRight.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        svg += `<line x1="${topRight.x}" y1="${topRight.y}" x2="${bottomRight.x}" y2="${bottomRight.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        svg += `<line x1="${bottomRight.x}" y1="${bottomRight.y}" x2="${bottomPos.x}" y2="${bottomPos.y}" stroke="white" stroke-width="1" stroke-dasharray="5,5"/>`;
        
        svg += `<polygon points="${bottomPos.x + 8},${bottomPos.y} ${bottomPos.x + 3},${bottomPos.y - 4} ${bottomPos.x + 3},${bottomPos.y + 4}" fill="white"/>`;
    }
    
    // Draw nodes
    for (const [elem, nodePos] of Object.entries(nodePositions)) {
        const { x, y, pos } = nodePos;
        
        svg += `<circle cx="${x}" cy="${y}" r="${nodeRadius}" fill="#6496c8" stroke="white" stroke-width="2"/>`;
        svg += `<text x="${x}" y="${y + 4}" text-anchor="middle" fill="white" font-size="14" font-weight="bold">${elem}</text>`;
        
        const groupElems = groups[pos];
        if (elem === groupElems[0]) {
            svg += `<text x="${x - nodeRadius - 25}" y="${y + 4}" text-anchor="end" fill="lightgray" font-size="10">(${pos})</text>`;
        }
    }
    
    svg += '</svg>';
    
    display.innerHTML = svg;
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
    setTimeout(() => { result.value = ''; result.style.color = ''; }, 2000);
}
