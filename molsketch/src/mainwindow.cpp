/***************************************************************************
 *   Copyright (C) 2007-2008 by Harm van Eersel                            *
 *   Copyright (C) 2009 Tim Vandermeersch                                  *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui>
#include <QGridLayout>

#include "mainwindow.h"

#include <molsketch/molecule.h>
#include <molsketch/molview.h>
#include <molsketch/molscene.h>
#include <molsketch/element.h>
#include <molsketch/fileio.h>
#include <molsketch/mollibitem.h>
#include <molsketch/itemplugin.h>
#include <molsketch/osra.h>

#include <molsketch/tool.h>
#include <molsketch/toolgroup.h>

#include <openbabel/mol.h>

using namespace OpenBabel;

// widgets
#include "settings.h"


#define PROGRAM_NAME "Molsketch"
#define PROGRAM_VERSION "Helium"

#define ALT_DOC_PATH ""
#define ALT_LIB_PATH ""
#define ALT_CUSTOM_LIB_PATH ""

#define OB_FILE_FORMATS "All supported types (*.*);;SMILES (*.smi);;MDL Molfile (*.mdl *.mol *.sd *.sdf);;XYZ (*.xyz);;ChemDraw Connection Table (*.ct);;Ghemical (*.gpr)"
#define OB_DEFAULT_FORMAT "CML (*.cml)"
#define GRAPHIC_FILE_FORMATS "Scalable Vector Graphics (*.svg);;Portable Network Graphics (*.png);;Windows Bitmap (*.bmp);;Joint Photo Expert Group (*.jpeg)"
#define GRAPHIC_DEFAULT_FORMAT "Portable Network Graphics (*.png)"
#define OSRA_GRAPHIC_FILE_FORMATS "All supported types (*.*);;Images (*.png *.bmp *.jpg *.jpeg *.gif *.tif *.tiff);;Documents (*.pdf *.ps)"

// Constructor

using namespace Molsketch;

MainWindow::MainWindow()
{
  // Creating the menus and actions
  createView();
  createToolBoxes();
  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  initializeAssistant();

  m_toolGroup = m_scene->toolGroup();
  QList<Tool*> tools = m_toolGroup->tools();

  /*
  QToolBar *toolbar = addToolBar(tr("Tools"));
  toolbar->setObjectName("drawToolBar"); // needed for saveState (window state)
  toolbar->show();
  */

  QHash<QString, QToolBar*> toolbars;
  foreach (Tool *tool, tools) {
    foreach (QAction *action, tool->actions()) {
      QString toolbarName = tool->toolbarName(action);

      // create the toolbar if it doesn't exist already
      if (!toolbars.contains(toolbarName)) {
        toolbars[toolbarName] = addToolBar(toolbarName);
        toolbars[toolbarName]->setObjectName(toolbarName); // needed for saveState (window state)
      }

      toolbars[toolbarName]->addAction(action);
    }
  }

  // Set icon
  QIcon icon;
  icon.addFile(":/images/molsketch.svg");
  icon.addFile(":/images/molsketch.png");
  setWindowIcon(icon);

  // Loading settings
  readSettings();

  QStringList args = qApp->arguments();
  args.removeFirst();
  QRegExp rx("^[^\\-]");
  QStringList loadedFiles;

  foreach(QString fileName, args.filter(rx)) {
    if (fileName.endsWith(".msk")) {
      readMskFile(fileName, m_scene);
      loadedFiles << fileName;
    } else {
      Molecule *mol = Molsketch::loadFile(fileName);
      if (mol) {
        m_scene->addMolecule(mol);
        loadedFiles << fileName;
      } else {
        // Display error message if load fails
        QMessageBox::critical(this,tr(PROGRAM_NAME),tr("Error while loading file"),QMessageBox::Ok,QMessageBox::Ok);
      }
    }
  }
  setCurrentFile("");
  if (loadedFiles.count() == 1) setCurrentFile(loadedFiles.first());

  // Connecting signals and slots
  connect(m_scene->stack(),SIGNAL(cleanChanged(bool)), this, SLOT(documentWasModified( )));
//   connect(m_scene,SIGNAL(newMolecule(QPointF,QString)),this, SLOT(newMolecule(QPointF,QString)));
  connect(m_scene,SIGNAL(editModeChange(int)),this,SLOT(updateEditMode(int)));

}

// Molecuul manipulation methods
// Molecule* MainWindow::newMolecule(const QPointF& position, const QString& element)
// {
//   // Creating a new molecule object
//   Molecule* mol = new Molecule(0,m_scene);
// 
//   // Adding the molecule to the scene
//   //   m_scene->addItem(mol);
//   mol->setPos(position);
// 
//   // Adding a atom to the molecule
//   mol->addAtom(element,position);
// 
//   //   cerr << "Molecule added \n";
// 
//   return mol;
// }


