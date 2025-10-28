#include "mainwindow.h"
#include <QSplitter>
#include <QMessageBox>
#include <QFont>
#include <sstream>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), algebra(nullptr), algebraInitialized(false), currentOperation("")
{
    setupUI();
    setWindowTitle("Finite Algebra Calculator");
    resize(1400, 800);
}

MainWindow::~MainWindow()
{
    if (algebra) {
        delete algebra;
    }
}

void MainWindow::setupUI()
{
    // Central widget with horizontal splitter
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // Create main splitter for 3-column layout: Calculator | History | Tables+Hasse
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left side: Calculator section (30%)
    QWidget* calcWidget = new QWidget();
    QVBoxLayout* calcLayout = new QVBoxLayout(calcWidget);
    createCalculatorSection(calcWidget, calcLayout);
    
    // Middle: History section (15%)
    QWidget* historyWidget = new QWidget();
    QVBoxLayout* historyLayout = new QVBoxLayout(historyWidget);
    createHistorySection(historyWidget, historyLayout);
    
    // Right side container: Tables + Hasse Diagram (55%)
    QWidget* rightWidget = new QWidget();
    QHBoxLayout* rightLayout = new QHBoxLayout(rightWidget);
    
    // Tables section (takes most of the right side)
    QWidget* tableWidget = new QWidget();
    QVBoxLayout* tableLayout = new QVBoxLayout(tableWidget);
    createTableSection(tableWidget, tableLayout);
    
    // Hasse Diagram widget (vertical strip on the far right)
    QWidget* hasseDiagramContainer = new QWidget();
    QVBoxLayout* hasseDiagramLayout = new QVBoxLayout(hasseDiagramContainer);
    hasseDiagramLayout->setContentsMargins(0, 0, 0, 0);
    hasseDiagramLayout->setSpacing(5);
    
    // Add label above Hasse diagram
    QLabel* hasseDiagramLabel = new QLabel("Hasse Diagram");
    QFont labelFont;
    labelFont.setPointSize(12);
    labelFont.setBold(true);
    hasseDiagramLabel->setFont(labelFont);
    hasseDiagramLabel->setAlignment(Qt::AlignCenter);
    hasseDiagramLabel->setMaximumHeight(30);
    
    hasseDiagramWidget = new HasseDiagramWidget();
    hasseDiagramWidget->setMinimumWidth(250);
    hasseDiagramWidget->setMaximumWidth(350);
    
    hasseDiagramLayout->addWidget(hasseDiagramLabel);
    hasseDiagramLayout->addWidget(hasseDiagramWidget, 1);  // Give diagram stretch factor
    
    // Add tables and hasse diagram to right layout
    rightLayout->addWidget(tableWidget, 3);  // Tables take 3 parts
    rightLayout->addWidget(hasseDiagramContainer, 1);  // Hasse takes 1 part
    
    mainSplitter->addWidget(calcWidget);
    mainSplitter->addWidget(historyWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 30);  // Calculator 30%
    mainSplitter->setStretchFactor(1, 25);  // History 15%
    mainSplitter->setStretchFactor(2, 45);  // Tables+Hasse 55%
    
    mainLayout->addWidget(mainSplitter);
}

