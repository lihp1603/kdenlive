/***************************************************************************
 *   Copyright (C) 2007 by Jean-Baptiste Mardelle (jb@kdenlive.org)        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDockWidget>
#include <QUndoView>
#include <QProgressBar>
#include <QEvent>
#include <QShortcut>
#include <QMap>
#include <QString>
#include <QImage>

#include <KXmlGuiWindow>
#include <QTabWidget>
#include <kautosavefile.h>
#include <KActionCategory>
#include <KSelectAction>
#include <KColorSchemeManager>

#include "kdenlivecore_export.h"
#include "effectslist/effectslist.h"
#include "gentime.h"
#include "definitions.h"
#include "statusbarmessagelabel.h"
#include "dvdwizard/dvdwizard.h"
#include "stopmotion/stopmotion.h"

class KdenliveDoc;
class TrackView;
class ProjectList;
class EffectsListView;
class EffectStackView;
class EffectStackView2;
class TransitionSettings;
class Monitor;
class RecMonitor;
class RenderWidget;
class DocClipBase;
class Render;
class Transition;
class KActionCollection;

class /*KDENLIVECORE_EXPORT*/ MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kdenlive.MainWindow")

public:

    /** @brief Initialises the main window.
     * @param MltPath (optional) path to MLT environment
     * @param Url (optional) file to open
     * @param clipsToLoad (optional) a comma separated list of clips to import in project
     *
     * If Url is present, it will be opened, otherwhise, if openlastproject is
     * set, latest project will be opened. If no file is open after trying this,
     * a default new file will be created. */
    explicit MainWindow(const QString &MltPath = QString(),
                        const QUrl &Url = QUrl(), const QString & clipsToLoad = QString(), QWidget *parent = 0);
    virtual ~MainWindow();

    static EffectsList videoEffects;
    static EffectsList audioEffects;
    static EffectsList customEffects;
    static EffectsList transitions;
    
    /** @brief Cache for luma files thumbnails. */
    static QMap <QString,QImage> m_lumacache;

    /** @brief Adds an action to the action collection and stores the name. */
    void addAction(const QString &name, QAction *action);
    /** @brief Adds an action to the action collection and stores the name. */
    QAction *addAction(const QString &name, const QString &text, const QObject *receiver,
                       const char *member, const QIcon &icon = QIcon(), const QKeySequence &shortcut = QKeySequence());

    /**
     * @brief Adds a new dock widget to this window.
     * @param title title of the dock widget
     * @param objectName objectName of the dock widget (required for storing layouts)
     * @param widget widget to use in the dock
     * @param area area to which the dock should be added to
     * @returns the created dock widget
     */
    QDockWidget *addDock(const QString &title, const QString &objectName, QWidget *widget, Qt::DockWidgetArea area = Qt::TopDockWidgetArea);

    // TODO make private again
    QTabWidget* m_timelineArea;
    ProjectList *m_projectList;
    StopmotionWidget *m_stopmotion;
    QUndoGroup *m_commandStack;
    KActionCollection *m_tracksActionCollection;
    EffectStackView2 *m_effectStack;
    TransitionSettings *m_transitionConfig;
    QUndoView *m_undoView;
    StatusBarMessageLabel *m_messageLabel;

protected:

    /** @brief Closes the window.
     * @return false if the user presses "Cancel" on a confirmation dialog or
     *     the operation requested (starting waiting jobs or saving file) fails,
     *     true otherwise */
    virtual bool queryClose();

    /** @brief Reports a message in the status bar when an error occurs. */
    virtual void customEvent(QEvent *e);

    /** @brief Stops the active monitor when the window gets hidden. */
    virtual void hideEvent(QHideEvent *e);

    /** @brief Saves the file and the window properties when saving the session. */
    virtual void saveProperties(KConfigGroup &config);

    /** @brief Saves the file and the window properties when saving the session. */
    virtual void saveGlobalProperties(KConfigGroup &config);    
    
    /** @brief Restores the window and the file when a session is loaded. */
    virtual void readProperties(const KConfigGroup &config);

    /** @brief Restores the window and the file when a session is loaded. */
    virtual void readGlobalProperties(const KConfigGroup &config);

