#include "mainwindow.h"
#include <QDockWidget>
#include "librarywidget.h"
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include "iconlibrary.h"
#include <stack>
//#include "libnetwork.h"
#include <QProgressDialog>
#include <QFile>
#include <string>
#include <QTextEdit>
#include "newprojectdialog.h"
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QSettings>
#include <QProcess>
#include <QMdiArea>
#include "fileassociationsdialog.h"
#include "textedit.h"
#include <QTextBlock>
#include <QCloseEvent>
#include "savechangesdialog.h"
#include "playersettingsdialog.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "gotolinedialog.h"
#include "giderosnetworkclient2.h"
#include <previewwidget.h>
#include <QProgressDialog>
#include <QThread>
#include <QTextStream>
#include <bytebuffer.h>
#include <platformutil.h>
#include "findinfilesdialog.h"
#include <QDesktopServices>
#include <QUrl>
#include "exportprojectdialog.h"
#include "startpagewidget2.h"
#include "aboutdialog.h"
#include <QSplitter>
#include <QImage>
#include "mdiarea.h"
#include <licensemanager.h>
#include <QDateTime>
#include "countly.h"

extern Countly *g_countly;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

	mdiArea_ = new MdiArea(this);

/*	mdiArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea_->setViewMode(QMdiArea::TabbedView);
	mdiArea_->setDocumentMode(true); */

//	mdiArea_->setTabShape(QTabWidget::Triangular);

	Q_FOREACH (QTabBar* tab, mdiArea_->findChildren<QTabBar *>())
	{
		tab->setTabsClosable(true);
		tab->setExpanding(false);
		tab->setDocumentMode(true);
		tab->setMovable(true);
		connect(tab, SIGNAL(tabCloseRequested(int)),
				this, SLOT(closeMdiTab(int)));
	}

	ui.mainToolBar->setIconSize(QSize(16, 16));

	ui.actionNew->setIcon(IconLibrary::instance().icon(0, "new"));
	ui.actionOpen->setIcon(IconLibrary::instance().icon(0, "open"));

	ui.actionSave->setIcon(IconLibrary::instance().icon(0, "save"));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(save()));

	ui.actionSave_All->setIcon(IconLibrary::instance().icon(0, "save all"));
	connect(ui.actionSave_All, SIGNAL(triggered()), this, SLOT(saveAll()));

	ui.actionUndo->setIcon(IconLibrary::instance().icon(0, "undo"));
	ui.actionRedo->setIcon(IconLibrary::instance().icon(0, "redo"));

	ui.actionCut->setIcon(IconLibrary::instance().icon(0, "cut"));
	ui.actionCopy->setIcon(IconLibrary::instance().icon(0, "copy"));
	ui.actionPaste->setIcon(IconLibrary::instance().icon(0, "paste"));

	ui.actionToggle_Bookmark->setIcon(IconLibrary::instance().icon(0, "toggle bookmark"));
	ui.actionNext_Bookmark->setIcon(IconLibrary::instance().icon(0, "next bookmark"));
	ui.actionPrevious_Bookmark->setIcon(IconLibrary::instance().icon(0, "previous bookmark"));
	ui.actionClear_Bookmarks->setIcon(IconLibrary::instance().icon(0, "clear bookmarks"));

	ui.actionStart_Player->setIcon(IconLibrary::instance().icon(0, "gamepad"));
	connect(ui.actionStart_Player, SIGNAL(triggered()), this, SLOT(startPlayer()));

	ui.actionStart->setIcon(IconLibrary::instance().icon(0, "start"));
	ui.actionStart->setEnabled(false);
	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));

	ui.actionStop->setIcon(IconLibrary::instance().icon(0, "stop"));
	ui.actionStop->setEnabled(false);
	connect(ui.actionStop, SIGNAL(triggered()), this, SLOT(stop()));

    connect(ui.actionCheck_Syntax, SIGNAL(triggered()), this, SLOT(compile()));
    connect(ui.actionCheck_Syntax_All, SIGNAL(triggered()), this, SLOT(compileAll()));
	connect(ui.actionCancel, SIGNAL(triggered()), this, SLOT(cancel()));

#if 0
	setCentralWidget(mdiArea_);

	{
		libraryDock_ = new QDockWidget(tr("Library[*]"), this);
		libraryDock_->setObjectName("library");
		libraryWidget_ = new LibraryWidget(libraryDock_);
		libraryDock_->setWidget(libraryWidget_);
		addDockWidget(Qt::RightDockWidgetArea, libraryDock_);
		connect(libraryWidget_, SIGNAL(modificationChanged(bool)), libraryDock_, SLOT(setWindowModified(bool)));
	}

	{
		outputDock_ = new QDockWidget(tr("Output"), this);
		outputDock_->setObjectName("output");
		outputWidget_ = new QTextEditEx(outputDock_);
		connect(outputWidget_, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(outputMouseDoubleClick(QMouseEvent*)));
		outputWidget_->setReadOnly(true);
		outputDock_->setWidget(outputWidget_);
		addDockWidget(Qt::BottomDockWidgetArea, outputDock_);
	}


	{
		previewDock_ = new QDockWidget(tr("Preview"), this);
		previewDock_->setObjectName("preview");
		previewWidget_ = new PreviewWidget(previewDock_);
		previewDock_->setWidget(previewWidget_);
		addDockWidget(Qt::RightDockWidgetArea, previewDock_);
	}
#else

	libraryDock_ = NULL;
	outputDock_ = NULL;
	previewDock_ = NULL;

	libraryWidget_ = new LibraryWidget;

	outputWidget_ = new QTextEditEx;
	outputWidget_->setReadOnly(true);
	connect(outputWidget_, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(outputMouseDoubleClick(QMouseEvent*)));


	QWidget* outputContainer;
	{
		outputContainer = new QWidget;
		outputContainer->setLayout(new QVBoxLayout);
		outputContainer->layout()->setMargin(0);
		outputContainer->layout()->setSpacing(0);

		QLabel* label = new QLabel("Output");
		label->setMargin(2);
		label->setStyleSheet(
			"border: 1px solid #AAAAAA;"
			"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FCFCFC, stop: 1 #E2E2E2);"
		);
		outputContainer->layout()->addWidget(label);
		outputContainer->layout()->addWidget(outputWidget_);

		outputWidget_->setLineWidth(0);
		outputWidget_->setMidLineWidth(0);
		outputWidget_->setFrameShape(QFrame::NoFrame);
		outputWidget_->setFrameShadow(QFrame::Plain);
	}

	previewWidget_ = new PreviewWidget;

	splitter1_ = new QSplitter(Qt::Vertical);
	splitter2_ = new QSplitter(Qt::Horizontal);
	splitter3_ = new QSplitter(Qt::Vertical);

	splitter3_->addWidget(libraryWidget_);
	splitter3_->addWidget(previewWidget_);

	splitter2_->addWidget(splitter3_);
	splitter2_->addWidget(mdiArea_);

	splitter1_->addWidget(splitter2_);
	splitter1_->addWidget(outputContainer);

	splitter3_->setSizes(QList<int>() << 200 << 200);

	setCentralWidget(splitter1_);
#endif

	QSettings settings;
	QString playerip = settings.value("player ip", QString("127.0.0.1")).toString();
	
#ifndef NEW_CLIENT
	client_ = new Client(qPrintable(playerip), 15000);
#else
	client_ = new GiderosNetworkClient(playerip, 15000);

	connect(client_, SIGNAL(connected()), this, SLOT(connected()));
	connect(client_, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(client_, SIGNAL(dataReceived(const QByteArray&)), this, SLOT(dataReceived(const QByteArray&)));
	connect(client_, SIGNAL(ackReceived(unsigned int)), this, SLOT(ackReceived(unsigned int)));

#endif

//	startTimer(1);

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(30);

	QTimer::singleShot(1, this, SLOT(onSingleShot())); 

	connect(ui.actionNew_Project, SIGNAL(triggered()), this, SLOT(newProject()));
	connect(ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(closeProject()));
	connect(ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(showStartPage()));
	connect(ui.actionSave_Project, SIGNAL(triggered()), this, SLOT(saveProject()));
	connect(ui.actionOpen_Project, SIGNAL(triggered()), this, SLOT(openProject()));
	connect(ui.actionFile_Associations, SIGNAL(triggered()), this, SLOT(fileAssociations()));
	connect(ui.actionExport_Project, SIGNAL(triggered()), this, SLOT(exportProject()));
	connect(ui.actionExport_Pack, SIGNAL(triggered()), this, SLOT(exportPack()));

//	connect(libraryWidget_, SIGNAL(modified()), this, SLOT(onModified()));
	connect(libraryWidget_, SIGNAL(openRequest(const QString&, const QString&)), this, SLOT(onOpenRequest(const QString&, const QString&)));
	connect(libraryWidget_, SIGNAL(previewRequest(const QString&, const QString&)), this, SLOT(onPreviewRequest(const QString&, const QString&)));
	connect(libraryWidget_, SIGNAL(insertIntoDocument(const QString&)), this, SLOT(onInsertIntoDocument(const QString&)));
	connect(libraryWidget_, SIGNAL(automaticDownsizingEnabled(const QString&)), this, SLOT(downsize(const QString&)));

#if 0
	compileThread_ = NULL;
#endif

	connect(ui.actionProject1, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject2, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject3, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject4, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject5, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject6, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject7, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject8, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject9, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject10, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject11, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject12, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject13, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject14, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject15, SIGNAL(triggered()), this, SLOT(openRecentProject()));

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));


	connect(ui.actionPlayer_Settings, SIGNAL(triggered()), this, SLOT(playerSettings()));

	connect(ui.actionAbout_Gideros_Studio, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
	connect(ui.actionDeveloper_Center, SIGNAL(triggered()), this, SLOT(developerCenter()));
	connect(ui.actionHelp_Support, SIGNAL(triggered()), this, SLOT(helpAndSupport()));
	connect(ui.actionAPI_Documentation, SIGNAL(triggered()), this, SLOT(apiDocumentation()));

	connect(mdiArea_, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateUI()));
	connect(mdiArea_, SIGNAL(subWindowActivated(MyMdiSubWindow*)), this, SLOT(updateUI()));
	updateUI();
	setWindowTitle(tr("Gideros"));

	updateRecentProjectActions();

	// find
	{
		findDialog_ = new FindDialog(this);
		connect(ui.actionFind, SIGNAL(triggered()), this, SLOT(find()));
		connect(findDialog_, SIGNAL(findNext()), this, SLOT(findFirst()));
		connect(ui.actionFind_Next, SIGNAL(triggered()), this, SLOT(findNext()));
		connect(ui.actionFind_Previous, SIGNAL(triggered()), this, SLOT(findPrevious()));

		replaceDialog_ = new ReplaceDialog(this);
		connect(ui.actionReplace, SIGNAL(triggered()), this, SLOT(replace()));

		connect(replaceDialog_, SIGNAL(findNext()), this, SLOT(replace_findNext()));
		connect(replaceDialog_, SIGNAL(replace()), this, SLOT(replace_replace()));
		connect(replaceDialog_, SIGNAL(replaceAll()), this, SLOT(replace_replaceAll()));

		findInFilesDialog_ = new FindInFilesDialog(this);
		connect(ui.actionFind_in_Files, SIGNAL(triggered()), this, SLOT(findInFiles()));
	}
	
	// bookmarks
	{
		connect(ui.actionToggle_Bookmark, SIGNAL(triggered()), this, SLOT(toogleBookmark()));
		connect(ui.actionNext_Bookmark, SIGNAL(triggered()), this, SLOT(nextBookmark()));
		connect(ui.actionPrevious_Bookmark, SIGNAL(triggered()), this, SLOT(previousBookmark()));
		connect(ui.actionClear_Bookmarks, SIGNAL(triggered()), this, SLOT(clearBookmarks()));
	}

	// undo-redo
	{
		connect(ui.actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
		connect(ui.actionRedo, SIGNAL(triggered()), this, SLOT(redo()));
	}

	connect(ui.actionGo_To_Line, SIGNAL(triggered()), this, SLOT(goToLine()));

    splitter1_->setSizes(QList<int>() << 550 << 200);
    splitter2_->setSizes(QList<int>() << 200 << 800);

	{
		QSettings settings;
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("windowState").toByteArray());

		splitter1_->restoreState(settings.value("splitter1").toByteArray());
		splitter2_->restoreState(settings.value("splitter2").toByteArray());
		splitter3_->restoreState(settings.value("splitter3").toByteArray());
	}

	connect(ui.actionOutput_Panel, SIGNAL(triggered()), outputDock_, SLOT(show()));
	connect(ui.actionLibrary_Manager, SIGNAL(triggered()), libraryDock_, SLOT(show()));
	connect(ui.actionPreview, SIGNAL(triggered()), previewDock_, SLOT(show()));
