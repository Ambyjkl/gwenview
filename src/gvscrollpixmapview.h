/*
Gwenview - A simple image viewer for KDE
Copyright (c) 2000-2003 Aur�lien G�teau

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#ifndef GVSCROLLPIXMAPVIEW_H
#define GVSCROLLPIXMAPVIEW_H

// Qt includes
#include <qmap.h>
#include <qscrollview.h>

class QLabel;
class QMouseEvent;
class QPainter;
class QTimer;
class QWheelEvent;
class QPopupMenu;
class KAction;
class KActionCollection;
class KConfig;
class KToggleAction;

class GVPixmap;

class GVScrollPixmapView : public QScrollView {
Q_OBJECT
public:
	enum WheelBehaviour { None, Browse, Scroll, Zoom };
	typedef QMap<ButtonState,WheelBehaviour> WheelBehaviours;

	GVScrollPixmapView(QWidget* parent,GVPixmap*,KActionCollection*);
	void readConfig(KConfig* config, const QString& group);
	void writeConfig(KConfig* config, const QString& group) const;

	// Properties
	KToggleAction* autoZoom() const { return mAutoZoom; }
	KAction* zoomIn() const { return mZoomIn; }
	KAction* zoomOut() const { return mZoomOut; }
	KAction* resetZoom() const { return mResetZoom; }
	KToggleAction* lockZoom() const { return mLockZoom; }
	double zoom() const { return mZoom; }
	void setZoom(double zoom);
	void setFullScreen(bool);
	bool showPathInFullScreen() const { return mShowPathInFullScreen; }
	void setShowPathInFullScreen(bool);
	bool smoothScale() const { return mSmoothScale; }
	void setSmoothScale(bool);
	bool enlargeSmallImages() const { return mEnlargeSmallImages; }
	void setEnlargeSmallImages(bool);
	void setShowScrollBars(bool);
	bool showScrollBars() const { return mShowScrollBars; }
	WheelBehaviours& wheelBehaviours() { return mWheelBehaviours; }


public slots:
	// File operations
	void showFileProperties();
	void openWithEditor();
	void renameFile();
	void copyFile();
	void moveFile();
	void deleteFile();

signals:
	void selectPrevious();
	void selectNext();
	void zoomChanged(double);

private:
	GVPixmap* mGVPixmap;
	QTimer* mAutoHideTimer;
	QLabel* mPathLabel;
	
	bool mShowPathInFullScreen;
	bool mSmoothScale;
	bool mEnlargeSmallImages;
	bool mShowScrollBars;
	WheelBehaviours mWheelBehaviours;
	QCursor mDrag,mDragging;

	// Offset to center images
	int mXOffset,mYOffset;
	QTime mLastWheelZoomTime;

	// Zoom info
	double mZoom;

	// point in image that should be centered as % of width and height
	bool mMouseZoomAction;
	double mPictX, mPictY;
	int mViewX, mViewY;

	// context menu
	QPopupMenu* mMenu;

	// Our actions
	KToggleAction* mAutoZoom;
	KAction* mZoomIn;
	KAction* mZoomOut;
	KAction* mResetZoom;
	KToggleAction* mLockZoom;
	KActionCollection* mActionCollection;

	// Object state info
	bool mFullScreen;
	bool mMouseToggleZoom; // Indicates that the user wants to toggle the zoom
	bool mDragStarted; // Indicates that the user is scrolling the image by dragging it
	int mScrollStartX,mScrollStartY;
	bool mOperaLikePrevious; // Flag to avoid showing the popup menu on Opera like previous
	double mLastZoomBeforeAuto;

	double computeAutoZoom();
	void updateScrollBarMode();
	void updateImageOffset();
	void updateContentSize();
	void makeContextMenu();
	void updatePathLabel();
	void updateZoomActions();

private slots:
	void slotURLChanged();
	void slotModified();
	void slotZoomIn();
	void slotZoomOut();
	void slotResetZoom();
	void setAutoZoom(bool);
	void hideCursor();

	
protected:
	// Overloaded methods
	void viewportMousePressEvent(QMouseEvent*);
	void viewportMouseMoveEvent(QMouseEvent*);
	void viewportMouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent* event);
	void resizeEvent(QResizeEvent* event);
	void drawContents(QPainter* p,int clipx,int clipy,int clipw,int cliph);
};


#endif
