#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <QtSql>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlQueryModel>

#include <QString>
#include <QColorDialog>

#include <QValueAxis>
#include <QGraphicsLayout>

#include <dialog_add_chart.h>

#include <vector>
#include <utility>

#include "user_set.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_pb_Load_clicked();
    void add_new_table(const QString tab_label, const int indx);
    void remove_all_tabs();
    void update_active_chart_view(const int indx);
    bool create_set_in_db(QStringList const& dialog_input);
    void delete_full_set_in_db(const QString label);
    void on_actionCreate_new_set_triggered();
    void on_Dialog_canceled();
    void on_actionExit_triggered();
    void on_pb_Delete_clicked();
    void on_pb_Save_clicked();

private:
    Ui::MainWindow *ui;
    QChartView* cview;
    QTabWidget* tabs;
    QPushButton* pb_AddTab;
    QWidget* init_tab;
    QSqlDatabase main_db;
    QSqlQueryModel main_query_model;
    Dialog_add_chart* dialog_chart;
    std::vector<QPoint> get_points_for_indx(const int indx);
    std::vector<User_set> get_available_sets();
    void update_table_wgt (QTableWidget& tbl_wgt, std::vector<QPoint>& points);
    void set_line_series(QLineSeries& series, std::vector<QPoint>& points) const;
    bool prepare_db_schema();
    bool append_points_from_table(QTableWidget& tbl_wgt);
    std::vector<QPoint> get_points_from_table(QTableWidget& tbl_wgt);
    bool save_points_to_db(const int indx, std::vector<QPoint> const& points);
    bool remove_points_from_db(const int indx);
    QStringList get_all_tab_labels() const;
};
#endif // MAINWINDOW_H
