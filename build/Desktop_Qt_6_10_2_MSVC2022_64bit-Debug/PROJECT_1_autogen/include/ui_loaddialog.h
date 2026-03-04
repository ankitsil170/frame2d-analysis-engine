/********************************************************************************
** Form generated from reading UI file 'loaddialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADDIALOG_H
#define UI_LOADDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_LoadDialog
{
public:
    QDialogButtonBox *buttonBox;
    QDoubleSpinBox *spinFx;
    QDoubleSpinBox *spinFy;
    QDoubleSpinBox *spinMz;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;

    void setupUi(QDialog *LoadDialog)
    {
        if (LoadDialog->objectName().isEmpty())
            LoadDialog->setObjectName("LoadDialog");
        LoadDialog->resize(400, 300);
        buttonBox = new QDialogButtonBox(LoadDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(110, 260, 167, 29));
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);
        spinFx = new QDoubleSpinBox(LoadDialog);
        spinFx->setObjectName("spinFx");
        spinFx->setGeometry(QRect(220, 60, 121, 29));
        spinFx->setMinimum(-99.989999999999995);
        spinFy = new QDoubleSpinBox(LoadDialog);
        spinFy->setObjectName("spinFy");
        spinFy->setGeometry(QRect(220, 120, 121, 29));
        spinFy->setMinimum(-99.989999999999995);
        spinMz = new QDoubleSpinBox(LoadDialog);
        spinMz->setObjectName("spinMz");
        spinMz->setGeometry(QRect(220, 180, 121, 29));
        spinMz->setMinimum(-99.989999999999995);
        spinMz->setValue(0.000000000000000);
        label = new QLabel(LoadDialog);
        label->setObjectName("label");
        label->setGeometry(QRect(110, 60, 141, 20));
        label_2 = new QLabel(LoadDialog);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(110, 120, 63, 20));
        label_3 = new QLabel(LoadDialog);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(110, 180, 71, 20));

        retranslateUi(LoadDialog);

        QMetaObject::connectSlotsByName(LoadDialog);
    } // setupUi

    void retranslateUi(QDialog *LoadDialog)
    {
        LoadDialog->setWindowTitle(QCoreApplication::translate("LoadDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("LoadDialog", "Fx (kN):", nullptr));
        label_2->setText(QCoreApplication::translate("LoadDialog", "Fy (kN):", nullptr));
        label_3->setText(QCoreApplication::translate("LoadDialog", "Mz (kN\302\267m):", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoadDialog: public Ui_LoadDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADDIALOG_H
