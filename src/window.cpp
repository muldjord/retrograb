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
  setMinimumWidth(1300);
  setMinimumHeight(600);
  if(settings.contains("main/windowState")) {
    restoreGeometry(settings.value("main/windowState", "").toByteArray());
  }

  viewport = new QLabel();
  viewport->setStyleSheet("background-color: #000000;");

  grabbed = new QLabel();
  grabbed->setStyleSheet("background-color: #000000;");
  frameStatusLabel = new QLabel(QString::number(frameIdx) + " / " + QString::number(frames.count()));

  recordButton = new QPushButton("Start Recording");
  connect(recordButton, &QPushButton::clicked, this, &Window::initRecording);
  QPushButton *clearButton = new QPushButton("Clear all frames");
  connect(clearButton, &QPushButton::clicked, this, &Window::clearFrames);
  QPushButton *exportButton = new QPushButton("Export");
  connect(exportButton, &QPushButton::clicked, this, &Window::exportFrames);

  Slider *viewportWidthSlider = new Slider(settings, "viewport/width", "Viewport width:", 256, 128);
  Slider *viewportHeightSlider = new Slider(settings, "viewport/height", "Viewport height:", 256, 128);

  Slider *viewportDividerSlider = new Slider(settings, "viewport/divider", "Viewport scale:", 32, 1);

  Slider *snapAlignmentXSlider = new Slider(settings, "viewport/snapAlignmentX", "Snap alignment X:", 16, 1);
  Slider *snapAlignmentYSlider = new Slider(settings, "viewport/snapAlignmentY", "Snap alignment Y:", 16, 1);

  Slider *grabWidthSlider = new Slider(settings, "grab/width", "Grab width:", 64, 16);
  Slider *grabHeightSlider = new Slider(settings, "grab/height", "Grab width:", 64, 16);

  Slider *fpsSlider = new Slider(settings, "viewport/fps", "FPS (frames per second):", 60, 30);
  connect(fpsSlider, &Slider::valueChanged, this, &Window::setFps);

  Slider *backBufferSlider = new Slider(settings, "grab/backBuffer", "Grab look-ahead (number of frames):", 20, 5);

  Slider *recordDelaySlider = new Slider(settings, "grab/delay", "Recording delay:", 20, 5);

  QHBoxLayout *labelLayout = new QHBoxLayout();
  mouseSnapLabel = new QLabel("Mouse pixel snap (ctrl+alt+s): " + QString(mouseSnap?"true":"false"));
  lockXLabel = new QLabel("Mouse X locked (ctrl+alt+x): " + QString(lockX?"true":"false"));
  lockYLabel = new QLabel("Mouse Y locked (ctrl+alt+y): " + QString(lockY?"true":"false"));
  labelLayout->addWidget(mouseSnapLabel);
  labelLayout->addWidget(lockXLabel);
  labelLayout->addWidget(lockYLabel);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(recordButton);
  buttonLayout->addWidget(clearButton);
  buttonLayout->addWidget(exportButton);

  QVBoxLayout *leftLayout = new QVBoxLayout();
  leftLayout->addWidget(viewport, 0, Qt::AlignTop | Qt::AlignCenter);
  leftLayout->addWidget(grabbed, 0, Qt::AlignTop | Qt::AlignCenter);
  leftLayout->addWidget(frameStatusLabel, 0, Qt::AlignTop | Qt::AlignCenter);
  leftLayout->addStretch(500);
  
  QVBoxLayout *rightLayout = new QVBoxLayout();
  rightLayout->addWidget(viewportWidthSlider);
  rightLayout->addWidget(viewportHeightSlider);
  rightLayout->addWidget(viewportDividerSlider);
  rightLayout->addWidget(snapAlignmentXSlider);
  rightLayout->addWidget(snapAlignmentYSlider);
  rightLayout->addWidget(fpsSlider);
  rightLayout->addWidget(backBufferSlider);
  rightLayout->addWidget(grabWidthSlider);
  rightLayout->addWidget(grabHeightSlider);
  rightLayout->addWidget(recordDelaySlider);
  rightLayout->addLayout(labelLayout);
  rightLayout->addLayout(buttonLayout);

  QHBoxLayout *layout = new QHBoxLayout();
  layout->addLayout(leftLayout);
  layout->addLayout(rightLayout);

  
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
  int snapAlignmentX = settings.value("viewport/snapAlignmentX", 1).toInt();
  int snapAlignmentY = settings.value("viewport/snapAlignmentY", 1).toInt();
  
  QPixmap viewportGrab = QGuiApplication::screenAt(QPoint(0, 0))->grabWindow(0,
                                                                             snapAlignmentX + (pos.x() - (mouseSnap?pos.x() % (int)scaleDivider:0)) - ((viewportWidth * scaleDivider) / 2.0),
                                                                             snapAlignmentY + (pos.y() - (mouseSnap?pos.y() % (int)scaleDivider:0)) - ((viewportHeight * scaleDivider) / 2.0),
                                                                             viewportWidth * scaleDivider,
                                                                             viewportHeight * scaleDivider);
  viewportGrab = viewportGrab.scaledToWidth(viewportWidth);
  backBufferPixmap.push_back(viewportGrab);
  backBufferMouse.push_back(pos);
  int backBufferLength = settings.value("grab/backBuffer", 5).toInt();
  while(backBufferPixmap.length() > backBufferLength) {
    backBufferPixmap.pop_front();
    backBufferMouse.pop_front();
  }
  QPainter painter(&viewportGrab);
  painter.setPen(QColor(0, 255, 0));
  painter.drawRect((viewportWidth / 2) - (grabWidth / 2) - 1, (viewportHeight / 2) - (grabHeight / 2) - 1, grabWidth + 1, grabHeight + 1);
  painter.end();
  
  viewport->setPixmap(viewportGrab.scaledToHeight(viewportGrab.height() * 4));

  if((recording || shiftRecording) &&
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
    if(frameIdx > frames.count()) {
      frameIdx = 0;
    }
    if(frameIdx < frames.count()) {
      grabbed->setPixmap(frames.at(frameIdx).scaledToHeight(frames.at(frameIdx).height() * 4));
      frameStatusLabel->setText(QString::number(frameIdx) + " / " + QString::number(frames.count()));
    }
    frameIdx++;
  }
}