// Event handlers

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (maybeSave())
    {
      writeSettings();
      //if (assistantClient) assistantClient->closeAssistant();
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void MainWindow::newFile()
{
  if (maybeSave())
    {
      m_scene->clear();
      // Resetting the view
      setCurrentFile("");
      m_molView->resetMatrix();
//       setWindowModified(false);
    }
}

void MainWindow::open()
{
  if (maybeSave())
    {
//       QFileDialog dialog(this);
//       dialog.(tr("Open - Molsketch"));
//       dialog.setFilter(tr(OB_FILE_FORMATS));
//       dialog.selectFilter(tr("Chemical Markup Language (*.mol)"));
//       dialog.setFileMode(QFileDialog::ExistingFile);
//       QString fileName;
//       if (dialog.exec()) fileName = dialog.selectedFiles()[0];
      QString fileName = QFileDialog::getOpenFileName(this,tr("Open - Molsketch"), m_lastAccessedPath,
      tr(OB_FILE_FORMATS));
      if (fileName.isEmpty()) return;

        // Save accessed path
        m_lastAccessedPath = QFileInfo(fileName).path();

          // Start a new document
          m_scene->clear();

          Molecule* mol;
          if (fileName.endsWith(".msk")) {
             readMskFile(fileName, m_scene);
             return;
          } else {

            mol = saveAs3DAct->isChecked() ? Molsketch::loadFile3D(fileName) : Molsketch::loadFile(fileName);
          }

          if (mol)
            {
              // Add molecule to scene
              if (mol->canSplit())
                {
                  QList<Molecule*> molList = mol->split();
                  foreach(Molecule* mol,molList) m_scene->addItem(mol);
                }
              else
                {
                  m_scene->addItem(mol);
                }

              // Updating view
              setCurrentFile(fileName);
//               setWindowModified(false);
            }
          else
            {
              // Display error message if load fails
              QMessageBox::critical(this,tr(PROGRAM_NAME),tr("Error while loading file"),QMessageBox::Ok,QMessageBox::Ok);
            }
    }
}

bool MainWindow::save()
{
  if (m_curFile.isEmpty()) {
      return saveAs();
  } else {
    if (m_curFile.endsWith(".msk"))
      writeMskFile(m_curFile, m_scene);
    else if (saveAs3DAct->isChecked() ? Molsketch::saveFile3D(m_curFile, m_scene) : Molsketch::saveFile(m_curFile, m_scene))	{
      m_scene->stack()->setClean();
    } else
      return false;
    }
//   setWindowModified(false);
  return false;
}

bool MainWindow::autoSave()
{
  QFileInfo fileName(m_curFile);

  // Do nothing if there is nothing to save
  if(m_scene->stack()->isClean()) return true;

  // Else construct the filename
  if (!fileName.exists())
   {
     fileName = QDir::homePath() + tr("/untitled.backup.mdl");
   }
  else
   {
     fileName = QFileInfo(m_curFile).path() + QFileInfo(m_curFile).baseName() +  ".backup." + QFileInfo(m_curFile).completeSuffix();
   }

  // And save the file
  if (saveAs3DAct->isChecked() ? Molsketch::saveFile3D(fileName.absoluteFilePath(), m_scene) : Molsketch::saveFile(fileName.absoluteFilePath(), m_scene)) {
    statusBar()->showMessage(tr("Document autosaved"), 10000);
    return true;
  }

  // or display a waring on failure
  statusBar()->showMessage(tr("Autosave failed!"), 10000);
  return false;
}

bool MainWindow::saveAs()
{
  // Get the filename to save under
  QString filter = OB_DEFAULT_FORMAT;
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as - Molsketch"), m_lastAccessedPath, tr(OB_FILE_FORMATS), &filter);
  if (fileName.isEmpty()) return false;

  // Save accessed path
  m_lastAccessedPath = QFileInfo(fileName).path();

  // Finding the right extension
  if (QFileInfo(fileName).suffix().isEmpty())
  {
    int index = filter.indexOf(QRegExp("\\*."));
    filter = filter.remove(0, index + 1);
    index = filter.indexOf(QRegExp("( \\*.)|(\\))"));
    if (index > 0) filter.truncate(index);
    fileName = fileName + filter;
  }
  qDebug() << "Trying to save as " << fileName << "\n";

  if (fileName.endsWith(".msk")) {
    writeMskFile(fileName, m_scene);
    setCurrentFile(fileName);


  // Try to save the document
  } else if (Molsketch::saveFile(fileName,m_scene))
    {
      setCurrentFile(fileName);
      m_scene->stack()->setClean();
      return true;
    }
  else
    {
      // Display error message if saving fails
      QMessageBox::critical(this,tr(PROGRAM_NAME),tr("Invalid name or unknown file type"),QMessageBox::Ok,QMessageBox::Ok);
      return false;
    }

  return false;
}



  bool MainWindow::importDoc()
  {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import - molsKetch"), m_lastAccessedPath, tr(OSRA_GRAPHIC_FILE_FORMATS));

    if (!fileName.isEmpty()) {
      // Save accessed path
      m_lastAccessedPath = QFileInfo(fileName).path();

      m_scene->clear();
      QProgressBar *pb = new QProgressBar(this);
      pb->setMinimum(0);
      pb->setMaximum(0);
      Molecule* mol=call_osra(fileName);
      if (mol) {
        if (mol->canSplit()) {
          QList<Molecule*> molList = mol->split();
          foreach(Molecule* mol,molList) 
            m_scene->addItem(mol);
        } else {
          m_scene->addItem(mol);          
        }
        
        setCurrentFile(fileName);
	return true;
      } else {
        QMessageBox::critical(this, tr(PROGRAM_NAME), tr("Error importing file"), QMessageBox::Ok, QMessageBox::Ok);
	return false;
      }

      /*
      Molecule* mol = Molsketch::loadFile(fileName);
      if (mol)
        {
          m_scene->addMolecule(mol);
          return true;
        }
      else
        {
          // Display error message if load fails
          QMessageBox::critical(this,tr(PROGRAM_NAME),tr("Error while loading file"),QMessageBox::Ok,QMessageBox::Ok);
          return false;
        }
        */
    }
    
    return false;
  }

bool MainWindow::exportDoc()
{
  // Getting the filename
  QString filter = GRAPHIC_DEFAULT_FORMAT;
  QString fileName = QFileDialog::getSaveFileName(this,tr("Export - Molsketch"), m_lastAccessedPath,tr(GRAPHIC_FILE_FORMATS), &filter);

  // Abort if filename is empty
  if (fileName.isEmpty()) return false;

  // Save accessed path
  m_lastAccessedPath = QFileInfo(fileName).path();

  // Finding the right extension
  if (QFileInfo(fileName).suffix().isEmpty())
  {
    int index = filter.indexOf(QRegExp("\\*."));
    filter = filter.remove(0, index + 1);
    index = filter.indexOf(QRegExp("( \\*.)|(\\))"));
    if (index > 0) filter.truncate(index);
    fileName = fileName + filter;
  }
  qDebug() << "Trying to export as " << fileName << "\n";

  m_lastAccessedPath = QFileInfo(fileName).path();

  // Try to export the file
  if (fileName.endsWith(".svg")) return Molsketch::saveToSVG(fileName, m_scene);

  if (Molsketch::exportFile(fileName,m_scene))
    {
      return true;
    }
  else
    {
      QMessageBox::critical(this,tr(PROGRAM_NAME),tr("Error while exporting file"),QMessageBox::Ok,QMessageBox::Ok);
      return false;
    }
}

void MainWindow::changeColor () {
	QColor new_color = QColorDialog::getColor(m_scene ->color(), this );
	if ( new_color.isValid () ) {
		m_scene ->setColor (new_color);
	}
	QPixmap pix(24, 24);
    pix.fill(new_color);
    colorAct->setIcon(pix);
	
}


void MainWindow::paintSceneOn (QPrinter *printer) {
	Molsketch::printFile(*printer,m_scene);
}

bool MainWindow::print()
{
	QPrintPreviewDialog printPreview;
	connect(&printPreview, SIGNAL(paintRequested(QPrinter * )), this, SLOT(paintSceneOn(QPrinter *)));

	printPreview.exec ();
	
	/*
  // Creating a new printerobject
  QPrinter printer(QPrinter::HighResolution);

  // Prompt for the printoptions
  QPrintDialog printDialog(&printer, this);

  // Try to print the scene
  if (printDialog.exec() == QDialog::Accepted)
    {
      if (Molsketch::printFile(printer,m_scene))
      {
      	return true;
      }
      else
      {
      	QMessageBox::critical(this,tr(PROGRAM_NAME),tr("Error while printing file"),QMessageBox::Ok,QMessageBox::Ok);
      	return false;
      }
    }
  else
    {
      return false;
    }
	 
	 */
	return true;
}

void MainWindow::zoomIn()
{
  m_molView->scale(2,2);
}

void MainWindow::zoomOut()
{
  m_molView->scale(0.5,0.5);
}

void MainWindow::zoomReset()
{
  m_molView->resetMatrix();
}

void MainWindow::zoomFit()
{
  m_molView->fitInView(m_scene->itemsBoundingRect(),Qt::KeepAspectRatio);
}

void MainWindow::assistant()
{
  QFileInfo file(ALT_DOC_PATH + QString("/index.html"));
  if (!file.exists()) file.setFile(QApplication::applicationDirPath() + "/doc/en/index.html");
  if (!file.exists()) file.setFile(QApplication::applicationDirPath() + "/../share/doc/molsketch/doc/en/index.html");
  //assistantClient->showPage(file.absoluteFilePath());
}



void MainWindow::about()
{
  QMessageBox::about(this, tr("About"),
                     tr("<H3>About Molsketch</H3> <P> Molsketch is a program for drawing molecular structures developed by Harm van Eersel at the <A href=\"http://www.tue.nl\">Eindhoven University of Technology</A>.<P> For more info check <A href=\"http://molsketch.sourceforge.net\">http://molsketch.sourceforge.net</A>  <P> It is <A href=\"http://www.gnu.org/philosophy/free-sw.html\">free software</A> and available under the <A>GPL</A>. <P> Special thanks to: <UL><LI>Prof. Dr. H. Zantema (coach of the initial version)</LI> <LI> Davy van der Vaart (tester)</LI><LI>Frans Visscher (tester)</LI><LI>Carsten Niehaus (reviewer)</LI></UL>Copyright 2007 - 2008, Harm van Eersel"));
}

void MainWindow::documentWasModified()
{
  setWindowModified(!m_scene->stack()->isClean());
}

void MainWindow::updateEditMode(int mode)
{
  Q_UNUSED(mode);
  m_molView->setDragMode(QGraphicsView::NoDrag);

}

// Widget creators

void MainWindow::createActions()
{
  newAct = new QAction(QIcon(":/images/document-new.png"), tr("&New"),this);
  newAct->setShortcut(tr("Ctrl+N"));
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

  openAct = new QAction(QIcon(":/images/document-open.png"),tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct = new QAction(QIcon(":/images/document-save.png"), tr("&Save"), this);
  saveAct->setShortcut(tr("Ctrl+S"));
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  saveAsAct = new QAction(QIcon(":/images/document-save-as.png"),tr("Save &As..."), this);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  saveAs3DAct = new QAction(QIcon(""),tr("Save As &3D"), this);
  saveAs3DAct->setStatusTip(tr("Save the document as 3D"));
  saveAs3DAct->setCheckable(true);
//   connect(saveAs3DAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  autoSaveAct = new QAction(tr("Autosave document"), this);
  m_autoSaveTimer = new QTimer(this);
//   m_autoSaveTimer->setInterval(m_autoSaveTime);
  connect(autoSaveAct, SIGNAL(triggered()), this, SLOT(autoSave()));
  connect(m_autoSaveTimer, SIGNAL(timeout()), autoSaveAct, SIGNAL(triggered()));
//   m_autoSaveTimer->start();

  importAct = new QAction(QIcon(":/images/document-import.png"),tr("&Import..."), this);
  importAct->setShortcut(tr("Ctrl+I"));
  importAct->setStatusTip(tr("Insert an existing molecule into the document"));
  connect(importAct, SIGNAL(triggered()), this, SLOT(importDoc()));

  exportAct = new QAction(QIcon(":/images/document-export.png"),tr("&Export..."), this);
  exportAct->setShortcut(tr("Ctrl+E"));
  exportAct->setStatusTip(tr("Export the current document as a picture"));
  connect(exportAct, SIGNAL(triggered()), this, SLOT(exportDoc()));

  printAct = new QAction(QIcon(":/images/document-print.png"),tr("&Print..."), this);
  printAct->setShortcut(tr("Ctrl+P"));
  printAct->setStatusTip(tr("Print the current document"));
  connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

  exitAct = new QAction(QIcon(":/images/application-exit.png"),tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // Edit actions
  undoAct = m_scene->stack()->createUndoAction(this);
  undoAct->setIcon(QIcon(":/images/edit-undo.png"));
  undoAct->setShortcut(tr("Ctrl+Z"));
  undoAct->setStatusTip(tr("Undo the last action"));

  redoAct = m_scene->stack()->createRedoAction(this);
  redoAct->setIcon(QIcon(":/images/edit-redo.png"));
  redoAct->setShortcut(tr("Ctrl+Shift+Z"));
  redoAct->setStatusTip(tr("Redo the last action"));

  cutAct = new QAction(QIcon(":/images/edit-cut.png"), tr("Cu&t"), this);
  cutAct->setShortcut(tr("Ctrl+X"));
  cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                          "clipboard"));
  connect(cutAct, SIGNAL(triggered()), m_scene, SLOT(cut()));

  copyAct = new QAction(QIcon(":/images/edit-copy.png"), tr("&Copy"), this);
  copyAct->setShortcut(tr("Ctrl+C"));
  copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                           "clipboard"));
  connect(copyAct, SIGNAL(triggered()), m_scene, SLOT(copy()));

  pasteAct = new QAction(QIcon(":/images/edit-paste.png"), tr("&Paste"), this);
  pasteAct->setShortcut(tr("Ctrl+V"));
  pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                            "selection"));
  connect(pasteAct, SIGNAL(triggered()), m_scene, SLOT(paste()));

  convertImageAct = new QAction(QIcon(""), tr("C&onvert Image to Mol"),this);
  convertImageAct->setShortcut(tr("Ctrl+M"));
  convertImageAct->setStatusTip(tr("Convert Image to Mol using OSRA"));
  connect(convertImageAct, SIGNAL(triggered()), m_scene, SLOT(convertImage()));

  selectAllAct = new QAction(QIcon(":/images/edit-select-all.png"), tr("&Select all"),this);
  selectAllAct->setShortcut(tr("Ctrl+A"));
  selectAllAct->setStatusTip(tr("Selects all elements on the scene"));
  connect(selectAllAct, SIGNAL(triggered()), m_scene, SLOT(selectAll()));

  alignAct = new QAction(QIcon(""), tr("Align to grid"), this);
  alignAct->setStatusTip(tr("Align all elements on the scene to the grid"));
  connect(alignAct, SIGNAL(triggered()), m_scene, SLOT(alignToGrid()));

  prefAct = new QAction(QIcon(":/images/configure.png"),tr("Edit Pre&ferences..."),this);
  prefAct->setShortcut(tr("Ctrl+F"));
  prefAct->setStatusTip(tr("Edit your preferences"));
  connect(prefAct, SIGNAL(triggered()), this, SLOT(editPreferences()));
	
	minimiseModeAct = new QAction(QIcon(":/images/minimise.png"),tr("Energy Refinement"),this);
	minimiseModeAct->setCheckable(true);
	//minimiseModeAct->setShortcut(tr("Ctrl+F"));
	minimiseModeAct->setStatusTip(tr("Adjust Geometry"));
