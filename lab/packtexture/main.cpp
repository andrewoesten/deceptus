#include "mainwindow.h"
#include "packtexture.h"

#include <iostream>
#include <QApplication>
#include <QCommandLineParser>

// support qt6
#if QT_VERSION > 0x060000
#include <QImageReader>
#endif


int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   // support qt6
#if QT_VERSION > 0x060000
   QImageReader::setAllocationLimit(4096);
#endif

   QCoreApplication::setApplicationName("packtexture");
   QCoreApplication::setApplicationVersion("1.0");

   QCommandLineParser parser;
   parser.addHelpOption();
   parser.addVersionOption();

   QCommandLineOption inputOption(
      QStringList() << "i" << "input",
      QCoreApplication::translate("main", "specify input file"),
      QCoreApplication::translate("main", "file path")
   );

   QCommandLineOption sizeOption(
      QStringList() << "s" << "size",
      QCoreApplication::translate("main", "specify quad size, available values: 16, 24, 32, 64, 128, 256, 512, 1024"),
      QCoreApplication::translate("main", "quad size")
   );

   parser.addOption(inputOption);
   parser.addOption(sizeOption);
   parser.process(a);

   if (parser.isSet(inputOption))
   {
      PackTexture pt;
      int32_t size = pt.mSize;

      if (parser.isSet(sizeOption))
      {
         bool ok = false;
         size = parser.value(sizeOption).toInt(&ok);

         if (!ok)
         {
            std::cerr << "[!] bad value for size parameter" << std::endl;
            parser.showHelp(1);
         }

         auto supportedValues = {16, 24, 32, 64, 128, 256, 512, 1024};

         auto it = std::find(supportedValues.begin(), supportedValues.end(), size);
         if (it == supportedValues.end())
         {
            std::cerr << "[!] bad value for size parameter" << std::endl;
            parser.showHelp(2);
         }
      }

      pt.mSize = size;
      if (!pt.load(parser.value(inputOption)))
      {
         std::cerr << "[!] unable to load file: " << parser.value(inputOption).toStdString() << std::endl;
         exit(3);
      }
      pt.pack();
      pt.dump();

      return 0;
   }

   MainWindow w;
   w.show();
   w.setFixedSize(550,550);

   return a.exec();
}
