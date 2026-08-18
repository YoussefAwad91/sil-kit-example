#include "SimplePlotWidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>

SimplePlotWidget::SimplePlotWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(240, 180);
}

void SimplePlotWidget::setTitle(const QString& t) { mTitle = t; update(); }

void SimplePlotWidget::setAxisLabels(const QString& xLabel, const QString& yLabel)
{
    mXLabel = xLabel;
    mYLabel = yLabel;
    update();
}

void SimplePlotWidget::setLegendVisible(bool on) { mLegendVisible = on; update(); }
void SimplePlotWidget::setGridVisible(bool on) { mGridVisible = on; update(); }
void SimplePlotWidget::setPanZoomEnabled(bool on) { mPanZoomEnabled = on; }
void SimplePlotWidget::setTicks(int xTicks, int yTicks)
{
    mXTicks = std::max(2, xTicks);
    mYTicks = std::max(2, yTicks);
    update();
}

void SimplePlotWidget::addSeries(const QString& name, const QPen& pen)
{
    Series s;
    s.pen = pen;
    mSeries[name] = s;
    update();
}

bool SimplePlotWidget::hasSeries(const QString& name) const
{
    return mSeries.contains(name);
}

void SimplePlotWidget::setSeriesData(const QString& name, const QVector<double>& x, const QVector<double>& y)
{
    if (x.size() != y.size()) return;
    if (!mSeries.contains(name)) addSeries(name);

    mSeries[name].x = x;
    mSeries[name].y = y;
    update();
}

void SimplePlotWidget::setXRange(double xmin, double xmax)
{
    mXMin = xmin;
    mXMax = xmax;
    ensureValidRanges();
    update();
}

void SimplePlotWidget::setYRange(double ymin, double ymax)
{
    mYMin = ymin;
    mYMax = ymax;
    ensureValidRanges();
    update();
}

void SimplePlotWidget::autoScaleXFromData()
{
    bool any = false;
    double lo = 0.0, hi = 0.0;

    for (auto it = mSeries.begin(); it != mSeries.end(); ++it) {
        const auto& sx = it.value().x;
        if (sx.isEmpty()) continue;

        double mn = sx[0], mx = sx[0];
        for (int i = 1; i < sx.size(); ++i) {
            mn = std::min(mn, sx[i]);
            mx = std::max(mx, sx[i]);
        }

        if (!any) { lo = mn; hi = mx; any = true; }
        else { lo = std::min(lo, mn); hi = std::max(hi, mx); }
    }

    if (!any) return;
    if (qFuzzyCompare(lo, hi)) hi = lo + 1.0;
    setXRange(lo, hi);
}

void SimplePlotWidget::autoScaleYFromData(double paddingFrac)
{
    bool any = false;
    double lo = 0.0, hi = 0.0;

    for (auto it = mSeries.begin(); it != mSeries.end(); ++it) {
        const auto& sy = it.value().y;
        if (sy.isEmpty()) continue;

        double mn = sy[0], mx = sy[0];
        for (int i = 1; i < sy.size(); ++i) {
            mn = std::min(mn, sy[i]);
            mx = std::max(mx, sy[i]);
        }

        if (!any) { lo = mn; hi = mx; any = true; }
        else { lo = std::min(lo, mn); hi = std::max(hi, mx); }
    }

    if (!any) return;

    if (qFuzzyCompare(lo, hi)) {
        const double eps = (lo == 0.0) ? 1.0 : std::abs(lo) * 0.1;
        lo -= eps;
        hi += eps;
    } else {
        const double span = hi - lo;
        lo -= span * paddingFrac;
        hi += span * paddingFrac;
    }

    setYRange(lo, hi);
}

QRect SimplePlotWidget::plotRect() const
{
    const int left = 55;
    const int right = mLegendVisible ? 130 : 20;
    const int top = 35;
    const int bottom = 45;

    return QRect(left, top, width() - left - right, height() - top - bottom);
}