//	textModeAct->setStatusTip(tr("Go to the minimise mode"));
	connect(minimiseModeAct, SIGNAL(triggered()), this, SLOT(setMinimiseMode()));

    colorAct = new QAction (tr("Color"), this);;
	QPixmap pix(24, 24);
    pix.fill(m_scene ->color());
    colorAct->setIcon(pix);
	connect (colorAct, SIGNAL (triggered ()), this, SLOT (changeColor ()));


  // Zoom actions
  zoomInAct = new QAction(QIcon(":/images/zoom-in.png"),tr("Zoom &In"), this);
  zoomInAct->setShortcut(tr("Ctrl++"));
  zoomInAct->setStatusTip(tr("Zoom in on the canvas"));
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAct = new QAction(QIcon(":/images/zoom-out.png"),tr("Zoom &Out"), this);
  zoomOutAct->setShortcut(tr("Ctrl+-"));
  zoomOutAct->setStatusTip(tr("Zoom out on the canvas"));
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

  zoomResetAct = new QAction(QIcon(":/images/zoom-original.png"),tr("Zoom &Reset"), this);
  zoomResetAct->setShortcut(tr("Ctrl+="));
  zoomResetAct->setStatusTip(tr("Reset the zoom level"));
  connect(zoomResetAct, SIGNAL(triggered()), this, SLOT(zoomReset()));

  zoomFitAct = new QAction(QIcon(":/images/zoom-fit-best.png"),tr("Zoom &Fit"), this);
  zoomFitAct->setShortcut(tr("Ctrl+*"));
  zoomFitAct->setStatusTip(tr("Fit to screen"));
  connect(zoomFitAct, SIGNAL(triggered()), this, SLOT(zoomFit()));

  // Help actions
  helpContentsAct = new QAction(QIcon(":/images/help-contents.png"),tr("&Help Contents..."), this);
  helpContentsAct->setShortcut(tr("F1"));
  helpContentsAct->setStatusTip(tr("Show the application's help contents"));
  connect(helpContentsAct, SIGNAL(triggered()), this, SLOT(assistant()));
  
  submitBugAct = new QAction(QIcon(""),tr("Submit &Bug..."), this);
  submitBugAct->setStatusTip(tr("Open the browser with the bug tracker"));
  connect(submitBugAct, SIGNAL(triggered()), this, SLOT(submitBug()));

  aboutAct = new QAction(QIcon(":/images/help-about.png"),tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // Setting actions in their initial states
  cutAct->setEnabled(false);
  copyAct->setEnabled(false);
  pasteAct->setEnabled(false);
  connect(m_scene, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
  connect(m_scene, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
  connect(m_scene, SIGNAL(pasteAvailable(bool)), pasteAct, SLOT(setEnabled(bool)));
}


void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAct);
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addAction(saveAs3DAct);
  fileMenu->addSeparator();
  fileMenu->addAction(importAct);
  fileMenu->addAction(exportAct);
  fileMenu->addAction(printAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(undoAct);
  editMenu->addAction(redoAct);
  editMenu->addSeparator();
  editMenu->addAction(cutAct);
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);
  editMenu->addAction(convertImageAct);
  editMenu->addSeparator();
  editMenu->addAction(selectAllAct);
  editMenu->addAction(alignAct);
  editMenu->addSeparator();
  editMenu->addSeparator();
  editMenu->addAction(prefAct);

  toolsMenu = menuBar()->addMenu(tr("&Insert Items"));
  foreach (ItemPluginFactory *factory, ItemPluginFactory::factories()) {
    QAction *pluginAct = new QAction(factory->input() + " -> " + factory->output(), this);
    pluginAct->setData(factory->output());
    connect(pluginAct, SIGNAL(triggered()), this, SLOT(pluginActionTriggered()));
    toolsMenu->addAction(pluginAct);
  }
	
  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);
  viewMenu->addAction(zoomResetAct);
  viewMenu->addAction(zoomFitAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(helpContentsAct);
  helpMenu->addSeparator();
  helpMenu->addAction(submitBugAct);
  helpMenu->addSeparator();
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->setObjectName("file-toolbar");
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  fileToolBar->setIconSize(QSize(22,22));
  fileToolBar->addAction(newAct);
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);
  fileToolBar->addAction(importAct);
  fileToolBar->addAction(exportAct);
  fileToolBar->addAction(printAct);

  editToolBar = addToolBar(tr("Edit"));
  editToolBar->setObjectName("edit-toolbar");
  editToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  editToolBar->setIconSize(QSize(22,22));
  editToolBar->addAction(undoAct);
  editToolBar->addAction(redoAct);
  editToolBar->addSeparator();
  editToolBar->addAction(cutAct);
  editToolBar->addAction(copyAct);
  editToolBar->addAction(pasteAct);
  editToolBar->addSeparator();
	editToolBar ->addAction (colorAct);
  editToolBar->addSeparator();
	
	
  zoomToolBar = addToolBar(tr("Zoom"));
  zoomToolBar->setObjectName("zoom-toolbar");
  zoomToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  zoomToolBar->setIconSize(QSize(22,22));
  zoomToolBar->addAction(zoomInAct);
  zoomToolBar->addAction(zoomOutAct);
  zoomToolBar->addAction(zoomResetAct);
  zoomToolBar->addAction(zoomFitAct);
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createToolBoxes()
{
  // Creating the dockwidgets
  toolBoxDock = new QDockWidget(tr("Toolbox"));
  toolBoxDock->setObjectName("toolbox-dockwidget");
  toolBoxDock->setMinimumWidth(270);


  // Create libraries
  genericLib = new QListWidget;
  customLib = new QListWidget;

  QPushButton* addButton = new QPushButton(tr("Add..."));
  QPushButton* delButton = new QPushButton(tr("Delete"));

  genericLib->setAlternatingRowColors(true);
  genericLib->setIconSize(QSize(64,64));

  customLib->setAlternatingRowColors(true);
  customLib->setIconSize(QSize(128,128));

  // Declaring variables
  QDir dir;
  Molecule* mol;

  // Loading generic molecules
  dir.setPath(ALT_LIB_PATH);
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) genericLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }
  
  dir.setPath(QDir::homePath() + "/.molsketch/library");
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) genericLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }

  dir.setPath(QApplication::applicationDirPath() + "/../share/molsketch/library");
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) genericLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }

  dir.setPath(QApplication::applicationDirPath() + "/library");
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) genericLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }

  // Loading custom molecules
  dir.setPath(ALT_CUSTOM_LIB_PATH);
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) customLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }
  
  dir.setPath(QDir::homePath() + "/.molsketch/library/custom");
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) customLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }

  dir.setPath(QApplication::applicationDirPath() + "/../share/molsketch/library/custom");
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) customLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }

  dir.setPath(QApplication::applicationDirPath() + "/library/custom");
  for (unsigned int i = 0; i < dir.count(); i++)
    {
      mol = Molsketch::loadFile(dir.filePath(dir[i]));
      if (mol) customLib->addItem(new MolLibItem(mol,dir.filePath(dir[i])));
    }



  // Composing customLib
  QHBoxLayout* hLayoutCL = new QHBoxLayout;
  hLayoutCL->addWidget(addButton);
  hLayoutCL->addWidget(delButton);
  QVBoxLayout* vLayoutCL = new QVBoxLayout;
  vLayoutCL->addWidget(customLib);
  vLayoutCL->addLayout(hLayoutCL);

  QFrame* frameCustomLib = new QFrame;
  frameCustomLib->setLayout(vLayoutCL);

  // Create a library toolbox and add the libraries
  toolBox = new QToolBox;
