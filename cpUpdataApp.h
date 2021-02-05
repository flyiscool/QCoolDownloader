#pragma once

#include <QtWidgets/QMainWindow>
#include "libusb.h"
#include "cpThread.h"
#include <QThread>
#include <QMutex>
#include <QMetaType>



int Get_File_Size(QString* filename);
