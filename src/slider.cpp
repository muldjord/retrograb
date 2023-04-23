/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            slider.cpp
 *
 *  Sun Jan 8 23:44:00 CEST 2023
 *  Copyright 2023 Lars Muldjord
 *  muldjordlars@gmail.com
 ****************************************************************************/
/*
 *  This file is part of RetroGrab.
 *
 *  RetroGrab is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  RetroGrab is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RetroGrab; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#include "slider.h"

#include <stdio.h>
#include <QLabel>
#include <QHBoxLayout>
#include <QIntValidator>

Slider::Slider(QSettings &settings, const QString &key, const QString &title, const int &maxValue, const int &def, const bool &preserve)
  : settings(settings), key(key), preserve(preserve)
{
  QLabel *titleLabel = new QLabel(title);
  slider = new QSlider(Qt::Horizontal, this);
  slider->setSingleStep(1);
  slider->setMinimum(1);
  slider->setMaximum(maxValue);
  if(preserve) {
    slider->setValue(settings.value(key, def).toInt());
  }
  connect(slider, &QSlider::valueChanged, this, &Slider::sliderMoved);
  
  sliderLineEdit = new QLineEdit("");
  if(preserve) {
    sliderLineEdit = new QLineEdit(QString::number(settings.value(key, def).toInt()));
  }
  sliderLineEdit->setMaximumWidth(50);
  connect(sliderLineEdit, &QLineEdit::textEdited, this, &Slider::textEdited);

  QValidator *intValidator = new QIntValidator(1, maxValue, this);
  sliderLineEdit->setValidator(intValidator);
  
  QHBoxLayout *layout = new QHBoxLayout();
  layout->addWidget(titleLabel);
  layout->addWidget(slider);
  layout->addWidget(sliderLineEdit);

  setLayout(layout);
}

Slider::~Slider()
{
}

void Slider::sliderMoved(int val)
{
  sliderLineEdit->setText(QString::number(val));
  if(preserve) {
    settings.setValue(key, val);
  }
  emit valueChanged(val);
}

void Slider::textEdited(const QString &text)
{
  slider->setValue(text.toInt());
  if(preserve) {
    settings.setValue(key, text.toInt());
  }

  emit valueChanged(text.toInt());
}

int Slider::getValue()
{
  return slider->value();
}

void Slider::setValue(int value)
{
  slider->setValue(value);
}

void Slider::setMaximum(int value)
{
  slider->setMaximum(value);
}