//   toolBox->addItem(elementLib,tr("Elements"));
  toolBox->addItem(genericLib, tr("Generic Molecules"));
  toolBox->addItem(frameCustomLib,tr("Custom Molecules"));
  toolBoxDock->setWidget(toolBox);

  // Placing the dockwidgets in their default position
  setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  addDockWidget(Qt::LeftDockWidgetArea,toolBoxDock);


  // Connecting signals and slots
  connect(genericLib,SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addMolecule(QListWidgetItem*)));
  connect(customLib,SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addMolecule(QListWidgetItem*)));
  connect(addButton, SIGNAL(released()), this, SLOT(addCustomMol()));
  connect(delButton, SIGNAL(released()), this, SLOT(delCustomMol()));
}

void MainWindow::addMolecule(QListWidgetItem *item)
{
  // Extract the molecule and add it to the sceneMolecule* m
  MolLibItem *libItem = dynamic_cast<MolLibItem*>(item);
  if (!libItem)
    return;
  m_scene->addMolecule(libItem->getMolecule());
}
 
void MainWindow::addCustomMol()
{
  foreach(QGraphicsItem* item, m_scene->selectedItems())
  {
    if (item->type() == Molecule::Type)
      {
        Molecule* mol = dynamic_cast<Molecule*>(item);
        QString name = QInputDialog::getText(this,tr("Enter a name"),tr("Enter a name for this item:"),QLineEdit::Normal,mol->formula());
        customLib->addItem(new MolLibItem(mol,name));
      }
  }
}