//	connect(ui.actionStart_Page, SIGNAL(triggered()), this, SLOT(showStartPage()));

	QList<QVariant> v = settings.value("fileAssociations", QList<QVariant>()).toList();
	for (int i = 0; i < v.size(); ++i)
		fileAssociations_.push_back(v[i].toStringList());

	isTransferring_ = false;

    makeProcess_ = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    toolsDir.cdUp();
#endif
    toolsDir.cd("Tools");

#if defined(Q_OS_WIN)
    env.insert("PATH", toolsDir.path() + ";" + env.value("PATH"));
#else
    env.insert("PATH", toolsDir.path() + ":" + env.value("PATH"));
#endif

    makeProcess_->setProcessEnvironment(env);

    connect(makeProcess_, SIGNAL(readyReadStandardOutput()), this, SLOT(stdoutToOutput()));
    connect(makeProcess_, SIGNAL(readyReadStandardError()), this, SLOT(stderrToOutput()));
    connect(makeProcess_, SIGNAL(started()), this, SLOT(makeStarted()));
    connect(makeProcess_, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(makeFinished()));
}

MainWindow::~MainWindow()
{
	delete client_;
}

void MainWindow::closeMdiTab(int i)
{
	// asagidaki yontemi
	// https://tora.svn.sourceforge.net/svnroot/tora/trunk/tora/src/tomain.cpp dan bulduk
	MdiSubWindow *sub = mdiArea_->subWindowList()[i];

	if (sub != mdiArea_->activeSubWindow())
		mdiArea_->setActiveSubWindow(sub);

	mdiArea_->closeActiveSubWindow();
}

void MainWindow::onSingleShot()
{
	showStartPage();
}

void MainWindow::showStartPage()
{
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		StartPageWidget2* startPage = qobject_cast<StartPageWidget2*>(window);
		if (startPage)
		{
			mdiArea_->setActiveSubWindow(startPage);
			startPage->setFocus(Qt::OtherFocusReason);
			return;
		}
	}

	StartPageWidget2* startPage = new StartPageWidget2;
	connect(startPage, SIGNAL(newProject()), this, SLOT(newProject()));
	connect(startPage, SIGNAL(openProject()), this, SLOT(openProject()));
	connect(startPage, SIGNAL(openProject(const QString&)), this, SLOT(openProject(const QString&)));

//	startPage_->load();
	mdiArea_->addSubWindow(startPage);
	startPage->showMaximized();
}

void MainWindow::hideStartPage()
{
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		StartPageWidget2* startPage = qobject_cast<StartPageWidget2*>(window);
		if (startPage)
		{
			if (startPage != mdiArea_->activeSubWindow())
				mdiArea_->setActiveSubWindow(startPage);
			mdiArea_->closeActiveSubWindow();
			break;
		}
	}
}

void MainWindow::start()
{
	if (client_->isConnected() == false)
		return;

	if (projectFileName_.isEmpty() == true)
		return;

	if (isTransferring_ == true)
		return;

	isTransferring_ = true;

	saveAll();

	// downsize changed images
	{
		std::vector<std::pair<QString, QString> > fileList = updateMD5(true);
		if (!fileList.empty())
		{
			saveMD5();

			QDir path(QFileInfo(projectFileName_).path());
			for (size_t i = 0; i < fileList.size(); ++i)
			{
				QString filename = fileList[i].second;
				QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));
				downsize(absfilename);
			}
		}
	}

	if (!updateMD5().empty())
		saveMD5();

	client_->sendStop();
	client_->sendProjectName(QFileInfo(projectFileName_).baseName());
	client_->sendGetFileList();

    g_countly->projectPlayed();
}

