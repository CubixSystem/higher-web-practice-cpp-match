#include "renderwidget.h"

#include <QMouseEvent>
#include <QPainter>

#include "field.h"

RenderWidget::RenderWidget(QWidget *parent) : QWidget{parent} {
    setAttribute(Qt::WA_OpaquePaintEvent);

    connect(&timer_, &QTimer::timeout, this, &RenderWidget::tick);

    timer_.start(16);
    frameTimer_.start();
}

RenderWidget::~RenderWidget() {}

void RenderWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::black);

    field_.render(painter);
}

void RenderWidget::mousePressEvent(QMouseEvent *event) {
    field_.click(static_cast<int>(event->position().x()),
                 static_cast<int>(event->position().y()), width(), height());
    QWidget::mousePressEvent(event);
}

void RenderWidget::tick() {
    double dt = frameTimer_.restart() / 1000.0;

    field_.update(dt);

    const int s = field_.score();
    const int c = field_.combo();
    if (s != lastScore_) {
        lastScore_ = s;
        emit scoreChanged(s, c);
    }

    update();
}
