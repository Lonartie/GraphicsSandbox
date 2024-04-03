#include "UI/MainWindow/MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QSurfaceFormat>

int main(int argc, char** argv) {
   QApplication app(argc, argv);

   app.setStyle("Fusion");

   QSurfaceFormat format = QSurfaceFormat::defaultFormat();
   format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
   QSurfaceFormat::setDefaultFormat(format);

   MainWindow window;
   window.show();

   return app.exec();
}
