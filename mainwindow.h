#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QListWidget>
#include <deque>
#include "algebra.h"
#include "hassediagramwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    struct HistoryEntry {
        std::string num1;
        std::string num2;
        std::string operation;
        std::string result;
    };

private slots:
    // Operation button slots
    void onPlusOneRuleChanged();
    void onBoundedModeChanged(int state);
    void onAddClicked();
    void onSubtractClicked();
    void onMultiplyClicked();
    void onDivideClicked();
    void onPowerClicked();
    void onModClicked();
    void onGcdClicked();
    void onLcmClicked();
    void onEqualsClicked();
    
    // Table display slots
    void onTable1TypeChanged(int index);
    void onTable2TypeChanged(int index);
    
    // History slots
    void onHistoryItemClicked(QListWidgetItem* item);
    void onClearHistoryClicked();
    
    // Utility slots
    void onClearClicked();

private:
    // Helper methods
    void setupUI();
    void createCalculatorSection(QWidget* parent, QVBoxLayout* layout);
    void createHistorySection(QWidget* parent, QVBoxLayout* layout);
    void createTableSection(QWidget* parent, QVBoxLayout* layout);
    void updateTableDisplay(QTextEdit* display, int tableType);
    void displayHasseDiagram(QTextEdit* display);
    void performOperation(const std::string& operation);
    bool validateInputs();
    std::string formatTableAsHtml(const std::string& plainText);
    void addToHistory(const std::string& num1, const std::string& num2, 
                      const std::string& operation, const std::string& result);
    void updateHistoryDisplay();
    
    // Algebra engine
    Algebra* algebra;
    bool algebraInitialized;
    
    // Current operation (for equals button)
    std::string currentOperation;
    
    // History
    std::deque<HistoryEntry> calculationHistory;
    const int MAX_HISTORY_ENTRIES = 20;
    
    // UI Components - Calculator Section (30% width)
    QCheckBox* boundedModeCheckBox;
    QLineEdit* bitsInput;
    QLineEdit* plusOneRuleInput;
    QLineEdit* input1;
    QLineEdit* input2;
    QLineEdit* resultDisplay;
    QPushButton* btnAdd;
    QPushButton* btnSubtract;
    QPushButton* btnMultiply;
    QPushButton* btnDivide;
    QPushButton* btnPower;
    QPushButton* btnMod;
    QPushButton* btnGcd;
    QPushButton* btnLcm;
    QPushButton* btnEquals;
    QPushButton* btnClear;
    
    // UI Components - History Section (15% width)
    QListWidget* historyListWidget;
    QPushButton* btnClearHistory;
    
    // UI Components - Table Section (55% width)
    QComboBox* table1Selector;
    QComboBox* table2Selector;
    QTextEdit* table1Display;
    QTextEdit* table2Display;
    
    // UI Components - Hasse Diagram (right side)
    HasseDiagramWidget* hasseDiagramWidget;
};

#endif // MAINWINDOW_H
