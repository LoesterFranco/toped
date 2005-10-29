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
// ------------------------------------------------------------------------ =
//           $URL$
//  Creation date: Fri Nov 08 2002
//     Created by: Svilen Krustev - s_krustev@yahoo.com
//      Copyright: (C) 2001-2004 by Svilen Krustev
//    Description: wxWidget version
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------                
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include <sstream>
#include <algorithm>
#include "tellyzer.h"
#include "../tpd_common/outbox.h"

//-----------------------------------------------------------------------------
// Definition of tell debug macros
//-----------------------------------------------------------------------------
//#define TELL_DEBUG_ON
#ifdef TELL_DEBUG_ON
#define TELL_DEBUG(a)  printf("%s \n", #a);
#else
#define TELL_DEBUG(a) 
#endif

extern void tellerror(std::string s, parsercmd::yyltype loc);
extern void tellerror(std::string s);

extern parsercmd::cmdBLOCK*       CMDBlock;
extern console::toped_logfile     LogFile;

//-----------------------------------------------------------------------------
// Initialize some static members
//-----------------------------------------------------------------------------
// Table of defined functions
parsercmd::functionMAP        parsercmd::cmdBLOCK::_funcMAP;    
// Table of current nested blocks
parsercmd::blockSTACK         parsercmd::cmdBLOCK::_blocks;
// Operand stack
telldata::operandSTACK       parsercmd::cmdVIRTUAL::OPstack;
// UNDO Operand stack
telldata::UNDOPerandQUEUE     parsercmd::cmdSTDFUNC::UNDOPstack;
// UNDO command queue
parsercmd::undoQUEUE          parsercmd::cmdSTDFUNC::UNDOcmdQ;


real parsercmd::cmdVIRTUAL::getOpValue(telldata::operandSTACK& OPs) {
   real value = 0;
   telldata::tell_var *op = OPs.top();OPs.pop();
   if (op->get_type() == telldata::tn_real) 
      value = static_cast<telldata::ttreal*>(op)->value();
   else if (op->get_type() == telldata::tn_int) 
      value = static_cast<telldata::ttint*>(op)->value();
   delete op;
   return value;
}

real parsercmd::cmdVIRTUAL::getOpValue(telldata::UNDOPerandQUEUE& OPs, bool front) {
   real value = 0;
   telldata::tell_var *op;
   if (front) {op = OPs.front();OPs.pop_front();}
   else       {op = OPs.back();OPs.pop_back();}
   if (op->get_type() == telldata::tn_real) 
      value = static_cast<telldata::ttreal*>(op)->value();
   else if (op->get_type() == telldata::tn_int) 
      value = static_cast<telldata::ttint*>(op)->value();
   delete op;
   return value;
}

word parsercmd::cmdVIRTUAL::getWordValue(telldata::operandSTACK& OPs) {
   telldata::ttint  *op = static_cast<telldata::ttint*>(OPs.top());OPs.pop();
   word value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_WORD_VALUE))
      {/*Error -> value out of expected range*/}
   else value = word(op->value());
   delete op;
   return value;
}

word parsercmd::cmdVIRTUAL::getWordValue(telldata::UNDOPerandQUEUE& OPs, bool front) {
   telldata::ttint  *op;
   if (front) {op = static_cast<telldata::ttint*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttint*>(OPs.back());OPs.pop_back();} 
   word value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_WORD_VALUE))
      {/*Error -> value out of expected range*/}
   else value = word(op->value());
   delete op;
   return value;
}

byte parsercmd::cmdVIRTUAL::getByteValue(telldata::operandSTACK& OPs) {
   telldata::ttint  *op = static_cast<telldata::ttint*>(OPs.top());OPs.pop();
   byte value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_BYTE_VALUE))
      {/*Error -> value out of expected range*/}
   else value = byte(op->value());
   delete op;
   return value;
}

byte parsercmd::cmdVIRTUAL::getByteValue(telldata::UNDOPerandQUEUE& OPs, bool front) {
   telldata::ttint  *op;
   if (front) {op = static_cast<telldata::ttint*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttint*>(OPs.back());OPs.pop_back();}
   byte value = 0;
   if ((op->value() < 0 ) || (op->value() > MAX_BYTE_VALUE))
      {/*Error -> value out of expected range*/}
   else value = byte(op->value());
   delete op;
   return value;
}

std::string parsercmd::cmdVIRTUAL::getStringValue(telldata::operandSTACK& OPs) {
   telldata::ttstring  *op = static_cast<telldata::ttstring*>(OPs.top());OPs.pop();
   std::string value = op->value();
   delete op;
   return value;
}

std::string parsercmd::cmdVIRTUAL::getStringValue(telldata::UNDOPerandQUEUE& OPs, bool front) {
   telldata::ttstring  *op;
   if (front) {op = static_cast<telldata::ttstring*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttstring*>(OPs.back());OPs.pop_back();}
   std::string value = op->value();
   delete op;
   return value;
}

bool parsercmd::cmdVIRTUAL::getBoolValue(telldata::operandSTACK& OPs) {
   telldata::ttbool  *op = static_cast<telldata::ttbool*>(OPs.top());OPs.pop();
   bool value = op->value();
   delete op;
   return value;
}

