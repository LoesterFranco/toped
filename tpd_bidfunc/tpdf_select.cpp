//===========================================================================
//                                                                          =
//   This program is free software; you can redistribute it and/or modify   =
//   it under the terms of the GNU General Public License as published by   =
//   the Free Software Foundation; either version 2 of the License, or      =
//   (at your option) any later version.                                    =
// ------------------------------------------------------------------------ =
//                  TTTTT    OOO    PPPP    EEEE    DDDD                    =
//                  T T T   O   O   P   P   E       D   D                   =
//                    T    O     O  PPPP    EEE     D    D                  =
//                    T     O   O   P       E       D   D                   =
//                    T      OOO    P       EEEEE   DDDD                    =
//                                                                          =
//   This file is a part of Toped project (C) 2001-2007 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Thu Apr 19 BST 2007 (from tellibin.h Fri Jan 24 2003)
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Definition of all TOPED select/unselect functions
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include "tpdf_select.h"
#include "datacenter.h"

extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern console::toped_logfile    LogFile;

//=============================================================================
tellstdfunc::stdSELECT::stdSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdSELECT::undo() {
   TEUNDO_DEBUG("select(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectInBox(p1DB, p2DB, unselable, false);
   DATC->unlockDB();
   delete w;delete p1DB; delete p2DB;
   UpdateLV();
}

int tellstdfunc::stdSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectInBox(p1DB, p2DB, unselable, PROPC->layselmask(), false);
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSELECT_I::stdSELECT_I(telldata::typeID retype, bool eor) :
      stdSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECT_TL::stdSELECT_TL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_layout)));
}

void tellstdfunc::stdSELECT_TL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   delete pl;
}

void tellstdfunc::stdSELECT_TL::undo() {
}

int tellstdfunc::stdSELECT_TL::execute() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectFromList(get_ttlaylist(pl), unselable);
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdSELECTIN::stdSELECTIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdSELECTIN::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected;
}

void tellstdfunc::stdSELECTIN::undo() {
   TEUNDO_DEBUG("select(point) UNDO");
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectFromList(get_ttlaylist(selected), unselable);
   DATC->unlockDB();
   delete selected;
   UpdateLV();
}

int tellstdfunc::stdSELECTIN::execute()
{
   // get the data from the stack
   assert(telldata::tn_pnt == OPstack.top()->get_type());
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   DWordSet unselectable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      laydata::AtticList* selectedl = ATDB->changeSelect(p1DB, unselectable, true);
   DATC->unlockDB();
   if (NULL != selectedl) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(selectedl));
      OPstack.push(make_ttlaylist(selectedl));
      LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
      for(laydata::AtticList::iterator CI = selectedl->begin();CI != selectedl->end(); CI++)
         delete CI->second;
      delete selectedl;
   }
   delete p1; delete p1DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTSELECT::stdPNTSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdPNTSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTSELECT::undo() {
   TEUNDO_DEBUG("pselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectInBox(p1DB, p2DB, unselable, true);
   DATC->unlockDB();
   delete w; delete p1DB; delete p2DB;
   UpdateLV();
}

int tellstdfunc::stdPNTSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectInBox(p1DB,  p2DB, unselable, PROPC->layselmask(), true);
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTSELECT_I::stdPNTSELECT_I(telldata::typeID retype, bool eor) :
      stdPNTSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdPNTSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   return stdPNTSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT::stdUNSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdUNSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdUNSELECT::undo() {
   TEUNDO_DEBUG("unselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectInBox(p1DB, p2DB, unselable, PROPC->layselmask(), false);
   DATC->unlockDB();
   delete w; delete p1DB; delete p2DB;
   UpdateLV();
}

int tellstdfunc::stdUNSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectInBox(p1DB, p2DB, unselable, false);
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECT_I::stdUNSELECT_I(telldata::typeID retype, bool eor) :
      stdUNSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdUNSELECT_TL::stdUNSELECT_TL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttlist(telldata::tn_layout)));
}

void tellstdfunc::stdUNSELECT_TL::undo() {
}

void tellstdfunc::stdUNSELECT_TL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   delete pl;
}

int tellstdfunc::stdUNSELECT_TL::execute() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(OPstack.top());OPstack.pop();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectFromList(get_ttlaylist(pl), unselable);
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECTIN::stdUNSELECTIN(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttpnt()));
}

void tellstdfunc::stdUNSELECTIN::undo_cleanup() {
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete selected;
}

void tellstdfunc::stdUNSELECTIN::undo() {
   TEUNDO_DEBUG("unselect(point) UNDO");
   telldata::ttlist* selected = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectFromList(get_ttlaylist(selected), unselable);
   DATC->unlockDB();
   delete selected;
   UpdateLV();
}

int tellstdfunc::stdUNSELECTIN::execute() {
   // get the data from the stack
   assert(telldata::tn_pnt == OPstack.top()->get_type());
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(p1->x(), p1->y(), DBscale);
   DWordSet unselectable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      laydata::AtticList* selectedl = ATDB->changeSelect(p1DB,unselectable,false);
   DATC->unlockDB();
   if (NULL != selectedl) {
      UNDOcmdQ.push_front(this);
      UNDOPstack.push_front(make_ttlaylist(selectedl));
      OPstack.push(make_ttlaylist(selectedl));
      LogFile << LogFile.getFN() << "("<< *p1 << ");"; LogFile.flush();
      delete selectedl;
   }
   delete p1; delete p1DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTUNSELECT::stdPNTUNSELECT(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttwnd()));
}