#if 0
void MainWindow::start()
{
// type (first byte)
// 0: folder
// 1; file

	saveAll();
	client_->sendGetFileList();
//	client_->sendDeleteFiles();

	QDir path(QFileInfo(projectFileName_).path());

//	sentMap_.clear();

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	std::vector<QString> dir;

	while (stack.empty() == false)
	{
		QDomNode n = stack.top();
		QDomElement e = n.toElement();
		stack.pop();

		if (n.isNull() == true)
		{
			dir.pop_back();
			continue;
		}

		QString type = e.tagName();

		if (type == "file")
		{
#ifndef NEW_CLIENT
			QString fileName = QDir::cleanPath(path.absoluteFilePath(e.attribute("file")));
			QFile file(fileName);
			if (file.open(QIODevice::ReadOnly) == false)
			{
				printf("cannot open file\n");
			}
			else
			{
				std::string n;
				for (std::size_t i = 0; i < dir.size(); ++i)
					n += qPrintable(dir[i] + "/");
				n += qPrintable(name);

				QByteArray b = file.readAll();

				std::vector<char> buffer(1 + n.size() + 1 + b.size());

				int pos = 0;
				buffer[pos] = 1;								// 1 for file
				pos += 1;
				strcpy(&buffer[pos], n.c_str());				// name
				pos += n.size() + 1;
				if (b.isEmpty() == false)
					memcpy(&buffer[pos], b.constData(), b.size());	// data
				pos += b.size();

				Q_ASSERT(buffer.size() == pos);
				int id = client_->sendData(&buffer[0], buffer.size());
				sentMap_[id] = name;
			}
#else
			QString fileName = e.attribute("file");
			QString name = QFileInfo(fileName).fileName();

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
			n += name;

/*			int id = client_->sendFile(n, fileName);
			if (id == 0)
				printf("cannot open file\n");
			else
				sentMap_[id] = name; */

			fileQueue_.push_back(qMakePair(n, fileName));
#endif
			continue;
		}
		
		if (type == "folder")
		{
			QString name = e.attribute("name");
			dir.push_back(name);

#ifndef NEW_CLIENT
			std::string n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += qPrintable(dir[i] + "/");

			std::vector<char> buffer(1 + n.size() + 1);
			int pos = 0;
			buffer[pos] = 0;						// 0 for folder
			pos += 1;
			strcpy(&buffer[pos], n.c_str());		// name
			pos += n.size() + 1;

			Q_ASSERT(buffer.size() == pos);
			int id = client_->sendData(&buffer[0], buffer.size());
			sentMap_[id] = "";
#else
			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
		
			client_->createFolder(n);

			//sentMap_[client_->createFolder(n)] = "";
#endif

			stack.push(QDomNode());
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}


//	QProgressDialog* p = new QProgressDialog("Sending files...", "Cancel", 0, 100, this);
//	p->exec();

//	if (fileQueue_.empty() == false)			//TODO: bunu birazdan yap
//		ui.actionStart->setEnabled(false);

	std::vector<QString> topologicalSort = libraryWidget_->topologicalSort();
	for (std::size_t i = 0; i < topologicalSort.size(); ++i)
	{
		int index = -1;
		for (std::size_t j = 0; j < fileQueue_.size(); ++j)
		{
			if (fileQueue_[j].second == topologicalSort[i])
			{
				index = j;
				break;
			}
		}
		
		if (index != -1)
		{
			QPair<QString, QString> item = fileQueue_[index];
			fileQueue_.erase(fileQueue_.begin() + index);
			fileQueue_.push_back(item);
		}
	}

	QStringList luaFiles;
	for (int i = 0; i < fileQueue_.size(); ++i)
		if (QFileInfo(fileQueue_[i].first).suffix().toLower() == "lua")
			luaFiles << fileQueue_[i].first;

	if (fileQueue_.empty() == false)
		fileQueue_.push_back(qMakePair(luaFiles.join("|"), QString("play")));
}
#endif

void MainWindow::onTimer()
{
	timerEvent(0);
}

void MainWindow::timerEvent(QTimerEvent*)
{
#ifndef NEW_CLIENT
//	static int i = 0;
//	printf("tick: %d\n", i++);

	int dataTotal = 0;

	while (true)
	{
		int dataSent0 = client_->dataSent();
		int dataReceived0 = client_->dataReceived();

		Event event;
		client_->tick(&event);

//		if (event.eventCode != eNone)
//			printf("%s\n", eventCodeString(event.eventCode));

		int dataSent1 = client_->dataSent();
		int dataReceived1 = client_->dataReceived();

		if (event.eventCode == eOtherSideConnected)
		{
			ui.actionStart->setEnabled(true);
			printf("other side connected\n");
		}
		else if (event.eventCode == eOtherSideClosedConnection)
		{
			ui.actionStart->setEnabled(false);
			printf("other side closed connection\n");
		}
		else if (event.eventCode == eDataSent)
		{
			std::map<int, QString>::iterator iter = sentMap_.find(event.id);
			if (iter != sentMap_.end())
			{
				QString name = iter->second;
				if (name.isEmpty() == false)
				{
					outputWidget_->moveCursor(QTextCursor::End);
					outputWidget_->insertPlainText(iter->second + " is uploaded.\n");
				}
				sentMap_.erase(iter);
				if (sentMap_.empty() == true)
				{
					outputWidget_->moveCursor(QTextCursor::End);
					outputWidget_->insertPlainText("Uploading finished!\n");
					sendPlayMessage();
				}
			}
		}
		else if (event.eventCode == eDataReceived)
		{
			const std::vector<char>& data = event.data;

			if (data[0] == 4)
			{
				std::string str = &data[1];
				outputWidget_->moveCursor(QTextCursor::End);
				outputWidget_->insertPlainText(str.c_str());
			}
		}

		int dataDelta = (dataSent1 - dataSent0) + (dataReceived1 - dataReceived0);
		dataTotal += dataDelta;

		if (dataDelta == 0 || dataTotal > 1024)
			break;
	}
#else
	QDir path(QFileInfo(projectFileName_).path());

	if (client_ && client_->bytesToWrite() == 0)
	{
		if (fileQueue_.empty() == false)
		{
			const QString& s1 = fileQueue_.front().first;
			const QString& s2 = fileQueue_.front().second;

			if (s2 == "play")
			{
				QStringList luafiles = s1.split("|");	
				outputWidget_->append("Uploading finished.\n");
				client_->sendProjectProperties(libraryWidget_->getProjectProperties());
				sendPlayMessage(luafiles);
				isTransferring_ = false;
			}
			else
			{
				// create remote directories
				{
					QStringList paths = s1.split("/");
					QStringList dir;
					for (int i = 0; i < paths.size() - 1; ++i)
					{
						dir << paths[i];
						client_->sendCreateFolder(dir.join("/"));
					}
				}

				QString fileName = QDir::cleanPath(path.absoluteFilePath(s2));
				if (client_->sendFile(s1, fileName) == 0)
				{
					outputWidget_->append(s1 + " cannot be opened.\n");
				}
				else
				{
					outputWidget_->append(s1 + " is uploading.\n");
				}
			}

			fileQueue_.pop_front();
		}
	}
#endif
}

unsigned int MainWindow::sendPlayMessage(const QStringList& luafiles)
{
#ifndef NEW_CLIENT
	char play = 2;
	client_->sendData(&play, sizeof(char));
#else
	return client_->sendPlay(luafiles);
#endif
}

unsigned int MainWindow::sendStopMessage()
{
#ifndef NEW_CLIENT
	char stop = 3;
	client_->sendData(&stop, sizeof(char));
#else
	return client_->sendStop();
#endif
}

void MainWindow::stop()
{
	fileQueue_.clear();
	sendStopMessage();
}

/*
void MainWindow::enableUI()
{
	libraryWidget_->setEnabled(true);
	outputWidget_->setEnabled(true);
	ui.actionClose_Project->setEnabled(true);
	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
//	setWindowModified(false);
}
*/
/*
void MainWindow::disableUI()
{
	libraryWidget_->setEnabled(false);
	outputWidget_->setEnabled(false);
	ui.actionClose_Project->setEnabled(false);
	setWindowTitle(tr("Gideros"));
//	setWindowModified(false);
}
*/
void MainWindow::newProject()
{
	if (maybeSave())
	{
		NewProjectDialog* newProjectDialog = new NewProjectDialog(this);
		if (newProjectDialog->exec() == QDialog::Accepted)
		{
			// try to create directory
			QString dir = newProjectDialog->fullDirectory();
			if (QDir().mkpath(dir) == false)
			{
				QMessageBox::information(this, "Information", "Could not create the directory: " + dir);
				return;
			}

			// try to create project
			QString fileName = newProjectDialog->fullName();
			QFile file(fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
			{
				QMessageBox::information(this, "Information", "Could not create the project: " + fileName);
				return;
			}

			closeProject();

			libraryWidget_->newProject(fileName);

			QTextStream(&file) << libraryWidget_->toXml().toString();

			file.close();

			hideStartPage();

			projectFileName_ = fileName;
			md5_.clear();

			updateUI();

			setWindowTitle(projectName() + "[*] - " + tr("Gideros"));

			QSettings settings;
			settings.setValue("location", newProjectDialog->location());

			addToRecentProjects(projectFileName_);

            g_countly->projectCreated();
		}
	}
}

void MainWindow::closeProject()
{
	if (maybeSave())
	{
		storeOpenFiles();

		projectFileName_ = "";
		md5_.clear();

		libraryWidget_->clear();
		outputWidget_->clear();
		previewWidget_->setFileName("", "");

		foreach (MdiSubWindow* window, mdiArea_->subWindowList())
		{
			if (window != mdiArea_->activeSubWindow())
				mdiArea_->setActiveSubWindow(window);

			mdiArea_->closeActiveSubWindow();
		}

		updateUI();

		setWindowTitle(tr("Gideros"));
	}
}

void MainWindow::saveProject()
{
	QFile file(projectFileName_);

	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		QMessageBox::information(this, "Information", "Could not save the project file: " + projectFileName_);
		return;
	}

	QTextStream(&file) << libraryWidget_->toXml().toString();
	file.close();
	libraryWidget_->setModified(false);
}

void MainWindow::openProject()
{
	if (maybeSave())
	{
		QSettings settings;

		QString dir = settings.value("lastOpenProjectDir", QString()).toString();

		QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
			dir,
			tr("Gideros Project (*.gproj)"));

		if (fileName.isEmpty() == true)
			return;

		QDir newdir = QFileInfo(fileName).absoluteDir();
		newdir.cdUp();
		settings.setValue("lastOpenProjectDir", newdir.absolutePath());

		openProject(fileName);
	}
}


void MainWindow::openProject(const QString& fileName)
{
	QDomDocument doc;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::information(this, "Information", "Could not open the project file: " + fileName);
		return;
	}

	if (!doc.setContent(&file))
	{
		QMessageBox::information(this, "Information", "Could not parse the project file: " + fileName);
		file.close();
		return;
	}
	file.close();

	hideStartPage();

	closeProject();

	projectFileName_ = fileName;
	loadMD5();

	libraryWidget_->loadXml(projectFileName_, doc);

	updateUI();
	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
	addToRecentProjects(projectFileName_);

	restoreOpenFiles();
}

QString MainWindow::projectName() const
{
	return QDir(projectFileName_).dirName();
}

QString MainWindow::projectDirectory() const
{
	return QDir::cleanPath(QDir(projectFileName_).absoluteFilePath(".."));
}

/*
void MainWindow::onModified()
{
	setWindowModified(true);
}
*/

void MainWindow::startPlayer()
{
#if defined(Q_OS_MAC)
        QProcess::startDetached("open \"../../Gideros Player.app\"");
#elif defined(Q_OS_WIN)
	QProcess::startDetached("GiderosPlayer.exe");
#else
	QProcess::startDetached("GiderosPlayer");
#endif
}

void MainWindow::fileAssociations()
{
	FileAssociationsDialog dialog(fileAssociations_, this);

	if (dialog.exec() == QDialog::Accepted)
	{
		fileAssociations_ = dialog.fileAssociations();

		QList<QVariant> v;
		for (int i = 0; i < fileAssociations_.size(); ++i)
			v.push_back(fileAssociations_[i]);

		QSettings settings;
		settings.setValue("fileAssociations", v);
	}
}

TextEdit* MainWindow::createTextEdit()
{
	TextEdit* textEdit = new TextEdit;
	connect(textEdit, SIGNAL(copyAvailable(bool)), this, SLOT(updateUI()));
	connect(textEdit, SIGNAL(textChanged()), this, SLOT(updateUI()));
	return textEdit;
}

TextEdit* MainWindow::findTextEdit(const QString& fileName) const
{
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);
		if (textEdit && textEdit->fileName() == fileName)
			return textEdit;
	}

	return 0;
}

void MainWindow::onOpenRequest(const QString& itemName, const QString& fileName)
{
	QString suffix = QFileInfo(fileName).suffix().toLower();

	if (suffix == "txt" || suffix == "lua")
		openFile(fileName);
	else
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::updateUI()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	bool hasMdiChild = textEdit != NULL;
	bool hasSelectedText = textEdit && textEdit->hasSelectedText();
	bool isRedoAvailable = textEdit && textEdit->isRedoAvailable();
	bool isUndoAvailable = textEdit && textEdit->isUndoAvailable();

	ui.actionCut->setEnabled(hasSelectedText);
	ui.actionCopy->setEnabled(hasSelectedText);
	ui.actionPaste->setEnabled(hasMdiChild);

	ui.actionUndo->setEnabled(isUndoAvailable);
	ui.actionRedo->setEnabled(isRedoAvailable);

	bool isProjectOpen = projectFileName_.isEmpty() == false;

//	libraryWidget_->setEnabled(isProjectOpen);
//	outputWidget_->setEnabled(isProjectOpen);
	ui.actionSave_Project->setEnabled(isProjectOpen);
	ui.actionClose_Project->setEnabled(isProjectOpen);
//	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
	ui.actionExport_Project->setEnabled(isProjectOpen);
	ui.actionExport_Pack->setEnabled(isProjectOpen);
	ui.actionFind_in_Files->setEnabled(isProjectOpen);

	ui.actionFind->setEnabled(hasMdiChild);
	ui.actionReplace->setEnabled(hasMdiChild);
	ui.actionFind_Next->setEnabled(hasMdiChild);
	ui.actionFind_Previous->setEnabled(hasMdiChild);
	ui.actionGo_To_Line->setEnabled(hasMdiChild);
}