void MainWindow::createCalculatorSection(QWidget* parent, QVBoxLayout* layout)
{
    // Title
    QLabel* title = new QLabel("Calculator");
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    
    // Bounded mode checkbox
    boundedModeCheckBox = new QCheckBox("Enable Bounded Mode");
    boundedModeCheckBox->setToolTip(QString("When enabled, results are clamped to min/max values.\n") +
                                     "Max = bits repetitions of largest element (e.g., gggggggg)\n" +
                                     "Min = negative of max (e.g., -gggggggg)");
    boundedModeCheckBox->setMinimumHeight(30);
    QFont checkboxFont;
    checkboxFont.setPointSize(11);
    boundedModeCheckBox->setFont(checkboxFont);
    layout->addWidget(boundedModeCheckBox);
    layout->addSpacing(10);
    
    // Connect checkbox signal
    connect(boundedModeCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onBoundedModeChanged);
    
    // Number of bits input
    QLabel* bitsLabel = new QLabel("Number of elements:");
    bitsInput = new QLineEdit();
    bitsInput->setPlaceholderText("e.g., 8 for Z8, 9 for Z9");
    bitsInput->setMinimumHeight(35);
    layout->addWidget(bitsLabel);
    layout->addWidget(bitsInput);
    layout->addSpacing(10);
    
    // Plus One Rule input
    QLabel* ruleLabel = new QLabel("+1 Rule:");
    plusOneRuleInput = new QLineEdit();
    plusOneRuleInput->setPlaceholderText("e.g., bhgecea{d,f} or b-g-f-e-c-a-h-d");
    plusOneRuleInput->setMinimumHeight(35);
    connect(plusOneRuleInput, &QLineEdit::returnPressed, this, &MainWindow::onPlusOneRuleChanged);
    
    layout->addWidget(ruleLabel);
    layout->addWidget(plusOneRuleInput);
    layout->addSpacing(20);
    
    // Input fields
    QLabel* input1Label = new QLabel("Number 1:");
    input1 = new QLineEdit();
    input1->setPlaceholderText("e.g., ggc or -bah");
    input1->setMinimumHeight(35);
    
    QLabel* input2Label = new QLabel("Number 2:");
    input2 = new QLineEdit();
    input2->setPlaceholderText("e.g., h or -cd");
    input2->setMinimumHeight(35);
    
    layout->addWidget(input1Label);
    layout->addWidget(input1);
    layout->addWidget(input2Label);
    layout->addWidget(input2);
    layout->addSpacing(20);
    
    // Result display
    QLabel* resultLabel = new QLabel("Result:");
    resultDisplay = new QLineEdit();
    resultDisplay->setReadOnly(true);
    resultDisplay->setMinimumHeight(35);
    resultDisplay->setStyleSheet("QLineEdit { background-color: #f0f0f0; font-weight: bold; }");
    
    layout->addWidget(resultLabel);
    layout->addWidget(resultDisplay);
    layout->addSpacing(30);
    
    // Calculator buttons
    QGroupBox* buttonGroup = new QGroupBox("Operations");
    QGridLayout* buttonLayout = new QGridLayout(buttonGroup);
    
    // Create buttons
    btnAdd = new QPushButton("+");
    btnSubtract = new QPushButton("-");
    btnMultiply = new QPushButton("×");
    btnDivide = new QPushButton("÷");
    btnPower = new QPushButton("^");
    btnMod = new QPushButton("mod");
    btnGcd = new QPushButton("NOD");
    btnLcm = new QPushButton("NOC");
    btnEquals = new QPushButton("=");
    btnClear = new QPushButton("Clear");
    
    // Set button sizes
    QList<QPushButton*> buttons = {btnAdd, btnSubtract, btnMultiply, btnDivide, 
                                    btnPower, btnMod, btnGcd, btnLcm, btnEquals, btnClear};
    for (auto btn : buttons) {
        btn->setMinimumHeight(50);
        btn->setMinimumWidth(80);
        QFont btnFont;
        btnFont.setPointSize(14);
        btn->setFont(btnFont);
    }
    
    // Style equals button
    btnEquals->setStyleSheet("QPushButton { background-color: #747190ff; color: white; font-weight: bold; }");
    btnClear->setStyleSheet("QPushButton { background-color: #68658eff; color: white; }");
    
    // Layout buttons in grid (3 columns)
    buttonLayout->addWidget(btnAdd, 0, 0);
    buttonLayout->addWidget(btnSubtract, 0, 1);
    buttonLayout->addWidget(btnMultiply, 0, 2);
    buttonLayout->addWidget(btnDivide, 1, 0);
    buttonLayout->addWidget(btnPower, 1, 1);
    buttonLayout->addWidget(btnMod, 1, 2);
    buttonLayout->addWidget(btnGcd, 2, 0);
    buttonLayout->addWidget(btnLcm, 2, 1);
    buttonLayout->addWidget(btnEquals, 2, 2);
    buttonLayout->addWidget(btnClear, 3, 0, 1, 3);
    
    layout->addWidget(buttonGroup);
    layout->addStretch();
    
    // Connect buttons to slots
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(btnSubtract, &QPushButton::clicked, this, &MainWindow::onSubtractClicked);
    connect(btnMultiply, &QPushButton::clicked, this, &MainWindow::onMultiplyClicked);
    connect(btnDivide, &QPushButton::clicked, this, &MainWindow::onDivideClicked);
    connect(btnPower, &QPushButton::clicked, this, &MainWindow::onPowerClicked);
    connect(btnMod, &QPushButton::clicked, this, &MainWindow::onModClicked);
    connect(btnGcd, &QPushButton::clicked, this, &MainWindow::onGcdClicked);
    connect(btnLcm, &QPushButton::clicked, this, &MainWindow::onLcmClicked);
    connect(btnEquals, &QPushButton::clicked, this, &MainWindow::onEqualsClicked);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
}

