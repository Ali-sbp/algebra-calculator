#ifndef HASSEDIAGRAMWIDGET_H
#define HASSEDIAGRAMWIDGET_H

#include <QWidget>
#include <QPainter>
#include <vector>
#include <map>
#include <string>

class HasseDiagramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HasseDiagramWidget(QWidget *parent = nullptr);
    ~HasseDiagramWidget();
    
    // Update the diagram with new algebra structure
    void updateDiagram(const std::map<char, int>& elementPositions, 
                      const std::vector<std::vector<char>>& plusOneRule);
    
    void clearDiagram();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Node {
        std::vector<char> elements;  // Elements at this position (e.g., {d, f})
        int position;                 // Position in the cycle
        QPointF drawPosition;         // Where to draw on screen
    };
    
    void calculateLayout();
    void drawNode(QPainter& painter, const Node& node, bool showPosition);
    void drawEdge(QPainter& painter, const QPointF& from, const QPointF& to, bool isDiagonal);
    QString getNodeLabel(const Node& node);
    
    // Data
    std::map<char, int> elementPosition;
    std::vector<std::vector<char>> plusOneRule;
    std::vector<Node> nodes;
    std::map<int, std::vector<char>> positionGroups;  // Group elements by position
    
    // Layout parameters
    static constexpr int NODE_RADIUS = 20;
    static constexpr int VERTICAL_SPACING = 80;
    static constexpr int HORIZONTAL_SPACING = 60;
    static constexpr int MARGIN_TOP = 40;
    static constexpr int MARGIN_BOTTOM = 40;
    static constexpr int MARGIN_HORIZONTAL = 30;
};

#endif // HASSEDIAGRAMWIDGET_H
