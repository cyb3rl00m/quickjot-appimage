#include "NoteStorage.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QUuid>

namespace NoteStorage {

QString storageFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dir.isEmpty())
        QDir().mkpath(dir);
    return dir + QStringLiteral("/notes.json");
}

QList<Note> load()
{
    QList<Note> result;

    const QString path = storageFilePath();
    QFile file(path);
    if (!file.exists())
        return result;

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "NoteStorage: could not open" << path << "for reading";
        return result;
    }
    const QByteArray data = file.readAll();
    file.close();

    if (data.trimmed().isEmpty())
        return result;

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "NoteStorage: JSON parse error:" << parseError.errorString();
        return result;
    }

    QJsonArray array;
    if (doc.isArray())
        array = doc.array();
    else if (doc.isObject())
        array = doc.object().value(QStringLiteral("notes")).toArray();

    result.reserve(array.size());
    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;
        const QJsonObject obj = value.toObject();

        Note note;
        note.id = obj.value(QStringLiteral("id")).toString();
        if (note.id.isEmpty())
            note.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        note.title = obj.value(QStringLiteral("title")).toString();
        note.content = obj.value(QStringLiteral("content")).toString();
        note.createdAt = static_cast<qint64>(obj.value(QStringLiteral("createdAt")).toDouble());
        note.updatedAt = static_cast<qint64>(obj.value(QStringLiteral("updatedAt")).toDouble());
        result.append(note);
    }

    return result;
}

bool save(const QList<Note> &notes)
{
    QJsonArray array;
    array.reserve(notes.size());
    for (const Note &note : notes) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), note.id);
        obj.insert(QStringLiteral("title"), note.title);
        obj.insert(QStringLiteral("content"), note.content);
        obj.insert(QStringLiteral("createdAt"), static_cast<double>(note.createdAt));
        obj.insert(QStringLiteral("updatedAt"), static_cast<double>(note.updatedAt));
        array.append(obj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("notes"), array);

    const QJsonDocument doc(root);
    const QByteArray payload = doc.toJson(QJsonDocument::Indented);

    const QString path = storageFilePath();
    if (path.isEmpty()) {
        qWarning() << "NoteStorage: no writable storage path";
        return false;
    }

    const QString tmpPath = path + QStringLiteral(".tmp");
    QFile tmp(tmpPath);
    if (!tmp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "NoteStorage: could not open" << tmpPath << "for writing";
        return false;
    }
    const qint64 written = tmp.write(payload);
    tmp.close();
    if (written != payload.size()) {
        qWarning() << "NoteStorage: short write to" << tmpPath;
        QFile::remove(tmpPath);
        return false;
    }

    if (QFile::exists(path) && !QFile::remove(path)) {
        qWarning() << "NoteStorage: could not remove old" << path;
        QFile::remove(tmpPath);
        return false;
    }

    if (!QFile::rename(tmpPath, path)) {
        qWarning() << "NoteStorage: could not rename" << tmpPath << "to" << path;
        QFile::remove(tmpPath);
        return false;
    }

    return true;
}

} // namespace NoteStorage
