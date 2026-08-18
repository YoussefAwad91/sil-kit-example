#pragma once

#include <QWidget>

class PendulumWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PendulumWidget(QWidget* parent = nullptr);

    void setTitle(const QString& t);
    void setXLabel(const QString& t);

    // theta in radians
    void setState(double cartX, double thetaRad);

    void setRodLength(double L);
    void setYRange(double ymin, double ymax);
    void setAutoExpandX(bool on);

    
    void setXTicks(int n);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QPointF worldToPixel(double x, double y, const QRect& pr) const;

private:
    QString mTitle;
    QString mXLabel;

    double mCartX = 0.0;
    double mTheta = 0.0;
    double mRodL = 0.5;

    double mYMin = -0.6;
    double mYMax =  0.6;

    bool mAutoExpandX = true;
    double mXMin = -1.0;
    double mXMax =  1.0;

    int mXTicks = 6;   
};
