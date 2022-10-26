//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <QtWidgets>
#include "mainwindow.h"
#include "bmgui.h"

int main(int argc, char *argv[]) {
#if 0
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);
    mainwindow w;
    w.show();
    return a.exec();
#endif

    bm::VideoUIAppPtr app = bm::VideoUIApp::create(argc, argv);
    app->bootUI(4);

    getchar();

}