#include "UI/MainWindow/MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QSurfaceFormat>
#include "Common/ShaderProvider.h"

int main(int argc, char** argv) {
   QSurfaceFormat format = QSurfaceFormat::defaultFormat();
   format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
   format.setSwapInterval(0);
   format.setVersion(4,1);
   format.setProfile(QSurfaceFormat::CoreProfile);
   format.setSamples(4);
   QSurfaceFormat::setDefaultFormat(format);

   QApplication app(argc, argv);

   app.setStyle("Fusion");

   MainWindow window;
   window.show();

   return app.exec();
}