void MainWindow::delCustomMol()
{
  //Check whether an item is selected
  if (!customLib->currentItem()) return;

  if (QMessageBox::warning(this,tr("Are you sure?"),tr("Do you really want to delete this item?"),QMessageBox::Yes|QMessageBox::No,QMessageBox::No) == QMessageBox::Yes)
    {
      MolLibItem* item = dynamic_cast<MolLibItem*>(customLib->currentItem());
      QFile::remove(item->getFileName().filePath());
      delete item;
    }
}
  
void MainWindow::pluginActionTriggered()
{
  Q_CHECK_PTR(m_scene);

  QAction *action = qobject_cast<QAction*>(sender());
  Q_CHECK_PTR(action);

  QString output = action->data().toString();
  ItemPlugin *item = ItemPluginFactory::createInstance(output);
  m_scene->addItem(item);
}


void MainWindow::createView()
{
  // Create new scene
  m_scene = new MolScene(this);

  // Create and set view
  m_molView = new MolView(m_scene);

  // Placing the view widget
  setCentralWidget(m_molView);
  m_molView->show();
}

void MainWindow::initializeAssistant()
{
  //assistantClient = new QAssistantClient("", this);
  assistantClient = nullptr;

  QStringList arguments;
  QFileInfo file(ALT_DOC_PATH + QString("/molsketch.adp"));
  if (!file.exists()) file.setFile(QApplication::applicationDirPath() + "/doc/en/molsketch.adp");
  if (!file.exists()) file.setFile(QApplication::applicationDirPath() + "/../share/doc/molsketch/doc/en/molsketch.adp");

  arguments << "-profile" << file.absoluteFilePath();
  //assistantClient->setArguments(arguments);
}

