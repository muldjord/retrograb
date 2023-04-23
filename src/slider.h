/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            slider.h
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

#ifndef __SLIDER_H__
#define __SLIDER_H__

#include <QWidget>
#include <QSettings>
#include <QSlider>
#include <QLineEdit>

class Slider : public QWidget
{
  Q_OBJECT
    
public:
  Slider(QSettings &settings, const QString &key, const QString &title, const int &maxValue, const int &def, const bool &preserve = true);
  ~Slider();
  int getValue();
  void setValue(int value);
  void setMaximum(int value);
  
signals:
  void valueChanged(int value);
           
private slots:
  void sliderMoved(int val);
  void textEdited(const QString &text);
  
private:
  QSettings &settings;
  QString key;
  bool preserve = true;
  QSlider *slider = nullptr;
  QLineEdit *sliderLineEdit = nullptr;
  
};

#endif // __SLIDER_H__
