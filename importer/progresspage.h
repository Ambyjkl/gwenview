// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
#ifndef PROGRESSPAGE_H
#define PROGRESSPAGE_H

// Qt
#include <QWidget>

// KDE
#include <kurl.h>

// Local

class KJob;

namespace Gwenview {


class ProgressPagePrivate;
class ProgressPage : public QWidget {
	Q_OBJECT
public:
	ProgressPage();
	~ProgressPage();

	void start(const KUrl::List&, const KUrl& destination);

Q_SIGNALS:
	void importFinished();

private Q_SLOTS:
	void slotResult(KJob*);

private:
	friend class ProgressPagePrivate;
	ProgressPagePrivate* const d;
	void finalizeImport();
};


} // namespace

#endif /* PROGRESSPAGE_H */
