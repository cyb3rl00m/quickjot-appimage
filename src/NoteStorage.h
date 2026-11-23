#pragma once

#include <QList>
#include <QString>

#include "NoteModel.h"

namespace NoteStorage {

QString storageFilePath();
QList<Note> load();
bool save(const QList<Note> &notes);

} // namespace NoteStorage