private:
    QProgressBar *m_statusProgressBar;

    /** @brief Sets up all the actions and attaches them to the collection. */
    void setupActions();
    
    KColorSchemeManager *m_colorschemes;
    
    KActionMenu *m_themesMenu;

    QDockWidget *m_projectListDock;

    QDockWidget *m_effectListDock;
    EffectsListView *m_effectList;

    QDockWidget *m_effectStackDock;

    QDockWidget *m_transitionConfigDock;

    QDockWidget *m_clipMonitorDock;
    Monitor *m_clipMonitor;

    QDockWidget *m_projectMonitorDock;
    Monitor *m_projectMonitor;

    QDockWidget *m_recMonitorDock;
    RecMonitor *m_recMonitor;

    QDockWidget *m_undoViewDock;

    KSelectAction *m_timeFormatButton;

    /** This list holds all the scopes used in Kdenlive, allowing to manage some global settings */
    QList <QDockWidget *> m_gfxScopesList;

    KActionCategory *m_effectActions;
    QMenu *m_effectsMenu;
    QMenu *m_transitionsMenu;
    QMenu *m_timelineContextMenu;
    QMenu *m_timelineContextClipMenu;
    QMenu *m_timelineContextTransitionMenu;

    /** Actions used in the stopmotion widget */
    KActionCategory *m_stopmotion_actions;

    /** Action names that can be used in the slotDoAction() slot, with their i18n() names */
    QStringList m_actionNames;


    /** @brief Shortcut to remove the focus from any element.
     *
     * It allows to get out of e.g. text input fields and to press another
     * shortcut. */
    QShortcut* m_shortcutRemoveFocus;

    RenderWidget *m_renderWidget;

    QAction *m_fileRevert;

    QAction **m_transitions;
    QAction *m_buttonAudioThumbs;
    QAction *m_buttonVideoThumbs;
    QAction *m_buttonShowMarkers;
    QAction *m_buttonFitZoom;
    QAction *m_buttonAutomaticSplitAudio;
    QAction *m_normalEditTool;
    QAction *m_overwriteEditTool;
    QAction *m_insertEditTool;
    QAction *m_buttonSelectTool;
    QAction *m_buttonRazorTool;
    QAction *m_buttonSpacerTool;
    QAction *m_buttonSnap;
    QAction *m_saveAction;
    QSlider *m_zoomSlider;
    QAction *m_zoomIn;
    QAction *m_zoomOut;
    QAction *m_loopZone;
    QAction *m_playZone;
    QAction *m_loopClip;
    QActionGroup *m_clipTypeGroup;
    KActionCollection *m_effectsActionCollection;


    void readOptions();
    void saveOptions();

    /** @brief Loads static and dynamic plugins.
     *
     * It scans static plugins as well as the filesystem: it doesn't load more
     * than one plugin per file name, to avoid duplication due to conflicting
     * installations. */
    void loadPlugins();
    void populateMenus(QObject *plugin);
    void addToMenu(QObject *plugin, const QStringList &texts,
                   QMenu *menu, const char *member,
                   QActionGroup *actionGroup);

    /** @brief Instantiates a "Get Hot New Stuff" dialog.
     * @param configFile configuration file for KNewStuff
     * @return number of installed items */
    int getNewStuff(const QString &configFile = QString());
    QStringList m_pluginFileNames;
    QByteArray m_timelineState;
    void loadTranscoders();
    void loadClipActions();

    QTime m_timer;
    /** @brief The last selected clip in timeline. */
    ClipItem *m_mainClip;
    /** @brief Update statusbar stylesheet (in case of color theme change). */
    void setStatusBarStyleSheet(const QPalette &p);

