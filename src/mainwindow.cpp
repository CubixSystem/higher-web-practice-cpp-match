#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>

#include "renderwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setWindowTitle("Match-3");

    auto *renderWidget = new RenderWidget(this);
    QVBoxLayout *containerLayout = new QVBoxLayout(ui->renderWidgetContainer);
    containerLayout->addWidget(renderWidget);

    scoreLabel_ = new QLabel("Score: 0", this);
    scoreLabel_->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(scoreLabel_, 1);

    connect(renderWidget, &RenderWidget::scoreChanged, this,
            &MainWindow::onScoreChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onScoreChanged(int score, int combo) {
    QString text = QString("Score: %1").arg(score);
    if (combo > 1) {
        text += QString("  ×%1 COMBO").arg(combo);
    }
    scoreLabel_->setText(text);
}