void MainWindow::createHistorySection(QWidget* parent, QVBoxLayout* layout)
{
    // Title
    QLabel* title = new QLabel("History");
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    
    // History list widget
    historyListWidget = new QListWidget();
    historyListWidget->setFont(QFont("Courier", 9));
    historyListWidget->setToolTip("Click an entry to reload the values");
    layout->addWidget(historyListWidget);
    
    // Clear History button
    btnClearHistory = new QPushButton("Clear History");
    btnClearHistory->setMinimumHeight(35);
    btnClearHistory->setStyleSheet("QPushButton { background-color: #68658eff; color: white; }");
    layout->addWidget(btnClearHistory);
    
    // Connect signals
    connect(historyListWidget, &QListWidget::itemClicked, this, &MainWindow::onHistoryItemClicked);
    connect(btnClearHistory, &QPushButton::clicked, this, &MainWindow::onClearHistoryClicked);
}

void MainWindow::createTableSection(QWidget* parent, QVBoxLayout* layout)
{
    // Title
    QLabel* title = new QLabel("Tables & Diagrams");
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    
    // Top table
    QLabel* table1Label = new QLabel("Display 1:");
    table1Selector = new QComboBox();
    table1Selector->addItem("Hasse Diagram");
    table1Selector->addItem("Addition Table");
    table1Selector->addItem("Multiplication Table");
    table1Selector->addItem("Subtraction Table");
    table1Selector->addItem("Division Table");
    table1Selector->addItem("Addition Carry Table");
    table1Selector->addItem("Multiplication Carry Table");
    table1Selector->setCurrentIndex(1); // Default to Addition Table
    table1Selector->setMinimumHeight(30);
    
    table1Display = new QTextEdit();
    table1Display->setReadOnly(true);
    table1Display->setFont(QFont("Courier", 10));
    
    layout->addWidget(table1Label);
    layout->addWidget(table1Selector);
    layout->addWidget(table1Display);
    
    layout->addSpacing(20);
    
    // Bottom table
    QLabel* table2Label = new QLabel("Display 2:");
    table2Selector = new QComboBox();
    table2Selector->addItem("Hasse Diagram");
    table2Selector->addItem("Addition Table");
    table2Selector->addItem("Multiplication Table");
    table2Selector->addItem("Subtraction Table");
    table2Selector->addItem("Division Table");
    table2Selector->addItem("Addition Carry Table");
    table2Selector->addItem("Multiplication Carry Table");
    table2Selector->setCurrentIndex(5); // Default to Addition Carry Table
    table2Selector->setMinimumHeight(30);
    
    table2Display = new QTextEdit();
    table2Display->setReadOnly(true);
    table2Display->setFont(QFont("Courier", 10));
    
    layout->addWidget(table2Label);
    layout->addWidget(table2Selector);
    layout->addWidget(table2Display);
    
    // Connect combo boxes
    connect(table1Selector, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onTable1TypeChanged);
    connect(table2Selector, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onTable2TypeChanged);
}

