#include "NoteModel.h"

#include <QDateTime>
#include <QUuid>

NoteModel::NoteModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NoteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_notes.size();
}

QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_notes.size())
        return {};

    const Note &note = m_notes.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return displayTitle(index.row());
    case IdRole:
        return note.id;
    case ContentRole:
        return note.content;
    case UpdatedAtRole:
        return note.updatedAt;
    default:
        return {};
    }
}

void NoteModel::setNotes(const QList<Note> &notes)
{
    beginResetModel();
    m_notes = notes;
    endResetModel();
}

int NoteModel::addNote()
{
    Note note;
    note.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    note.createdAt = QDateTime::currentMSecsSinceEpoch();
    note.updatedAt = note.createdAt;

    beginInsertRows(QModelIndex(), 0, 0);
    m_notes.prepend(note);
    endInsertRows();

    return 0;
}

void NoteModel::updateNote(int row, const QString &title, const QString &content)
{
    if (row < 0 || row >= m_notes.size())
        return;

    Note &note = m_notes[row];
    if (note.title == title && note.content == content)
        return;

    note.title = title;
    note.content = content;
    note.updatedAt = QDateTime::currentMSecsSinceEpoch();

    const QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, {TitleRole, ContentRole, UpdatedAtRole});
}

void NoteModel::removeNote(int row)
{
    if (row < 0 || row >= m_notes.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_notes.removeAt(row);
    endRemoveRows();
}

QString NoteModel::displayTitle(int row) const
{
    if (row < 0 || row >= m_notes.size())
        return {};

    const Note &note = m_notes.at(row);
    const QString trimmedTitle = note.title.trimmed();
    if (!trimmedTitle.isEmpty())
        return trimmedTitle;

    const QStringList lines = note.content.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const QString candidate = line.trimmed();
        if (!candidate.isEmpty()) {
            if (candidate.size() > 60)
                return candidate.left(60) + QStringLiteral("…");
            return candidate;
        }
    }

    return QStringLiteral("Untitled note");
}

// ---------------------------------------------------------------------------
// NoteFilterProxy
// ---------------------------------------------------------------------------

NoteFilterProxy::NoteFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void NoteFilterProxy::setSearchText(const QString &text)
{
    if (m_search == text)
        return;
    m_search = text;
    invalidateFilter();
}

bool NoteFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_search.isEmpty())
        return true;

    QAbstractItemModel *model = sourceModel();
    if (!model)
        return true;

    const QModelIndex idx = model->index(sourceRow, 0, sourceParent);
    if (!idx.isValid())
        return false;

    const QString title = model->data(idx, NoteModel::TitleRole).toString();
    if (title.contains(m_search, Qt::CaseInsensitive))
        return true;

    const QString content = model->data(idx, NoteModel::ContentRole).toString();
    return content.contains(m_search, Qt::CaseInsensitive);
}
