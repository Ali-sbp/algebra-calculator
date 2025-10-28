#include "hassediagramwidget.h"
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <cmath>
#include <algorithm>

HasseDiagramWidget::HasseDiagramWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(200);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

HasseDiagramWidget::~HasseDiagramWidget()
{
}

void HasseDiagramWidget::updateDiagram(const std::map<char, int>& elementPositions, 
                                       const std::vector<std::vector<char>>& rule)
{
    elementPosition = elementPositions;
    plusOneRule = rule;
    
    // Build a map of element -> which input index produces it
    std::map<char, int> elementSourceIndex;
    for (size_t inputIdx = 0; inputIdx < plusOneRule.size(); inputIdx++) {
        for (char outputElem : plusOneRule[inputIdx]) {
            // Use the first (or smallest) input index that produces this element
            if (elementSourceIndex.find(outputElem) == elementSourceIndex.end()) {
                elementSourceIndex[outputElem] = inputIdx;
            }
        }
    }
    
    // Group elements by their position
    positionGroups.clear();
    for (const auto& [elem, pos] : elementPosition) {
        positionGroups[pos].push_back(elem);
    }
    
    // Sort elements in each group by their source index (which input produces them)
    for (auto& [pos, elems] : positionGroups) {
        std::sort(elems.begin(), elems.end(), [&elementSourceIndex](char a, char b) {
            int sourceA = (elementSourceIndex.find(a) != elementSourceIndex.end()) ? elementSourceIndex[a] : 999;
            int sourceB = (elementSourceIndex.find(b) != elementSourceIndex.end()) ? elementSourceIndex[b] : 999;
            return sourceA < sourceB;
        });
    }
    
    // Create nodes
    nodes.clear();
    for (const auto& [pos, elems] : positionGroups) {
        Node node;
        node.elements = elems;
        node.position = pos;
        nodes.push_back(node);
    }
    
    // Sort nodes by position
    std::sort(nodes.begin(), nodes.end(), 
              [](const Node& a, const Node& b) { return a.position < b.position; });
    
    calculateLayout();
    update();
}

void HasseDiagramWidget::clearDiagram()
{
    nodes.clear();
    positionGroups.clear();
    elementPosition.clear();
    update();
}

void HasseDiagramWidget::calculateLayout()
{
    if (nodes.empty()) return;
    
    int availableHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    int centerX = width() / 2;
    
    // Calculate vertical positions (bottom to top)
    int numLevels = nodes.size();
    double verticalStep = availableHeight / (double)(numLevels + 1);
    
    for (size_t i = 0; i < nodes.size(); i++) {
        Node& node = nodes[i];
        
        // Position from bottom to top (position 0 at bottom)
        double y = height() - MARGIN_BOTTOM - (i + 1) * verticalStep;
        
        // Store the Y position - X will be calculated during drawing for multi-elements
        node.drawPosition = QPointF(centerX, y);
    }
}

void HasseDiagramWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    if (nodes.empty()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "No Hasse Diagram\nEnter +1 rule to display");
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw edges first (so they appear behind nodes)
    // Build a map from element to its screen position
    std::map<char, QPointF> elementScreenPos;
    for (const Node& node : nodes) {
        if (node.elements.size() > 1) {
            // Multiple elements at same level - calculate each position
            for (size_t j = 0; j < node.elements.size(); j++) {
                double offsetX = (j - (node.elements.size() - 1) / 2.0) * HORIZONTAL_SPACING;
                elementScreenPos[node.elements[j]] = QPointF(node.drawPosition.x() + offsetX, node.drawPosition.y());
            }
        } else {
            elementScreenPos[node.elements[0]] = node.drawPosition;
        }
    }
    
    // Draw edges based on actual +1 rule equations
    QPen edgePen(Qt::white, 2);
    painter.setPen(edgePen);
    
    // Find the maximum position (top level)
    int maxPosition = 0;
    for (const auto& [elem, pos] : elementPosition) {
        maxPosition = std::max(maxPosition, pos);
    }
    
    for (size_t inputIdx = 0; inputIdx < plusOneRule.size(); inputIdx++) {
        if (plusOneRule[inputIdx].empty()) continue;
        
        char inputElem = 'a' + inputIdx;
        
        // Check if input element exists in diagram
        if (elementScreenPos.find(inputElem) == elementScreenPos.end()) continue;
        
        // Get input element's position
        int inputPos = elementPosition[inputElem];
        
        QPointF fromPos = elementScreenPos[inputElem];
        
        // Draw edge to each output element in the rule
        for (char outputElem : plusOneRule[inputIdx]) {
            if (elementScreenPos.find(outputElem) != elementScreenPos.end()) {
                int outputPos = elementPosition[outputElem];
                
                // Skip edges from top level back to bottom (cycle edges)
                // These wrap around and are shown by the cycle arrow
                if (inputPos == maxPosition && outputPos == 0) {
                    continue;
                }
                
                QPointF toPos = elementScreenPos[outputElem];
                drawEdge(painter, fromPos, toPos, false);
            }
        }
    }
    
    // Draw cycle indicator (arrow from top back to bottom)
    if (nodes.size() > 1) {
        QPen dashedPen(Qt::white, 1, Qt::DashLine);
        painter.setPen(dashedPen);
        
        const Node& top = nodes.back();
        const Node& bottom = nodes.front();
        
        // Calculate actual top and bottom positions considering multi-elements
        QPointF topPos = top.drawPosition;
        QPointF bottomPos = bottom.drawPosition;
        
        // If top has multiple elements, connect them horizontally and use rightmost for cycle
        if (top.elements.size() > 1) {
            // Draw horizontal line connecting all top elements
            double leftOffset = -((top.elements.size() - 1) / 2.0) * HORIZONTAL_SPACING;
            double rightOffset = ((top.elements.size() - 1) / 2.0) * HORIZONTAL_SPACING;
            
            QPointF leftPos(top.drawPosition.x() + leftOffset, top.drawPosition.y());
            QPointF rightPos(top.drawPosition.x() + rightOffset, top.drawPosition.y());
            
            painter.drawLine(leftPos, rightPos);
            
            // Use rightmost for the cycle arrow
            topPos = rightPos;
        }
        
        // If bottom has multiple elements, use rightmost
        if (bottom.elements.size() > 1) {
            double offsetX = ((bottom.elements.size() - 1) / 2.0) * HORIZONTAL_SPACING;
            bottomPos = QPointF(bottom.drawPosition.x() + offsetX, bottom.drawPosition.y());
        }
        
        // Draw curved arrow on the right side
        int curveOffset = 40;
        QPointF topRight(topPos.x() + curveOffset, topPos.y());
        QPointF bottomRight(bottomPos.x() + curveOffset, bottomPos.y());
        
        painter.drawLine(topPos, topRight);
        painter.drawLine(topRight, bottomRight);
        painter.drawLine(bottomRight, bottomPos);
        
        // Draw arrow head
        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(QBrush(Qt::white));
        QPointF arrowHead = bottomPos + QPointF(10, 0);
        QPolygonF arrow;
        arrow << arrowHead << (arrowHead + QPointF(-5, -5)) << (arrowHead + QPointF(-5, 5));
        painter.drawPolygon(arrow);
    }
    
    // Draw nodes on top
    for (const Node& node : nodes) {
        if (node.elements.size() > 1) {
            // Draw multiple nodes at the same vertical level
            for (size_t j = 0; j < node.elements.size(); j++) {
                double offsetX = (j - (node.elements.size() - 1) / 2.0) * HORIZONTAL_SPACING;
                QPointF nodePos(node.drawPosition.x() + offsetX, node.drawPosition.y());
                
                // Create a single-element node for this position
                Node singleNode;
                singleNode.elements.push_back(node.elements[j]);
                singleNode.position = node.position;
                singleNode.drawPosition = nodePos;
                
                // Only draw position number for the leftmost node
                bool showPosition = (j == 0);
                drawNode(painter, singleNode, showPosition);
            }
        } else {
            drawNode(painter, node, true);
        }
    }
}

void HasseDiagramWidget::drawNode(QPainter& painter, const Node& node, bool showPosition)
{
    // Draw circle
    QPen nodePen(Qt::white, 2);
    painter.setPen(nodePen);
    
    // All nodes same color
    painter.setBrush(QBrush(QColor(100, 150, 200))); // Blue shade
    
    painter.drawEllipse(node.drawPosition, NODE_RADIUS, NODE_RADIUS);
    
    // Draw label inside circle
    QString label = getNodeLabel(node);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    
    QRectF textRect(node.drawPosition.x() - NODE_RADIUS, 
                    node.drawPosition.y() - NODE_RADIUS,
                    NODE_RADIUS * 2, 
                    NODE_RADIUS * 2);
    painter.drawText(textRect, Qt::AlignCenter, label);
    
    // Draw position number to the left of the node (only if showPosition is true)
    if (showPosition) {
        font.setPointSize(8);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(Qt::lightGray);
        
        // Position text to the left of the node, vertically centered
        QRectF posRect(node.drawPosition.x() - NODE_RADIUS - 35, 
                       node.drawPosition.y() - 10,
                       30, 20);
        painter.drawText(posRect, Qt::AlignRight | Qt::AlignVCenter, QString("(%1)").arg(node.position));
    }
}

void HasseDiagramWidget::drawEdge(QPainter& painter, const QPointF& from, const QPointF& to, bool isDiagonal)
{
    // Calculate start and end points on the edge of circles
    double dx = to.x() - from.x();
    double dy = to.y() - from.y();
    double angle = std::atan2(dy, dx);
    
    QPointF start(from.x() + NODE_RADIUS * std::cos(angle),
                  from.y() + NODE_RADIUS * std::sin(angle));
    QPointF end(to.x() - NODE_RADIUS * std::cos(angle),
                to.y() - NODE_RADIUS * std::sin(angle));
    
    painter.drawLine(start, end);
}

QString HasseDiagramWidget::getNodeLabel(const Node& node)
{
    if (node.elements.size() == 1) {
        return QString(node.elements[0]);
    } else {
        QString label = "{";
        for (size_t i = 0; i < node.elements.size(); i++) {
            label += node.elements[i];
            if (i < node.elements.size() - 1) {
                label += ",";
            }
        }
        label += "}";
        return label;
    }
}