void MainWindow::onPlusOneRuleChanged()
{
    QString bitsText = bitsInput->text().trimmed();
    QString rule = plusOneRuleInput->text().trimmed();
    
    if (bitsText.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter the number of elements.");
        return;
    }
    
    if (rule.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a +1 rule.");
        return;
    }
    
    // Parse bits
    bool ok;
    int bits = bitsText.toInt(&ok);
    
    if (!ok || bits < 2 || bits > 26) {
        QMessageBox::warning(this, "Invalid Input", "Number of elements must be between 2 and 26.");
        return;
    }
    
    // Initialize algebra with user-specified bits
    if (algebra) {
        delete algebra;
    }
    algebra = new Algebra(bits);
    algebra->setPlusOneRule(rule.toStdString());
    algebra->setBoundedMode(boundedModeCheckBox->isChecked());
    algebraInitialized = true;
    
    // Update Hasse diagram widget
    hasseDiagramWidget->updateDiagram(algebra->getElementPosition(), algebra->getPlusOneRule());
    
    // Update table displays
    onTable1TypeChanged(table1Selector->currentIndex());
    onTable2TypeChanged(table2Selector->currentIndex());
    
    QMessageBox::information(this, "Success", 
        QString("Algebra Z%1 initialized with rule: %2").arg(bits).arg(rule));
}

void MainWindow::performOperation(const std::string& operation)
{
    if (!algebraInitialized) {
        QMessageBox::warning(this, "Not Initialized", 
            "Please enter a +1 rule first and press Enter.");
        return;
    }
    
    if (!validateInputs()) {
        return;
    }
    
    std::string num1 = input1->text().toStdString();
    std::string num2 = input2->text().toStdString();
    std::string result;
    
    try {
        if (operation == "add") {
            result = algebra->addArithmetic(num1, num2);
        } else if (operation == "subtract") {
            result = algebra->subtractArithmetic(num1, num2);
        } else if (operation == "multiply") {
            result = algebra->multiplyArithmetic(num1, num2);
        } else if (operation == "divide") {
            std::string remainder;
            result = algebra->divideArithmetic(num1, num2, remainder);
            // Check for division by zero
            if (result == "∅") {
                resultDisplay->setText("∅ (remainder: ∅)");
                addToHistory(num1, num2, operation, "∅ (remainder: ∅)");
                return;
            }
            // Format quotient and remainder separately
            std::string formattedQuotient = algebra->formatMultiDigitResult(result);
            std::string formattedRemainder = algebra->formatMultiDigitResult(remainder);
            result = formattedQuotient + " (remainder: " + formattedRemainder + ")";
            // Display directly without calling formatMultiDigitResult again
            resultDisplay->setText(QString::fromStdString(result));
            addToHistory(num1, num2, operation, result);
            return;
        } else if (operation == "power") {
            result = algebra->powerArithmetic(num1, num2);
        } else if (operation == "mod") {
            result = algebra->modArithmetic(num1, num2);
        } else if (operation == "gcd") {
            result = algebra->gcdArithmetic(num1, num2);
        } else if (operation == "lcm") {
            result = algebra->lcmArithmetic(num1, num2);
        }
        
        // Format result with braces notation
        std::string formattedResult = algebra->formatMultiDigitResult(result);
        resultDisplay->setText(QString::fromStdString(formattedResult));
        
        // Add to history
        addToHistory(num1, num2, operation, formattedResult);
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", 
            QString("Operation failed: ") + e.what());
    }
}

