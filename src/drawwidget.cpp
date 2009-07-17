/**********************************************************************
  DrawWidget - Widget containing draw buttons

  Copyright (C) 2009 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://github.com/timvdm/Molsketch>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "drawwidget.h"
#include "molscene.h"

namespace Molsketch {

  DrawWidget::DrawWidget(MolScene *scene) : QWidget()
  {
    Q_CHECK_PTR(scene);
    
    m_scene = scene;
    ui.setupUi(this);

    // H C N ... Br I X R Any
    connect(ui.buttonH, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonC, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonN, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonO, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonP, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonS, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonF, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonCl, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonBr, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonI, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonX, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonR, SIGNAL(clicked()), this, SLOT(textButtonClicked()));
    connect(ui.buttonAny, SIGNAL(clicked()), this, SLOT(textButtonClicked()));

    // bonds
    connect(ui.buttonSingleBond, SIGNAL(clicked()), this, SLOT(singleBondClicked()));
    connect(ui.buttonDoubleBond, SIGNAL(clicked()), this, SLOT(doubleBondClicked()));
    connect(ui.buttonTripleBond, SIGNAL(clicked()), this, SLOT(tripleBondClicked()));
    connect(ui.buttonWedge, SIGNAL(clicked()), this, SLOT(wedgeBondClicked()));
    connect(ui.buttonHash, SIGNAL(clicked()), this, SLOT(hashBondClicked()));
    connect(ui.buttonWedgeOrHash, SIGNAL(clicked()), this, SLOT(wedgeOrHashBondClicked()));
    connect(ui.buttonCisOrTrans, SIGNAL(clicked()), this, SLOT(cisOrTransBondClicked()));

    // rings
    connect(ui.buttonRing3, SIGNAL(clicked()), this, SLOT(ring3Clicked()));
    connect(ui.buttonRing4, SIGNAL(clicked()), this, SLOT(ring4Clicked()));
    connect(ui.buttonRing5, SIGNAL(clicked()), this, SLOT(ring5Clicked()));
    connect(ui.buttonRing6, SIGNAL(clicked()), this, SLOT(ring6Clicked()));
    connect(ui.buttonRing7, SIGNAL(clicked()), this, SLOT(ring7Clicked()));
    connect(ui.buttonRing8, SIGNAL(clicked()), this, SLOT(ring8Clicked()));
    connect(ui.buttonRing5Arom, SIGNAL(clicked()), this, SLOT(aromaticRing5Clicked()));
    connect(ui.buttonRing6Arom, SIGNAL(clicked()), this, SLOT(aromaticRing6Clicked()));
  }


  void DrawWidget::uncheckAllButtons()
  {
    uncheckAtomButtons();
    uncheckBondButtons();
    uncheckRingButtons();
  }

  void DrawWidget::uncheckAtomButtons()
  {
    ui.buttonH->setChecked(false);
    ui.buttonC->setChecked(false);
    ui.buttonN->setChecked(false);
    ui.buttonO->setChecked(false);
    ui.buttonP->setChecked(false);
    ui.buttonS->setChecked(false);
    ui.buttonF->setChecked(false);
    ui.buttonCl->setChecked(false);
    ui.buttonBr->setChecked(false);
    ui.buttonI->setChecked(false);
    ui.buttonX->setChecked(false);
    ui.buttonR->setChecked(false);
    ui.buttonAny->setChecked(false);
  }
   
  void DrawWidget::uncheckBondButtons()
  {
    ui.buttonSingleBond->setChecked(false);
    ui.buttonDoubleBond->setChecked(false);
    ui.buttonTripleBond->setChecked(false);
    ui.buttonWedge->setChecked(false);
    ui.buttonHash->setChecked(false);
    ui.buttonWedgeOrHash->setChecked(false);
    ui.buttonCisOrTrans->setChecked(false);
  }

  void DrawWidget::uncheckRingButtons()
  {  
    ui.buttonRing3->setChecked(false);
    ui.buttonRing4->setChecked(false);
    ui.buttonRing5->setChecked(false);
    ui.buttonRing6->setChecked(false);
    ui.buttonRing7->setChecked(false);
    ui.buttonRing8->setChecked(false);
    ui.buttonRing5Arom->setChecked(false);
    ui.buttonRing6Arom->setChecked(false);
  }

  QPushButton* DrawWidget::checkedBondButton()
  {
    if (ui.buttonSingleBond->isChecked())
      return ui.buttonSingleBond;
    if (ui.buttonDoubleBond->isChecked())
      return ui.buttonDoubleBond;
    if (ui.buttonTripleBond->isChecked())
      return ui.buttonTripleBond;
    if (ui.buttonWedge->isChecked())
      return ui.buttonWedge;
    if (ui.buttonHash->isChecked())
      return ui.buttonHash;
    if (ui.buttonWedgeOrHash->isChecked())
      return ui.buttonWedgeOrHash;
    if (ui.buttonCisOrTrans->isChecked())
      return ui.buttonCisOrTrans;  
    return 0;
  }

  QPushButton* DrawWidget::checkedAtomButton()
  {
    if (ui.buttonH->isChecked())
      return ui.buttonH;
    if (ui.buttonC->isChecked())
      return ui.buttonC;
    if (ui.buttonN->isChecked())
      return ui.buttonN;
    if (ui.buttonO->isChecked())
      return ui.buttonO;
    if (ui.buttonP->isChecked())
      return ui.buttonP;
    if (ui.buttonS->isChecked())
      return ui.buttonS;
    if (ui.buttonF->isChecked())
      return ui.buttonF;
    if (ui.buttonCl->isChecked())
      return ui.buttonCl;
    if (ui.buttonBr->isChecked())
      return ui.buttonBr;
    if (ui.buttonI->isChecked())
      return ui.buttonI;
    if (ui.buttonX->isChecked())
      return ui.buttonX;
    if (ui.buttonR->isChecked())
      return ui.buttonR;
    if (ui.buttonAny->isChecked())
      return ui.buttonAny;
    return 0;
  }

  //
  // Atoms
  //

  void DrawWidget::textButtonClicked()
  {
    QObject *sender = QObject::sender();
    QPushButton *button = qobject_cast<QPushButton*>(sender);
    QString text = button->text();
    m_scene->setElement(text);

    uncheckAtomButtons();
    if (text == "H")
      ui.buttonH->setChecked(true);
    else if (text == "C")
      ui.buttonC->setChecked(true);
    else if (text == "N")
      ui.buttonN->setChecked(true);
    else if (text == "O")
      ui.buttonO->setChecked(true);
    else if (text == "P")
      ui.buttonP->setChecked(true);
    else if (text == "S")
      ui.buttonS->setChecked(true);
    else if (text == "F")
      ui.buttonF->setChecked(true);
    else if (text == "Cl")
      ui.buttonCl->setChecked(true);
    else if (text == "Br")
      ui.buttonBr->setChecked(true);
    else if (text == "I")
      ui.buttonI->setChecked(true);
    else if (text == "X")
      ui.buttonX->setChecked(true);
    else if (text == "R")
      ui.buttonR->setChecked(true);
    else if (text == "Any")
      ui.buttonAny->setChecked(true);
   
    if (!checkedBondButton())
      ui.buttonSingleBond->setChecked(true);
    uncheckRingButtons();
  }

  //
  // Bonds
  //

  void DrawWidget::singleBondClicked()
  {
    m_scene->setBondOrder(1);
    m_scene->setBondType(Bond::InPlane);

    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonSingleBond->setChecked(true);
    uncheckRingButtons();
  }

  void DrawWidget::doubleBondClicked()
  {
    m_scene->setBondOrder(2);
    m_scene->setBondType(Bond::InPlane);

    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonDoubleBond->setChecked(true);
    uncheckRingButtons();
   }

  void DrawWidget::tripleBondClicked()
  {
    m_scene->setBondOrder(3);
    m_scene->setBondType(Bond::InPlane);  
    
    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonTripleBond->setChecked(true);
    uncheckRingButtons();
  }

  void DrawWidget::wedgeBondClicked()
  {
    // also sets order
    m_scene->setBondType(Bond::Wedge);
 
    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonWedge->setChecked(true);    
    uncheckRingButtons();
  }
  
  void DrawWidget::hashBondClicked()
  {
    // also sets order
    m_scene->setBondType(Bond::Hash);  

    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonHash->setChecked(true);
    uncheckRingButtons();
  }
  
  void DrawWidget::wedgeOrHashBondClicked()
  {
    // also sets order
    m_scene->setBondType(Bond::WedgeOrHash);  
    
    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonWedgeOrHash->setChecked(true); 
    uncheckRingButtons();
  }
  
  void DrawWidget::cisOrTransBondClicked()
  {
    // also sets order
    m_scene->setBondType(Bond::CisOrTrans); 

    if (!checkedAtomButton())
      ui.buttonC->setChecked(true);
    uncheckBondButtons();
    ui.buttonCisOrTrans->setChecked(true);
    uncheckRingButtons();
  }
 
  //
  // Rings
  //

  void DrawWidget::ring3Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing3->setChecked(true);
    m_scene->setHintRing(3);
  }

  void DrawWidget::ring4Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing4->setChecked(true);
    m_scene->setHintRing(4);
  }
  
  void DrawWidget::ring5Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing5->setChecked(true);
    m_scene->setHintRing(5);
  }
  
  void DrawWidget::ring6Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing6->setChecked(true);
    m_scene->setHintRing(6);
  }
  
  void DrawWidget::ring7Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing7->setChecked(true);
    m_scene->setHintRing(7);
  }
  
  void DrawWidget::ring8Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing8->setChecked(true);
    m_scene->setHintRing(8);
  }
  
  void DrawWidget::aromaticRing5Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing5Arom->setChecked(true);
    m_scene->setHintRing(5);
  }

  void DrawWidget::aromaticRing6Clicked()
  {
    uncheckAllButtons();
    ui.buttonRing6Arom->setChecked(true);
    m_scene->setHintRing(6);
  }


} // end namespace Molsketch

//#include "drawwidget.moc"