void MainWindow::save()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit && textEdit->save() == true)
		statusBar()->showMessage(tr("Items(s) Saved"), 2000);
}

void MainWindow::saveAll()
{
	bool itemsSaved = false;
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);
		if (textEdit != 0 && textEdit->save() == true)
			itemsSaved = true;
	}

	if (libraryWidget_->isModified())
		saveProject();

	if (itemsSaved == true)
		statusBar()->showMessage(tr("Items(s) Saved"), 2000);
}

#if 0
void MainWindow::compile()
{
	if (compileThread_ != 0)
		return;

	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit == NULL)
		return;

	QString fileName = textEdit->fileName();
	QFileInfo fileInfo(fileName);

	if (fileInfo.suffix().toLower() == "lua")
	{
		QDir dir(QFileInfo(projectFileName_).path());
		QString outdir = QDir::cleanPath(dir.absoluteFilePath("obj"));

		textEdit->save();
		outputWidget_->clear();
		compileThread_ = new CompileThread(QStringList() << fileName, false, ".bin", outdir, this);
		connect(compileThread_, SIGNAL(message(const QString&)), outputWidget_, SLOT(append(const QString&)));
		connect(compileThread_, SIGNAL(finished()), this, SLOT(compileThreadFinishedOrTerminated()));
		connect(compileThread_, SIGNAL(terminated()), this, SLOT(compileThreadFinishedOrTerminated()));
		connect(compileThread_, SIGNAL(compileFinished(bool)), this, SLOT(compileFinished(bool)));
		compileThread_->start();
	}
}
#endif

static QString escape(const QString &str)
{
    QString result = str;

    result.replace(" ", "\\ ");
    result.replace("#", "\\#");

    return result;
}

static QString quote(const QString &str)
{
    return "\"" + str + "\"";
}


void MainWindow::compile()
{
    if (makeProcess_->state() != QProcess::NotRunning)
        return;

    TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

    if (textEdit == NULL)
        return;

    textEdit->save();

    QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    toolsDir.cdUp();
#endif
    toolsDir.cd("Tools");

#if defined(Q_OS_WIN)
    QString make = toolsDir.filePath("make.exe");
    QString luac = toolsDir.filePath("luac.exe");
#else
    QString make = toolsDir.filePath("make");
    QString luac = toolsDir.filePath("luac");
#endif

    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/makefile"));
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&file);

        out << "LUAC = " << quote(luac) << "\n";
        out << "\n";

        QString f = dir.relativeFilePath(textEdit->fileName());
        QString o = ".tmp/" + QFileInfo(f).fileName() + ".bin";

        out << "all:" << "\n";
        out << "\t" << "$(LUAC) -o " << quote(o) << " " << quote(f) << "\n";

        file.close();
    }

    makeProcess_->setWorkingDirectory(dir.path());
    makeProcess_->start(make, QStringList() << "-f" << ".tmp/makefile");
}

#if 0
CompileThread* MainWindow::createCompileAllThread() const
{
	if (projectFileName_.isEmpty() == true)
		return 0;

	QString path = QFileInfo(projectFileName_).path();
	QDir dir(path);

	QStringList fileNames;

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	while (stack.empty() == false)
	{
		QDomNode n = stack.top();
		QDomElement e = n.toElement();
		stack.pop();

		QString type = e.tagName();
		QString name = e.attribute("name");

		if (type == "file")
		{
			QString fileName = e.attribute("source");
			if (QFileInfo(fileName).suffix().toLower() == "lua")
				fileNames << QDir::cleanPath(dir.absoluteFilePath(fileName));
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}

	// TODO: bunu baska bir fonksiyon yapip disariya al
	for (int i = 0; i < fileNames.size(); ++i)
	{
		TextEdit* textEdit = findTextEdit(fileNames[i]);

		if (textEdit != 0)
			textEdit->save();
	}

	if (fileNames.isEmpty() == false)
	{
		QDir dir(QFileInfo(projectFileName_).path());
		QString outdir = QDir::cleanPath(dir.absoluteFilePath("obj"));

		return new CompileThread(fileNames, true, ".bin", outdir, const_cast<MainWindow*>(this));
	}

	return 0;
}
#endif

#if 0
void MainWindow::compileAll()
{
	if (compileThread_ != 0)
		return;

	compileThread_ = createCompileAllThread();

	if (compileThread_ == 0)
		return;

	outputWidget_->clear();
	connect(compileThread_, SIGNAL(message(const QString&)), outputWidget_, SLOT(append(const QString&)));
	connect(compileThread_, SIGNAL(finished()), this, SLOT(compileThreadFinishedOrTerminated()));
	connect(compileThread_, SIGNAL(terminated()), this, SLOT(compileThreadFinishedOrTerminated()));
	compileThread_->start();
}
#endif




void MainWindow::compileAll()
{
    if (projectFileName_.isEmpty())
        return;

    if (makeProcess_->state() != QProcess::NotRunning)
        return;

    saveAll();

    QStringList fileNames;
    {
        QDir dir = QFileInfo(projectFileName_).dir();

        QDomDocument doc = libraryWidget_->toXml();

        std::stack<QDomNode> stack;
        stack.push(doc.documentElement());

        while (stack.empty() == false)
        {
            QDomNode n = stack.top();
            QDomElement e = n.toElement();
            stack.pop();

            if (e.tagName() == "file")
            {
                QString fileName = e.attribute("source");
                if (QFileInfo(fileName).suffix().toLower() == "lua")
                    fileNames << fileName;
            }

            QDomNodeList childNodes = n.childNodes();
            for (int i = 0; i < childNodes.size(); ++i)
                stack.push(childNodes.item(i));
        }
    }

    QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    toolsDir.cdUp();
#endif
    toolsDir.cd("Tools");

#if defined(Q_OS_WIN)
    QString make = toolsDir.filePath("make.exe");
    QString luac = toolsDir.filePath("luac.exe");
#else
    QString make = toolsDir.filePath("make");
    QString luac = toolsDir.filePath("luac");
#endif

    QString all = "all:";
    for (int i = 0; i < fileNames.size(); ++i)
    {
        QString f = fileNames[i];
        QString o = ".tmp/" + QFileInfo(f).fileName() + ".bin";
        all += " " + escape(o);
    }

    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/makefile"));
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&file);

        out << "LUAC = " << quote(luac) << "\n";
        out << "\n";

        out << all << "\n";
        out << "\n";

        for (int i = 0; i < fileNames.size(); ++i)
        {
            QString f = fileNames[i];
            QString o = ".tmp/" + QFileInfo(f).fileName() + ".bin";

            out << escape(o) << ": " << escape(f) << "\n";
            out << "\t" << "$(LUAC) -o " << quote(o) << " " << quote(f) << "\n";

            out << "\n";
        }
        file.close();
    }

    makeProcess_->setWorkingDirectory(dir.path());
    makeProcess_->start(make, QStringList() << "-f" << ".tmp/makefile");
}

void MainWindow::cancel()
{
#if 0
	if (compileThread_ == 0)
		return;
#endif

}

#if 0
void MainWindow::compileThreadFinishedOrTerminated()
{
	delete compileThread_;
	compileThread_ = 0;
}
#endif

static bool parseFileLineString(const QString& l, QString* fileName = 0, unsigned int* lineNumber = 0)
{
	QString line = l.trimmed();

	QStringList list = line.split(':');

	if (list.size() < 3)
		return false;

	if (list[0].size() == 1)
	{
		if (list[1].isEmpty() == false)
		{
			if (list[1][0] == '/' || list[1][0] == '\\')
			{
				QString file = list[0] + ":" + list[1];
				list.pop_front();
				list[0] = file;
			}
		}
	}
	
	bool ok;
	unsigned int list1 = list[1].toUInt(&ok);

	if (ok == false)
		return false;

	if (fileName != 0)
		*fileName = list[0];

	if (lineNumber != 0)
		*lineNumber = list1;

	return true;
}

/*void MainWindow::outputMouseDoubleClick(QMouseEvent* e)
{
	QTextCursor	cursor = outputWidget_->cursorForPosition(e->pos());

	QString fileName;
	unsigned int lineNumber;
	if (parseFileLineString(cursor.block().text(), &fileName, &lineNumber) == true)
	{
		TextEdit* textEdit = openFile(fileName);

		if (textEdit)
			textEdit->setCursorPosition(lineNumber - 1, 0);
	}
}*/

// check if string starts with [space]*[digit]+[:]
static bool lineNumber(const QString& str, int* line = NULL)
{
	QRegExp regexp("^\\s*(\\d+)\\:");

	if (regexp.indexIn(str) == -1)
		return false;

	if (line)
		*line = regexp.cap(1).toInt();

	return true;
}

void MainWindow::outputMouseDoubleClick(QMouseEvent* e)
{
	QTextCursor	cursor = outputWidget_->cursorForPosition(e->pos());
	QTextBlock block = cursor.block();
	QString text = block.text();

	{
		QString fileName;
		unsigned int lineNumber;
		if (parseFileLineString(text, &fileName, &lineNumber) == true)
		{
			TextEdit* textEdit = openFile(fileName);

			if (textEdit)
				textEdit->setCursorPosition(lineNumber - 1, 0);

			return;
		}
	}

	int line;
	if (lineNumber(text, &line))
	{
		while (true)
		{
			block = block.previous();

			QString text = block.text();
			if (text.isEmpty())
				break;
			
			if (text.right(1) == ":")
			{
				TextEdit* textEdit = openFile(text.left(text.size() - 1));

				if (textEdit)
					textEdit->setCursorPosition(line - 1, 0);
//				printf("%s%d\n", qPrintable(text), line);
				break;
			}
		}
	}
}