bool MainWindow::validateInputs()
{
    if (input1->text().trimmed().isEmpty() || input2->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", 
            "Please enter both numbers.");
        return false;
    }
    
    // Validate characters
    std::string num1 = input1->text().toStdString();
    std::string num2 = input2->text().toStdString();
    
    // Get the last element dynamically from the algebra
    if (!algebra) {
        QMessageBox::warning(this, "Error", 
            "Algebra not initialized. Please enter number of elements and +1 rule first.");
        return false;
    }
    
    char lastElem = 'a' + (algebra->getElements().size() - 1);
    
    auto validateNumber = [lastElem](const std::string& num) {
        for (size_t i = 0; i < num.length(); i++) {
            if (i == 0 && num[i] == '-') continue; // Allow minus sign at start
            if (num[i] < 'a' || num[i] > lastElem) {
                return false;
            }
        }
        return true;
    };
    
    if (!validateNumber(num1) || !validateNumber(num2)) {
        QMessageBox::warning(this, "Invalid Input", 
            QString("Numbers must contain only elements from 'a' to '%1' (with optional '-' at start).").arg(lastElem));
        return false;
    }
    
    return true;
}

void MainWindow::onAddClicked()
{
    currentOperation = "add";
    performOperation("add");
}

void MainWindow::onSubtractClicked()
{
    currentOperation = "subtract";
    performOperation("subtract");
}

void MainWindow::onMultiplyClicked()
{
    currentOperation = "multiply";
    performOperation("multiply");
}

void MainWindow::onDivideClicked()
{
    currentOperation = "divide";
    performOperation("divide");
}

void MainWindow::onPowerClicked()
{
    currentOperation = "power";
    performOperation("power");
}

void MainWindow::onModClicked()
{
    currentOperation = "mod";
    performOperation("mod");
}

void MainWindow::onGcdClicked()
{
    currentOperation = "gcd";
    performOperation("gcd");
}

void MainWindow::onLcmClicked()
{
    currentOperation = "lcm";
    performOperation("lcm");
}

void MainWindow::onEqualsClicked()
{
    if (!currentOperation.empty()) {
        performOperation(currentOperation);
    } else {
        QMessageBox::information(this, "No Operation", 
            "Please select an operation first (+, -, ×, ÷, etc.)");
    }
}

void MainWindow::onClearClicked()
{
    // Clear all input fields
    bitsInput->clear();
    plusOneRuleInput->clear();
    input1->clear();
    input2->clear();
    resultDisplay->clear();
    currentOperation = "";
    
    // Uncheck bounded mode
    boundedModeCheckBox->setChecked(false);
    
    // Delete algebra and reset initialization flag
    if (algebra) {
        delete algebra;
        algebra = nullptr;
    }
    algebraInitialized = false;
    
    // Clear table displays
    table1Display->clear();
    table2Display->clear();
    table1Display->setPlainText("Please initialize algebra by entering number of elements and +1 rule.");
    table2Display->setPlainText("Please initialize algebra by entering number of elements and +1 rule.");
    
    // Clear Hasse diagram
    hasseDiagramWidget->clearDiagram();
}

void MainWindow::onTable1TypeChanged(int index)
{
    if (!algebraInitialized) {
        table1Display->setPlainText("Please initialize algebra by entering a +1 rule above.");
        return;
    }
    updateTableDisplay(table1Display, index);
}

void MainWindow::onTable2TypeChanged(int index)
{
    if (!algebraInitialized) {
        table2Display->setPlainText("Please initialize algebra by entering a +1 rule above.");
        return;
    }
    updateTableDisplay(table2Display, index);
}