void Window::initRecording()
{
  if(!recording) {
    recordButton->setText("Waiting...");
    delayTimer.setInterval(settings.value("grab/delay", 5).toInt() * 1000);
    delayTimer.start();
  } else {
    recording = false;
    delayTimer.stop();
    recordButton->setText("Start Recording");
  }
}

void Window::startRecording()
{
  recording = true;
  recordButton->setText("Stop recording");
}

void Window::exportFrames()
{
  QDir exportDir(settings.value("export/path", "./export").toString());
  if(!exportDir.exists()) {
    if(!exportDir.mkpath(exportDir.absolutePath())) {
      QMessageBox::information(this, tr("Cancelled"), tr("The export path could not be created. Export has been cancelled."));
      return;
    }
  }
  bool overwriteAsk = settings.value("export/overwriteAsk", true).toBool();
  int idx = 0;
  for(const auto &frame: frames) {
    QString zeros = QString::number(idx);
    while(zeros.length() < 6) {
      zeros.prepend("0");
    }
    QString frameName = "frame";
    QString currentFrame = exportDir.absolutePath() + "/" + frameName + zeros + ".png";
    if(QFileInfo::exists(currentFrame)) {
      if(overwriteAsk &&
         QMessageBox::question(this, tr("Overwrite?"),
                               tr("An export already exists. Do you want to overwrite it (the existing one will be removed)?"),
                               QMessageBox::Yes | QMessageBox::No)
         != QMessageBox::Yes) {
        QMessageBox::information(this, tr("Cancelled"), tr("The export has been cancelled."));
        break;
      } else {
        for(const auto &fileInfo: exportDir.entryInfoList(QDir::Files)) {
          if(fileInfo.fileName().left(frameName.length()) == frameName && fileInfo.suffix() == "png") {
            QFile::remove(fileInfo.absoluteFilePath());
          }
        }
      }
    }
    overwriteAsk = false;
    frame.save(currentFrame);
    idx++;
  }
}

void Window::setFps(int fps)
{
  grabTimer.start(1000 / fps, Qt::PreciseTimer, this);
}

void Window::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Shift &&
     !event->isAutoRepeat()) {
    shiftRecording = true;
    printf("Started recording with shift...\n");
  }
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

void Window::keyReleaseEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Shift &&
     !event->isAutoRepeat()) {
    shiftRecording = false;
    printf("Ended recording with shift...\n");
  }
}

void Window::clearFrames()
{
  if(QMessageBox::question(this, tr("Clear all frames?"),
                           tr("Are you sure you want to clear all grabbed frames?"),
                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
    frames.clear();
    frameStatusLabel->setText("0 / 0");
  }
}
