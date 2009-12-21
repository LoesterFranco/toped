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
//        Created: Wed May  5 23:27:33 BST 2004
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Top file in the project
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#include "tpdph.h"
#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <sstream>
#if WIN32
#include <crtdbg.h>
#endif

#include "toped.h"
#include "viewprop.h"
#include "datacenter.h"
#include "calbr_reader.h"
#include "glf.h"

#include "tellibin.h"
#include "tpdf_db.h"
#include "tpdf_props.h"
#include "tpdf_cells.h"
#include "tpdf_edit.h"
#include "tpdf_add.h"
#include "tpdf_select.h"
#include "tllf_list.h"
#include "tpdf_get.h"

tui::TopedFrame*                 Toped = NULL;
extern DataCenter*               DATC;
extern layprop::PropertyCenter*  PROPC;
extern parsercmd::cmdBLOCK*      CMDBlock;
extern console::toped_logfile    LogFile;
extern console::ted_cmd*         Console;
extern console::TELLFuncList*    CmdList;
extern Calbr::CalbrFile*         DRCData;

//-----------------------------------------------------------------------------

void InitInternalFunctions(parsercmd::cmdMAIN* mblock) {
   //-----------------------------------------------------------------------------------------------------------
   // First the internal types
   //-----------------------------------------------------------------------------------------------------------
   telldata::point_type*   pntype      = DEBUG_NEW telldata::point_type();
   telldata::box_type*     bxtype      = DEBUG_NEW telldata::box_type(pntype);
   telldata::bnd_type*     bndtype     = DEBUG_NEW telldata::bnd_type(pntype);
   telldata::hsh_type*     hshtype     = DEBUG_NEW telldata::hsh_type();
   telldata::hshstr_type*  hshstrtype  = DEBUG_NEW telldata::hshstr_type();

   mblock->addGlobalType("point"     , pntype);
   mblock->addGlobalType("box"       , bxtype);
   mblock->addGlobalType("bind"      , bndtype);
   mblock->addGlobalType("lmap"      , hshtype);
   mblock->addGlobalType("strmap"    , hshstrtype);
   //-----------------------------------------------------------------------------------------------------------
   // Internal variables
   //-----------------------------------------------------------------------------------------------------------
   // layout type masks
   mblock->addconstID("_lmbox"   , DEBUG_NEW telldata::ttint( laydata::_lmbox  ), true);
   mblock->addconstID("_lmpoly"  , DEBUG_NEW telldata::ttint( laydata::_lmpoly ), true);
   mblock->addconstID("_lmwire"  , DEBUG_NEW telldata::ttint( laydata::_lmwire ), true);
   mblock->addconstID("_lmtext"  , DEBUG_NEW telldata::ttint( laydata::_lmtext ), true);
   mblock->addconstID("_lmref"   , DEBUG_NEW telldata::ttint( laydata::_lmref  ), true);
   mblock->addconstID("_lmaref"  , DEBUG_NEW telldata::ttint( laydata::_lmaref ), true);
   mblock->addconstID("_lmpref"  , DEBUG_NEW telldata::ttint( laydata::_lmpref ), true);
   mblock->addconstID("_lmapref" , DEBUG_NEW telldata::ttint( laydata::_lmapref), true);
   // Toolbar properties
   mblock->addconstID("horizontal", DEBUG_NEW telldata::ttint( tui::_tuihorizontal), true);
   mblock->addconstID("vertical"  , DEBUG_NEW telldata::ttint( tui::_tuivertical),   true);
   mblock->addconstID("_iconsize16", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_16x16),true);
   mblock->addconstID("_iconsize24", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_24x24),true);
   mblock->addconstID("_iconsize32", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_32x32),true);
   mblock->addconstID("_iconsize48", DEBUG_NEW telldata::ttint( tui::ICON_SIZE_48x48),true);
   // Renderer properties

   //-----------------------------------------------------------------------------------------------------------
   // tell build-in functions                                                                              execute on recovery
   //             TELL function name                      Implementation class               return type  (when ignoreOnRecovery
   //                                                                                                      is active see cmdFUNCCALL::execute()
   //-----------------------------------------------------------------------------------------------------------
   mblock->addFUNC("length"           ,(DEBUG_NEW                   tellstdfunc::lstLENGTH(telldata::tn_int, true )));
   mblock->addFUNC("abs"              ,(DEBUG_NEW                     tellstdfunc::stdABS(telldata::tn_real, true )));
   mblock->addFUNC("sin"              ,(DEBUG_NEW                     tellstdfunc::stdSIN(telldata::tn_real, true )));
   mblock->addFUNC("cos"              ,(DEBUG_NEW                     tellstdfunc::stdCOS(telldata::tn_real, true )));
   mblock->addFUNC("getlaytype"       ,(DEBUG_NEW               tellstdfunc::stdGETLAYTYPE(telldata::tn_int, true )));
   mblock->addFUNC("getlaytext"       ,(DEBUG_NEW         tellstdfunc::stdGETLAYTEXTSTR(telldata::tn_string, true )));
   mblock->addFUNC("getlayref"        ,(DEBUG_NEW          tellstdfunc::stdGETLAYREFSTR(telldata::tn_string, true )));
   //-----------------------------------------------------------------------------------------------------------
   // toped build-in functions
   //-----------------------------------------------------------------------------------------------------------
   mblock->addFUNC("echo"             ,(DEBUG_NEW                     tellstdfunc::stdECHO(telldata::tn_void, true)));
   mblock->addFUNC("status"           ,(DEBUG_NEW               tellstdfunc::stdTELLSTATUS(telldata::tn_void, true)));
   mblock->addFUNC("undo"             ,(DEBUG_NEW                     tellstdfunc::stdUNDO(telldata::tn_void,false)));
   //
   mblock->addFUNC("report_selected"  ,(DEBUG_NEW              tellstdfunc::stdREPORTSLCTD(telldata::tn_void,true )));
   mblock->addFUNC("report_layers"    ,(DEBUG_NEW        tellstdfunc::stdREPORTLAY(TLISTOF(telldata::tn_int),true )));
   mblock->addFUNC("report_layers"    ,(DEBUG_NEW       tellstdfunc::stdREPORTLAYc(TLISTOF(telldata::tn_int),true )));
   mblock->addFUNC("report_gdslayers" ,(DEBUG_NEW                tellstdfunc::GDSreportlay(telldata::tn_void,true )));
   mblock->addFUNC("report_ciflayers" ,(DEBUG_NEW                tellstdfunc::CIFreportlay(telldata::tn_void,true )));
   //
   mblock->addFUNC("newdesign"        ,(DEBUG_NEW                tellstdfunc::stdNEWDESIGN(telldata::tn_void, true)));
   mblock->addFUNC("newdesign"        ,(DEBUG_NEW               tellstdfunc::stdNEWDESIGNd(telldata::tn_void, true)));
   mblock->addFUNC("newcell"          ,(DEBUG_NEW                  tellstdfunc::stdNEWCELL(telldata::tn_void,false)));
   mblock->addFUNC("removecell"       ,(DEBUG_NEW               tellstdfunc::stdREMOVECELL(telldata::tn_void,false)));
