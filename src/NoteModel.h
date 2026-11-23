#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>
#include <QString>

struct Note
{
    QString id;
    QString title;
    QString content;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;
};

class NoteModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        ContentRole,
        UpdatedAtRole
    };

    explicit NoteModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setNotes(const QList<Note> &notes);
    QList<Note> notes() const { return m_notes; }

    int addNote();
    void updateNote(int row, const QString &title, const QString &content);
    void removeNote(int row);

    const Note &noteAt(int row) const { return m_notes.at(row); }
    QString displayTitle(int row) const;

private:
    QList<Note> m_notes;
};

class NoteFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit NoteFilterProxy(QObject *parent = nullptr);
    void setSearchText(const QString &text);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_search;
};
