#include "PendulumWidget.h"

#include <QPainter>
#include <QtMath>

PendulumWidget::PendulumWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(240, 180);
}

void PendulumWidget::setTitle(const QString& t) { mTitle = t; update(); }
void PendulumWidget::setXLabel(const QString& t) { mXLabel = t; update(); }

void PendulumWidget::setRodLength(double L) { mRodL = L; update(); }
void PendulumWidget::setYRange(double ymin, double ymax) { mYMin = ymin; mYMax = ymax; update(); }
void PendulumWidget::setAutoExpandX(bool on) { mAutoExpandX = on; }

void PendulumWidget::setXTicks(int n)
{
    mXTicks = (n < 2) ? 2 : n;
    update();
}

void PendulumWidget::setState(double cartX, double thetaRad)
{
    mCartX = cartX;
    mTheta = thetaRad;

    if (mAutoExpandX) {
        const double span = (mXMax - mXMin);
        const double margin = 0.10 * span;

        if (mCartX < mXMin + margin || mCartX > mXMax - margin) {
            mXMin = mCartX - 1.0;
            mXMax = mCartX + 1.0;
        }
    }

    update();
}

QPointF PendulumWidget::worldToPixel(double x, double y, const QRect& pr) const
{
    const double nx = (x - mXMin) / (mXMax - mXMin);
    const double ny = (y - mYMin) / (mYMax - mYMin);

    const double px = pr.left() + nx * pr.width();
    const double py = pr.bottom() - ny * pr.height();

    return QPointF(px, py);
}

void PendulumWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);


    const int left = 55;
    const int right = 20;
    const int top = 35;
    const int bottom = 45;
    const QRect pr(left, top, width() - left - right, height() - top - bottom);

    p.fillRect(rect(), palette().window());

    // Title
    if (!mTitle.isEmpty()) {
        QFont tf = font();
        tf.setPointSize(tf.pointSize() + 2);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(palette().text().color());

        const int tw = p.fontMetrics().horizontalAdvance(mTitle);
        p.drawText(pr.center().x() - tw / 2, 22, mTitle);
    }

    // Border
    p.setPen(QPen(palette().mid().color(), 1));
    p.drawRect(pr.adjusted(0, 0, -1, -1));

    // X axis ticks + numeric labels (bottom)
    p.setPen(palette().text().color());

    for (int i = 0; i <= mXTicks; ++i) {
        const double a = double(i) / double(mXTicks);
        const double xVal = mXMin + a * (mXMax - mXMin);
        const int xPix = pr.left() + int(a * pr.width());

        // tick mark
        p.drawLine(xPix, pr.bottom(), xPix, pr.bottom() + 4);

        // label
        const QString txt = QString::number(xVal, 'g', 4);
        const int tw = p.fontMetrics().horizontalAdvance(txt);
        p.drawText(xPix - tw / 2, pr.bottom() + 18, txt);
    }

    // X label
    if (!mXLabel.isEmpty()) {
        QFont lf = font();
        p.setFont(lf);
        p.setPen(palette().text().color());
        const int tw = p.fontMetrics().horizontalAdvance(mXLabel);
        p.drawText(pr.center().x() - tw / 2, height() - 10, mXLabel);
    }

    // Ground line at y=0
    p.setPen(QPen(palette().midlight().color(), 1, Qt::DashLine));
    p.drawLine(worldToPixel(mXMin, 0.0, pr), worldToPixel(mXMax, 0.0, pr));

    // Pendulum endpoints in world coords
    const double x0 = mCartX;
    const double y0 = 0.0;
    const double x1 = x0 + qSin(mTheta) * mRodL;
    const double y1 = y0 + qCos(mTheta) * mRodL;

    const QPointF cartP = worldToPixel(x0, y0, pr);
    const QPointF tipP  = worldToPixel(x1, y1, pr);

    // Rod
    p.setPen(QPen(Qt::red, 3));
    p.drawLine(cartP, tipP);

    // Cart (square)
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    const double cartSize = 14.0;
    p.drawRect(QRectF(cartP.x() - cartSize / 2.0,
                      cartP.y() - cartSize / 2.0,
                      cartSize,
                      cartSize));

    // Tip
    p.setBrush(Qt::red);
    p.drawEllipse(QRectF(tipP.x() - 4.0, tipP.y() - 4.0, 8.0, 8.0));
}