//   mblock->addFUNC("removerefdcell"   ,(DEBUG_NEW           tellstdfunc::stdREMOVEREFDCELL(telldata::tn_void,false)));
   mblock->addFUNC("cifread"          ,(DEBUG_NEW          tellstdfunc::CIFread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("cifimport"        ,(DEBUG_NEW               tellstdfunc::CIFimportList(telldata::tn_void, true)));
   mblock->addFUNC("cifimport"        ,(DEBUG_NEW                   tellstdfunc::CIFimport(telldata::tn_void, true)));
   mblock->addFUNC("cifexport"        ,(DEBUG_NEW                tellstdfunc::CIFexportLIB(telldata::tn_void,false)));
   mblock->addFUNC("cifexport"        ,(DEBUG_NEW                tellstdfunc::CIFexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("cifclose"         ,(DEBUG_NEW                    tellstdfunc::CIFclose(telldata::tn_void, true)));
   mblock->addFUNC("getciflaymap"     ,(DEBUG_NEW        tellstdfunc::CIFgetlaymap(TLISTOF(telldata::tn_hsh), true)));
   mblock->addFUNC("setciflaymap"     ,(DEBUG_NEW                tellstdfunc::CIFsetlaymap(telldata::tn_void, true)));
   mblock->addFUNC("gdsread"          ,(DEBUG_NEW          tellstdfunc::GDSread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("gdsimport"        ,(DEBUG_NEW               tellstdfunc::GDSimportList(telldata::tn_void, true)));
   mblock->addFUNC("gdsimport"        ,(DEBUG_NEW                   tellstdfunc::GDSimport(telldata::tn_void, true)));
   mblock->addFUNC("gdsexport"        ,(DEBUG_NEW                tellstdfunc::GDSexportLIB(telldata::tn_void,false)));
   mblock->addFUNC("gdsexport"        ,(DEBUG_NEW                tellstdfunc::GDSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("gdssplit"         ,(DEBUG_NEW                    tellstdfunc::GDSsplit(telldata::tn_void,false)));
   mblock->addFUNC("gdsclose"         ,(DEBUG_NEW                    tellstdfunc::GDSclose(telldata::tn_void, true)));
   mblock->addFUNC("getgdslaymap"     ,(DEBUG_NEW        tellstdfunc::GDSgetlaymap(TLISTOF(telldata::tn_hsh), true)));
   mblock->addFUNC("setgdslaymap"     ,(DEBUG_NEW                tellstdfunc::GDSsetlaymap(telldata::tn_void, true)));
   mblock->addFUNC("oasisread"        ,(DEBUG_NEW          tellstdfunc::OASread(TLISTOF(telldata::tn_string), true)));
   mblock->addFUNC("oasisclose"       ,(DEBUG_NEW                    tellstdfunc::OASclose(telldata::tn_void, true)));
   mblock->addFUNC("oasisimport"      ,(DEBUG_NEW                   tellstdfunc::OASimport(telldata::tn_void, true)));
   mblock->addFUNC("drccalibreimport" ,(DEBUG_NEW            tellstdfunc::DRCCalibreimport(telldata::tn_void, true)));
   mblock->addFUNC("drcshowerror"     ,(DEBUG_NEW               tellstdfunc::DRCshowerror(telldata::tn_void, true)));
   mblock->addFUNC("drcshowallerrors" ,(DEBUG_NEW           tellstdfunc::DRCshowallerrors(telldata::tn_void, true)));
   mblock->addFUNC("drchideallerrors" ,(DEBUG_NEW           tellstdfunc::DRChideallerrors(telldata::tn_void, true)));
   mblock->addFUNC("psexport"         ,(DEBUG_NEW                 tellstdfunc::PSexportTOP(telldata::tn_void,false)));
   mblock->addFUNC("tdtread"          ,(DEBUG_NEW                     tellstdfunc::TDTread(telldata::tn_void, true)));
   mblock->addFUNC("tdtread"          ,(DEBUG_NEW                  tellstdfunc::TDTreadIFF(telldata::tn_void, true)));
   mblock->addFUNC("loadlib"          ,(DEBUG_NEW                  tellstdfunc::TDTloadlib(telldata::tn_void, true)));
   mblock->addFUNC("unloadlib"        ,(DEBUG_NEW                tellstdfunc::TDTunloadlib(telldata::tn_void, true)));
   mblock->addFUNC("tdtsave"          ,(DEBUG_NEW                     tellstdfunc::TDTsave(telldata::tn_void, true)));
   mblock->addFUNC("tdtsave"          ,(DEBUG_NEW                  tellstdfunc::TDTsaveIFF(telldata::tn_void, true)));
   mblock->addFUNC("tdtsaveas"        ,(DEBUG_NEW                   tellstdfunc::TDTsaveas(telldata::tn_void, true)));
   mblock->addFUNC("opencell"         ,(DEBUG_NEW                 tellstdfunc::stdOPENCELL(telldata::tn_void, true)));
   mblock->addFUNC("editpush"         ,(DEBUG_NEW                 tellstdfunc::stdEDITPUSH(telldata::tn_void, true)));
   mblock->addFUNC("editpop"          ,(DEBUG_NEW                  tellstdfunc::stdEDITPOP(telldata::tn_void, true)));
   mblock->addFUNC("edittop"          ,(DEBUG_NEW                  tellstdfunc::stdEDITTOP(telldata::tn_void, true)));
   mblock->addFUNC("editprev"         ,(DEBUG_NEW                 tellstdfunc::stdEDITPREV(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(DEBUG_NEW               tellstdfunc::stdUSINGLAYER(telldata::tn_void, true)));
   mblock->addFUNC("usinglayer"       ,(DEBUG_NEW             tellstdfunc::stdUSINGLAYER_S(telldata::tn_void, true)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                 tellstdfunc::stdADDBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW               tellstdfunc::stdADDBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdADDBOXr(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXr_D(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdADDBOXp(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdADDBOXp_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW                tellstdfunc::stdADDPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW              tellstdfunc::stdADDPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW                tellstdfunc::stdADDWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW              tellstdfunc::stdADDWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(DEBUG_NEW                tellstdfunc::stdADDTEXT(telldata::tn_layout,false)));
   mblock->addFUNC("addtext"          ,(DEBUG_NEW              tellstdfunc::stdADDTEXT_D(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(DEBUG_NEW                tellstdfunc::stdCELLREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellref"          ,(DEBUG_NEW              tellstdfunc::stdCELLREF_D(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW               tellstdfunc::stdCELLAREF(telldata::tn_layout,false)));
   mblock->addFUNC("cellaref"         ,(DEBUG_NEW             tellstdfunc::stdCELLAREF_D(telldata::tn_layout,false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW        tellstdfunc::stdSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW      tellstdfunc::stdSELECTIN(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW      tellstdfunc::stdSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select"           ,(DEBUG_NEW     tellstdfunc::stdSELECT_TL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("pselect"          ,(DEBUG_NEW     tellstdfunc::stdPNTSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("pselect"          ,(DEBUG_NEW   tellstdfunc::stdPNTSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW      tellstdfunc::stdUNSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW    tellstdfunc::stdUNSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW   tellstdfunc::stdUNSELECT_TL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect"         ,(DEBUG_NEW    tellstdfunc::stdUNSELECTIN(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("punselect"        ,(DEBUG_NEW   tellstdfunc::stdPNTUNSELECT(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("punselect"        ,(DEBUG_NEW tellstdfunc::stdPNTUNSELECT_I(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("select_all"       ,(DEBUG_NEW     tellstdfunc::stdSELECTALL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("unselect_all"     ,(DEBUG_NEW              tellstdfunc::stdUNSELECTALL(telldata::tn_void,false)));
   mblock->addFUNC("selectmask"       ,(DEBUG_NEW             tellstdfunc::stdSETSELECTMASK(telldata::tn_int,false)));
   // operation on the toped data
   mblock->addFUNC("move"             ,(DEBUG_NEW                  tellstdfunc::stdMOVESEL(telldata::tn_void,false)));
   mblock->addFUNC("move"             ,(DEBUG_NEW                tellstdfunc::stdMOVESEL_D(telldata::tn_void,false)));
   mblock->addFUNC("copy"             ,(DEBUG_NEW       tellstdfunc::stdCOPYSEL(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("copy"             ,(DEBUG_NEW     tellstdfunc::stdCOPYSEL_D(TLISTOF(telldata::tn_layout),false)));
   mblock->addFUNC("rotate"           ,(DEBUG_NEW                tellstdfunc::stdROTATESEL(telldata::tn_void,false)));
   mblock->addFUNC("rotate"           ,(DEBUG_NEW              tellstdfunc::stdROTATESEL_D(telldata::tn_void,false)));
   mblock->addFUNC("flipX"            ,(DEBUG_NEW                 tellstdfunc::stdFLIPXSEL(telldata::tn_void,false)));
   mblock->addFUNC("flipX"            ,(DEBUG_NEW               tellstdfunc::stdFLIPXSEL_D(telldata::tn_void,false)));
   mblock->addFUNC("flipY"            ,(DEBUG_NEW                 tellstdfunc::stdFLIPYSEL(telldata::tn_void,false)));
   mblock->addFUNC("flipY"            ,(DEBUG_NEW               tellstdfunc::stdFLIPYSEL_D(telldata::tn_void,false)));
   mblock->addFUNC("delete"           ,(DEBUG_NEW                tellstdfunc::stdDELETESEL(telldata::tn_void,false)));
   mblock->addFUNC("group"            ,(DEBUG_NEW                    tellstdfunc::stdGROUP(telldata::tn_void,false)));
   mblock->addFUNC("ungroup"          ,(DEBUG_NEW                  tellstdfunc::stdUNGROUP(telldata::tn_void,false)));
   // logical operations
   mblock->addFUNC("polycut"          ,(DEBUG_NEW                  tellstdfunc::lgcCUTPOLY(telldata::tn_void,false)));
   mblock->addFUNC("polycut"          ,(DEBUG_NEW                tellstdfunc::lgcCUTPOLY_I(telldata::tn_void,false)));
   mblock->addFUNC("boxcut"           ,(DEBUG_NEW                 tellstdfunc::lgcCUTBOX_I(telldata::tn_void,false)));
   mblock->addFUNC("merge"            ,(DEBUG_NEW                    tellstdfunc::lgcMERGE(telldata::tn_void,false)));
   mblock->addFUNC("resize"           ,(DEBUG_NEW                  tellstdfunc::lgcSTRETCH(telldata::tn_void,false)));
   // layer/reference operations
   mblock->addFUNC("changelayer"      ,(DEBUG_NEW                tellstdfunc::stdCHANGELAY(telldata::tn_void,false)));
   mblock->addFUNC("changeref"        ,(DEBUG_NEW                tellstdfunc::stdCHANGEREF(telldata::tn_void,false)));
   mblock->addFUNC("changestr"        ,(DEBUG_NEW             tellstdfunc::stdCHANGESTRING(telldata::tn_void,false)));
   //-----------------------------------------------------------------------------------------------------------
   // toped specific functons
   //-----------------------------------------------------------------------------------------------------------
   mblock->addFUNC("redraw"           ,(DEBUG_NEW                   tellstdfunc::stdREDRAW(telldata::tn_void, true)));
   mblock->addFUNC("addruler"         ,(DEBUG_NEW                 tellstdfunc::stdDISTANCE(telldata::tn_void, true)));
   mblock->addFUNC("addruler"         ,(DEBUG_NEW               tellstdfunc::stdDISTANCE_D(telldata::tn_void, true)));
   mblock->addFUNC("clearrulers"      ,(DEBUG_NEW              tellstdfunc::stdCLEARRULERS(telldata::tn_void, true)));
   mblock->addFUNC("longcursor"       ,(DEBUG_NEW               tellstdfunc::stdLONGCURSOR(telldata::tn_void, true)));
   mblock->addFUNC("zoom"             ,(DEBUG_NEW                  tellstdfunc::stdZOOMWIN(telldata::tn_void, true)));
   mblock->addFUNC("zoom"             ,(DEBUG_NEW                 tellstdfunc::stdZOOMWINb(telldata::tn_void, true)));
   mblock->addFUNC("zoomall"          ,(DEBUG_NEW                  tellstdfunc::stdZOOMALL(telldata::tn_void, true)));
   mblock->addFUNC("zoomvisible"      ,(DEBUG_NEW              tellstdfunc::stdZOOMVISIBLE(telldata::tn_void, true)));
   mblock->addFUNC("layprop"          ,(DEBUG_NEW                  tellstdfunc::stdLAYPROP(telldata::tn_void, true)));
   mblock->addFUNC("hidelayer"        ,(DEBUG_NEW                tellstdfunc::stdHIDELAYER(telldata::tn_void, true)));
   mblock->addFUNC("hidelayer"        ,(DEBUG_NEW               tellstdfunc::stdHIDELAYERS(telldata::tn_void, true)));
   mblock->addFUNC("hidecellmarks"    ,(DEBUG_NEW             tellstdfunc::stdHIDECELLMARK(telldata::tn_void, true)));
   mblock->addFUNC("hidecellbox"      ,(DEBUG_NEW             tellstdfunc::stdHIDECELLBOND(telldata::tn_void, true)));
   mblock->addFUNC("hidetextmarks"    ,(DEBUG_NEW             tellstdfunc::stdHIDETEXTMARK(telldata::tn_void, true)));
   mblock->addFUNC("hidetextbox"      ,(DEBUG_NEW             tellstdfunc::stdHIDETEXTBOND(telldata::tn_void, true)));
   mblock->addFUNC("locklayer"        ,(DEBUG_NEW                tellstdfunc::stdLOCKLAYER(telldata::tn_void, true)));
   mblock->addFUNC("locklayer"        ,(DEBUG_NEW               tellstdfunc::stdLOCKLAYERS(telldata::tn_void, true)));
   mblock->addFUNC("filllayer"        ,(DEBUG_NEW                tellstdfunc::stdFILLLAYER(telldata::tn_void, true)));
   mblock->addFUNC("filllayer"        ,(DEBUG_NEW               tellstdfunc::stdFILLLAYERS(telldata::tn_void, true)));
   mblock->addFUNC("savelaystatus"    ,(DEBUG_NEW              tellstdfunc::stdSAVELAYSTAT(telldata::tn_void, true)));
   mblock->addFUNC("restorelaystatus" ,(DEBUG_NEW              tellstdfunc::stdLOADLAYSTAT(telldata::tn_void, true)));
   mblock->addFUNC("deletelaystatus"  ,(DEBUG_NEW               tellstdfunc::stdDELLAYSTAT(telldata::tn_void, true)));
   mblock->addFUNC("definecolor"      ,(DEBUG_NEW                 tellstdfunc::stdCOLORDEF(telldata::tn_void, true)));
   mblock->addFUNC("definefill"       ,(DEBUG_NEW                  tellstdfunc::stdFILLDEF(telldata::tn_void, true)));
   mblock->addFUNC("defineline"       ,(DEBUG_NEW                  tellstdfunc::stdLINEDEF(telldata::tn_void, true)));
   mblock->addFUNC("definegrid"       ,(DEBUG_NEW                  tellstdfunc::stdGRIDDEF(telldata::tn_void, true)));
   mblock->addFUNC("step"             ,(DEBUG_NEW                     tellstdfunc::stdSTEP(telldata::tn_void, true)));
   mblock->addFUNC("grid"             ,(DEBUG_NEW                     tellstdfunc::stdGRID(telldata::tn_void, true)));
   mblock->addFUNC("autopan"          ,(DEBUG_NEW                  tellstdfunc::stdAUTOPAN(telldata::tn_void, true)));
   mblock->addFUNC("zerocross"        ,(DEBUG_NEW                tellstdfunc::stdZEROCROSS(telldata::tn_void, true)));
   mblock->addFUNC("shapeangle"       ,(DEBUG_NEW               tellstdfunc::stdSHAPEANGLE(telldata::tn_void, true)));
   mblock->addFUNC("getpoint"         ,(DEBUG_NEW                    tellstdfunc::getPOINT(telldata::tn_pnt ,false)));
   mblock->addFUNC("getpointlist"     ,(DEBUG_NEW        tellstdfunc::getPOINTLIST(TLISTOF(telldata::tn_pnt),false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW                tellstdfunc::stdDRAWBOX(telldata::tn_layout,false)));
   mblock->addFUNC("addbox"           ,(DEBUG_NEW              tellstdfunc::stdDRAWBOX_D(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW               tellstdfunc::stdDRAWPOLY(telldata::tn_layout,false)));
   mblock->addFUNC("addpoly"          ,(DEBUG_NEW             tellstdfunc::stdDRAWPOLY_D(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW               tellstdfunc::stdDRAWWIRE(telldata::tn_layout,false)));
   mblock->addFUNC("addwire"          ,(DEBUG_NEW             tellstdfunc::stdDRAWWIRE_D(telldata::tn_layout,false)));
   mblock->addFUNC("propsave"         ,(DEBUG_NEW                 tellstdfunc::stdPROPSAVE(telldata::tn_void, true)));
   mblock->addFUNC("addmenu"          ,(DEBUG_NEW                  tellstdfunc::stdADDMENU(telldata::tn_void, true)));
   mblock->addFUNC("toolbarsize"      ,(DEBUG_NEW              tellstdfunc::stdTOOLBARSIZE(telldata::tn_void, true)));
   mblock->addFUNC("definetoolbar"    ,(DEBUG_NEW            tellstdfunc::stdDEFINETOOLBAR(telldata::tn_void, true)));
   mblock->addFUNC("toolbaradditem"   ,(DEBUG_NEW           tellstdfunc::stdTOOLBARADDITEM(telldata::tn_void, true)));
   mblock->addFUNC("toolbaradditem"   ,(DEBUG_NEW         tellstdfunc::stdTOOLBARADDITEM_S(telldata::tn_void, true)));
   mblock->addFUNC("toolbardeleteitem",(DEBUG_NEW        tellstdfunc::stdTOOLBARDELETEITEM(telldata::tn_void, true)));
   mblock->addFUNC("setparams"        ,(DEBUG_NEW            tellstdfunc::stdSETPARAMETERS(telldata::tn_void, true)));
   mblock->addFUNC("setparams"        ,(DEBUG_NEW             tellstdfunc::stdSETPARAMETER(telldata::tn_void, true)));

   console::TellFnSort();
}

class TopedApp : public wxApp
{
   public:
      virtual bool   OnInit();
      virtual int    OnExit();
      virtual       ~TopedApp(){};
//      bool           ignoreOnRecovery() { return _ignoreOnRecovery;}
//      void           set_ignoreOnRecovery(bool ior) {_ignoreOnRecovery = ior;}
   protected:
      bool           GetLogFileName();
//      bool           LoadFontFile(std::string fontname);
      std::string    CheckFontFile(std::string);
      bool           CheckCrashLog();
      void           GetLocalDirs();
      void           GetGlobalDirs(); //get directories in TPD_GLOBAL
      void           FinishSessionLog();
      void           SaveIgnoredCrashLog();
      wxString       logFileName;
      wxString       tpdLogDir;
      wxString       tpdFontDir;
      wxString       tpdUIDir;
      wxString       globalDir;
      wxString       localDir;
      TpdPost*       _tPost;
//      bool           _ignoreOnRecovery;
};


void TopedApp::GetLocalDirs()
{
   wxFileName* logDIR = DEBUG_NEW wxFileName(wxT("$TPD_LOCAL/"));
   logDIR->Normalize();
   wxString dirName = logDIR->GetPath();
   wxString info;
   bool undefined = dirName.Matches(wxT("*$TPD_LOCAL*"));
   if (!undefined)
   {
      localDir = logDIR->GetFullPath();
      logDIR->AppendDir(wxT("log"));
      logDIR->Normalize();
   }
   if (logDIR->IsOk())
   {
      bool exist = logDIR->DirExists();
      if (!exist)
      {
         if (undefined)
            info = wxT("Environment variable $TPD_LOCAL is not defined");
         else
         {
            info << wxT("Directory ") << logDIR->GetFullPath() << wxT(" doesn't exists");
         }
         info << wxT(". Log file will be created in the current directory \"");
         info << wxGetCwd() << wxT("\"");
         tell_log(console::MT_WARNING,info);
         tpdLogDir = wxT(".");
      }
      else
         tpdLogDir = logDIR->GetFullPath();
   }
   else
   {
      info = wxT("Can't evaluate properly \"$TPD_LOCAL\" env. variable");
      info << wxT(". Log file will be created in the current directory \"");
      tell_log(console::MT_WARNING,info);
      tpdLogDir = wxT(".");
      localDir = wxT(".");
   }
   delete logDIR;
}

void TopedApp::GetGlobalDirs()
{
   wxFileName* fontsDIR = DEBUG_NEW wxFileName(wxT("$TPD_GLOBAL/"));
   fontsDIR->Normalize();
   wxString dirName = fontsDIR->GetPath();

   wxFileName* UIDir = DEBUG_NEW wxFileName(wxT("$TPD_GLOBAL/"));
   UIDir->Normalize();

   wxString info;
   bool undefined = dirName.Matches(wxT("*$TPD_GLOBAL*"));
   if (!undefined)
   {
      globalDir = UIDir->GetFullPath();
      fontsDIR->AppendDir(wxT("fonts"));
      fontsDIR->Normalize();
      UIDir->AppendDir(wxT("icons"));
      UIDir->Normalize();
   }
   if (fontsDIR->IsOk())
   {
      bool exist = fontsDIR->DirExists();
      if (!exist)
      {
         if (undefined)
            info = wxT("Environment variable $TPD_GLOBAL is not defined");
         else
         {
            info << wxT("Directory ") << fontsDIR->GetFullPath() << wxT(" doesn't exists");
         }
         info << wxT(". Looking for fonts in the current directory \"");
         info << wxGetCwd() << wxT("\"");
         tell_log(console::MT_WARNING,info);
         tpdFontDir = wxT("./fonts/");
      }
      else
         tpdFontDir = fontsDIR->GetFullPath();
   }
   else
   {
      info = wxT("Can't evaluate properly \"$TPD_GLOBAL\" env. variable");
      info << wxT(". Looking for fonts in the current directory \"");
      tell_log(console::MT_WARNING,info);
      tpdFontDir = wxT("./fonts/");
   }

   if(UIDir->IsOk())
   {
      bool exist = UIDir->DirExists();
      if (!exist)
      {
         info << wxT("Directory ") << UIDir->GetFullPath() << wxT(" doesn't exists");
         info << wxT(". Looking for icons in the current directory \"");
         info << wxGetCwd() << wxT("\"");
         tell_log(console::MT_WARNING,info);
         tpdUIDir = wxT("./icons/");
      }
      else
      {
         tpdUIDir = UIDir->GetFullPath();
      }
   }
   delete fontsDIR;
   delete UIDir;
}


bool TopedApp::GetLogFileName()
{
   bool status = false;
   wxString fullName;
   fullName << tpdLogDir << wxT("toped_session.log");
   wxFileName* logFN = DEBUG_NEW wxFileName(fullName);
   logFN->Normalize();
   if (logFN->IsOk())
   {
      logFileName = logFN->GetFullPath();
      status =  true;
   }
   else status = false;
   delete logFN;
   return status;
}

//bool TopedApp::LoadFontFile(std::string fontname)
//{
//   wxString fontFile;
//   fontFile << tpdFontDir << wxString(fontname.c_str(), wxConvUTF8) << wxT(".glf");
//   wxFileName fontFN(fontFile);
//   fontFN.Normalize();
//   if (!(fontFN.IsOk() && (-1 != glfLoadFont(fontFN.GetFullPath().mb_str(wxConvFile)))))
//   {
//      wxString errmsg;
//      bool wbox_status = true;
//      errmsg << wxT("Font library \"") << fontFN.GetFullPath() << wxT("\" not found or corrupted. \n") <<
//                wxT("Toped will be unstable.\n Continue?");
//      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
//            errmsg,
//            wxT("Toped"),
//            wxYES_NO | wxICON_WARNING);
//      if (wxID_NO == dlg1->ShowModal())
//         wbox_status = false;
//      dlg1->Destroy();
//      if (wbox_status)
//      {
//         std::string info("Font library is not loaded. All text objects will not be properly processed");
//         tell_log(console::MT_ERROR,info);
//      }
//      return wbox_status;
//   }
//   else
//      return true;
//}

std::string TopedApp::CheckFontFile(std::string fontname)
{
   wxString fontFile;
   fontFile << tpdFontDir << wxString(fontname.c_str(), wxConvUTF8) << wxT(".glf");
   wxFileName fontFN(fontFile);
   fontFN.Normalize();
   if (fontFN.IsOk() && fontFN.FileExists())
      return std::string (std::string(fontFN.GetFullPath().mb_str(wxConvFile)));
   else
   {
      wxString errmsg;
      errmsg << wxT("Font not \"") << fontFN.GetFullPath() << wxT("\" not found. \n") <<
                wxT("Text objects will not be visualised.\n");
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
            errmsg,
            wxT("Toped"),
            wxOK | wxICON_ERROR);
      dlg1->ShowModal();
      dlg1->Destroy();
      return "";
   }
}

bool TopedApp::CheckCrashLog()
{
   if (wxFileExists(logFileName))
   {
      tell_log(console::MT_WARNING,"Previous session didn't exit normally.");
      return true;
   }
   else return false;
}

void TopedApp::SaveIgnoredCrashLog()
{
   time_t timeNow = time(NULL);
   tm* broken_time = localtime(&timeNow);
   char* btm = DEBUG_NEW char[256];
   strftime(btm, 256, "_%y%m%d_%H%M%S", broken_time);
   wxString fullName;
   fullName << tpdLogDir + wxT("/crash") + wxString(btm, wxConvUTF8) + wxT(".log");
   wxFileName* lFN = DEBUG_NEW wxFileName(fullName.c_str());
   delete [] btm;
   lFN->Normalize();
   assert(lFN->IsOk());
   wxRenameFile(logFileName.c_str(), lFN->GetFullPath());
   delete lFN;
}

void TopedApp::FinishSessionLog()
{
   LogFile.close();
   wxString fullName;
   fullName << tpdLogDir << wxT("/tpd_previous.log");
   wxFileName* lFN = DEBUG_NEW wxFileName(fullName.c_str());
   lFN->Normalize();
   assert(lFN->IsOk());
   wxRenameFile(logFileName.c_str(), lFN->GetFullPath());
   delete lFN;
}

bool TopedApp::OnInit() {
//   DATC = DEBUG_NEW DataCenter();
   wxString inputfile;
   bool force_basic_rendering = false;
   bool render_type = false;
   wxImage::AddHandler(DEBUG_NEW wxPNGHandler);
   GetLocalDirs();
   GetGlobalDirs();
   PROPC = DEBUG_NEW layprop::PropertyCenter();
   DATC  = DEBUG_NEW DataCenter(std::string(localDir.mb_str(wxConvFile)), std::string(globalDir.mb_str(wxConvFile)));
   Toped = DEBUG_NEW tui::TopedFrame( wxT( "Toped" ), wxPoint(50,50), wxSize(1200,900) );

   if (1 < argc)
   {
      for (int i=1; i<argc; i++)
      {
         wxString curar(argv[i]);
//            if (wxT("-ogltrd") == curar) Toped->setOglThread(true);
         if (wxT("-ogl_safe") == curar) force_basic_rendering = true;
         else if (!(0 == curar.Find('-')))
         {
            inputfile.Clear();
            inputfile << wxT("#include \"") << curar << wxT("\"");
         }
         else
         {
            std::string invalid_argument(curar.mb_str(wxConvUTF8));
            std::cout << "Unknown command line option \"" << invalid_argument <<"\". Ignored" << std::endl ;
         }
      }
   }

   if (!Toped->view()->initStatus())
   {
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
            wxT("Toped can't obtain required GLX Visual. Check your video driver/setup please"),
            wxT("Toped"),
            wxOK | wxICON_ERROR);
      dlg1->ShowModal();
      dlg1->Destroy();
      //std::string info("Toped can't obtain required GLX Visual. Check your video driver/setup please");
      //tell_log(console::MT_ERROR,info);
      return FALSE;
   }
   if (!force_basic_rendering)
      render_type = Toped->view()->initializeGL();
   PROPC->loadLayoutFonts(CheckFontFile("arial1"), render_type);
//   if (!LoadFontFile("arial1")) return FALSE;

   Toped->setIconDir(std::string(tpdUIDir.mb_str(wxConvFile)));
   Toped->initToolBars();

   console::ted_log_ctrl *logWindow = DEBUG_NEW console::ted_log_ctrl(Toped->logwin());
   delete wxLog::SetActiveTarget(logWindow);

   CmdList = Toped->cmdlist();
   // Create the main block parser block - WARNING! blockSTACK structure MUST already exist!
   CMDBlock = DEBUG_NEW parsercmd::cmdMAIN();
   tellstdfunc::initFuncLib(Toped, Toped->view());
   InitInternalFunctions(static_cast<parsercmd::cmdMAIN*>(CMDBlock));

   SetTopWindow(Toped);
   Toped->Show(TRUE);
   _tPost = DEBUG_NEW TpdPost(Toped);

   if (!GetLogFileName()) return FALSE;
   bool recovery_mode = false;
   if (CheckCrashLog())
   {
      wxMessageDialog* dlg1 = DEBUG_NEW  wxMessageDialog(Toped,
            wxT("Last session didn't exit normally. Start recovery?"),
            wxT("Toped"),
            wxYES_NO | wxICON_WARNING);
      if (wxID_YES == dlg1->ShowModal())
         recovery_mode = true;
      else
         tell_log(console::MT_WARNING,"Recovery rejected.");
      dlg1->Destroy();
      if (!recovery_mode) SaveIgnoredCrashLog();
   }
   if (recovery_mode)
   {
      tell_log(console::MT_WARNING,"Starting recovery ...");
      wxString inputfile;
      inputfile << wxT("#include \"") << logFileName.c_str() << wxT("\"");
      Console->parseCommand(inputfile, false);
      tell_log(console::MT_WARNING,"Exit recovery mode.");
      static_cast<parsercmd::cmdMAIN*>(CMDBlock)->recoveryDone();
      LogFile.init(std::string(logFileName.mb_str(wxConvFile)), true);
   }
   else
   {
      LogFile.init(std::string(logFileName.mb_str(wxConvFile )));
//      wxLog::AddTraceMask(wxT("thread"));
//      wxLog::AddTraceMask(wxTRACE_MemAlloc);
      if ( !inputfile.IsEmpty() )
         Console->parseCommand(inputfile);

   }
   if (render_type)
   {
      tell_log(console::MT_INFO,"...using VBO rendering");
   }
   else if (force_basic_rendering)
   {
      tell_log(console::MT_INFO,"...basic rendering forced from the command line");
   }
   else
   {
      tell_log(console::MT_WARNING,"OpenGL version 1.5 is not supported");
      tell_log(console::MT_INFO,"...Using basic rendering");
   }
   tell_log(console::MT_INFO,"Toped loaded.");
   tell_log(console::MT_WARNING,"Please submit your feedback to feedback@toped.org.uk");
   return TRUE;
}

int TopedApp::OnExit() {
   if (DRCData)
   {
      delete DRCData;
   }
   delete CMDBlock;
   delete PROPC;
   delete DATC;

   FinishSessionLog();
   delete _tPost;
   return wxApp::OnExit();
}

// Starting macro
IMPLEMENT_APP(TopedApp)
//DECLARE_APP(TopedApp)
