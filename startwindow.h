#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QMainWindow>

#define PREEMPTIVE_PRIORITY 1195
#define NON_PRIORITY 1196
#define ROUND_ROBIN 1197

namespace Ui {
class StartWindow;
}

class StartWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void startConfig(int startMode);

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_exit_clicked();

private:
    Ui::StartWindow *ui;
};

#endif // STARTWINDOW_H
