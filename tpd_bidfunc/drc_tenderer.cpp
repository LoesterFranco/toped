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
//           $URL: https://toped.googlecode.com/svn/trunk/tpd_ifaces/drc_tenderer.cpp $
//        Created: Mon Mar 02 2009
//     Originator: Sergey Gaitukevich - gaitukevich.s@toped.org.uk
//    Description: Interlayer between CalbrFile and Toped database
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision: 1204 $
//          $Date: 2009-08-26 23:20:02 +0800 (��, 26 ��� 2009) $
//        $Author: gaitukevich.s $
//===========================================================================
//      Comments :
//===========================================================================

#include "tpdph.h"
#include "drc_tenderer.h"
#include "datacenter.h"

// Global variables
Calbr::CalbrFile *DRCData = NULL;
extern DataCenter*               DATC;

Calbr::drcTenderer::drcTenderer(laydata::drclibrary* library)
{
	_ATDB = library;
}

Calbr::drcTenderer::~drcTenderer()
{
}

void Calbr::drcTenderer::setError(unsigned int numError)
{
	_numError = numError;
}

void Calbr::drcTenderer::drawBegin()
{
   _startDrawing = true;
	_DRCCell = DEBUG_NEW laydata::tdtcell("drc");
}

void Calbr::drcTenderer::drawPoly(const CoordsVector   &coords)
{
   if (_startDrawing)
   {
      _startDrawing = false;
      _maxx = coords.begin()->x;
      _minx = coords.begin()->x;
      _maxy = coords.begin()->y;
      _miny = coords.begin()->y;
   }

   if (_ATDB)
   {
      real DBscale = 1000 /*DATC->DBscale()*/;
      pointlist *plDB = DEBUG_NEW pointlist();
      plDB->reserve(coords.size());

      for(CoordsVector::const_iterator it = coords.begin(); it!= coords.end(); ++it)
      {
         _maxx = std::max(it->x, _maxx);
         _maxy = std::max(it->y, _maxy);
         _minx = std::min(it->x, _minx);
         _miny = std::min(it->y, _miny);
         plDB->push_back(TP(it->x, it->y, DBscale));
      }
		laydata::tdtlayer* dwl = static_cast<laydata::tdtlayer*>(_DRCCell->securelayer(_numError));
		dwl->addpoly(*plDB, false);
   }
}

void Calbr::drcTenderer::drawLine(const edge &edge)
{
   if (_startDrawing)
   {
      _maxx = std::max(edge.x1, edge.x2);
      _maxy = std::max(edge.y1, edge.y2);
      _minx = std::min(edge.x1, edge.x2);
      _miny = std::min(edge.y1, edge.y2);
   }
   else
   {
      _maxx = std::max(_maxx, std::max(edge.x1, edge.x2));
      _maxy = std::max(_maxy, std::max(edge.y1, edge.y2));
      _minx = std::min(_minx, std::min(edge.x1, edge.x2));
      _miny = std::min(_miny, std::min(edge.y1, edge.y2));
   }

   real DBscale = 1000 ;
   //Convert drcEdge to pointlist
   pointlist *plDB = DEBUG_NEW pointlist();
   plDB->reserve(2);

   plDB->push_back(TP(edge.x1, edge.y1, DBscale));
   plDB->push_back(TP(edge.x2, edge.y2, DBscale));

   real      w = 0.01;   //width of line

	laydata::tdtlayer* dwl = static_cast<laydata::tdtlayer*>(_DRCCell->securelayer(_numError));
	dwl->addwire(*plDB, static_cast<word>(rint(w * DBscale)), false);
   delete plDB;
}

void Calbr::drcTenderer::hideAll(void)
{
	DATC->setState(layprop::DRC);
		WordList lays = DATC->getAllLayers();
		for(WordList::const_iterator it = lays.begin(); it != lays.end(); ++it)
		{
			DATC->hideLayer((*it), true);
		}
	DATC->setState(layprop::DB);	
}

void Calbr::drcTenderer::showError(unsigned int numError)
{
	DATC->setState(layprop::DRC);
		DATC->hideLayer(numError, false);
	DATC->setState(layprop::DB);
}

void Calbr::drcTenderer::drawEnd()
{
		_ATDB->registercellread("drc", _DRCCell);
}