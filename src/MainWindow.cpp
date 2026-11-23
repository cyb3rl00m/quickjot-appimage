#include "MainWindow.h"

#include "NoteModel.h"
#include "NoteStorage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    m_model = new NoteModel(this);
    m_model->setNotes(NoteStorage::load());

    m_proxy = new NoteFilterProxy(this);
    m_proxy->setSourceModel(m_model);
    m_list->setModel(m_proxy);

    connect(m_list->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onSelectionChanged);

    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(400);
    connect(m_saveTimer, &QTimer::timeout, this, &MainWindow::onSaveTimer);

    updateEmptyState();

    if (m_proxy->rowCount() > 0)
        m_list->setCurrentIndex(m_proxy->index(0, 0));
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("QuickJot"));
    resize(1000, 660);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    // ----- Left panel -----
    auto *left = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(14, 14, 14, 14);
    leftLayout->setSpacing(10);

    m_search = new QLineEdit(left);
    m_search->setObjectName(QStringLiteral("searchEdit"));
    m_search->setPlaceholderText(QStringLiteral("Search notes…"));
    m_search->setClearButtonEnabled(true);
    leftLayout->addWidget(m_search);

    m_list = new QListView(left);
    m_list->setObjectName(QStringLiteral("notesList"));
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setUniformItemSizes(false);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    leftLayout->addWidget(m_list, 1);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    m_newButton = new QPushButton(QStringLiteral("New Note"), left);
    m_newButton->setObjectName(QStringLiteral("primaryButton"));
    m_deleteButton = new QPushButton(QStringLiteral("Delete"), left);
    buttonRow->addWidget(m_newButton);
    buttonRow->addWidget(m_deleteButton);
    leftLayout->addLayout(buttonRow);

    // ----- Right panel -----
    auto *right = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(20, 16, 20, 12);
    rightLayout->setSpacing(8);

    m_titleEdit = new QLineEdit(right);
    m_titleEdit->setObjectName(QStringLiteral("titleEdit"));
    m_titleEdit->setPlaceholderText(QStringLiteral("Title"));
    rightLayout->addWidget(m_titleEdit);

    m_contentEdit = new QPlainTextEdit(right);
    m_contentEdit->setObjectName(QStringLiteral("contentEdit"));
    m_contentEdit->setPlaceholderText(QStringLiteral("Start writing…"));
    m_contentEdit->setFrameShape(QFrame::NoFrame);
    m_contentEdit->setTabChangesFocus(true);
    rightLayout->addWidget(m_contentEdit, 1);

    m_statusLabel = new QLabel(QString(), right);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->addWidget(m_statusLabel);

    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({300, 700});

    // ----- Connections -----
    connect(m_search, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    connect(m_newButton, &QPushButton::clicked, this, &MainWindow::onNewNote);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteNote);
    connect(m_titleEdit, &QLineEdit::textChanged, this, &MainWindow::onTitleChanged);
    connect(m_contentEdit, &QPlainTextEdit::textChanged, this, &MainWindow::onContentChanged);

    // Keyboard shortcuts
    auto *newShortcut = new QShortcut(QKeySequence::New, this);
    connect(newShortcut, &QShortcut::activated, this, &MainWindow::onNewNote);

    auto *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, [this]() {
        m_search->setFocus();
        m_search->selectAll();
    });

    // Delete key while the list has focus
    auto *deleteShortcut = new QShortcut(QKeySequence::Delete, m_list);
    deleteShortcut->setContext(Qt::WidgetShortcut);
    connect(deleteShortcut, &QShortcut::activated, this, &MainWindow::onDeleteNote);
}

int MainWindow::currentSourceRow() const
{
    const QModelIndex proxyIndex = m_list->currentIndex();
    if (!proxyIndex.isValid())
        return -1;
    const QModelIndex sourceIndex = m_proxy->mapToSource(proxyIndex);
    if (!sourceIndex.isValid())
        return -1;
    return sourceIndex.row();
}