public slots:
    void slotGotProgressInfo(const QString &message, int progress, MessageType type = DefaultMessage);
    void slotReloadEffects();
    Q_SCRIPTABLE void setRenderingProgress(const QString &url, int progress);
    Q_SCRIPTABLE void setRenderingFinished(const QString &url, int status, const QString &error);

    void slotSwitchVideoThumbs();
    void slotSwitchAudioThumbs();

    void slotPreferences(int page = -1, int option = -1);
    void connectDocument();
    void slotTimelineClipSelected(ClipItem* item, bool raise = true);

private slots:
    /** @brief Shows the shortcut dialog. */
    void slotEditKeys();

    /** @brief Reflects setting changes to the GUI. */
    void updateConfiguration();
    void slotConnectMonitors();
    void slotUpdateClip(const QString &id);
    void slotUpdateMousePosition(int pos);
    void slotUpdateProjectDuration(int pos);
    void slotAddEffect(const QDomElement &effect);
    void slotEditProfiles();
    void slotEditProjectSettings();
    /** @brief Change current document MLT profile. */
    void slotUpdateProjectProfile(const QString &profile);
    void slotDisplayActionMessage(QAction *a);

    /** @brief Turns automatic splitting of audio and video on/off. */
    void slotSwitchSplitAudio();
    void slotSwitchMarkersComments();
    void slotSwitchSnap();
    void slotRenderProject();
    void slotFullScreen();
    void slotUpdateDocumentState(bool modified);

    /** @brief Sets the timeline zoom slider to @param value.
    *
    * Also disables zoomIn and zoomOut actions if they cannot be used at the moment. */
    void slotSetZoom(int value);
    /** @brief Decreases the timeline zoom level by 1. */
    void slotZoomIn();
    /** @brief Increases the timeline zoom level by 1. */
    void slotZoomOut();
    /** @brief Makes the timeline zoom level fit the timeline content. */
    void slotFitZoom();
    /** @brief Updates the zoom slider tooltip to fit @param zoomlevel. */
    void slotUpdateZoomSliderToolTip(int zoomlevel);

    /** @brief Displays the zoom slider tooltip.
    * @param zoomlevel (optional) The zoom level to show in the tooltip.
    *
    * Adopted from Dolphin (src/statusbar/dolphinstatusbar.cpp) */
    void slotShowZoomSliderToolTip(int zoomlevel = -1);
    /** @brief Deletes item in timeline, project tree or effect stack depending on focus. */
    void slotDeleteItem();
    void slotAddClipMarker();
    void slotDeleteClipMarker();
    void slotDeleteAllClipMarkers();
    void slotEditClipMarker();

    /** @brief Adds marker or auide at the current position without showing the marker dialog.
     *
     * Adds a marker if clip monitor is active, otherwise a guide.
     * The comment is set to the current position (therefore not dialog).
     * This can be useful to mark something during playback. */
    void slotAddMarkerGuideQuickly();
    void slotCutTimelineClip();
    void slotInsertClipOverwrite();
    void slotSelectTimelineClip();
    void slotSelectTimelineTransition();
    void slotDeselectTimelineClip();
    void slotDeselectTimelineTransition();
    void slotSelectAddTimelineClip();
    void slotSelectAddTimelineTransition();
    void slotAddVideoEffect(QAction *result);
    void slotAddTransition(QAction *result);
    void slotAddProjectClip(const QUrl &url, const stringMap &data = stringMap());
    void slotAddProjectClipList(const QList<QUrl> &urls);
    void slotTrackSelected(int index, const TrackInfo &info, bool raise = true);
    void slotActivateTransitionView(Transition *transition);
    void slotChangeTool(QAction * action);
    void slotChangeEdit(QAction * action);
    void slotSetTool(ProjectTool tool);
    void slotSnapForward();
    void slotSnapRewind();
    void slotClipStart();
    void slotClipEnd();
    void slotSelectClipInTimeline();
    void slotClipInTimeline(const QString &clipId);

    void slotInsertSpace();
    void slotRemoveSpace();
    void slotAddGuide();
    void slotEditGuide();
    void slotDeleteGuide();
    void slotDeleteAllGuides();
    void slotGuidesUpdated();

    void slotCopy();
    void slotPaste();
    void slotPasteEffects();

    void slotAdjustClipMonitor();
    void slotAdjustProjectMonitor();
    void slotSaveZone(Render *render, const QPoint &zone, DocClipBase *baseClip = NULL, QUrl path = QUrl());

    void slotResizeItemStart();
    void slotResizeItemEnd();
    void configureNotifications();
    void slotInsertTrack(int ix = -1);
    void slotDeleteTrack(int ix = -1);
    /** @brief Shows the configure tracks dialog and updates transitions afterwards. */
    void slotConfigTrack(int ix = -1);
    /** @brief Select all clips in active track. */
    void slotSelectTrack();
    /** @brief Select all clips in timeline. */
    void slotSelectAllTracks();
    void slotGetNewLumaStuff();
    void slotGetNewTitleStuff();
    void slotGetNewRenderStuff();
    void slotGetNewMltProfileStuff();
    void slotAutoTransition();
    void slotRunWizard();
    /** @brief Lets the sampleplugin create a generator.  */
    void generateClip();
    void slotZoneMoved(int start, int end);
    void slotDvdWizard(const QString &url = QString());
    void slotGroupClips();
    void slotUnGroupClips();
    void slotEditItemDuration();
    void slotClipInProjectTree();
    //void slotClipToProjectTree();
    void slotSplitAudio();
    void slotSetAudioAlignReference();
    void slotAlignAudio();
    void slotUpdateClipType(QAction *action);
    void slotShowTimeline(bool show);
    void slotTranscode(const QStringList &urls = QStringList());
    void slotStartClipAction();
    void slotTranscodeClip();
    /** @brief Archive project: creates a copy of the project file with all clips in a new folder. */
    void slotArchiveProject();
    void slotSetDocumentRenderProfile(const QMap<QString, QString> &props);
    void slotPrepareRendering(bool scriptExport, bool zoneOnly, const QString &chapterFile);

    /** @brief Switches between displaying frames or timecode.
    * @param ix 0 = display timecode, 1 = display frames. */
    void slotUpdateTimecodeFormat(int ix);

    /** @brief Removes the focus of anything. */
    void slotRemoveFocus();
    void slotCleanProject();
    void slotUpdateClipMarkers(DocClipBase *clip);
    void slotShutdown();
    void slotUpdateTrackInfo();

    /** @brief Changes the color scheme. */
    void slotChangePalette(QAction *action);
    void slotSwitchMonitors();
    void slotCheckRenderStatus();
    void slotInsertZoneToTree();
    void slotInsertZoneToTimeline();

    /** @brief Deletes items from timeline and document.
    * @param ids The ids of the clips to delete.
    * @param folderids The names and ids of the folders to delete. */
    void slotDeleteProjectClips(const QStringList &ids, const QMap<QString, QString> &folderids);
    /** @brief Update the capture folder if user asked a change. */
    void slotUpdateCaptureFolder();

    /** @brief Delete a clip from current project */
    void slotDeleteClip(const QString &id);

    /** @brief The monitor informs that it needs (or not) to have frames sent by the renderer. */
    void slotMonitorRequestRenderFrame(bool request);
    /** @brief Open the stopmotion dialog. */
    void slotOpenStopmotion();
    /** @brief Update project because the use of proxy clips was enabled / disabled. */
    void slotUpdateProxySettings();
    /** @brief Disable proxies for this project. */
    void slotDisableProxies();

    void slotElapsedTime();
    /** @brief Open the online services search dialog. */
    void slotDownloadResources();

    /** @brief Save current timeline clip as mlt playlist. */
    void slotSaveTimelineClip();
    /** @brief Process keyframe data sent from a clip to effect / transition stack. */
    void slotProcessImportKeyframes(GraphicsRectItem type, const QString& data, int maximum);
    /** @brief Move playhead to mouse curser position if defined key is pressed */
    void slotAlignPlayheadToMousePos();

signals:
    Q_SCRIPTABLE void abortRenderJob(const QString &url);
    void configurationChanged();
    void GUISetupDone();
};


#endif