void SimplePlotWidget::ensureValidRanges()
{
    if (mXMax <= mXMin) mXMax = mXMin + 1.0;
    if (mYMax <= mYMin) mYMax = mYMin + 1.0;
}

QPointF SimplePlotWidget::dataToPixel(double x, double y, const QRect& pr) const
{
    const double nx = (x - mXMin) / (mXMax - mXMin);
    const double ny = (y - mYMin) / (mYMax - mYMin);

    const double px = pr.left() + nx * pr.width();
    const double py = pr.bottom() - ny * pr.height();

    return QPointF(px, py);
}

QPointF SimplePlotWidget::pixelToData(const QPointF& p, const QRect& pr) const
{
    const double nx = (p.x() - pr.left()) / double(pr.width());
    const double ny = (pr.bottom() - p.y()) / double(pr.height());

    const double x = mXMin + nx * (mXMax - mXMin);
    const double y = mYMin + ny * (mYMax - mYMin);

    return QPointF(x, y);
}

void SimplePlotWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRect pr = plotRect();
    ensureValidRanges();

    drawBackground(painter, pr);
    if (mGridVisible) drawGrid(painter, pr);
    drawAxes(painter, pr);
    drawSeries(painter, pr);
    drawTitleAndLabels(painter, pr);
    if (mLegendVisible) drawLegend(painter, pr);
}

void SimplePlotWidget::drawBackground(QPainter& p, const QRect& pr)
{
    p.fillRect(rect(), palette().window());
    p.setPen(QPen(palette().mid().color(), 1));
    p.drawRect(pr.adjusted(0, 0, -1, -1));
}

void SimplePlotWidget::drawGrid(QPainter& p, const QRect& pr)
{
    QPen gridPen(palette().midlight().color(), 1, Qt::DashLine);
    p.setPen(gridPen);

    for (int i = 1; i < mXTicks; ++i) {
        const double a = double(i) / double(mXTicks);
        const int x = pr.left() + int(a * pr.width());
        p.drawLine(x, pr.top(), x, pr.bottom());
    }

    for (int j = 1; j < mYTicks; ++j) {
        const double a = double(j) / double(mYTicks);
        const int y = pr.bottom() - int(a * pr.height());
        p.drawLine(pr.left(), y, pr.right(), y);
    }
}

void SimplePlotWidget::drawAxes(QPainter& p, const QRect& pr)
{
    p.setPen(palette().text().color());

    // X ticks/labels
    for (int i = 0; i <= mXTicks; ++i) {
        const double a = double(i) / double(mXTicks);
        const double xVal = mXMin + a * (mXMax - mXMin);
        const int x = pr.left() + int(a * pr.width());

        p.drawLine(x, pr.bottom(), x, pr.bottom() + 4);
        const QString txt = QString::number(xVal, 'g', 4);
        const int tw = p.fontMetrics().horizontalAdvance(txt);
        p.drawText(x - tw/2, pr.bottom() + 18, txt);
    }

    // Y ticks/labels
    for (int j = 0; j <= mYTicks; ++j) {
        const double a = double(j) / double(mYTicks);
        const double yVal = mYMin + a * (mYMax - mYMin);
        const int y = pr.bottom() - int(a * pr.height());

        p.drawLine(pr.left() - 4, y, pr.left(), y);
        const QString txt = QString::number(yVal, 'g', 4);
        const int tw = p.fontMetrics().horizontalAdvance(txt);
        p.drawText(pr.left() - 8 - tw, y + 4, txt);
    }
}

void SimplePlotWidget::drawSeries(QPainter& p, const QRect& pr)
{
    for (auto it = mSeries.begin(); it != mSeries.end(); ++it) {
        const Series& s = it.value();
        if (s.x.size() < 2) continue;

        p.setPen(s.pen);

        QPainterPath path;
        path.moveTo(dataToPixel(s.x[0], s.y[0], pr));
        for (int i = 1; i < s.x.size(); ++i) {
            path.lineTo(dataToPixel(s.x[i], s.y[i], pr));
        }
        p.drawPath(path);
    }
}

