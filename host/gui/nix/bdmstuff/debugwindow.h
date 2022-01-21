#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QPushButton>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QDialog>
#include <QCloseEvent>
#include <QDebug>

namespace Ui {
class debugwindow;
class registerBtn;
}

class registerBtn : public QDialog
{
    Q_OBJECT
public:
    explicit registerBtn(){}
    ~registerBtn() {}
    void actRegisterBtn(QRadioButton *parent, uint32_t index);
private slots:
    void btnRadioSlot();
private:
    uint32_t m_index;
};

class debugwindow : public QDialog
{
    Q_OBJECT

public:
    explicit debugwindow(QWidget *parent = nullptr);
    ~debugwindow() override;
    void Populate(int index);

private:
    void InsertRegisterRow(const void *rSetPtr, uint32_t noItems);
    Ui::debugwindow *ui;
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // DEBUGWINDOW_H
