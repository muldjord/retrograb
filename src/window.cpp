/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            window.cpp
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

#include "window.h"

#include <stdio.h>
#include <QtWidgets>
#include <QSettings>
#include <QMessageBox>

Window::Window(QSettings &settings)
  : settings(settings)
{
  setWindowTitle("RetroGrab v" VERSION);
  setMinimumWidth(1000);
  setMinimumHeight(1200);
  if(settings.contains("main/windowState")) {
    restoreGeometry(settings.value("main/windowState", "").toByteArray());
  }

  viewport = new QLabel();
  viewport->setStyleSheet("background-color: #000000;");

  grabbed = new QLabel();
  grabbed->setStyleSheet("background-color: #000000;");

  recordButton = new QPushButton("Start Recording");
  connect(recordButton, &QPushButton::clicked, this, &Window::initRecording);
  QPushButton *exportButton = new QPushButton("Export");
  connect(exportButton, &QPushButton::clicked, this, &Window::exportFrames);

  Slider *viewportWidthSlider = new Slider(settings, "viewport/width", "Viewport width:", 256, 128);
  Slider *viewportHeightSlider = new Slider(settings, "viewport/height", "Viewport height:", 256, 128);
  Slider *viewportDividerSlider = new Slider(settings, "viewport/divider", "Viewport scale:", 32, 1);

  Slider *grabWidthSlider = new Slider(settings, "grab/width", "Grab width:", 64, 16);
  Slider *grabHeightSlider = new Slider(settings, "grab/height", "Grab width:", 64, 16);

  firstFrameSlider = new Slider(settings, "", "[->", 0, 0, false);
  connect(firstFrameSlider, &Slider::valueChanged, this, &Window::resetFrameIdx);
  lastFrameSlider = new Slider(settings, "", "<-]", 0, 0, false);
  connect(lastFrameSlider, &Slider::valueChanged, this, &Window::resetFrameIdx);

  Slider *fpsSlider = new Slider(settings, "viewport/fps", "FPS (frames per second):", 60, 30);
  connect(fpsSlider, &Slider::valueChanged, this, &Window::setFps);

  Slider *backBufferSlider = new Slider(settings, "grab/backBuffer", "Grab look-ahead (number of frames):", 20, 5);

  QHBoxLayout *labelLayout = new QHBoxLayout();
  mouseSnapLabel = new QLabel("Mouse pixel snap (ctrl+alt+s): " + QString(mouseSnap?"true":"false"));
  lockXLabel = new QLabel("Mouse X locked (ctrl+alt+x): " + QString(lockX?"true":"false"));
  lockYLabel = new QLabel("Mouse Y locked (ctrl+alt+y): " + QString(lockY?"true":"false"));
  labelLayout->addWidget(mouseSnapLabel);
  labelLayout->addWidget(lockXLabel);
  labelLayout->addWidget(lockYLabel);

  QVBoxLayout *layout = new QVBoxLayout();
  //layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(viewport, 0, Qt::AlignTop | Qt::AlignCenter);
  layout->addWidget(grabbed, 0, Qt::AlignTop | Qt::AlignCenter);
  layout->addStretch(500);
  layout->addWidget(viewportWidthSlider);
  layout->addWidget(viewportHeightSlider);
  layout->addWidget(viewportDividerSlider);
  layout->addWidget(fpsSlider);
  layout->addWidget(backBufferSlider);
  layout->addWidget(grabWidthSlider);
  layout->addWidget(grabHeightSlider);
  layout->addWidget(firstFrameSlider);
  layout->addWidget(lastFrameSlider);
  layout->addLayout(labelLayout);
  layout->addWidget(recordButton);
  layout->addWidget(exportButton);

  delayTimer.setInterval(5000);
  delayTimer.setSingleShot(true);
  connect(&delayTimer, &QTimer::timeout, this, &Window::startRecording);
  
  setLayout(layout);
  show();
  grabTimer.start(1000 / settings.value("viewport/fps", 30).toInt(), Qt::PreciseTimer, this);
}

Window::~Window()
{
  settings.setValue("main/windowState", saveGeometry());
}

