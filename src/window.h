/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            window.h
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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "slider.h"

#include <QWidget>
#include <QLabel>
#include <QSettings>
#include <QPushButton>
#include <QBasicTimer>
#include <QTimer>
#include <QKeyEvent>

class Window : public QWidget
{
  Q_OBJECT
    
 public:
  Window(QSettings &settings);
  ~Window();
  
public slots:
  
protected:
  void timerEvent(QTimerEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void keyReleaseEvent(QKeyEvent *event);

private slots:
  void initRecording();
  void startRecording();
  void exportFrames();
  void setFps(int fps);
  void clearFrames();

private:
  QSettings &settings;
  bool recording = false;
  bool shiftRecording = false;
  QTimer delayTimer;
  QLabel *viewport = nullptr;
  QLabel *grabbed = nullptr;
  QLabel *mouseSnapLabel = nullptr;
  QLabel *lockXLabel = nullptr;
  QLabel *lockYLabel = nullptr;
  QLabel *frameStatusLabel = nullptr;
  QPushButton *recordButton = nullptr;
  QBasicTimer grabTimer;
  int frameIdx = 0;
  QList<QPixmap> frames;
  QVector<QPixmap> backBufferPixmap;
  QVector<QPoint> backBufferMouse;
  bool lockX = false;
  bool lockY = false;
  bool mouseSnap = true;
  int lockPosX = -1;
  int lockPosY = -1;
  
};

#endif // __WINDOW_H__
