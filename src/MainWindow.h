#pragma once

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QListView;
class QPlainTextEdit;
class QPushButton;
class QTimer;

class NoteFilterProxy;
class NoteModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewNote();
    void onDeleteNote();
    void onSearchChanged(const QString &text);
    void onSelectionChanged();
    void onTitleChanged();
    void onContentChanged();
    void onSaveTimer();

private:
    void setupUi();
    void loadCurrentIntoEditor(int sourceRow);
    void saveAll();
    void scheduleSave();
    int currentSourceRow() const;
    void updateEmptyState();

    NoteModel *m_model = nullptr;
    NoteFilterProxy *m_proxy = nullptr;

    QLineEdit *m_search = nullptr;
    QListView *m_list = nullptr;
    QPushButton *m_newButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QLineEdit *m_titleEdit = nullptr;
    QPlainTextEdit *m_contentEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    QTimer *m_saveTimer = nullptr;

    int m_currentSourceRow = -1;
    bool m_loadingFromModel = false;
};