// Auxillary methods

void MainWindow::readSettings()
{
  // Reading the settings
  QSettings settings;

  // Setting the window position
  QPoint pos = settings.value("pos",QPoint(100,100)).toPoint();
  QSize size = settings.value("size",QSize(800,600)).toSize();
  resize(size);
  move(pos);

  // Restoring the state of the toolbars and dockwidgets
  //QByteArray state = settings.value("window-state", QByteArray("\0\0\0\xff\0\0\0\0\xfd\0\0\0\x2\0\0\0\0\0\0\x1\xe\0\0\x2#\xfc\x2\0\0\0\x2\xfb\0\0\0$\0t\0o\0o\0l\0\x62\0o\0x\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x1\0\0\0\x43\0\0\x1G\0\0\0\xbe\0\xff\xff\xff\xfb\0\0\0$\0i\0n\0\x66\0o\0\x62\0o\0x\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x1\0\0\x1\x90\0\0\0\xd6\0\0\0R\0\xff\xff\xff\0\0\0\x1\xff\xff\xff\xfa\0\0\x2#\xfc\x2\0\0\0\x1\xfb\0\0\0\x32\0p\0\x65\0r\0i\0o\0\x64\0i\0\x63\0-\0t\0\x61\0\x62\0l\0\x65\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x3\0\0\x3 \0\0\x2g\0\0\x1\xcc\0\0\x1\x18\0\0\x2\x87\0\0\x2#\0\0\0\x1\0\0\0\x4\0\0\0\x1\0\0\0\b\xfc\0\0\0\x1\0\0\0\x2\0\0\0\x3\0\0\0\x18\0\x66\0i\0l\0\x65\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\0\0\0\0\x1\b\0\0\0\0\0\0\0\0\0\0\0\x18\0\x65\0\x64\0i\0t\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\x1\b\0\0\x1\xf9\0\0\0\0\0\0\0\0\0\0\0\x18\0z\0o\0o\0m\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\x3\x1\0\0\0\x9a\0\0\0\0\0\0\0\0")).toByteArray();
  //QByteArray state = settings.value("window-state", QByteArray("\0\0\0\xff\0\0\0\0\xfd\0\0\0\x1\0\0\0\0\0\0\x1\xe\0\0\x1\xc9\xfc\x2\0\0\0\x2\xfb\0\0\0$\0t\0o\0o\0l\0\x62\0o\0x\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x1\0\0\0x\0\0\x1I\0\0\0\xe8\0\xff\xff\xff\xfb\0\0\0$\0i\0n\0\x66\0o\0\x62\0o\0x\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x1\0\0\x1\xc7\0\0\0z\0\0\0l\0\xff\xff\xff\0\0\x1\xe1\0\0\x1\xc9\0\0\0\x1\0\0\0\x4\0\0\0\x1\0\0\0\b\xfc\0\0\0\x3\0\0\0\0\0\0\0\x2\0\0\0\n\0T\0o\0o\0l\0s\x3\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x10\0R\0\x65\0\x61\0\x63\0t\0i\0o\0n\x3\0\0\0\xfe\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x2\0\0\0\x3\0\0\0\x18\0\x66\0i\0l\0\x65\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x18\0\x65\0\x64\0i\0t\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\x1\x33\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x18\0z\0o\0o\0m\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\x2s\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x2\0\0\0\x2\0\0\0\b\0\x44\0r\0\x61\0w\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\n\0R\0i\0n\0g\0s\x1\0\0\x2G\xff\xff\xff\xff\0\0\0\0\0\0\0\0")).toByteArray();

  QByteArray state = settings.value("window-state", QByteArray("@ByteArray(\0\0\0\xff\0\0\0\0\xfd\0\0\0\x1\0\0\0\0\0\0\x1\xe\0\0\x3N\xfc\x2\0\0\0\x2\xfb\0\0\0$\0t\0o\0o\0l\0\x62\0o\0x\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x1\0\0\0x\0\0\x2\x65\0\0\0\xe8\0\xff\xff\xff\xfb\0\0\0$\0i\0n\0\x66\0o\0\x62\0o\0x\0-\0\x64\0o\0\x63\0k\0w\0i\0\x64\0g\0\x65\0t\x1\0\0\x2\xe3\0\0\0\xe3\0\0\0l\0\xff\xff\xff\0\0\x6l\0\0\x3N\0\0\0\x1\0\0\0\x4\0\0\0\x1\0\0\0\b\xfc\0\0\0\x2\0\0\0\x2\0\0\0\x3\0\0\0\x18\0\x66\0i\0l\0\x65\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x18\0\x65\0\x64\0i\0t\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\x1\x33\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x18\0z\0o\0o\0m\0-\0t\0o\0o\0l\0\x62\0\x61\0r\x1\0\0\x2s\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x2\0\0\0\x4\0\0\0\b\0\x44\0r\0\x61\0w\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\n\0R\0i\0n\0g\0s\x1\0\0\x2G\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\n\0T\0o\0o\0l\0s\x1\0\0\x3t\xff\xff\xff\xff\0\0\0\0\0\0\0\0\0\0\0\x10\0R\0\x65\0\x61\0\x63\0t\0i\0o\0n\x1\0\0\x4\x80\xff\xff\xff\xff\0\0\0\0\0\0\0\0)")).toByteArray();


  restoreState(state);

  // Load preferences
  readPreferences(settings);
}

