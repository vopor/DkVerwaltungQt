#ifndef HTML2PDFCONVERTER_H
#define HTML2PDFCONVERTER_H

#include <QObject>
#include <QWebEnginePage>
#include <QEventLoop>

class Html2PdfConverter : public QObject
{
    Q_OBJECT
public:
    explicit Html2PdfConverter(QString inputPath, QString outputPath);
    int run();

private slots:
    void loadFinished(bool ok);
    void pdfPrintingFinished(const QString &filePath, bool success);

private:
    QString m_inputPath;
    QString m_outputPath;
    QScopedPointer<QWebEnginePage> m_page;
    QEventLoop m_eventLoop;
};

#endif // HTML2PDFCONVERTER_H
