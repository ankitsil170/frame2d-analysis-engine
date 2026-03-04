#include "loaddialog.h"
#include "ui_loaddialog.h"

LoadDialog::LoadDialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::LoadDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);

    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}

LoadDialog::~LoadDialog()
{
    delete ui;
}

double LoadDialog::fx() const { return ui->spinFx->value(); }
double LoadDialog::fy() const { return ui->spinFy->value(); }
double LoadDialog::mz() const { return ui->spinMz->value(); }