void MainWindow::readPreferences(const QSettings & settings)
{
  // Loading auto-save time
  m_autoSaveTime = settings.value("auto-save-time", 300000).toInt();
  m_autoSaveTimer->setInterval(m_autoSaveTime);
  m_autoSaveTimer->start();

  // Loading paths
  m_libPath = settings.value("library-path","/usr/share/molsketch/library/").toString();
  m_lastAccessedPath = settings.value("last-save-path", QDir::homePath()).toString();

  // Load the draw settings
  m_scene->setAtomSize(settings.value("atom-size",30).toDouble());
  m_scene->setAutoAddHydrogen(settings.value("auto-add-hydrogen",true).toBool());
  m_scene->setCarbonVisible(settings.value("carbon-visible",false).toBool());
  m_scene->setHydrogenVisible(settings.value("hydrogen-visible",true).toBool());
  m_scene->setChargeVisible(settings.value("charge-visible",true).toBool());
  m_scene->setElectronSystemsVisible(settings.value("electronSystems-visible", false).toBool());

  m_scene->setAtomSymbolFont(settings.value("atom-symbol-font").value<QFont>());

  //m_scene->setBondLength(settings.value("bond-length",40).toDouble());
  //m_scene->setBondWidth(settings.value("bond-width",1).toDouble());
  //m_scene->setBondAngle(settings.value("bond-angle",30).toInt());

  // Update the scene contents
  m_scene->update();
}

