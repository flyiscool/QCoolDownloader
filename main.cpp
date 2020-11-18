#include "QCoolDownlaod.h"
#include <QtWidgets/QApplication>

#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifdef __cplusplus
FILE iob[] = { *stdin, *stdout, *stderr };
extern "C" {
    FILE* __cdecl _iob(void) { return iob; }
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoolDownlaod w;
    w.show();
    return a.exec();
}