void tellstdfunc::stdPNTUNSELECT::undo_cleanup() {
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete w;
}

void tellstdfunc::stdPNTUNSELECT::undo() {
   TEUNDO_DEBUG("punselect(box) UNDO");
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(UNDOPstack.front());UNDOPstack.pop_front();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectInBox(p1DB, p2DB, unselable, PROPC->layselmask(), true);
   DATC->unlockDB();
   delete w; delete p1DB; delete p2DB;
   UpdateLV();
}

int tellstdfunc::stdPNTUNSELECT::execute() {
   UNDOcmdQ.push_front(this);
   UNDOPstack.push_front(OPstack.top());
   // get the data from the stack
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   real DBscale = PROPC->DBscale();
   TP* p1DB = DEBUG_NEW TP(w->p1().x(), w->p1().y(), DBscale);
   TP* p2DB = DEBUG_NEW TP(w->p2().x(), w->p2().y(), DBscale);
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectInBox(p1DB, p2DB, unselable, true);
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "("<< *w << ");"; LogFile.flush();
   //DONT delete w; - undo will delete it
   delete p1DB; delete p2DB;
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdPNTUNSELECT_I::stdPNTUNSELECT_I(telldata::typeID retype, bool eor) :
      stdPNTUNSELECT(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdPNTUNSELECT_I::execute() {
   // stop the thread and wait for input from the GUI
   if (!tellstdfunc::waitGUInput(console::op_dbox, &OPstack)) return EXEC_ABORT;
   else return stdPNTUNSELECT::execute();
}

//=============================================================================
tellstdfunc::stdSELECTALL::stdSELECTALL(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdSELECTALL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdSELECTALL::undo() {
   TEUNDO_DEBUG("select_all() UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->unselectAll();
      ATDB->selectFromList(get_ttlaylist(pl), unselable);
   DATC->unlockDB();
   delete pl;
   UpdateLV();
}

int tellstdfunc::stdSELECTALL::execute() {
   UNDOcmdQ.push_front(this);
   DWordSet unselectable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapeSel()));
      ATDB->selectAll(unselectable, PROPC->layselmask());
      OPstack.push(make_ttlaylist(ATDB->shapeSel()));
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdUNSELECTALL::stdUNSELECTALL(telldata::typeID retype, bool eor) :
                               cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

void tellstdfunc::stdUNSELECTALL::undo_cleanup() {
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.back());UNDOPstack.pop_back();
   delete pl;
}

void tellstdfunc::stdUNSELECTALL::undo() {
   TEUNDO_DEBUG("unselect_all() UNDO");
   telldata::ttlist* pl = static_cast<telldata::ttlist*>(UNDOPstack.front());UNDOPstack.pop_front();
   DWordSet unselable = PROPC->allUnselectable();
   laydata::TdtDesign* ATDB = DATC->lockDB();
      ATDB->selectFromList(get_ttlaylist(pl), unselable);
   DATC->unlockDB();
   delete (pl);
   UpdateLV();
}

int tellstdfunc::stdUNSELECTALL::execute() {
   UNDOcmdQ.push_front(this);
   laydata::TdtDesign* ATDB = DATC->lockDB();
      UNDOPstack.push_front(make_ttlaylist(ATDB->shapeSel()));
      ATDB->unselectAll();
   DATC->unlockDB();
   LogFile << LogFile.getFN() << "();"; LogFile.flush();
   UpdateLV();
   return EXEC_NEXT;
}

//=============================================================================
tellstdfunc::stdREPORTSLCTD::stdREPORTSLCTD(telldata::typeID retype, bool eor) :
      cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{}

int tellstdfunc::stdREPORTSLCTD::execute() {
   if (DATC->numselected() == 0)
   {
      tell_log(console::MT_ERROR,"No objects selected.");
   }
   else
   {
      real DBscale = PROPC->DBscale();
      laydata::TdtDesign* ATDB = DATC->lockDB();
         ATDB->reportSelected(DBscale);
      DATC->unlockDB();
   }
   return EXEC_NEXT;
}


//=============================================================================
tellstdfunc::stdSETSELECTMASK::stdSETSELECTMASK(telldata::typeID retype, bool eor) :
                               cmdSTDFUNC(DEBUG_NEW parsercmd::argumentLIST,retype,eor)
{
   arguments->push_back(DEBUG_NEW argumentTYPE("", DEBUG_NEW telldata::ttint()));
}

void tellstdfunc::stdSETSELECTMASK::undo_cleanup()
{
   getWordValue(UNDOPstack,false);
}

void tellstdfunc::stdSETSELECTMASK::undo()
{
   TEUNDO_DEBUG("setselectmask() UNDO");
   word mask = getWordValue(UNDOPstack,true);
   PROPC->setLaySelMask(mask);
//   UpdateLV();
}

int tellstdfunc::stdSETSELECTMASK::execute()
{
   UNDOcmdQ.push_front(this);
   word mask = getWordValue();
   word oldmask = PROPC->layselmask();
   UNDOPstack.push_front(DEBUG_NEW telldata::ttint(oldmask));
   PROPC->setLaySelMask(mask);
   OPstack.push(DEBUG_NEW telldata::ttint(oldmask));
   LogFile << LogFile.getFN() << "("<< mask <<");"; LogFile.flush();
   return EXEC_NEXT;
}

