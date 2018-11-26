#ifndef DIALOG_ADD_CHART_H
#define DIALOG_ADD_CHART_H

#include <QDialog>
#include <QColorDialog>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QStringList>

namespace Ui {
class Dialog_add_chart;
}

class Dialog_add_chart : public QDialog
{
    Q_OBJECT
signals:
    void setup_ready(QStringList) const;
private slots:
    void updateConfig() const;
public:
    explicit Dialog_add_chart(QWidget *parent = nullptr,QStringList labels = QStringList(""));
    ~Dialog_add_chart();

private:
    Ui::Dialog_add_chart *ui;
    const QStringList existing_labels;
    void verify_input() const;
    bool is_label_unique(QString label) const;
};

#endif // DIALOG_ADD_CHART_H