void MainWindow::updateTableDisplay(QTextEdit* display, int tableType)
{
    // Capture output to string
    std::ostringstream oss;
    std::streambuf* oldCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(oss.rdbuf());
    
    switch (tableType) {
        case 0: // Hasse Diagram
            displayHasseDiagram(display);
            std::cout.rdbuf(oldCoutBuf);
            return;
        case 1: // Addition Table
            algebra->printAdditionTable();
            break;
        case 2: // Multiplication Table
            algebra->printMultiplicationTable();
            break;
        case 3: // Subtraction Table
            algebra->printSubtractionTable();
            break;
        case 4: // Division Table
            algebra->printDivisionTable();
            break;
        case 5: // Addition Carry Table
            algebra->printAdditionCarryTable();
            break;
        case 6: // Multiplication Carry Table
            algebra->printMultiplicationCarryTable();
            break;
        default:
            std::cout.rdbuf(oldCoutBuf);
            display->setPlainText("Unknown table type.");
            return;
    }
    
    std::cout.rdbuf(oldCoutBuf);
    display->setPlainText(QString::fromStdString(oss.str()));
}

void MainWindow::displayHasseDiagram(QTextEdit* display)
{
    std::ostringstream oss;
    std::streambuf* oldCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(oss.rdbuf());
    
    algebra->printHasseDiagram();
    
    std::cout.rdbuf(oldCoutBuf);
    display->setPlainText(QString::fromStdString(oss.str()));
}

void MainWindow::onBoundedModeChanged(int state)
{
    if (algebra) {
        bool enabled = (state == Qt::Checked);
        algebra->setBoundedMode(enabled);
        
        if (enabled && algebraInitialized) {
            // Get the actual max/min values from the algebra
            std::string maxVal = algebra->formatMultiDigitResult(algebra->getMaxValue());
            std::string minVal = algebra->formatMultiDigitResult(algebra->getMinValue());
            
            QMessageBox::information(this, "Bounded Mode Enabled",
                QString("Arithmetic results will be clamped to:\n") +
                QString("Maximum: %1\n").arg(QString::fromStdString(maxVal)) +
                QString("Minimum: %2").arg(QString::fromStdString(minVal)));
        }
    }
}

void MainWindow::addToHistory(const std::string& num1, const std::string& num2, 
                               const std::string& operation, const std::string& result)
{
    // Create history entry
    HistoryEntry entry;
    entry.num1 = num1;
    entry.num2 = num2;
    entry.operation = operation;
    entry.result = result;
    
    // Add to front of deque
    calculationHistory.push_front(entry);
    
    // Limit to MAX_HISTORY_ENTRIES
    if (calculationHistory.size() > MAX_HISTORY_ENTRIES) {
        calculationHistory.pop_back();
    }
    
    // Update display
    updateHistoryDisplay();
}

void MainWindow::updateHistoryDisplay()
{
    historyListWidget->clear();
    
    // Map operation names to symbols
    auto getOpSymbol = [](const std::string& op) -> QString {
        if (op == "add") return "+";
        if (op == "subtract") return "-";
        if (op == "multiply") return "×";
        if (op == "divide") return "÷";
        if (op == "power") return "^";
        if (op == "mod") return "mod";
        if (op == "gcd") return "NOD";
        if (op == "lcm") return "NOC";
        return QString::fromStdString(op);
    };
    
    int index = 1;
    for (const auto& entry : calculationHistory) {
        QString display = QString("[%1] %2 %3 %4 = %5")
            .arg(index)
            .arg(QString::fromStdString(entry.num1))
            .arg(getOpSymbol(entry.operation))
            .arg(QString::fromStdString(entry.num2))
            .arg(QString::fromStdString(entry.result));
        
        historyListWidget->addItem(display);
        index++;
    }
}

void MainWindow::onHistoryItemClicked(QListWidgetItem* item)
{
    // Get the index of clicked item
    int index = historyListWidget->row(item);
    
    if (index >= 0 && index < static_cast<int>(calculationHistory.size())) {
        const HistoryEntry& entry = calculationHistory[index];
        
        // Load values back into input fields
        input1->setText(QString::fromStdString(entry.num1));
        input2->setText(QString::fromStdString(entry.num2));
        
        // Set current operation
        currentOperation = entry.operation;
        
        // Optionally show the result
        resultDisplay->setText(QString::fromStdString(entry.result));
    }
}

void MainWindow::onClearHistoryClicked()
{
    calculationHistory.clear();
    updateHistoryDisplay();
}