TextEdit* MainWindow::openFile(const QString& fn, bool suppressErrors/* = false*/)
{
	QString fileName;

	if (QFileInfo(fn).isRelative())
	{
		QDir dir(QFileInfo(projectFileName_).path());
		fileName = QDir::cleanPath(dir.absoluteFilePath(libraryWidget_->fileName(fn)));
	}
	else
	{
		fileName = fn;
	}

	TextEdit* existing = findTextEdit(fileName);
	if (existing)
	{
		mdiArea_->setActiveSubWindow(existing);
		existing->setFocus(Qt::OtherFocusReason);
		return existing;
	}

	TextEdit* child = createTextEdit();
	if (child->loadFile(fileName, suppressErrors) == true)
	{
		mdiArea_->addSubWindow(child);
		child->showMaximized();
	}
	else
	{
		child->close();
		child = 0;
	}

	return child;
}

#if 0
void MainWindow::compileFinished(bool success)
{

}
#endif

#define MAX_RECENT_FILES 15
void MainWindow::addToRecentProjects(const QString& fileName)
{
	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MAX_RECENT_FILES)
		files.removeLast();
	settings.setValue("recentProjectList", files);

	updateRecentProjectActions();
}

void MainWindow::updateRecentProjectActions()
{
	QAction* recentProjectActions[MAX_RECENT_FILES];
	recentProjectActions[0] = ui.actionProject1;
	recentProjectActions[1] = ui.actionProject2;
	recentProjectActions[2] = ui.actionProject3;
	recentProjectActions[3] = ui.actionProject4;
	recentProjectActions[4] = ui.actionProject5;
	recentProjectActions[5] = ui.actionProject6;
	recentProjectActions[6] = ui.actionProject7;
    recentProjectActions[7] = ui.actionProject8;
    recentProjectActions[8] = ui.actionProject9;
    recentProjectActions[9] = ui.actionProject10;
    recentProjectActions[10] = ui.actionProject11;
    recentProjectActions[11] = ui.actionProject12;
    recentProjectActions[12] = ui.actionProject13;
    recentProjectActions[13] = ui.actionProject14;
    recentProjectActions[14] = ui.actionProject15;

	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();

	int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
		recentProjectActions[i]->setText(text);
		recentProjectActions[i]->setData(files[i]);
		recentProjectActions[i]->setVisible(true);
	}

	for (int i = numRecentFiles; i < MAX_RECENT_FILES; ++i)
		recentProjectActions[i]->setVisible(false);
}

void MainWindow::openRecentProject()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (action != 0)
		openProject(action->data().toString());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (maybeSave())
	{
		storeOpenFiles();

		QSettings settings;
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());
		settings.setValue("splitter1", splitter1_->saveState());
		settings.setValue("splitter2", splitter2_->saveState());
		settings.setValue("splitter3", splitter3_->saveState());

		event->accept();
	} 
	else
	{
		event->ignore();
	}
}

void MainWindow::playerSettings()
{
	PlayerSettingsDialog dialog(this);

	if (dialog.exec() == QDialog::Accepted)
	{
		ui.actionStart->setEnabled(false);
		ui.actionStop->setEnabled(false);
		
		//sentMap_.clear();
		
		QSettings settings;
		QString playerip = settings.value("player ip", QString("127.0.0.1")).toString();

#ifndef NEW_CLIENT
		delete client_;
		client_ = new Client(qPrintable(playerip), 15000);
#else
		client_->connectToHost(playerip, 15000);
#endif
	}
}

void MainWindow::cancelUpload()
{

}

void MainWindow::find()
{
	replaceDialog_->hide();
//	findInFilesDialog_->hide();
	findDialog_->show();
    findDialog_->raise();
    findDialog_->activateWindow();
	findDialog_->focusToFindWhat();
}

void MainWindow::findFirst()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = findDialog_->findWhat();
		matchCase_ = findDialog_->matchCase();
		wholeWord_ = findDialog_->wholeWord();
		bool forward = findDialog_->forward();

		if (findWhat_.isEmpty() == false)
			if (textEdit->findFirst(findWhat_, false, matchCase_, wholeWord_, false, forward) == false)
				QMessageBox::information(findDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::findNext()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		if (findWhat_.isEmpty() == false)
			if (textEdit->findFirst(findWhat_, false, matchCase_, wholeWord_, false, true) == false)
				QMessageBox::information(findDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::findPrevious()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		if (findWhat_.isEmpty() == false)
			if (textEdit->findFirst(findWhat_, false, matchCase_, wholeWord_, false, false) == false)
				QMessageBox::information(findDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::replace()
{
	findDialog_->hide();
//	findInFilesDialog_->hide();
	replaceDialog_->show();
    replaceDialog_->raise();
    replaceDialog_->activateWindow();
    replaceDialog_->focusToFindWhat();
}

void MainWindow::toogleBookmark()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->toogleBookmark();
}

void MainWindow::nextBookmark()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->nextBookmark();
}

void MainWindow::previousBookmark()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->previousBookmark();
}

void MainWindow::clearBookmarks()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->clearBookmarks();
}

void MainWindow::undo()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->undo();
}

void MainWindow::redo()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->redo();
}

void MainWindow::goToLine()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		int line, index;
		textEdit->sciScintilla()->getCursorPosition(&line, &index);

		int lines = textEdit->sciScintilla()->lines();

		GoToLineDialog dialog(this);
		dialog.setLineNumbers(line + 1, lines);
		if (dialog.exec() == QDialog::Accepted)
		{
			int lineNumber = dialog.lineNumber();
			lineNumber = qMin(qMax(lineNumber, 1), lines);

			textEdit->sciScintilla()->setCursorPosition(lineNumber - 1, 0);
		}
	}
}

#ifndef NEW_CLIENT

#else
void MainWindow::connected()
{
	fileQueue_.clear();
	isTransferring_ = false;

	ui.actionStart->setEnabled(true);
	ui.actionStop->setEnabled(true);
	printf("other side connected\n");
}

void MainWindow::disconnected()
{
	fileQueue_.clear();
	isTransferring_ = false;

	ui.actionStart->setEnabled(false);
	ui.actionStop->setEnabled(false);
	printf("other side closed connection\n");
}

void MainWindow::dataReceived(const QByteArray& d)
{
//	if (!updateMD5().empty())	// bunu neden koymusuz acaba?
//		saveMD5();

	const char* data = d.constData();

	if (data[0] == 4)
	{
		std::string str = &data[1];
		outputWidget_->append(QString::fromUtf8(str.c_str()));
	}
	if (data[0] == 6 && isTransferring_ == true)
	{
		printf("file list has got\n");

		fileQueue_.clear();

		std::map<QString, QString> localFileMap;
		std::map<QString, QString> localFileMapReverse;
		{
			std::vector<std::pair<QString, QString> > fileList = libraryFileList();

			for (std::size_t i = 0; i < fileList.size(); ++i)
			{
				localFileMap[fileList[i].first] = fileList[i].second;
				localFileMapReverse[fileList[i].second] = fileList[i].first;
			}
		}

		std::map<QString, std::pair<int, QByteArray> > remoteFileMap;

		ByteBuffer buffer(d.constData(), d.size());

		char chr;
		buffer >> chr;
	
		while (buffer.eob() == false)
		{
			std::string file;
			buffer >> file;

			if (file[0] == 'F')
			{
				int age;
				buffer >> age;

				unsigned char md5[16];
				buffer.get(md5, 16);

				remoteFileMap[file.c_str() + 1] = std::make_pair(age, QByteArray((char*)md5, 16));
			}
			else if (file[0] == 'D')
			{
			}
		}

		// delete unused files
		for (std::map<QString, std::pair<int, QByteArray> >::iterator iter = remoteFileMap.begin(); iter != remoteFileMap.end(); ++iter)
		{
			if (localFileMap.find(iter->first) == localFileMap.end())
			{
				printf("deleting: %s\n", qPrintable(iter->first));
				client_->sendDeleteFile(qPrintable(iter->first));
			}
		}

		// upload files
		QDir path(QFileInfo(projectFileName_).path());
		for (std::map<QString, QString>::iterator iter = localFileMap.begin(); iter != localFileMap.end(); ++iter)
		{
			std::map<QString, std::pair<int, QByteArray> >::iterator riter = remoteFileMap.find(iter->first);
			
			QString localfile = QDir::cleanPath(path.absoluteFilePath(iter->second));

			bool send = false;
			if (riter == remoteFileMap.end())
			{
				printf("always upload: %s\n", qPrintable(iter->first));
				send = true;
			}
			else
			{
				int localage = fileAge(qPrintable(localfile));
				int remoteage = riter->second.first;
				const QByteArray& localmd5 = md5_[iter->second].second;
				const QByteArray& remotemd5 = riter->second.second;

				if (localage < remoteage || localmd5 != remotemd5)
				{
					printf("upload new file: %s\n", qPrintable(iter->first));
					send = true;
				}
			}

			if (send == true)
				fileQueue_.push_back(qMakePair(iter->first, localfile));
			else
				printf("don't upload: %s\n", qPrintable(iter->first));
		}

        std::vector<std::pair<QString, bool> > topologicalSort = libraryWidget_->topologicalSort();

		QStringList luaFiles;
		for (std::size_t i = 0; i < topologicalSort.size(); ++i)
            if (topologicalSort[i].second == false)
                luaFiles << localFileMapReverse[topologicalSort[i].first];

		if (luaFiles.empty() == false)
			fileQueue_.push_back(qMakePair(luaFiles.join("|"), QString("play")));
	}
}

void MainWindow::ackReceived(unsigned int id)
{
/*	std::map<int, QString>::iterator iter = sentMap_.find(id);
	if (iter != sentMap_.end())
	{
		QString name = iter->second;
		if (name.isEmpty() == false)
		{
			outputWidget_->moveCursor(QTextCursor::End);
			outputWidget_->insertPlainText(iter->second + " is uploaded.\n");
		}
		sentMap_.erase(iter);
		if (sentMap_.empty() == true)
		{
			outputWidget_->moveCursor(QTextCursor::End);
			outputWidget_->insertPlainText("Uploading finished!\n");
			sendPlayMessage();
		}
	} */
}
#endif

