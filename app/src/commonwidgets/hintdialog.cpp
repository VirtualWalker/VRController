/*
 * This file is part of VRController.
 * Copyright (c) 2015 Fabien Caylus <toutjuste13@gmail.com>
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hintdialog.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>

HintDialog::HintDialog(QString text, QPixmap image, QWidget *parent): QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QLabel *imageLabel = new QLabel(this);
    imageLabel->setPixmap(image);

    QLabel *textLabel = new QLabel(QString("<b>%1</b>").arg(text), this);

    QPushButton *okButton = new QPushButton(tr("OK"), this);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(imageLabel, 0, 0, 1, 4);
    mainLayout->addWidget(textLabel, 1, 0, 1, 4);
    mainLayout->addWidget(okButton, 2, 3, 1, 1);
    setLayout(mainLayout);
}