void MainWindow::writeSettings()
{
  // Saving the settings
  QSettings settings;

  // Saving the window position
  settings.setValue("pos",pos());
  settings.setValue("size",size());

  // Saving the state of the toolbars and dockwidgets
  settings.setValue("window-state", saveState());

  // Saving paths
//   settings.setValue("library-path",m_libPath);
  settings.setValue("last-save-path", m_lastAccessedPath);

  // Saving preferences
//   settings.setValue("atom-size",m_scene->atomSize());
   settings.setValue("auto-add-hydrogen",m_scene->autoAddHydrogen());
//   settings.setValue("bond-length",m_scene->bondLength());
//   settings.setValue("bond-angle",m_scene->bondAngle());
   settings.setValue("carbon-visible",m_scene->carbonVisible());
   settings.setValue("hydrogen-visible",m_scene->hydrogenVisible());
   settings.setValue("charge-visible",m_scene->chargeVisible());
   settings.setValue("electronSystems-visible",m_scene->electronSystemsVisible());

}


bool MainWindow::maybeSave()
{
  /* TODO */
  if (isWindowModified())
    {
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this,tr(PROGRAM_NAME),
                                 tr("This document has been modified.\n"
                                    "Do you want to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      if (ret == QMessageBox::Save)
        return save();
      else if (ret == QMessageBox::Cancel)
        return false;
    }
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
  // Synthesizing the correct name
  m_curFile = fileName;
  QString shownName;
  if (m_curFile.isEmpty())
    shownName = tr("untitled.mol");
  else
    shownName = strippedName(m_curFile);

  // Setting the windowtitle
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr(PROGRAM_NAME)));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}


void MainWindow::editPreferences( )
{
  QSettings settings;

  // Opens the settings dialog
  SettingsDialog dialog;
  dialog.exec();
  readPreferences(settings);
}

void MainWindow::submitBug()
{
  // Opens a browser with the bug tracker
  QDesktopServices::openUrl(QUrl("http://sourceforge.net/tracker/?func=add&group_id=191562&atid=937880"));
}