void MainWindow::showOutputPanel()
{
	outputDock_->show();
}

void MainWindow::showLibraryManager()
{
	libraryDock_->show();
}

bool MainWindow::maybeSave()
{
	QStringList toBeSaved;

	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);
		if (textEdit != 0 && textEdit->isModified() == true)
			toBeSaved.push_back(QFileInfo(textEdit->fileName()).fileName());
	}

	if (libraryWidget_->isModified())
		toBeSaved.push_back(QFileInfo(projectFileName_).fileName());

	if (toBeSaved.empty() == false)
	{
		SaveChangesDialog dialog(toBeSaved, this);
		int result = dialog.exec();
		if (result == QDialog::Accepted)			// yes
		{
			saveAll();
			return true;
		}
		else if (result == QDialog::Rejected)		// cancel
		{
			return false;
		}
		else if (result == 2)						// no
		{
			return true;
		}
	}

	return true;
}

void MainWindow::onPreviewRequest(const QString& itemName, const QString& fileName)
{
	QString title = QFileInfo(projectFileName_).dir().relativeFilePath(fileName);
	previewWidget_->setFileName(fileName, title);
}

static void fileCopy(	const QString& srcName,
						const QString& destName,
						const QList<QStringList>& wildcards,
						const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList)
{
    QString srcName2 = QFileInfo(srcName).fileName();

    int match = -1;
	for (int j = 0; j < wildcards.size(); ++j)
	{
		bool submatch = false;
		for (int i = 0; i < wildcards[j].size(); ++i)
		{
			QRegExp rx(wildcards[j][i]);
			rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(srcName2))
			{
				submatch = true;
				break;
			}
		}
		if (submatch)
		{
			match = j;
			break;
		}
	}

	if (match != -1)
	{
		QFile in(srcName);
		if (!in.open(QFile::ReadOnly))
			return;
		QByteArray data = in.readAll();
		in.close();;

		for (int i = 0; i < replaceList[match].size(); ++i)
			data.replace(replaceList[match][i].first, replaceList[match][i].second);

		QFile out(destName);
		if (!out.open(QFile::WriteOnly))
			return;
		out.write(data);
		out.close();
	}
	else
	{
		QFile::copy(srcName, destName);
	}
}

static bool shouldCopy(const QString &fileName, const QStringList &include, const QStringList &exclude)
{
    QString fileName2 = QFileInfo(fileName).fileName();

    bool result = false;

    for (int i = 0; i < include.size(); ++i)
    {
        QRegExp rx(include[i]);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(fileName2))
        {
            result = true;
            break;
        }
    }

    for (int i = 0; i < exclude.size(); ++i)
    {
        QRegExp rx(exclude[i]);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(fileName2))
        {
            result = false;
            break;
        }
    }

    return result;
}


static void copyFolder(	const QDir& sourceDir,
						const QDir& destDir,
						const QList<QPair<QString, QString> >& renameList,
						const QList<QStringList>& wildcards,
                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList,
                        const QStringList &include,
                        const QStringList &exclude)
{
	if(!sourceDir.exists())
		return;

	QStringList files;

	files = sourceDir.entryList(QDir::Files | QDir::Hidden);
	for(int i = 0; i < files.count(); i++)
	{
		QString srcName = sourceDir.absoluteFilePath(files[i]);
		QString destFile = files[i];
		for (int i = 0; i < renameList.size(); ++i)
			destFile.replace(renameList[i].first, renameList[i].second);
		QString destName = destDir.absoluteFilePath(destFile);
        if (shouldCopy(srcName, include, exclude))
            fileCopy(srcName, destName, wildcards, replaceList);
	}

	files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for(int i = 0; i < files.count(); i++)
	{
		QDir sourceDir2 = sourceDir;
		bool b1 = sourceDir2.cd(files[i]);

		QDir destDir2 = destDir;
		QString destFile = files[i];
		for (int i = 0; i < renameList.size(); ++i)
			destFile.replace(renameList[i].first, renameList[i].second);
		destDir2.mkdir(destFile);
		bool b2 = destDir2.cd(destFile);

		if (b1 && b2)
			copyFolder(sourceDir2,
					   destDir2,
					   renameList,
					   wildcards,
                       replaceList,
                       include,
                       exclude);
	}
}


