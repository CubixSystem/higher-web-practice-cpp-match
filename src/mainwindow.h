#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void onScoreChanged(int score, int combo);

  private:
    Ui::MainWindow *ui;
    QLabel *scoreLabel_ = nullptr;
};
#endif // MAINWINDOW_H
