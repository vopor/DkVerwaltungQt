#include "html2pdfconverter.h"

using namespace std;

Html2PdfConverter::Html2PdfConverter(QString inputPath, QString outputPath)
    : m_inputPath(move(inputPath))
    , m_outputPath(move(outputPath))
    , m_page(new QWebEnginePage)
{
    connect(m_page.data(), &QWebEnginePage::loadFinished,
            this, &Html2PdfConverter::loadFinished);
    connect(m_page.data(), &QWebEnginePage::pdfPrintingFinished,
            this, &Html2PdfConverter::pdfPrintingFinished);
}

int Html2PdfConverter::run()
{
    m_page->load(QUrl::fromUserInput(m_inputPath));
    return m_eventLoop.exec();
}

void Html2PdfConverter::loadFinished(bool ok)
{
    if (!ok) {
        QTextStream(stderr)
            << tr("failed to load URL '%1'").arg(m_inputPath) << "\n";
        m_eventLoop.exit(1);
        return;
    }

    m_page->printToPdf(m_outputPath, QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(25,25,25,25)));
}

void Html2PdfConverter::pdfPrintingFinished(const QString &filePath, bool success)
{
    if (!success) {
        QTextStream(stderr)
            << tr("failed to print to output file '%1'").arg(filePath) << "\n";
        m_eventLoop.exit(1);
    } else {
        m_eventLoop.quit();
    }
}