void MainWindow::exportProject()
{
	if (g_checkHash() == false)
	{
		QMessageBox::information(this, tr("Gideros"), tr("Inconsistent license file. Please run Gideros License Manager and update your license."));
		return;
	}

	LicenseManager licenseManager;
	int licenseType = licenseManager.getLicenseType();

	if (licenseType == -1)
	{
		QMessageBox::information(this, tr("Gideros"), tr("You do not have a license installed. Please run Gideros License Manager."));
		return;
	}

	bool licensed = (licenseType == 2 || licenseType == 3);

    ExportProjectDialog dialog(&libraryWidget_->getProjectProperties(), licensed, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		ExportProjectDialog::DeviceFamily deviceFamily = dialog.deviceFamily();

		QString templatedir;
		QString templatename;
		QString templatenamews;
		bool underscore;

		switch (deviceFamily)
		{
            case ExportProjectDialog::e_iOS:
                templatedir = "Xcode4";
                templatename = "iOS Template";
                templatenamews = "iOS_Template";
				underscore = true;
				break;

			case ExportProjectDialog::e_Android:
				templatedir = "Eclipse";
				templatename = "Android Template";
				templatenamews = "AndroidTemplate";
				underscore = false;
				break;
		}


		QSettings settings;
		QString lastExportDirectory = settings.value("lastExportDirectory", QString()).toString();

		QString output = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			lastExportDirectory,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

		if (output.isEmpty() == true)
			return;

		settings.setValue("lastExportDirectory", output);

		QDir outputDir(output);

//		outputDir.mkdir("export");
//		outputDir.cd("export");

		QString base = QFileInfo(projectFileName_).baseName();

		QString basews;
		if (underscore)
		{
			basews = base;
			for (int i = 0; i < basews.size(); ++i)
			{
                char c = basews[i].toLatin1();

				bool number = ('0' <= c) && (c <= '9');
				bool upper = ('A' <= c) && (c <= 'Z');
				bool lower = ('a' <= c) && (c <= 'z');

				if ((!number && !upper && !lower) || (number && i == 0))
					basews[i] = QChar('_');
			}
		}
		else
		{
			// 1234 Hebe Gube 456 --> HebeGube456
			bool letter = false;
			for (int i = 0; i < base.size(); ++i)
			{
                char c = base[i].toLatin1();

				bool number = ('0' <= c) && (c <= '9');
				bool upper = ('A' <= c) && (c <= 'Z');
				bool lower = ('a' <= c) && (c <= 'z');

				if (upper || lower)
					letter = true;

				if ((number || upper || lower) && letter)
					basews += base[i];
			}
		}

		outputDir.mkdir(base);
		outputDir.cd(base);

		saveAll();

        QByteArray codePrefix("312e68c04c6fd22922b5b232ea6fb3e1");
        QByteArray assetsPrefix("312e68c04c6fd22922b5b232ea6fb3e2");
        QByteArray encryptionZero(16, '\0');
        QByteArray codeKey(16, '\0');
        QByteArray assetsKey(16, '\0');
        QByteArray encryptionKey(16, '\0');

        {
            QSettings settings;
            if (settings.contains("encryptionKey"))
            {
                encryptionKey = settings.value("encryptionKey").toByteArray();
            }
            else
            {
                qsrand(time(NULL));
                for (int i = 0; i < 16; ++i)
                    encryptionKey[i] = qrand() % 256;
                settings.setValue("encryptionKey", encryptionKey);
                settings.sync();
            }
        }

        if (dialog.encryptCode())
            codeKey = encryptionKey;

        if (dialog.encryptAssets())
            assetsKey = encryptionKey;

		// copy template
        if (true)
		{
			QDir dir = QDir::currentPath();
			dir.cd("Templates");
			dir.cd(templatedir);
			dir.cd(templatename);

			QList<QPair<QString, QString> > renameList;
			renameList << qMakePair(templatename, base);
			renameList << qMakePair(templatenamews, basews);

			QList<QStringList> wildcards;
			QList<QList<QPair<QByteArray, QByteArray> > > replaceList;

			QStringList wildcards1;
			wildcards1 <<
				"*.pch" <<
				"*.plist" <<
				"*.pbxproj" <<
				"*.java" <<
				"*.xml" <<
				"*.project";
			wildcards << wildcards1;

			QList<QPair<QByteArray, QByteArray> > replaceList1;
			replaceList1 << qMakePair(templatename.toUtf8(), base.toUtf8());
            replaceList1 << qMakePair(templatenamews.toLatin1(), basews.toLatin1());
			if (deviceFamily == ExportProjectDialog::e_Android)
				replaceList1 << qMakePair(QString("com.giderosmobile.androidtemplate").toUtf8(), dialog.packageName().toUtf8());
			replaceList << replaceList1;

			if (licensed)
			{
				QStringList wildcards2;
                wildcards2 << "libgideros.so" << "libgideros.a";
				wildcards << wildcards2;

				QList<QPair<QByteArray, QByteArray> > replaceList2;
				replaceList2 << qMakePair(QByteArray("9852564f4728e0c11e34ca3eb5fe20b2"), QByteArray("9852564f4728e0cffe34ca3eb5fe20b2"));
                replaceList2 << qMakePair(codePrefix + encryptionZero, codePrefix + codeKey);
                replaceList2 << qMakePair(assetsPrefix + encryptionZero, assetsPrefix + assetsKey);
                replaceList << replaceList2;
			};

            if (dialog.assetsOnly())
                copyFolder(dir, outputDir, renameList, wildcards, replaceList, QStringList() << "libgideros.so" << "libgideros.a" << "gideros.jar", QStringList());
            else
                copyFolder(dir, outputDir, renameList, wildcards, replaceList, QStringList() << "*", QStringList());
		}

        if (deviceFamily == ExportProjectDialog::e_iOS)
        {
            outputDir.mkdir(base);
            outputDir.cd(base);
        }
        else if (deviceFamily == ExportProjectDialog::e_Android)
		{
            outputDir.mkdir("assets");
            outputDir.cd("assets");
        }
        outputDir.mkdir("assets");
        outputDir.cd("assets");

		std::deque<QPair<QString, QString> > fileQueue;

		QDomDocument doc = libraryWidget_->toXml();

		std::stack<QDomNode> stack;
		stack.push(doc.documentElement());

		std::vector<QString> dir;

		while (stack.empty() == false)
		{
			QDomNode n = stack.top();
			QDomElement e = n.toElement();
			stack.pop();

			if (n.isNull() == true)
			{
				dir.pop_back();
				continue;
			}

			QString type = e.tagName();

			if (type == "file")
			{

				QString fileName = e.attribute("source");
				QString name = QFileInfo(fileName).fileName();

				QString n;
				for (std::size_t i = 0; i < dir.size(); ++i)
					n += dir[i] + "/";
				n += name;

				fileQueue.push_back(qMakePair(n, fileName));

				continue;
			}

			if (type == "folder")
			{
				QString name = e.attribute("name");
				dir.push_back(name);


				QString n;
				for (std::size_t i = 0; i < dir.size(); ++i)
					n += dir[i] + "/";

				outputDir.mkdir(n);

				stack.push(QDomNode());
			}

			QDomNodeList childNodes = n.childNodes();
			for (int i = 0; i < childNodes.size(); ++i)
				stack.push(childNodes.item(i));
		}


        std::vector<std::pair<QString, bool> > topologicalSort = libraryWidget_->topologicalSort();
		for (std::size_t i = 0; i < topologicalSort.size(); ++i)
		{
			int index = -1;
			for (std::size_t j = 0; j < fileQueue.size(); ++j)
			{
                if (fileQueue[j].second == topologicalSort[i].first)
				{
					index = j;
					break;
				}
			}

			if (index != -1)
			{
				QPair<QString, QString> item = fileQueue[index];
				fileQueue.erase(fileQueue.begin() + index);
				fileQueue.push_back(item);
			}
		}

		QStringList luafiles;
		QStringList luafiles_abs;
		QStringList allfiles;
		QStringList allfiles_abs;
        QStringList allluafiles;
        QStringList allluafiles_abs;

		QProgressDialog progress("Copying files...", QString(), 0, fileQueue.size(), this);
		progress.setWindowModality(Qt::WindowModal);

		QSet<QString> jetset;
		jetset << "mp3" << "png" << "jpg" << "jpeg" << "wav";

		QDir path(QFileInfo(projectFileName_).path());

		for (std::size_t i = 0; i < fileQueue.size(); ++i)
		{
			const QString& s1 = fileQueue[i].first;
			const QString& s2 = fileQueue[i].second;

			QString src = QDir::cleanPath(path.absoluteFilePath(s2));
			QString dst = QDir::cleanPath(outputDir.absoluteFilePath(s1));

			if (deviceFamily == ExportProjectDialog::e_Android)
			{
				QString suffix = QFileInfo(dst).suffix().toLower();
				if (!jetset.contains(suffix))
					dst += ".jet";
			}

			allfiles.push_back(s1);
			allfiles_abs.push_back(dst);

			if (QFileInfo(src).suffix().toLower() == "lua")
			{
                allluafiles.push_back(s1);
                allluafiles_abs.push_back(dst);

                if (std::find(topologicalSort.begin(), topologicalSort.end(), std::make_pair(s2, true)) == topologicalSort.end())
                {
                    luafiles.push_back(s1);
                    luafiles_abs.push_back(dst);
                }
			}

			progress.setValue(i);

			QFile::remove(dst);
			QFile::copy(src, dst);
		}

#if 0
		// compile lua files
		if (false)
		{
			compileThread_ = new CompileThread(luafiles_abs, false, "", QString(), this);
			compileThread_->start();
			compileThread_->wait();
			delete compileThread_;
		}
#endif

		// compile lua files (with luac)
		if (true)
		{
            for (int i = 0; i < allluafiles_abs.size(); ++i)
			{
                QString file = "\"" + allluafiles_abs[i] + "\"";
                QProcess::execute("Tools/luac -o " + file + " " + file);
			}
		}

        // encrypt lua, png, jpg, jpeg and wav files
        if (true)
        {
            for (int i = 0; i < allfiles_abs.size(); ++i)
            {
                QString ext = QFileInfo(allfiles[i]).suffix().toLower();
                if (ext != "lua" && ext != "png" && ext != "jpeg" && ext != "jpg" && ext != "wav")
                    continue;

                QByteArray encryptionKey = (ext == "lua") ? codeKey : assetsKey;

                QString filename = allfiles_abs[i];

                QFile fis(filename);
                if (!fis.open(QIODevice::ReadOnly))
                    continue;
                QByteArray data = fis.readAll();
                fis.close();

                for (int j = 0; j < data.size(); ++j)
                    data[j] = data[j] ^ encryptionKey[j % encryptionKey.size()];

                QFile fos(filename);
                if (!fos.open(QIODevice::WriteOnly))
                    continue;
                fos.write(data);
                fos.close();
            }
        }

		// compress lua files
		if (false)
		{
			for (int i = 0; i < luafiles_abs.size(); ++i)
			{
				QString file = "\"" + luafiles_abs[i] + "\"";
				QProcess::execute("Tools/lua Tools/LuaSrcDiet.lua --quiet " + file + " -o " + file);
			}
		}

		// write luafiles.txt
		{
			QString filename = "luafiles.txt";
			if (deviceFamily == ExportProjectDialog::e_Android)
				filename += ".jet";
			QFile file(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
			if (file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QTextStream out(&file);

				for (int i = 0; i < luafiles.size(); ++i)
					out << luafiles[i] << "\n";
			}
		}

		// write allfiles.txt
		if (deviceFamily == ExportProjectDialog::e_Android)
		{
			QFile file(QDir::cleanPath(outputDir.absoluteFilePath("allfiles.txt")));
			if (file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QTextStream out(&file);

				for (int i = 0; i < allfiles.size(); ++i)
				{
					QString file = allfiles[i];
					QString suffix = QFileInfo(file).suffix().toLower();
					if (!jetset.contains(suffix))
						file += "*";
					out << file << "\n";
				}
			}
		}

		// write properties.bin
		{
			QString filename = "properties.bin";
			if (deviceFamily == ExportProjectDialog::e_Android)
				filename += ".jet";
			QFile file(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
			if (file.open(QIODevice::WriteOnly))
			{
				const ProjectProperties& properties = libraryWidget_->getProjectProperties();

				ByteBuffer buffer;

				buffer << properties.scaleMode;
				buffer << properties.logicalWidth;
				buffer << properties.logicalHeight;

				buffer << (int)properties.imageScales.size();
				for (size_t i = 0; i < properties.imageScales.size(); ++i)
				{
					buffer << properties.imageScales[i].first.toUtf8().constData();
					buffer << (float)properties.imageScales[i].second;
				}

				buffer << properties.orientation;
				buffer << properties.fps;

                buffer << properties.retinaDisplay;
				buffer << properties.autorotation;

                buffer << (properties.mouseToTouch ? 1 : 0);
                buffer << (properties.touchToMouse ? 1 : 0);
                buffer << properties.mouseTouchOrder;

				file.write(buffer.data(), buffer.size());
			}
		}

		progress.setValue(fileQueue.size());

		if (licensed)
			QMessageBox::information(this, tr("Gideros"), tr("Project is exported successfully."));
		else
			QMessageBox::information(this, tr("Gideros"), tr("Project is exported successfully with splash screen."));

        g_countly->projectExported();
	}
}

std::vector<std::pair<QString, QString> > MainWindow::libraryFileList(bool downsizing)
{
	std::vector<std::pair<QString, QString> > result;

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	std::vector<QString> dir;

	while (stack.empty() == false)
	{
		QDomNode n = stack.top();
		QDomElement e = n.toElement();
		stack.pop();

		if (n.isNull() == true)
		{
			dir.pop_back();
			continue;
		}

		QString type = e.tagName();

		if (type == "file")
		{
			QString fileName = e.attribute("source");
			QString name = QFileInfo(fileName).fileName();

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
			n += name;

			if (downsizing)
			{
				if (e.attribute("downsizing", "0").toInt())
					result.push_back(std::make_pair(n, fileName));
			}
			else
			{
				result.push_back(std::make_pair(n, fileName));
			}

			continue;
		}

		if (type == "folder")
		{
			QString name = e.attribute("name");
			dir.push_back(name);

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";

			stack.push(QDomNode());
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}

	return result;
}

void MainWindow::replace_findNext()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = replaceDialog_->findWhat();
		matchCase_ = replaceDialog_->matchCase();
		wholeWord_ = replaceDialog_->wholeWord();

		if (findWhat_.isEmpty() == false)
			if (textEdit->findFirst(findWhat_, false, matchCase_, wholeWord_, false, true) == false)
				QMessageBox::information(replaceDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::replace_replace()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = replaceDialog_->findWhat();
		matchCase_ = replaceDialog_->matchCase();
		wholeWord_ = replaceDialog_->wholeWord();

		if (findWhat_.isEmpty() == false)
			if (textEdit->replace(findWhat_, replaceDialog_->replaceWith(), false, matchCase_, wholeWord_, false) == false)
				QMessageBox::information(replaceDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}


void MainWindow::replace_replaceAll()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = replaceDialog_->findWhat();
		matchCase_ = replaceDialog_->matchCase();
		wholeWord_ = replaceDialog_->wholeWord();

		if (findWhat_.isEmpty() == false)
		{
			int all = textEdit->replaceAll(findWhat_, replaceDialog_->replaceWith(), false, matchCase_, wholeWord_, false);
			QMessageBox::information(replaceDialog_, tr("Gideros"), tr("%1 occurrences were replaced.").arg(all));
		}
	}
}

void MainWindow::findInFiles()
{
	findInFilesDialog_->focusToFindWhat();
	if (findInFilesDialog_->exec() == QDialog::Accepted)
	{
		QString findWhat = findInFilesDialog_->findWhat();
		bool matchCase = findInFilesDialog_->matchCase();
		bool wholeWord = findInFilesDialog_->wholeWord();

		QDir path(QFileInfo(projectFileName_).path());

		outputWidget_->setUpdatesEnabled(false);

		outputWidget_->clear();

		std::vector<std::pair<QString, QString> > fileListAll = libraryFileList();
		std::vector<std::pair<QString, QString> > fileList;

		QRegExp rx(findInFilesDialog_->filter());
		rx.setPatternSyntax(QRegExp::Wildcard);
		for (std::size_t i = 0; i < fileListAll.size(); ++i)
		{
			QString filename = fileListAll[i].second;
			
			if (rx.exactMatch(filename))
				fileList.push_back(fileListAll[i]);
		}

		outputWidget_->append(tr("Searching %1 files for \"%2\"\n").arg(fileList.size()).arg(findWhat));

		int nfound = 0;
		for (std::size_t i = 0; i < fileList.size(); ++i)
		{
			QString filename = fileList[i].second;
			QString relfilename = fileList[i].first;
			QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));

			QFile file(absfilename);

			if (file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QMap<int, QList<QPair<int, int> > > found;

				QsciScintilla sci;
				QTextStream in(&file);
				sci.setText(in.readAll());

				int line = -1, index = -1;
				while (sci.findFirst(findWhat, false, matchCase, wholeWord, false, true, line, index))
				{
					int lineFrom, indexFrom, lineTo, indexTo;
					sci.getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

					found[lineFrom].push_back(qMakePair(indexFrom, indexTo));

					line = lineTo;
					index = indexTo;

					nfound++;
				}

				if (!found.isEmpty())
				{
					outputWidget_->append("\n");
					outputWidget_->append(absfilename + ":\n");

					QMap<int, QList<QPair<int, int> > >::iterator iter, e = found.end();
					for (iter = found.begin(); iter != e; ++iter)
					{
						int line = iter.key();
						
						outputWidget_->append(QString::number(line + 1).rightJustified(5) + ": ");
						outputWidget_->append(sci.text(line), iter.value());
					}
				}
			}
		}

		outputWidget_->append("\n" + tr("%1 matches found.").arg(nfound) + "\n");

		outputWidget_->setUpdatesEnabled(true);
	}
}

