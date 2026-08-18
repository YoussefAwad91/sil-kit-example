#pragma once

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QPen>
#include <QPainterPath>  

class SimplePlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimplePlotWidget(QWidget* parent = nullptr);

    void setTitle(const QString& t);
    void setAxisLabels(const QString& xLabel, const QString& yLabel);
    void setLegendVisible(bool on);
    void setGridVisible(bool on);
    void setPanZoomEnabled(bool on);
    void setTicks(int xTicks, int yTicks);

    void addSeries(const QString& name, const QPen& pen = QPen(Qt::blue));
    bool hasSeries(const QString& name) const;
    void setSeriesData(const QString& name, const QVector<double>& x, const QVector<double>& y);

    void setXRange(double xmin, double xmax);
    void setYRange(double ymin, double ymax);

    void autoScaleXFromData();
    void autoScaleYFromData(double paddingFrac = 0.08);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    struct Series {
        QPen pen;
        QVector<double> x;
        QVector<double> y;
    };

    QRect plotRect() const;
    QPointF dataToPixel(double x, double y, const QRect& pr) const;
    QPointF pixelToData(const QPointF& p, const QRect& pr) const;

    void ensureValidRanges();
    void drawBackground(QPainter& p, const QRect& pr);
    void drawGrid(QPainter& p, const QRect& pr);
    void drawAxes(QPainter& p, const QRect& pr);
    void drawSeries(QPainter& p, const QRect& pr);
    void drawTitleAndLabels(QPainter& p, const QRect& pr);
    void drawLegend(QPainter& p, const QRect& pr);

private:
    QString mTitle;
    QString mXLabel;
    QString mYLabel;

    bool mLegendVisible = true;
    bool mGridVisible = true;
    bool mPanZoomEnabled = true;

    int mXTicks = 6;
    int mYTicks = 6;

    double mXMin = 0.0, mXMax = 1.0;
    double mYMin = 0.0, mYMax = 1.0;

    QMap<QString, Series> mSeries;

    // interaction
    bool mDragging = false;
    QPoint mLastMousePos;
};
