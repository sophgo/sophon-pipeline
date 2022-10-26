//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOUI_MAINWINDOW_H
#define VIDEOUI_MAINWINDOW_H

#include <QMainWindow>
#include "thread_queue.h"
#include <unordered_map>
#include "video_widget.h"
namespace Ui {
class mainwindow;
}

class container_widget;
class mainwindow : public QMainWindow
{
    Q_OBJECT

    container_widget *m_pMainWidget;
    std::unordered_map<int, video_widget *> m_mapWidgets;
public:
    explicit mainwindow(QWidget *parent = 0);
    ~mainwindow();

    int createWidgets(int num);
    video_widget* videoWidget(int id);

private:
    Ui::mainwindow *ui;
};

#endif // VIDEOUI_MAINWINDOW_H
