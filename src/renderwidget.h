#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>

#include "field.h"

class RenderWidget : public QWidget {
    Q_OBJECT
  public:
    explicit RenderWidget(QWidget *parent = nullptr);
    ~RenderWidget();

  signals:
    void scoreChanged(int score, int combo);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void tick();

  private:
    Field field_;
    QTimer timer_;
    QElapsedTimer frameTimer_;
    int lastScore_ = 0;
};

#endif // RENDERWIDGET_H
