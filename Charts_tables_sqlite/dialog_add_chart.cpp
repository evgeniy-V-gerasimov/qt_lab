#include "dialog_add_chart.h"
#include "ui_dialog_add_chart.h"

void Dialog_add_chart::updateConfig() const
{
    QStringList lst;
    lst.append(ui->le_name->text());
    lst.append(ui->le_color->text());
    emit setup_ready(lst);
}

Dialog_add_chart::Dialog_add_chart(QWidget *parent, QStringList labels) :
    QDialog(parent),
    ui(new Ui::Dialog_add_chart),
    existing_labels(labels)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);


    connect(ui->le_name, &QLineEdit::editingFinished,
            this, &Dialog_add_chart::verify_input);
    connect(ui->le_color, &QLineEdit::editingFinished,
            this, &Dialog_add_chart::verify_input);
    connect(ui->le_name, &QLineEdit::textChanged,
            this, &Dialog_add_chart::verify_input);
    connect(ui->le_color, &QLineEdit::textChanged,
            this, &Dialog_add_chart::verify_input);

    QRegExp sets_naming_regx ("^[0-9A-Za-zА-Яа-я]{3}[\\s\\_0-9A-Za-zА-Яа-я]{,63}$");
    ui->le_name->setValidator(new QRegExpValidator(sets_naming_regx, ui->le_name));

    QRegExp color_regx ("^#(?:[0-9a-fA-F]{6})$");
    ui->le_color->setValidator(new QRegExpValidator(color_regx, ui->le_color));

    connect(ui->pb_select, &QPushButton::clicked,
            [this]()
            {
                this->ui->le_color->setText(QColorDialog::getColor().name());
            });
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &Dialog_add_chart::updateConfig);
}

Dialog_add_chart::~Dialog_add_chart()
{
    delete ui;
}

void Dialog_add_chart::verify_input() const
{
    QPalette ready;
    ready.setColor(QPalette::Base, Qt::green);
    ready.setColor(QPalette::Text, Qt::black);
    QPalette bad_input;
    bad_input.setColor(QPalette::Base, Qt::red);
    bad_input.setColor(QPalette::Text, Qt::black);

    bool name_good {ui->le_name->hasAcceptableInput()};
    bool name_unique = is_label_unique(ui->le_name->text()); // Labels must be unique
    bool color_good {ui->le_color->hasAcceptableInput()};
    if(name_good && color_good) {
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(false);
    }
    else {
        ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setDisabled(true);
    }
    if(name_good && name_unique){
        ui->le_name->setPalette(ready);
    }
    else {
        ui->le_name->setPalette(bad_input);
    }
    if(color_good){
        ui->le_color->setPalette(ready);
    }
    else {
        ui->le_color->setPalette(bad_input);
    }
}

bool Dialog_add_chart::is_label_unique(const QString label) const
{
    return !existing_labels.contains(label);
}
