/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <aurelien.gateau@free.fr>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#ifndef SIDEBAR_H
#define SIDEBAR_H

// Qt
#include <QFrame>
#include <QTabWidget>

class KIcon;

namespace Gwenview {

class SideBar;

class SideBarGroupPrivate;
class SideBarGroup : public QFrame {
	Q_OBJECT
public:
	SideBarGroup(const QString& title);
	~SideBarGroup();

	void addWidget(QWidget*);
	void addAction(QAction*);
	void clear();

protected:
	virtual void paintEvent(QPaintEvent*);

private:
	SideBarGroupPrivate* const d;
};


class SideBarPagePrivate;
class SideBarPage : public QWidget {
	Q_OBJECT
public:
	SideBarPage(const QString& title, const QString& iconName);
	void addWidget(QWidget*);
	void addStretch();

	const QString& title() const;
	const KIcon& icon() const;

private:
	SideBarPagePrivate* const d;
};


class SideBarPrivate;
class SideBar : public QTabWidget {
	Q_OBJECT
public:
	SideBar(QWidget* parent);
	~SideBar();

	void addPage(SideBarPage*);

	virtual QSize sizeHint() const;

private:
	SideBarPrivate* const d;
};

} // namespace

#endif /* SIDEBAR_H */