void SimplePlotWidget::drawTitleAndLabels(QPainter& p, const QRect& pr)
{
    // Title
    if (!mTitle.isEmpty()) {
        QFont tf = font();
        tf.setPointSize(tf.pointSize() + 2);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(palette().text().color());

        const int tw = p.fontMetrics().horizontalAdvance(mTitle);
        p.drawText(pr.center().x() - tw/2, 22, mTitle);
    }

    // Axis labels
    QFont lf = font();
    lf.setBold(false);
    p.setFont(lf);

    if (!mXLabel.isEmpty()) {
        const int tw = p.fontMetrics().horizontalAdvance(mXLabel);
        p.drawText(pr.center().x() - tw/2, height() - 10, mXLabel);
    }

    if (!mYLabel.isEmpty()) {
        p.save();
        p.translate(14, pr.center().y());
        p.rotate(-90.0);
        const int tw = p.fontMetrics().horizontalAdvance(mYLabel);
        p.drawText(-tw/2, 0, mYLabel);
        p.restore();
    }
}

void SimplePlotWidget::drawLegend(QPainter& p, const QRect& pr)
{
    if (mSeries.isEmpty()) return;

    const int boxW = 120;
    const int boxH = 18 * mSeries.size() + 10;
    const QRect box(pr.right() + 8, pr.top(), boxW, boxH);

    p.setPen(palette().mid().color());
    p.setBrush(palette().base());
    p.drawRect(box.adjusted(0, 0, -1, -1));

    int y = box.top() + 8;
    for (auto it = mSeries.begin(); it != mSeries.end(); ++it) {
        const QString name = it.key();
        const Series& s = it.value();

        p.setPen(s.pen);
        p.drawLine(box.left() + 8, y + 6, box.left() + 32, y + 6);

        p.setPen(palette().text().color());
        p.drawText(box.left() + 38, y + 10, name);

        y += 18;
    }
}

void SimplePlotWidget::mousePressEvent(QMouseEvent* e)
{
    if (!mPanZoomEnabled) return;
    if (e->button() == Qt::LeftButton) {
        const QRect pr = plotRect();
        if (pr.contains(e->pos())) {
            mDragging = true;
            mLastMousePos = e->pos();
        }
    }
}

void SimplePlotWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!mPanZoomEnabled) return;
    if (!mDragging) return;

    const QRect pr = plotRect();
    if (pr.width() <= 0 || pr.height() <= 0) return;

    const QPoint cur = e->pos();
    const QPoint delta = cur - mLastMousePos;
    mLastMousePos = cur;

    const double dx = -delta.x() * (mXMax - mXMin) / double(pr.width());
    const double dy =  delta.y() * (mYMax - mYMin) / double(pr.height());

    mXMin += dx; mXMax += dx;
    mYMin += dy; mYMax += dy;

    update();
}

void SimplePlotWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        mDragging = false;
    }
}

void SimplePlotWidget::wheelEvent(QWheelEvent* e)
{
    if (!mPanZoomEnabled) return;

    const QRect pr = plotRect();
    const QPoint pos = e->position().toPoint();
    if (!pr.contains(pos)) return;

    const QPointF anchorData = pixelToData(e->position(), pr);

    const double steps = (e->angleDelta().y() / 8.0) / 15.0; // standard wheel steps
    const double factor = std::pow(0.85, steps);            // wheel up -> zoom in

    const double xSpanNew = (mXMax - mXMin) * factor;
    const double ySpanNew = (mYMax - mYMin) * factor;

    const double ax = anchorData.x();
    const double ay = anchorData.y();

    const double xMinNew = ax - (ax - mXMin) * factor;
    const double yMinNew = ay - (ay - mYMin) * factor;

    mXMin = xMinNew;
    mXMax = xMinNew + xSpanNew;

    mYMin = yMinNew;
    mYMax = yMinNew + ySpanNew;

    ensureValidRanges();
    update();
}
