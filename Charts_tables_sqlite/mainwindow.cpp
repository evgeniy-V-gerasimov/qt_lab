#include "mainwindow.h"
#include "ui_mainwindow.h"

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cview(new QChartView),
    tabs(new QTabWidget),
    pb_AddTab(new QPushButton("+")),
    init_tab(new QWidget),
    main_db(QSqlDatabase::addDatabase("QSQLITE")),
    dialog_chart(nullptr)
{
    ui->setupUi(this);

    main_db.setDatabaseName("28751105.sqlite");
    if(main_db.open())
    {
        prepare_db_schema();
    }
    else{
        statusBar()->showMessage(tr("SQLite database failed to open"), 3000);
    }
    ui->vl_chart->addWidget(cview);
    ui->vl_tables->addWidget(tabs);

    ui->vl_chart->setStretchFactor(cview,2);
    ui->vl_tables->setStretchFactor(tabs,1);
    tabs->setMinimumSize(QSize(600,100));
    tabs->setCornerWidget(pb_AddTab, Qt::TopRightCorner);
    tabs->addTab(init_tab, QString("★")); // adding first tab, to reveal pb_AddTab

    connect(pb_AddTab, &QPushButton::clicked,
            this, &MainWindow::on_actionCreate_new_set_triggered);

    connect(tabs, &QTabWidget::currentChanged,
            this, &MainWindow::update_active_chart_view);

    ui->pb_Save->setDisabled(true);
    ui->pb_Delete->setDisabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pb_Load_clicked()
{
    auto sets = get_available_sets();
    if(! sets.empty())
    {
        remove_all_tabs();
        for(auto& item : sets) {
            add_new_table(item.label, item.indx);
        }
        ui->pb_Delete->setDisabled(false);
        ui->pb_Save->setDisabled(false);
    }
}

std::vector<QPoint> MainWindow::get_points_for_indx(const int indx)
{
    std::vector<QPoint> result;
    main_query_model.setQuery(QString("select * from points where indx = %1 order by X;").arg(indx));
    const int row_count { main_query_model.rowCount() };
    for (int i = 0; i < row_count; ++i) {
        int x = main_query_model.record(i).value("X").toInt();
        int y = main_query_model.record(i).value("Y").toInt();
        result.emplace_back(QPoint(x, y));
    }
    return result;
}

std::vector<User_set> MainWindow::get_available_sets()
{
    std::vector<User_set> indx_list;
    main_query_model.setQuery(QString("select * from setup order by indx;") );
    const int row_count { main_query_model.rowCount() };
    for (int i = 0; i < row_count; ++i) {
        int indx =      main_query_model.record(i).value("indx").toInt();
        QString label = main_query_model.record(i).value("label").toString();
        QColor color =  main_query_model.record(i).value("color").toString();
        indx_list.emplace_back(User_set(indx,label,color));
    }
    return indx_list;
}

void MainWindow::update_table_wgt(QTableWidget &tbl_wgt, std::vector<QPoint> &points)
{

    const int col_count = static_cast<const int>(points.size()) ;
    tbl_wgt.setRowCount(2);
    tbl_wgt.setColumnCount(col_count + 42);
    tbl_wgt.resizeColumnsToContents();
    QStringList s;
    s.append("X");
    s.append("Y");
    tbl_wgt.setVerticalHeaderLabels(s);
    tbl_wgt.horizontalHeader()->hide();
    if(points.size() != 0) {
        int i {0};
        for(auto& p : points){
            QTableWidgetItem* itemX = new QTableWidgetItem(QString("%1").arg(p.x()));
            tbl_wgt.setItem(0, i, itemX);
            QTableWidgetItem* itemY = new QTableWidgetItem(QString("%1").arg(p.y()));
            tbl_wgt.setItem(1, i, itemY);
            ++i;
        }
    }
}

void MainWindow::set_line_series(QLineSeries &series, std::vector<QPoint> &points) const
{
    for(auto& p : points) {
        series.append(p.x(), p.y());
    }
}

bool MainWindow::prepare_db_schema()
{
    if(! main_db.tables().contains(QString("points")))
    {
        QString sql = QString("create table points (indx int not null, X int not null, Y int not null);");
        QSqlQuery qry(main_db);
        if(qry.exec(sql)) {
        }
        else{
            return false;
        }
        sql = QString("create table setup (indx primary key, label string not null, color string not null);");
        if(qry.exec(sql)){
        }
        else{
            return  false;
        }
    }
    return true;
}

bool MainWindow::append_points_from_table(QTableWidget &tbl_wgt)
{
    auto points = get_points_from_table(tbl_wgt);

    const int tab_index = tabs->currentIndex();
    const QString label = tabs->tabText(tab_index);
    main_query_model.setQuery(QString("select indx from setup where label='%1';").arg(label));
    int indx = main_query_model.record(0).value("indx").toInt();

    remove_points_from_db(indx);
    save_points_to_db(indx, points);

    return false;
}

std::vector<QPoint> MainWindow::get_points_from_table(QTableWidget &tbl_wgt)
{
    std::vector<QPoint> points;
    QStringList X_lst, Y_lst;
    const int rows { tbl_wgt.rowCount() };
    const int cols { tbl_wgt.columnCount() };
    for(int c_row {0}; c_row < rows; ++c_row){
        for(int c_col {0}; c_col < cols; ++c_col){
            auto* item = tbl_wgt.item(c_row, c_col);
            if(item != nullptr) {
                (c_row == 0)? X_lst.append(item->text()) : Y_lst.append(item->text());
            }
            else {
                (c_row == 0)? X_lst.append("") : Y_lst.append("");
            }
        }
    }
    QRegExp rgx ("^(\\D+|\\d{6,})"); // replacing non-digits and digits longer than 6
    X_lst.replaceInStrings(rgx, "");
    Y_lst.replaceInStrings(rgx, "");

    const int size = X_lst.size();
    for(int j {0}; size > j; ++j){
        auto sx = X_lst.at(j);
        auto sy = Y_lst.at(j);

        if(sx != QString("") ){
            if(sy != QString("") ){
                points.emplace_back(sx.toInt(), sy.toInt());
            }
        }
    }
    return points;
}

bool MainWindow::save_points_to_db(const int indx, std::vector<QPoint> const& points)
{
    QSqlQuery qry(main_db);
    for(auto& p: points){
        QString sql = QString("insert into points(indx, X, Y) values(%1, %2, %3);").arg(indx).arg(p.x()).arg(p.y());
        if(qry.exec(sql))
        {
            on_pb_Load_clicked(); // reload tabs from db
            tabs->setCurrentIndex(tabs->count() - 1); // set last tab active
        }
        else {
            return false;
        }
    }
    return true;
}

bool MainWindow::remove_points_from_db(const int indx)
{
    QSqlQuery qry(main_db);
    QString sql = QString("delete from points where indx=%1;").arg(indx);
    if(qry.exec(sql)) {
    }
    else {
         return false;
    }
    return true;
}

QStringList MainWindow::get_all_tab_labels() const
{
    QStringList labels;
    const int t_count = tabs->count();
    for(int i{0};t_count > i; ++i){
        labels.append(tabs->tabText(i));
    }
    return labels;
}

void MainWindow::add_new_table(const QString tab_label, const int indx)
{
    QTableWidget* tbl = new QTableWidget;
    tabs->addTab(tbl, tab_label);
    auto points = get_points_for_indx(indx);
    update_table_wgt(*tbl, points);
}

void MainWindow::remove_all_tabs()
{
    for (int i {tabs->count()-1}; i >= 0; --i) {
       tabs->removeTab(i);
    }
}

void MainWindow::update_active_chart_view(const int tab_index)
{
    QChart* chart = new QChart();
    chart->setMinimumSize(QSizeF(600,300));
    if(tab_index >= 0) {
        auto sets = get_available_sets();
        int indx {0};
        QColor color;
        for(auto& s: sets) {
            if(s.label == tabs->tabText(tab_index)){
                indx = s.indx;
                color = s.color;
            }
        }
        auto points = get_points_for_indx(indx);
        if(!points.empty())
        {
            int max_y {0};
            for(auto& p : points) { // aquire maximum for Y axis
                const int y = p.y();
                if(y > max_y) max_y = y;
            }
            QPen pen(color.rgb());
            pen.setWidth(2);

            QLineSeries* series = new QLineSeries();
            series->setPointsVisible();
            series->setName(tabs->tabText(tab_index));
            series->setPen(pen);
            set_line_series(*series, points);

            QValueAxis* axisX = new QValueAxis;
            QValueAxis* axisY = new QValueAxis;
            axisX->setRange(0, points.back().x());
            axisY->setRange(0, max_y);
            axisX->setGridLineVisible(false);
            axisY->setGridLineVisible(true);
            axisX->setLabelFormat("%d");
            axisY->setLabelFormat("%d");
            chart->addSeries(series);
            chart->legend()->setAlignment(Qt::AlignBottom);
            chart->addAxis(axisX, Qt::AlignBottom);
            chart->addAxis(axisY, Qt::AlignLeft);
            chart->layout()->setContentsMargins(0, 0, 0, 0);
            chart->setBackgroundRoundness(0);
        }
    }
    cview->setChart(chart); // setting empty chart
}

bool MainWindow::create_set_in_db(const QStringList &dialog_input)
{
    QString label = dialog_input.at(0);
    QString color = dialog_input.at(1);
    auto sets = get_available_sets();
    if(! sets.empty()) { // when previous entries exist, use last index
        auto last_index = sets.back().indx;
        QString sql = QString("insert into setup(indx, label, color) values(%1, '%2', '%3');").arg(last_index + 1).arg(label).arg(color);
        QSqlQuery qry(main_db);
        if(qry.exec(sql))
        {
            on_pb_Load_clicked(); // reload tabs from db
            tabs->setCurrentIndex(tabs->count() - 1); // set last tab active
            ui->pb_Delete->setDisabled(false);
            ui->pb_Save->setDisabled(false);
        }
        else {
            return false;
        }
    }
    else { // set starting index to (1) for new set
        QString sql = QString("insert into setup(indx, label, color) values(%1, '%2', '%3');").arg(0).arg(label).arg(color);
        QSqlQuery qry(main_db);
        if(qry.exec(sql))
        {
            on_pb_Load_clicked(); // reload tabs from db
            tabs->setCurrentIndex(tabs->count() - 1); // set last tab active
            ui->pb_Delete->setDisabled(false);
            ui->pb_Save->setDisabled(false);
        }
        else {
            return false;
        }
    }
    return true;
}

void MainWindow::delete_full_set_in_db(const QString label)
{
    main_query_model.setQuery(QString("select indx from setup where label='%1';").arg(label));
    int indx = main_query_model.record(0).value("indx").toInt();

    QString sql = QString("delete from setup where label='%1';").arg(label);
    QSqlQuery qry(main_db);
    if(qry.exec(sql))
    {
        remove_points_from_db(indx);
    }
}

void MainWindow::on_actionCreate_new_set_triggered()
{
    dialog_chart = new Dialog_add_chart(this, get_all_tab_labels());
    dialog_chart->show();
    connect(dialog_chart, &Dialog_add_chart::setup_ready,
            this, &MainWindow::create_set_in_db);
    connect(dialog_chart, &Dialog_add_chart::rejected,
            this, &MainWindow::on_Dialog_canceled);
}

void MainWindow::on_Dialog_canceled()
{
    delete dialog_chart;
    dialog_chart = nullptr;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_pb_Delete_clicked()
{
    int const tab_index = tabs->currentIndex();
    const QString tab_lbl = tabs->tabText(tab_index);

    delete_full_set_in_db(tab_lbl);
    tabs->removeTab(tab_index);

    if( tabs->count() == 0){
        tabs->addTab(init_tab, QString("★"));
        ui->pb_Save->setDisabled(true);
        ui->pb_Delete->setDisabled(true);
    }
}

void MainWindow::on_pb_Save_clicked()
{
    auto* t = static_cast<QTableWidget*>(tabs->currentWidget());
    append_points_from_table(*t);
}
