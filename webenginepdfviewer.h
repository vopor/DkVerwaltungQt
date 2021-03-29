#ifndef WEBENGINEPDFVIEWER_H
#define WEBENGINEPDFVIEWER_H

#include <QtWebEngineWidgets>

class PdfViewer: public QWidget{
    Q_OBJECT
public:
    PdfViewer(const QString &pdf_path, QWidget *parent=nullptr)
        : QWidget(parent), m_View(new QWebEngineView(this)), m_ExitButton(new QPushButton())
    {
        QUrl url = QUrl::fromLocalFile(pdf_path);

        m_View->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
        m_View->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
        m_View->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
        m_View->load(url);

        m_ExitButton->setIcon(QIcon("Ok.png"));

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(m_View);
        layout->addWidget(m_ExitButton);
        this->setLayout(layout);

        connect(m_ExitButton, &QPushButton::clicked, this, &PdfViewer::close);
        connect(m_View, &QWebEngineView::loadFinished, this, &PdfViewer::on_finished);
    }
private Q_SLOTS:
    void on_finished(bool ok){
        if(ok){
            // QTimer::singleShot(100, this, [this](){ goToPage(9); });
        }
    }
private:
    void goToPage(int page){
        // m_View->page()->runJavaScript(QString("window.viewer.viewport_.goToPage(%1)").arg(page));
    }
    QWebEngineView *m_View;
    QPushButton *m_ExitButton;
};

#endif // WEBENGINEPDFVIEWER_H