void MainWindow::loadCurrentIntoEditor(int sourceRow)
{
    m_loadingFromModel = true;
    if (sourceRow < 0 || sourceRow >= m_model->rowCount()) {
        m_titleEdit->clear();
        m_contentEdit->clear();
    } else {
        const Note &note = m_model->noteAt(sourceRow);
        m_titleEdit->setText(note.title);
        m_contentEdit->setPlainText(note.content);
    }
    m_loadingFromModel = false;
}

void MainWindow::updateEmptyState()
{
    const bool hasNote = m_currentSourceRow >= 0;
    m_titleEdit->setEnabled(hasNote);
    m_contentEdit->setEnabled(hasNote);
    m_deleteButton->setEnabled(hasNote);
}

void MainWindow::scheduleSave()
{
    m_statusLabel->setText(QStringLiteral("Saving…"));
    m_saveTimer->start();
}

void MainWindow::saveAll()
{
    NoteStorage::save(m_model->notes());
}

void MainWindow::onSaveTimer()
{
    saveAll();
    m_statusLabel->setText(QStringLiteral("Saved"));
    QTimer::singleShot(1500, this, [this]() {
        if (m_saveTimer->isActive())
            return;
        m_statusLabel->clear();
    });
}

void MainWindow::onSelectionChanged()
{
    if (m_loadingFromModel)
        return;

    const int row = currentSourceRow();
    if (row == m_currentSourceRow)
        return;

    if (m_saveTimer->isActive()) {
        m_saveTimer->stop();
        saveAll();
        m_statusLabel->setText(QStringLiteral("Saved"));
    }

    m_currentSourceRow = row;
    loadCurrentIntoEditor(row);
    updateEmptyState();
}

void MainWindow::onTitleChanged()
{
    if (m_loadingFromModel || m_currentSourceRow < 0)
        return;
    m_model->updateNote(m_currentSourceRow,
                        m_titleEdit->text(),
                        m_contentEdit->toPlainText());
    scheduleSave();
}

void MainWindow::onContentChanged()
{
    if (m_loadingFromModel || m_currentSourceRow < 0)
        return;
    m_model->updateNote(m_currentSourceRow,
                        m_titleEdit->text(),
                        m_contentEdit->toPlainText());
    scheduleSave();
}

void MainWindow::onSearchChanged(const QString &text)
{
    if (m_saveTimer->isActive()) {
        m_saveTimer->stop();
        saveAll();
    }
    m_proxy->setSearchText(text);
}

void MainWindow::onNewNote()
{
    if (m_saveTimer->isActive()) {
        m_saveTimer->stop();
        saveAll();
    }

    if (!m_search->text().isEmpty())
        m_search->clear();

    const int row = m_model->addNote();
    const QModelIndex proxyIndex = m_proxy->mapFromSource(m_model->index(row, 0));
    if (proxyIndex.isValid()) {
        m_list->setCurrentIndex(proxyIndex);
        m_list->scrollTo(proxyIndex);
    }
    m_titleEdit->setFocus();
    saveAll();
    m_statusLabel->setText(QStringLiteral("Saved"));
}

void MainWindow::onDeleteNote()
{
    const int row = m_currentSourceRow;
    if (row < 0)
        return;

    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        QStringLiteral("Delete Note"),
        QStringLiteral("Delete this note? This cannot be undone."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    if (m_saveTimer->isActive())
        m_saveTimer->stop();

    m_loadingFromModel = true;
    m_model->removeNote(row);
    m_loadingFromModel = false;

    if (m_proxy->rowCount() > 0) {
        const int nextRow = qMin(row, m_proxy->rowCount() - 1);
        m_list->setCurrentIndex(m_proxy->index(nextRow, 0));
    } else {
        m_currentSourceRow = -1;
        loadCurrentIntoEditor(-1);
        updateEmptyState();
    }

    saveAll();
    m_statusLabel->setText(QStringLiteral("Saved"));
}
