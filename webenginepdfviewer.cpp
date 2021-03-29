#include "webenginepdfviewer.h"

#if 0

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString fileName = QFileDialog::getOpenFileName(nullptr,
                                                    QObject::tr("Open Image"),
                                                    QDir::homePath(),
                                                    QObject::tr("PDF Files (*.pdf)"));
    if(fileName.isEmpty())
        return 0;
    PdfViewer w(fileName);
    w.resize(640, 480);
    w.show();
    return a.exec();
}

#endif
