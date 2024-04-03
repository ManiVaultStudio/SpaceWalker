#include "GradientGraph.h"

#include "Timer.h"

#include <QVBoxLayout>

#include <iostream>
#include <algorithm>

LineSeries::LineSeries(QObject* parent, dint dim) :
    QLineSeries(parent),
    _penWidth(2),
    _dim(dim)
{
    connect(this, &QXYSeries::hovered, this, &LineSeries::onHover);
    connect(this, &QXYSeries::clicked, this, &LineSeries::onClicked);
}

void LineSeries::onHover(const QPointF& point, bool state)
{
    QPen p = pen();
    p.setWidth(state ? _penWidth * 2 : _penWidth);
    setPen(p);
}

void LineSeries::onClicked(const QPointF& point)
{
    emit lineClicked(_dim);
}

// Replace series values with new values
QColor grey = QColor(128, 128, 128, 25);
QColor selected = QColor(255, 0, 0, 255);
QColor primary = QColor(255, 128, 0, 255);
QColor secondary = QColor(255, 128, 0, 128);

GradientGraph::GradientGraph() :
    _numDimensions(0),
    _chart(new QChart()),
    _chartView(nullptr),
    _xAxis(nullptr),
    _yAxis(nullptr),
    _topDimension1(-1),
    _topDimension2(-1),
    _selectedDimension(-1)
{
    _chart->legend()->hide();

    _xAxis = new QValueAxis();
    _xAxis->setRange(0, 1);
    _xAxis->setTickCount(4);
    _xAxis->setLabelFormat("%d");
    _chart->addAxis(_xAxis, Qt::AlignBottom);

    _yAxis = new QValueAxis();
    _yAxis->setRange(0, 1);
    _yAxis->setTickCount(4);
    _yAxis->setLabelFormat("%d");
    _chart->addAxis(_yAxis, Qt::AlignLeft);

    _chartView = new QChartView(_chart);
    _chartView->setRenderHint(QPainter::Antialiasing);

    setContentsMargins(0, 0, 0, 0);
    _chart->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_chartView);
    setLayout(layout);
}

void GradientGraph::reset()
{
    _numDimensions = 0;

    for (int i = 0; i < _seriesArray.size(); i++)
    {
        delete _seriesArray[i];
    }
    _seriesArray.clear();
    _chart->removeAllSeries();

    _xAxis->setRange(0, 1);
    _yAxis->setRange(0, 1);

    _topDimension1 = -1;
    _topDimension2 = -1;
    _selectedDimension = -1;
}

void GradientGraph::setNumDimensions(dint numDimensions)
{
    _numDimensions = numDimensions;

    _seriesArray.resize(numDimensions);

    // Add numDimensions lineseries to the chart
    _chart->removeAllSeries();
    for (dint d = 0; d < _numDimensions; d++)
    {
        // Create series
        _seriesArray[d] = new LineSeries(nullptr, d);
        //_seriesArray[d]->setUseOpenGL(); // Speed up rendering

        connect(_seriesArray[d], &LineSeries::lineClicked, this, &GradientGraph::onLineClicked);

        // Add series to chart
        _chart->addSeries(_seriesArray[d]);

        // Attach chart axes to each series
        _seriesArray[d]->attachAxis(_xAxis);
        _seriesArray[d]->attachAxis(_yAxis);
    }
}

void GradientGraph::setValues(const std::vector<std::vector<float>>& values)
{
    // Compute the maximum value in the data and store values in a QList<QPointF>
    float maxValue = 0;
    std::vector<QList<QPointF>> pointLists(values.size());
    for (int d = 0; d < values.size(); d++)
    {
        float maxVal = *std::max_element(values[d].begin(), values[d].end());
        maxValue = maxVal > maxValue ? maxVal : maxValue;

        size_t i = 0;
        int divisions = 30;
        int step = std::max(1, (int)(values[d].size() / divisions));
        size_t size = values[d].size() - 1;
        while (true)
        {
            pointLists[d].append(QPointF(i, values[d][i]));

            if (i >= size) break;
            i += (i + step) < size ? step : size - i;
        }
    }

    // Update axes to proper values
    _xAxis->setRange(0, values[0].size());
    _yAxis->setRange(0, maxValue);

    for (int d = 0; d < values.size(); d++)
        _seriesArray[d]->replace(pointLists[d]);

    updateChartColors();
}

void GradientGraph::setBins(const std::vector<std::vector<int>>& bins)
{
    util::Timer timer;
    timer.start();

    // Store values in a QList<QPointF>
    std::vector<QList<QPointF>> pointLists(bins.size());

    int binTotal = 0;

    for (int d = 0; d < bins.size(); d++)
    {
        binTotal = 0;
        pointLists[d].resize(bins[d].size()+1);
        pointLists[d][0] = QPointF(0, 0);
        for (int i = 0; i < bins[d].size(); i++)
        {
            binTotal += bins[d][i];
            pointLists[d][i+1] = QPointF(binTotal, i / 30.0f);
        }
    }

    // Update axes to proper values
    _xAxis->setRange(0, binTotal);
    _yAxis->setRange(0, 1);

    for (int d = 0; d < bins.size(); d++)
        _seriesArray[d]->replace(pointLists[d]);

    updateChartColors();

    timer.finish("Graph inner");
}

void GradientGraph::setTopDimensions(dint dimension1, dint dimension2)
{
    _topDimension1 = dimension1;
    _topDimension2 = dimension2;
}

void GradientGraph::updateChartColors()
{
    // Replace series values with new values
    for (dint d = 0; d < (dint) _seriesArray.size(); d++)
    {
        QColor color = grey;
        if (d == _topDimension1) color = primary;
        if (d == _topDimension2) color = secondary;
        if (d == _selectedDimension) color = selected;

        _seriesArray[d]->setColor(color);
    }

    _chartView->update();
}

void GradientGraph::onLineClicked(dint dim)
{
    emit lineClicked(dim);
    _selectedDimension = dim;

    updateChartColors();
}