void MainWindow::loadMD5()
{
	md5_.clear();

    QDir dir = QFileInfo(projectFileName_).dir();

    QFile file(dir.filePath(".tmp/" + QFileInfo(projectFileName_).completeBaseName() + ".md5"));
	if (!file.open(QIODevice::ReadOnly))
		return;

	QDataStream out(&file);
	out >> md5_;
}

void MainWindow::saveMD5()
{
    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/" + QFileInfo(projectFileName_).completeBaseName() + ".md5"));
	if (!file.open(QIODevice::WriteOnly))
		return;

	QDataStream out(&file);
	out << md5_;
}

std::vector<std::pair<QString, QString> > MainWindow::updateMD5(bool downsizing)
{
	std::vector<std::pair<QString, QString> > updated;

	std::vector<std::pair<QString, QString> > fileList = libraryFileList(downsizing);

	QDir path(QFileInfo(projectFileName_).path());

	for (std::size_t i = 0; i < fileList.size(); ++i)
	{
		QString filename = fileList[i].second;
		QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));
		time_t mtime = fileLastModifiedTime(qPrintable(absfilename));

		QMap<QString, QPair<qint64, QByteArray> >::iterator iter = md5_.find(filename);

		if (iter == md5_.end() || mtime != iter.value().first)
		{
			QPair<qint64, QByteArray>& md5 = md5_[filename];

			md5.first = mtime;
			md5.second.resize(16);
			unsigned char* data = (unsigned char*)md5.second.data();
			md5_fromfile(qPrintable(absfilename), data);
//			updated = true;
			updated.push_back(fileList[i]);

			qDebug() << "update md5: " + filename;
		}
	}

	return updated;
}


void MainWindow::openAboutDialog()
{
	AboutDialog dialog(this);
	dialog.exec();
}

void MainWindow::onInsertIntoDocument(const QString& text)
{
	qDebug() << text;
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit != 0)
	{
		textEdit->sciScintilla()->replaceSelectedText(text);
		textEdit->sciScintilla()->setFocus();
		textEdit->setFocus();
	}
}

void MainWindow::exportPack()
{
	QString output = QFileDialog::getExistingDirectory(	this,
														tr("Open Directory"),
														QString(),
														QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


	if (output.isEmpty() == true)
		return;

	QDir outputDir(output);
}


void MainWindow::downsize(const QString& filename)
{
	QFileInfo fileinfo(filename);

	QString ext = fileinfo.suffix();

	if (ext.isEmpty())
		return;

	QString basename = filename.left(filename.size() - ext.size() - 1);

	std::vector<std::pair<QString, double> > imageScales = libraryWidget_->getProjectProperties().imageScales;

	int match = -1;
	for (size_t i = 0; i < imageScales.size(); ++i)
	{
		if (!imageScales[i].first.isEmpty() && basename.endsWith(imageScales[i].first))
		{
			match = i;
			break;
		}
	}

	QString suffix;
	double scale;
	if (match == -1)
	{
		suffix = "";
		scale = 1;
	}
	else
	{
		suffix = imageScales[match].first;
		scale = imageScales[match].second;
		imageScales.push_back(std::make_pair(QString(), 1.0));
	}

	if (scale == 0)
		return;

	QImage image(filename);

	basename.remove(basename.size() - suffix.size(), suffix.size());

	std::set<QString> saved;

	for (size_t i = 0; i < imageScales.size(); ++i)
	{
		if (i != match)
		{
			if (imageScales[i].second == 0)
				continue;

			double ratio = imageScales[i].second / scale;

			if (ratio >= 1)
				continue;		// do not upscale

			if (saved.find(imageScales[i].first) != saved.end())
				continue;		// do not save more than one time

			int w = image.width() * ratio + 0.5f;
			int h = image.height() * ratio + 0.5f;

			QImage newimage = image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

			int quality = -1;
			if (ext == "jpg" || ext == "jpeg")
				quality = 90;

			newimage.save(basename + imageScales[i].first + "." + ext, 0, quality);

			saved.insert(imageScales[i].first);
		}
	}
}

void MainWindow::developerCenter()
{
	QDesktopServices::openUrl(QUrl("http://www.giderosmobile.com/DevCenter"));
}

void MainWindow::helpAndSupport()
{
	QDesktopServices::openUrl(QUrl("http://www.giderosmobile.com/forum"));
}

void MainWindow::apiDocumentation()
{
#if defined(Q_OS_MAC)
	QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("../../Documentation/reference_manual.html")));
#else
	QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("Documentation/reference_manual.html")));
#endif
}


void MainWindow::storeOpenFiles()
{
	if (projectFileName_.isEmpty())
		return;

	QSettings settings;

	QList<QPair<QString, QMap<QString, QVariant> > > projectInfo;

	// read from settings
	{
		QByteArray b = settings.value("projectInfo").toByteArray();

		if (!b.isEmpty())
		{
			QDataStream d(b);
			d >> projectInfo;
		}
	}

	for (int i = projectInfo.size() - 1; i >= 0; --i)
	{
		if (projectInfo[i].first == projectFileName_)
			projectInfo.removeAt(i);
	}

	QMap<QString, QVariant> properties;

	QStringList fileNames;
	QList<QVariant> cursorPositions;
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);

		if (textEdit)
		{
			fileNames << textEdit->fileName();

			int line, index;
			textEdit->sciScintilla()->getCursorPosition(&line, &index);
			cursorPositions << QPoint(line, index);

			if (textEdit == mdiArea_->activeSubWindow())
				properties["activeSubWindow"] = textEdit->fileName();
		}
	}
	properties["fileNames"] = fileNames;
	properties["cursorPositions"] = cursorPositions;

	projectInfo.prepend(qMakePair(projectFileName_, properties));

	while (projectInfo.size() > 10)
		projectInfo.removeLast();

	// write settings
	{
		QByteArray b;
		QDataStream d(&b, QIODevice::WriteOnly);
		d << projectInfo;

		settings.setValue("projectInfo", b);
	}
}

void MainWindow::restoreOpenFiles()
{
	QSettings settings;

	QList<QPair<QString, QMap<QString, QVariant> > > projectInfo;

	// read from settings
	{
		QByteArray b = settings.value("projectInfo").toByteArray();

		if (!b.isEmpty())
		{
			QDataStream d(b);
			d >> projectInfo;
		}
	}

	for (int i = 0; i < projectInfo.size(); ++i)
	{
		if (projectInfo[i].first == projectFileName_)
		{
			QMap<QString, QVariant>& properties = projectInfo[i].second;

			QStringList fileNames = properties["fileNames"].toStringList();
			QList<QVariant> cursorPositions = properties["cursorPositions"].toList();
			QString activeSubWindow = properties["activeSubWindow"].toString();

			TextEdit* active = NULL;
			for (int i = 0; i < fileNames.size(); ++i)
			{
				TextEdit* textEdit = openFile(fileNames[i], true);
				if (textEdit)
				{
					QPoint p = cursorPositions[i].toPoint();
					textEdit->sciScintilla()->setCursorPosition(p.x(), p.y());

					if (fileNames[i] == activeSubWindow)
						active = textEdit;
				}
			}

			if (active)
				mdiArea_->setActiveSubWindow(active);

			break;
		}
	}
}


void MainWindow::stdoutToOutput()
{
    QString output = makeProcess_->readAllStandardOutput();
    outputWidget_->append(output);
}

void MainWindow::stderrToOutput()
{
    QString output = makeProcess_->readAllStandardError();
    outputWidget_->append(output);
}

void MainWindow::makeStarted()
{
    outputWidget_->clear();
    QString dt = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    outputWidget_->append((tr("---------- Compile started at %1 ----------") + "\n").arg(dt));
}

void MainWindow::makeFinished()
{
    outputWidget_->append((tr("========== Compile finished ==========") + "\n"));
}