void Window::timerEvent(QTimerEvent *)
{
  QPoint pos = QCursor::pos();
  if(lockX) {
    pos.setX(lockPosX);
  }
  if(lockY) {
    pos.setY(lockPosY);
  }
  int viewportWidth = settings.value("viewport/width", 128).toInt();
  int viewportHeight = settings.value("viewport/height", 128).toInt();
  int grabWidth = settings.value("grab/width", 16).toInt();
  int grabHeight = settings.value("grab/height", 16).toInt();
  double scaleDivider = settings.value("viewport/divider", 1.0).toDouble();

  /*
  QPixmap viewportGrab = QGuiApplication::screenAt(QPoint(0, 0))->grabWindow(0,
                                                                             pos.x() - ((viewportWidth * scaleDivider) / 2.0),
                                                                             pos.y() - ((viewportHeight * scaleDivider) / 2.0),
                                                                             viewportWidth * scaleDivider,
                                                                             viewportHeight * scaleDivider);
  */
  QPixmap viewportGrab = QGuiApplication::screenAt(QPoint(0, 0))->grabWindow(0,
                                                                             (pos.x() - (mouseSnap?pos.x() % (int)scaleDivider:0)) - ((viewportWidth * scaleDivider) / 2.0),
                                                                             (pos.y() - (mouseSnap?pos.y() % (int)scaleDivider:0)) - ((viewportHeight * scaleDivider) / 2.0),
                                                                             viewportWidth * scaleDivider,
                                                                             viewportHeight * scaleDivider);
  viewportGrab = viewportGrab.scaledToWidth(viewportWidth);
  backBufferPixmap.push_back(viewportGrab);
  backBufferMouse.push_back(pos);
  int backBufferLength = settings.value("grab/backBuffer", 5).toInt();
  if(backBufferPixmap.length() > backBufferLength) {
    backBufferPixmap.pop_front();
    backBufferMouse.pop_front();
  }
  QPainter painter(&viewportGrab);
  painter.setPen(QColor(0, 255, 0));
  painter.drawRect((viewportWidth / 2) - (grabWidth / 2) - 1, (viewportHeight / 2) - (grabHeight / 2) - 1, grabWidth + 1, grabHeight + 1);
  painter.end();
  
  viewport->setPixmap(viewportGrab.scaledToHeight(viewportGrab.height() * 4)); // 0 (entire screen), x, y, w, h

  if(recording &&
     backBufferPixmap.length() >= backBufferLength) {
    QPixmap grab = backBufferPixmap.first().copy(((viewportWidth / 2) - (grabWidth / 2)) + ((pos.x() - backBufferMouse.first().x()) / scaleDivider),
                                                 ((viewportHeight / 2) - (grabHeight / 2)) + ((pos.y() - backBufferMouse.first().y()) / scaleDivider),
                                                 grabWidth,
                                                 grabHeight);
    if(grab.width() == grabWidth &&
       grab.height() == grabHeight) {
      frames.append(grab);
    }
  }
  
  if(!frames.isEmpty()) {
    if(frameIdx > frames.count() ||
       frameIdx > lastFrameSlider->getValue()) {
      frameIdx = firstFrameSlider->getValue();
    }
    if(frameIdx < frames.count()) {
      grabbed->setPixmap(frames.at(frameIdx).scaledToHeight(frames.at(frameIdx).height() * 4));
    }
    frameIdx++;
  }
}

void Window::initRecording()
{
  if(recording) {
    recording = false;
    delayTimer.stop();
    firstFrameSlider->setMaximum(frames.count());
    firstFrameSlider->setValue(0);
    lastFrameSlider->setMaximum(frames.count());
    lastFrameSlider->setValue(frames.count());
    recordButton->setText("Start Recording");
  } else {
    recordButton->setText("Waiting...");
    frames.clear();
    delayTimer.start();
  }
}

void Window::startRecording()
{
  recording = true;
  recordButton->setText("Stop recording");
}

void Window::exportFrames()
{
  QString exportPath = settings.value("export/path", "./export").toString();
  if(!QFileInfo::exists(exportPath)) {
    if(!QDir::current().mkpath(exportPath)) {
      QMessageBox::information(this, tr("Cancelled"), tr("The export path could not be created. Export has been cancelled."));
      return;
    }
  }
  bool overwriteAsk = settings.value("export/overwriteAsk", true).toBool();
  int idx = 0;
  for(const auto &frame: frames) {
    if(idx >= firstFrameSlider->getValue()) {
      QString zeros = QString::number(idx);
      while(zeros.length() < 6) {
        zeros.prepend("0");
      }
      QString currentFrame = (exportPath.right(1) == "/"?exportPath:exportPath + "/") + "frame" + zeros + ".png";
      if(QFileInfo::exists(currentFrame)) {
        if(overwriteAsk &&
           QMessageBox::question(this, tr("Overwrite?"), tr("The export already exists, do you want to overwrite it?"),
                                 QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
          QMessageBox::information(this, tr("Cancelled"), tr("The export has been cancelled."));
          break;
        }
      }
      overwriteAsk = false;
      frame.save(currentFrame);
    }
    if(idx >= lastFrameSlider->getValue()) {
      break;
    }
    idx++;
  }
}

void Window::setFps(int fps)
{
  grabTimer.start(1000 / fps, Qt::PreciseTimer, this);
}

void Window::resetFrameIdx(int)
{
  frameIdx = firstFrameSlider->getValue();
}

void Window::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_S &&
     event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
    mouseSnap = !mouseSnap;
    mouseSnapLabel->setText("Mouse pixel snap (ctrl+alt+s): " + QString(mouseSnap?"true":"false"));
  }
  if(event->key() == Qt::Key_X &&
     event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
    lockX = !lockX;
    lockXLabel->setText("Mouse X locked (ctrl+alt+x): " + QString(lockX?"true":"false"));
    if(lockX) {
      lockPosX = QCursor::pos().x();
    }
  }
  if(event->key() == Qt::Key_Y &&
     event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
    lockY = !lockY;
    lockYLabel->setText("Mouse Y locked (ctrl+alt+y): " + QString(lockY?"true":"false"));
    if(lockY) {
      lockPosY = QCursor::pos().y();
    }
  }
}