bool parsercmd::cmdVIRTUAL::getBoolValue(telldata::UNDOPerandQUEUE& OPs, bool front) {
   telldata::ttbool  *op;
   if (front) {op = static_cast<telldata::ttbool*>(OPs.front());OPs.pop_front();}
   else       {op = static_cast<telldata::ttbool*>(OPs.back());OPs.pop_back();}
   bool value = op->value();
   delete op;
   return value;
}
//=============================================================================
int parsercmd::cmdPLUS::execute() {
   TELL_DEBUG(cmdPLUS);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttreal(value1 + value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdMINUS::execute() {
   TELL_DEBUG(cmdMINUS);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttreal(value1 - value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT::execute() {
   TELL_DEBUG(cmdSHIFTPNT);
   real shift = getOpValue();
   telldata::ttpnt  *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt* r = new telldata::ttpnt(p->x()+_sign*shift,p->y()+_sign*shift);
   delete p; 
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTPNT2::execute() {
   TELL_DEBUG(cmdSHIFTPNT2);
   telldata::ttpnt  *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttpnt* r = new telldata::ttpnt(p->x()+_sign*p1->x(),p->y()+_sign*p1->y());
   delete p; delete p1;
   OPstack.push(r);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSHIFTBOX::execute() {
   TELL_DEBUG(cmdSHIFTBOX);
   telldata::ttpnt *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
   telldata::ttwnd* r = new telldata::ttwnd(w->p1().x() + _sign*p->x(),w->p1().y() + _sign*p->y(),
                        w->p2().x() + _sign*p->x(),w->p2().y() + _sign*p->y());
   OPstack.push(r);
   delete p; delete w;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdMULTIPLY::execute() {
   TELL_DEBUG(cmdMULTIPLY);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttreal(value1 * value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdDIVISION::execute() {
   TELL_DEBUG(cmdDIVISION);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttreal(value1 / value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLT::execute() {
   TELL_DEBUG(cmdLT);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttbool(value1 < value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdLET::execute() {
   TELL_DEBUG(cmdLET);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttbool(value1 <= value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdGT::execute() {
   TELL_DEBUG(cmdGT);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttbool(value1 > value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdGET::execute() {
   TELL_DEBUG(cmdGET);
   real value2 = getOpValue();
   real value1 = getOpValue();
   OPstack.push(new telldata::ttbool(value1 >= value2));
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdEQ::execute() {
   TELL_DEBUG(cmdEQ);
   if (NUMBER_TYPE(OPstack.top()->get_type()))
      OPstack.push(new telldata::ttbool(getOpValue() == getOpValue()));
//   else if (tn_
// box & poly equal
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdNE::execute() {
   TELL_DEBUG(cmdNE);
   if (NUMBER_TYPE(OPstack.top()->get_type()))
      OPstack.push(new telldata::ttbool(getOpValue() != getOpValue()));
//   else if (tn_
// box & poly equal
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdAND::execute() {
   TELL_DEBUG(cmdAND);
   telldata::ttbool *op = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
   static_cast<telldata::ttbool*>(OPstack.top())->AND(op->value());
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdOR::execute() {
   TELL_DEBUG(cmdOR);
   telldata::ttbool *op = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
   static_cast<telldata::ttbool*>(OPstack.top())->OR(op->value());
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdUMINUS::execute() {
   TELL_DEBUG(cmdUMINUS);
   if      (_type == telldata::tn_real) static_cast<telldata::ttreal*>(OPstack.top())->uminus();
   else if (_type == telldata::tn_int ) static_cast<telldata::ttint*>(OPstack.top())->uminus();
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdSTACKRST::execute() {
   TELL_DEBUG(cmdSTACKRST);
   while (!OPstack.empty()) {
      delete OPstack.top(); OPstack.pop();
   }   
   return EXEC_NEXT;
}

//=============================================================================
// I don't like this rubbish, but it works.
// Virtual operator can't be defined (we need '=' here), but
// we still can think about virtual function -> assign for example
int parsercmd::cmdASSIGN::execute() {
   TELL_DEBUG(cmdREALASSIGN);
   telldata::tell_var *op = OPstack.top();OPstack.pop();
   telldata::typeID typeis = _var->get_type();
   if (typeis & telldata::tn_listmask) {
      *(static_cast<telldata::ttlist*>(_var)) = *(static_cast<telldata::ttlist*>(op));
      OPstack.push(new telldata::ttlist(*static_cast<telldata::ttlist*>(_var)));
//      OPstack.push(static_cast<telldata::ttlist*>(_var)->selfcopy());
   }
   else
   switch (typeis) {
      case telldata::tn_real:
         if (op->get_type() == telldata::tn_real) 
            *(static_cast<telldata::ttreal*>(_var)) = *(static_cast<telldata::ttreal*>(op));
         else if (op->get_type() == telldata::tn_int)
            *(static_cast<telldata::ttreal*>(_var)) = *(static_cast<telldata::ttint*>(op));
         OPstack.push(new telldata::ttreal(*static_cast<telldata::ttreal*>(_var)));
         break;
      case telldata::tn_int:
         if (op->get_type() == telldata::tn_real) 
            *(static_cast<telldata::ttint*>(_var)) = *(static_cast<telldata::ttreal*>(op));
         else if (op->get_type() == telldata::tn_int)
            *(static_cast<telldata::ttint*>(_var)) = *(static_cast<telldata::ttint*>(op));
         OPstack.push(new telldata::ttint(*static_cast<telldata::ttint*>(_var)));
         break;
      case   telldata::tn_bool: 
         *(static_cast<telldata::ttbool*>(_var)) = *(static_cast<telldata::ttbool*>(op));
         OPstack.push(new telldata::ttbool(*static_cast<telldata::ttbool*>(_var)));
         break;
      case   telldata::tn_pnt: 
         *(static_cast<telldata::ttpnt*>(_var)) = *(static_cast<telldata::ttpnt*>(op));
         OPstack.push(new telldata::ttpnt(*static_cast<telldata::ttpnt*>(_var)));
         break;
      case   telldata::tn_box: 
         *(static_cast<telldata::ttwnd*>(_var)) = *(static_cast<telldata::ttwnd*>(op));
         OPstack.push(new telldata::ttwnd(*static_cast<telldata::ttwnd*>(_var)));
         break;
      case telldata::tn_string:
         *(static_cast<telldata::ttstring*>(_var)) = *(static_cast<telldata::ttstring*>(op));
         OPstack.push(new telldata::ttstring(*static_cast<telldata::ttstring*>(_var)));
         break;
      case telldata::tn_layout:
         *(static_cast<telldata::ttlayout*>(_var)) = *(static_cast<telldata::ttlayout*>(op));
         OPstack.push(new telldata::ttlayout(*static_cast<telldata::ttlayout*>(_var)));
         break;
      default:
         if (NULL != CMDBlock->getTypeByID(typeis)) {
            static_cast<telldata::user_struct*>(_var)->assign(op);
            OPstack.push(_var->selfcopy());
         }
         else 
            tellerror("Bad or unsupported type in assign statement");
   }
   delete op;
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdPUSH::execute() {
   // The temptation here is to put the constants in the operand stack directly,
   // i.e. without self-copy. It is wrong though - for many cases - for example
   // for conditional block "while (count > 0)". It should be executed many 
   // times but the variable will exists only the first time, because it will 
   // be cleaned-up from the operand stack after the first execution
   TELL_DEBUG(cmdPUSH);
   OPstack.push(_var->selfcopy());
   return EXEC_NEXT;
}

//=============================================================================
//int parsercmd::cmdPOINT::execute() {
//   TELL_DEBUG(cmdPOINT);
//   real y= getOpValue();
//   real x= getOpValue();
//   OPstack.push(new telldata::ttpnt(x, y));
//   return EXEC_NEXT;
//}

//=============================================================================
//parsercmd::cmdPOINTFIELD::cmdPOINTFIELD(char* f, yyltype loc) {
//   if (('x' != *f) && ('y' != *f)) 
//      tellerror("bad field name. 'x' or 'y' expected", loc);
//   else _field = *f;
////   delete f;
//}

//int parsercmd::cmdPOINTFIELD::execute() {
//   TELL_DEBUG(cmdPOINTFIELD);
//   telldata::ttpnt *p = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
//   telldata::ttreal *e;
//   if ('x' == _field) e = new telldata::ttreal(p->x());
//   else               e = new telldata::ttreal(p->y());
//   OPstack.push(e);
//   delete p;
//   return EXEC_NEXT;
//}

//=============================================================================
//parsercmd::cmdWINDOWFIELD::cmdWINDOWFIELD(char* f, yyltype loc) {
//   if (('1' != *f) && ('2' != *f)) 
//      tellerror("bad field name. 'p1' or 'p2' expected", loc);
//   else _field = *f;
////   delete f;
//}

//int parsercmd::cmdWINDOWFIELD::execute() {
//   TELL_DEBUG(cmdWINDOWFIELD);
//   telldata::ttwnd *w = static_cast<telldata::ttwnd*>(OPstack.top());OPstack.pop();
//   telldata::ttpnt *p;
//   if ('1' == _field) p = new telldata::ttpnt(w->p1().x(),w->p1().y() );
//   else               p = new telldata::ttpnt(w->p2().x(),w->p2().y() );
//   OPstack.push(p);
//   delete w;
//   return EXEC_NEXT;
//}

//=============================================================================
//int parsercmd::cmdWINDOW::execute() {
//   TELL_DEBUG(cmdWINDOW);
//   telldata::ttwnd *w;
//      telldata::ttpnt *p2 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
//      telldata::ttpnt *p1 = static_cast<telldata::ttpnt*>(OPstack.top());OPstack.pop();
//      w = new telldata::ttwnd(p1->x(),p1->y(),p2->x(),p2->y());
//      delete p1; delete p2;
//   OPstack.push(w);
//   return EXEC_NEXT;
//}

int parsercmd::cmdSTRUCT::execute() {
   const telldata::tell_type *stype = CMDBlock->getTypeByID( (*_arg)() );
   assert(NULL != stype);

   telldata::tell_var *ustrct;
   switch( (*_arg)() ) {
      case telldata::tn_pnt: {
         telldata::ttint* y = static_cast<telldata::ttint*>(OPstack.top());
         OPstack.pop();
         telldata::ttint* x = static_cast<telldata::ttint*>(OPstack.top());
         OPstack.pop();
         ustrct = new telldata::ttpnt(x->value(), y->value());
         delete x; delete y;
         break;
      }
      case telldata::tn_box: {
         telldata::ttpnt* p2 = static_cast<telldata::ttpnt*>(OPstack.top());
         OPstack.pop();
         telldata::ttpnt* p1 = static_cast<telldata::ttpnt*>(OPstack.top());
         OPstack.pop();
         ustrct = new telldata::ttwnd(*p1, *p2);
         delete p1; delete p2;
         break;
      }
      default:ustrct = new telldata::user_struct(stype, OPstack);
   }
//   telldata::user_struct *ustrct = new telldata::user_struct(stype, OPstack);
   OPstack.push(ustrct);
   return EXEC_NEXT;
}


//=============================================================================
parsercmd::cmdLIST::cmdLIST(telldata::argumentID* ttn) {
  _ttype = (*ttn)() & ~telldata::tn_listmask ;
  _length = ttn->child()->size();
}

int parsercmd::cmdLIST::execute() {
   TELL_DEBUG(cmdLIST);
   telldata::ttlist *pl = new telldata::ttlist(_ttype);
   pl->reserve(_length);
   telldata::tell_var  *p;
   for (unsigned i = 0; i < _length; i++) {
      p = OPstack.top();OPstack.pop();
      pl->add(p); //Dont delete p; here! And don't get confused!
   }
   pl->reverse(); 
   OPstack.push(pl);
   return EXEC_NEXT;
}

//=============================================================================
int parsercmd::cmdFUNCCALL::execute() {
   TELL_DEBUG(cmdFUNC);
   LogFile.setFN(funcname);
   int fresult;
   try {fresult = funcbody->execute();}
   catch (EXPTN) {return EXEC_RETURN;}
   funcbody->cmdSTDFUNC::undo_cleanup();
   return fresult;
}

//=============================================================================
parsercmd::cmdBLOCK::cmdBLOCK() {
   assert(!_blocks.empty());
   _next_lcl_typeID = _blocks.front()->_next_lcl_typeID;
}

telldata::tell_var* parsercmd::cmdBLOCK::getID(char*& name, bool local){
   TELL_DEBUG(***getID***);
   // Roll back the blockSTACK until name is found. return NULL otherwise
   typedef blockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = local ? _blocks.begin() : _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++) {
        if ((*cmd)->VARlocal.find(name) != (*cmd)->VARlocal.end())
            return (*cmd)->VARlocal[name];
   }
   return NULL;
}

void parsercmd::cmdBLOCK::addID(char*& name, telldata::tell_var* var) {
   TELL_DEBUG(addID);
   VARlocal[name] = var;
}

void parsercmd::cmdBLOCK::addlocaltype(char*& ttypename, telldata::tell_type* ntype) {
   assert(TYPElocal.end() == TYPElocal.find(ttypename));
   _next_lcl_typeID = ntype->ID() + 1;
   TYPElocal[ttypename] = ntype;
}

telldata::tell_type* parsercmd::cmdBLOCK::requesttypeID(char*& ttypename) {
   if (TYPElocal.end() == TYPElocal.find(ttypename)) {
      telldata::tell_type* ntype = new telldata::tell_type(_next_lcl_typeID);
      return ntype;
   }
   else return NULL;
}

const telldata::tell_type* parsercmd::cmdBLOCK::getTypeByName(char*& ttypename) const {
   TELL_DEBUG(***gettypeID***);
   // Roll back the blockSTACK until name is found. return NULL otherwise
   typedef blockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   for (BS cmd = blkstart; cmd != blkend; cmd++) {
        if ((*cmd)->TYPElocal.end() != TYPElocal.find(ttypename))
            return (*cmd)->TYPElocal[ttypename];
   }
   return NULL;
}

const telldata::tell_type* parsercmd::cmdBLOCK::getTypeByID(const telldata::typeID ID) const {
   TELL_DEBUG(***getTypeByID***);
   // Roll back the blockSTACK until name is found. return NULL otherwise
   typedef blockSTACK::const_iterator BS;
   BS blkstart = _blocks.begin();
   BS blkend   = _blocks.end();
   typedef telldata::typeMAP::const_iterator CT;
   for (BS cmd = blkstart; cmd != blkend; cmd++) 
      for (CT ctp = (*cmd)->TYPElocal.begin(); ctp != (*cmd)->TYPElocal.end(); ctp++)
         if (ID == ctp->second->ID()) return ctp->second;
   return NULL;
}
/*
telldata::typeID* parsercmd::cmdBLOCK::checkfield(telldata::typeID ID, char*& fname, yyltype loc) const {
   telldata::tell_type* stype = getTypeByID(ID);
   if (NULL != stype) {
      telldata::tell_type* ftype = stype.getfieldtype(fname);
      if (NULL != ftype)
         return ftype->ID();
      else 
         tellerror("Bad field name", loc);
   }
   else tellerror("Variable has no fields", loc);
   return telldata::tn_void;
}
*/
telldata::tell_var* parsercmd::cmdBLOCK::newTellvar(telldata::typeID ID, yyltype loc) {
   if (ID & telldata::tn_listmask) return(new telldata::ttlist(ID));
   else
   switch (ID) {
      case   telldata::tn_real: return(new telldata::ttreal());
      case    telldata::tn_int: return(new telldata::ttint());
      case   telldata::tn_bool: return(new telldata::ttbool());
      case    telldata::tn_pnt: return(new telldata::ttpnt());
      case    telldata::tn_box: return(new telldata::ttwnd());
      case telldata::tn_string: return(new telldata::ttstring());
      case telldata::tn_layout: return(new telldata::ttlayout());
      default: {
         const telldata::tell_type* utype = getTypeByID(ID);
         if (NULL == utype) tellerror("Bad type specifier", loc);
         else return (new telldata::user_struct(utype));
      }
   }
   return NULL;
}

parsercmd::cmdBLOCK* parsercmd::cmdBLOCK::popblk() {
   TELL_DEBUG(cmdBLOCK_popblk);
   _blocks.pop_front();
   return _blocks.front();;
}

void parsercmd::cmdBLOCK::addFUNC(std::string, cmdSTDFUNC* cQ) {
   TELL_DEBUG(addFUNC);
   tellerror("Nested function definitions are not allowed");
   if (cQ)    delete cQ;
}

int parsercmd::cmdBLOCK::execute() {
   TELL_DEBUG(cmdBLOCK_execute);
   int retexec = EXEC_NEXT; // to secure an empty block
   for (cmdQUEUE::const_iterator cmd = cmdQ.begin(); cmd != cmdQ.end(); cmd++) {
      if ((retexec = (*cmd)->execute())) break;
   }
   return retexec;
}

void parsercmd::cmdBLOCK::cleaner() {
   TELL_DEBUG(cmdBLOCK_cleaner);
   while (!cmdQ.empty()) {
      cmdVIRTUAL *a = cmdQ.front();cmdQ.pop_front();
      delete a;
   }
}

//void parsercmd::cmdBLOCK::pushCompositeCmd(telldata::typeID ID) {
//   telldata::tell_type* comptype = getTypeByID(ID);
//   
//}

parsercmd::cmdBLOCK::~cmdBLOCK() {
   for (cmdQUEUE::iterator CMDI = cmdQ.begin(); CMDI != cmdQ.end(); CMDI++)
      delete *CMDI;
   cmdQ.clear();
   for (telldata::variableMAP::iterator VMI = VARlocal.begin(); VMI != VARlocal.end(); VMI++)
      delete VMI->second;
   VARlocal.clear();
}

//=============================================================================
parsercmd::cmdSTDFUNC* const parsercmd::cmdBLOCK::getFuncBody
                                        (char*& fn, telldata::argumentQ* amap) const {
   cmdSTDFUNC *fbody = NULL;
   typedef functionMAP::iterator MM;
   std::pair<MM,MM> range = _funcMAP.equal_range(fn);
   telldata::argumentQ* arguMap = (NULL == amap) ? new telldata::argumentQ : amap;
   for (MM fb = range.first; fb != range.second; fb++) {
      fbody = fb->second;
      if (0 == fbody->argsOK(arguMap)) break;
   }
   if (NULL == amap) delete arguMap;
   return fbody;
}

parsercmd::cmdSTDFUNC* const parsercmd::cmdBLOCK::funcDefined
                                        (char*& fn, argumentLIST* alst) const {
   // convert argumentLIST to argumentMAP
   telldata::argumentQ amap;
   typedef argumentLIST::const_iterator AT;
   for (AT arg = alst->begin(); arg != alst->end(); arg++) {
      amap.push_back(new telldata::argumentID((*arg)->second->get_type())); }
   // call
   return getFuncBody(fn,&amap);
}

//=============================================================================
int parsercmd::cmdSTDFUNC::argsOK(telldata::argumentQ* amap) {
// This is supposed to be called only by user defined functions.
// Standard functions will overwrite this function although they
// also can use it
   int i = amap->size();
   if (i != arguments->size()) return -1;
   while (i-- > 0) {
      if (!NUMBER_TYPE( (*(*amap)[i])() )) {
         // for non-number types there is no internal conversion,
         // so check strictly the type
         if ((*(*amap)[i])() != (*arguments)[i]->second->get_type()) break;
      }
      else // for number types - allow compatablity
         if (!NUMBER_TYPE((*arguments)[i]->second->get_type())) break;
         else if ((*(*amap)[i])() > (*arguments)[i]->second->get_type()) break;
   }
   return (i+1);
}

void parsercmd::cmdSTDFUNC::undo_cleanup() {
   if (UNDOcmdQ.size() > 20) {
      UNDOcmdQ.back()->undo_cleanup(); UNDOcmdQ.pop_back();
   }
}   

parsercmd::cmdSTDFUNC::~cmdSTDFUNC() {
   if (NULL == arguments) return;
   for (argumentLIST::iterator ALI = arguments->begin(); ALI != arguments->end(); ALI++) {
      delete (*ALI)->second;
      delete (*ALI);
   }   
   arguments->clear();   
   delete arguments;
}   

//=============================================================================
parsercmd::cmdFUNC::cmdFUNC(argumentLIST* vm, telldata::typeID tt):
                                                cmdSTDFUNC(vm,tt), cmdBLOCK() {
   // copy the arguments in the structure of the local variables
   typedef argumentLIST::const_iterator AT;
   for (AT arg = arguments->begin(); arg != arguments->end(); arg++) {
//      VARlocal.insert(**arg);
      VARlocal[(*arg)->first] = (*arg)->second->selfcopy();
   }
}

int parsercmd::cmdFUNC::execute() {
   // get the arguments from the operands stack and replace the values
   // of the function arguments
   int i = arguments->size();
   while (i-- > 0) {
      //get the argument name
      std::string   argname = (*arguments)[i]->first;
      // get the tell variable (by name)
      telldata::tell_var* argvar = VARlocal[argname];
      // get a value from the operand stack
      telldata::tell_var* argval = OPstack.top();
      // replace the value of the local variable with the argument value
      argvar->set_value(argval);
      OPstack.pop();
   }
   LogFile << "// Executing UDF " << LogFile.getFN();LogFile.flush();   
   cmdBLOCK::execute();
   return EXEC_NEXT;
}

std::string parsercmd::cmdFUNC::callingConv() {
   std::ostringstream ost;
   int argnum = arguments->size();
   ost << "( ";
   for (int i = 0; i != argnum; i++) {
      //get the argument name
      switch ((*arguments)[i]->second->get_type() & ~telldata::tn_listmask) {
        case telldata::tn_int    : ost << "int"        ; break;
        case telldata::tn_real   : ost << "real"       ; break;
        case telldata::tn_bool   : ost << "bool"       ; break;
        case telldata::tn_string : ost << "string"     ; break;
        case telldata::tn_pnt    : ost << "point"      ; break;
        case telldata::tn_box    : ost << "box"        ; break;
        default                  : ost << "?usertype?";
      };
      if ((*arguments)[i]->second->get_type() & telldata::tn_listmask) 
        ost << " list";
      if (i < argnum - 1) ost << " , ";
   }
   ost << " )";
   return ost.str();
}

//=============================================================================
int parsercmd::cmdIFELSE::execute() {
   TELL_DEBUG(cmdIFELSE);
   int retexec = EXEC_NEXT;
   telldata::ttbool *cond = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
   if (cond->value())   retexec =  trueblock->execute();
   else if (falseblock) retexec = falseblock->execute();
   return retexec;
}

//=============================================================================
int parsercmd::cmdWHILE::execute() {
   TELL_DEBUG(cmdWHILE);
   int retexec = EXEC_NEXT;
   telldata::ttbool *cond;
   bool    condvalue;
   while (true) {
      condblock->execute();
      cond = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
      condvalue = cond->value(); delete cond;
      if (condvalue)    retexec = body->execute();
      else              return retexec;
      if (EXEC_NEXT != retexec) return retexec;
   }
}

//=============================================================================
int parsercmd::cmdREPEAT::execute() {
   TELL_DEBUG(cmdREPEAT);
   int retexec;
   telldata::ttbool *cond;
   bool    condvalue;
   while (true) {
      retexec = body->execute();
      if (EXEC_NEXT != retexec) return retexec;
      condblock->execute();
      cond = static_cast<telldata::ttbool*>(OPstack.top());OPstack.pop();
      condvalue = cond->value(); delete cond;
      if (!condvalue)           return retexec;
   }
}

//=============================================================================
int parsercmd::cmdMAIN::execute() {
   TELL_DEBUG(cmdMAIN_execute);
   int retexec = EXEC_NEXT;
   while (!cmdQ.empty()) {
      cmdVIRTUAL *a = cmdQ.front();cmdQ.pop_front();
      if (EXEC_NEXT == retexec) retexec = a->execute();
      delete a;
   }
   return retexec;
}

void parsercmd::cmdMAIN::addFUNC(std::string fname , cmdSTDFUNC* cQ)  {
   _funcMAP.insert(std::make_pair(fname,cQ));
}

parsercmd::cmdMAIN::cmdMAIN():cmdBLOCK(telldata::tn_usertypes) {
   pushblk();
};

void  parsercmd::cmdMAIN::addGlobalType(char* ttypename, telldata::tell_type* ntype) {
   assert(TYPElocal.end() == TYPElocal.find(ttypename));
   TYPElocal[ttypename] = ntype;
}

parsercmd::cmdMAIN::~cmdMAIN(){
   for (functionMAP::iterator FMI = _funcMAP.begin(); FMI != _funcMAP.end(); FMI ++)
      delete FMI->second;
   _funcMAP.clear();
};


//=============================================================================
//telldata::typeID parsercmd::(telldata::typeID op1, telldata::typeID op2,
//                                                  yyltype loc1, yyltype loc2) {
//   yyltype loc;
//   if (NUMBER_TYPE(op1))
//      if (NUMBER_TYPE(op2)) {
//         CMDBlock->pushcmd(new parsercmd::cmdPOINT());
//         return telldata::tn_pnt; }
//      else loc = loc2;
//   else
//      if (telldata::tn_pnt == op1)
//         if (telldata::tn_pnt == op2) {
//            CMDBlock->pushcmd(new parsercmd::cmdWINDOW());
//            return telldata::tn_box; }
//         else loc = loc2;
//      else loc = loc1;
//   tellerror("unexepected operand type",loc);
//  return telldata::tn_void;
//}

//=============================================================================
telldata::typeID parsercmd::UMinus(telldata::typeID op1, yyltype loc1) {
   if (NUMBER_TYPE(op1)) {
      CMDBlock->pushcmd(new parsercmd::cmdUMINUS(op1));
      return op1;}
   else {
      tellerror("unexepected operand type",loc1);
      return telldata::tn_void;
   }
}

//=============================================================================
//     +/-    |real |point| box |
//------------+-----+-----+-----+
//   real     |  x  |  -  |  -  |
//   point    |shift|shift|  -  |
//   box      |blow |shift| or  |
//---------------------------------------
telldata::typeID parsercmd::Plus(telldata::typeID op1, telldata::typeID op2,
                                                  yyltype loc1, yyltype loc2) {
   switch (op1)   {
      case  telldata::tn_int: 
      case telldata::tn_real:
         switch(op2) {
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdPLUS());
                            return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT());
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT2());
                           return telldata::tn_pnt;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case telldata::tn_real: // inflate
            case telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTBOX());
                        return telldata::tn_box;
            case telldata::tn_box:    // or
//            case tn_poly: // or
                  default: tellerror("unexepected operand type",loc2); break;
         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::Minus(telldata::typeID op1, telldata::typeID op2,
                                                  yyltype loc1, yyltype loc2) {
   switch (op1)   {
       case telldata::tn_int:
      case telldata::tn_real:
         switch(op2) {
            case   telldata::tn_int:
            case  telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdMINUS());
                            return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_pnt:
         switch(op2) {
            case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT(-1));
                           return telldata::tn_pnt;
            case    telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT2(-1));
                           return telldata::tn_pnt;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      case telldata::tn_box:
         switch(op2) {
            case telldata::tn_real: // deflate
            case telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTBOX(-1));
                        return telldata::tn_box;
            case telldata::tn_box:    // or not ???
            //case tn_ttPoly:
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

//=============================================================================
//     *//    |real |point| box |
//------------+-----+-----+-----+
//   real     |  x  |  -  |  -  |
//   point    |scale| rota|  -  |
//   box      |scale| rota| and |
//-------------------------------
telldata::typeID parsercmd::Multiply(telldata::typeID op1, telldata::typeID op2,
                                                  yyltype loc1, yyltype loc2) {
   switch (op1)   {
       case telldata::tn_int:
      case telldata::tn_real:
         switch(op2) {
              case telldata::tn_int:
             case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdMULTIPLY());
                         return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
//      case telldata::tn_pnt:
//         switch(op2) {
//            case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT(-1));
//                           return telldata::tn_pnt;
//            case    telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT2(-1));
//                           return telldata::tn_pnt;
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
//      case telldata::tn_box:
//         switch(op2) {
//            case telldata::tn_real: // deflate
//            case telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTBOX(-1));
//                        return telldata::tn_box;
//            case telldata::tn_box:    // or not ???
//            //case tn_ttPoly:
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::Divide(telldata::typeID op1, telldata::typeID op2,
                                                  yyltype loc1, yyltype loc2) {
   switch (op1)   {
       case telldata::tn_int:
      case telldata::tn_real:
         switch(op2) {
            case  telldata::tn_int:
            case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdDIVISION());
                         return telldata::tn_real;
                  default: tellerror("unexepected operand type",loc2);break;
         };break;
//      case telldata::tn_pnt:
//         switch(op2) {
//            case telldata::tn_real:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT(-1));
//                           return telldata::tn_pnt;
//            case    telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTPNT2(-1));
//                           return telldata::tn_pnt;
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
//      case telldata::tn_box:
//         switch(op2) {
//            case telldata::tn_real: // deflate
//            case telldata::tn_pnt:CMDBlock->pushcmd(new parsercmd::cmdSHIFTBOX(-1));
//                        return telldata::tn_box;
//            case telldata::tn_box:    // or not ???
//            //case tn_ttPoly:
//                  default: tellerror("unexepected operand type",loc2);break;
//         };break;
      default: tellerror("unexepected operand type",loc1);break;
   }
   return telldata::tn_void;
}

telldata::typeID parsercmd::Assign(telldata::tell_var* lval, telldata::argumentID* op2,
                                                                 yyltype loc) {
   if (!lval) {
      tellerror("Lvalue undefined in assign statement", loc);
      return telldata::tn_void;
   }   
   telldata::typeID op1 = lval->get_type();
   // Here if user structure is used - clarify that it is compatible
   // The thing is that op2 could be a struct of a struct list or a list of
   // tell basic types. This should be checked in the following order:
   // 1. Get the type of the recipient (lval)
   // 2. If it is a list
   //    a) strip the list atribute and get the type of the list component
   //    b) if the type of the lval list component is compound (struct list), check the
   //       input structure for struct list
   //    c) if the type of the list component is basic, check directly that
   //       op2 is a list
   // 3. If it is not a list
   //    a) if the type of the lval is compound (struct list), check the
   //       input structure for struct 
   if (TLUNKNOWN_TYPE((*op2)())) {
      telldata::typeID lvalID = lval->get_type();
      const telldata::tell_type* vartype;
      if (TLISALIST(lvalID)) { // we have a list lval
          vartype = CMDBlock->getTypeByID(lvalID & ~telldata::tn_listmask);
          if (NULL != vartype) vartype->userStructListCheck(op2);
          else op2->toList();
      }
      else { // we have a struct only
         vartype = CMDBlock->getTypeByID(lvalID);
         if (NULL != vartype) vartype->userStructCheck(op2);
      }
   }
   if ((op1 == (*op2)()) || (NUMBER_TYPE(op1) && NUMBER_TYPE((*op2)()))) {
//      if (TLISALIST((*op2)())) {
//        CMDBlock->pushcmd(new parsercmd::cmdLIST(op2));
//      }
//      else if (TLCOMPOSIT_TYPE((*op2)())) {
//        CMDBlock->pushCompositeCmd( (*op)() );
//        CMDBlock->pushcmd(new parsercmd::cmdSTRUCT( (*op2)() ) );
//      }
      CMDBlock->pushcmd(new parsercmd::cmdASSIGN(lval));
      return op1;
   }
//   else if (TLUNKNOWN_TYPE((*op2)()) && userStructCheck(lval, op2)) {}
   else {
      tellerror("Operands must be the same type", loc);
      return telldata::tn_void;
   }
}


//=============================================================================
// in case real/bool -> real is casted to bool automatically during the
// execution such as:
// 0 -> false, everything else -> true
//
// </>,<=/>=  |real |point| box | poly|
//------------+-----+-----+-----+-----+
//   real     |  x  |  -  |  -  |  -  |
//   point    |  -  |  -  |  -  |  -  |
//   box      |  -  |  -  |  -  |  -  |
//   poly     |  -  |  -  |  -  |  -  |
//---------------------------------------
//     =/!=   |real |point| box | poly|
//------------+-----+-----+-----+-----+
//   real     |  x  |  -  |  -  |  -  |
//   point    |  -  |  x  |  -  |  -  |
//   box      |  -  |  -  |  x  |  -  |
//   poly     |  -  |  -  |  -  |  x  |
//---------------------------------------
//    &&/||   |real |point| box | poly|
//------------+-----+-----+-----+-----+
//   real     |  x  |  -  |  -  |  -  |
//   point    |  -  |  -  |  -  |  -  |
//   box      |  -  |  -  | ??? |  -  |  how about +-*/
//   poly     |  -  |  -  |  -  | ??? |  how about +-*/
//---------------------------------------
telldata::typeID parsercmd::BoolEx(telldata::typeID op1, telldata::typeID op2,
                                 std::string ope, yyltype loc1, yyltype loc2) {
//   std::ostringstream ost;
//   ost << "bad or unsuported pair of operands in " << ope << "operator";
//   if ((op1 != op2) and (((op1 != telldata::tn_real) and (op1 != telldata::tn_bool)) or
//                         ((op2 != telldata::tn_real) and (op2 != telldata::tn_bool))))  {
   
   if (NUMBER_TYPE(op1) && NUMBER_TYPE(op2)) {
      if      (ope == "<" ) CMDBlock->pushcmd(new parsercmd::cmdLT());
      else if (ope == "<=") CMDBlock->pushcmd(new parsercmd::cmdLET());
      else if (ope ==  ">") CMDBlock->pushcmd(new parsercmd::cmdGT());
      else if (ope == ">=") CMDBlock->pushcmd(new parsercmd::cmdGET());
      else if (ope == "==") CMDBlock->pushcmd(new parsercmd::cmdEQ());
      else if (ope == "!=") CMDBlock->pushcmd(new parsercmd::cmdNE());
      else {tellerror("unexepected operand type",loc1);return telldata::tn_void;}
      return telldata::tn_bool;
   }
   else if (op1 == op2 == telldata::tn_bool) {
      if      (ope == "&&") CMDBlock->pushcmd(new parsercmd::cmdAND());
      else if (ope == "||") CMDBlock->pushcmd(new parsercmd::cmdOR());
      else {tellerror("unexepected operand type",loc1);return telldata::tn_void;}
      return telldata::tn_bool;
   }
   else tellerror("unexepected operand type",loc2);return telldata::tn_void;
//      case    telldata::tn_pnt:
//      case    telldata::tn_box:
//      case   tn_poly:
//         if      (ope == "==") CMDBlock->pushcmd(new parsercmd::cmdEQ());
//         else if (ope == "!=") CMDBlock->pushcmd(new parsercmd::cmdNE());
//         else {tellerror("unexepected operand type",loc1);return telldata::tn_void;}
//         break;
}


//-----------------------------------------------------------------------------
// class toped_logfile
//-----------------------------------------------------------------------------
// TOPED log file header
#define LFH_SEPARATOR "//=============================================================================="
#define LFH_HEADER    "//                                TOPED log file"
#define LFH_REVISION  "//    TOPED revision: "
#define LFH_ENVIRONM  "// Current directory: "
#define LFH_TIMESTAMP "//   Session started: "

void console::toped_logfile::init() {
   std::string logfilename = "toped.log";
   _file.open(logfilename.c_str(), std::ios::out);
   _file << LFH_SEPARATOR << std::endl;
   _file << LFH_HEADER    << std::endl;
   _file << LFH_SEPARATOR << std::endl;
   _file << LFH_REVISION  << "0.3" << std::endl;
   _file << LFH_ENVIRONM  << "Bali go" << std::endl;
   _file << LFH_TIMESTAMP << "Bali go 2" << std::endl;
   _file << LFH_SEPARATOR << std::endl;
}

console::toped_logfile& console::toped_logfile::operator<< (const byte _i) {
   _file << static_cast<unsigned short>(_i) ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const word _i) {
   _file << _i ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const int4b _i) {
   _file << _i ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const real _r) {
   _file << _r ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttpnt& _p) {
   _file << "(" << _p.x() << "," << _p.y() << ")";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const std::string& _s) {
   _file << _s ; return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttwnd& _w) {
   _file << "((" << _w.p1().x() << "," << _w.p1().y() << ")," <<
         "(" << _w.p2().x() << "," << _w.p2().y() << "))";
   return *this;
}

console::toped_logfile& console::toped_logfile::operator<< (const telldata::ttlist& _tl) {
   _file << "{";
   for (unsigned i = 0; i < _tl.size(); i++) {
      if (i != 0) _file << ",";
      switch (~telldata::tn_listmask & _tl.get_type()) {
         case telldata::tn_int:
            _file << static_cast<telldata::ttint*>((_tl.mlist())[i])->value();
            break;
         case telldata::tn_real:
            _file << static_cast<telldata::ttreal*>((_tl.mlist())[i])->value();
            break;
         case telldata::tn_bool:
            *this << _2bool(static_cast<telldata::ttbool*>((_tl.mlist())[i])->value());
            break;
         case telldata::tn_string:
            _file << static_cast<telldata::ttstring*>((_tl.mlist())[i])->value();
            break;
         case telldata::tn_pnt:
            *this << *(static_cast<telldata::ttpnt*>((_tl.mlist())[i]));
            break;
         case telldata::tn_box:
            *this << *(static_cast<telldata::ttwnd*>((_tl.mlist())[i]));
            break;
//         case tn_layout:
            default:{};
      }
   }
   _file << "}";
   return *this;
}
console::toped_logfile& console::toped_logfile::flush() {
   _file << std::endl; return *this;
}
 
console::toped_logfile::~toped_logfile() {
   _file.close();
}

