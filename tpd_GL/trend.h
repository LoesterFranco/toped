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
//   This file is a part of Toped project (C) 2001-2012 Toped developers    =
// ------------------------------------------------------------------------ =
//           $URL$
//        Created: Sun Sep  9 2012
//     Originator: Svilen Krustev - skr@toped.org.uk
//    Description: Render dispatcher
//---------------------------------------------------------------------------
//  Revision info
//---------------------------------------------------------------------------
//      $Revision$
//          $Date$
//        $Author$
//===========================================================================

#ifndef TREND_H_
#define TREND_H_

#include "ttt.h"
#include "basetrend.h"

namespace trend {

   typedef enum { tocom      // command line
                 ,tolder     // basic (i.e. openGL 1.1)
                 ,tenderer   // VBO
                 , toshader  // shaders
                } RenderType;

   //=============================================================================
   //
   //
   //
   class TGlfSymbol {
      public:
                        TGlfSymbol(FILE*);
                       ~TGlfSymbol();
         void           dataCopy(GLfloat*, GLuint*, word);
         byte           alvrtxs()   { return _alvrtxs;}
         byte           alchnks()   { return _alchnks;}
         friend class TGlfRSymbol;
      protected:
         byte           _alvrtxs;    //! Number of vertexs
         byte           _alcntrs;    //! Number of contours
         byte           _alchnks;    //! Number of index (tesselation) chunks

         float*         _vdata;      //! Vertex data
         byte*          _cdata;      //! Contour data
         byte*          _idata;      //! Index data

         float          _minX;
         float          _maxX;
         float          _minY;
         float          _maxY;
   };

   //=============================================================================
   //
   //
   //
   class TGlfRSymbol {
      public:
                        TGlfRSymbol(TGlfSymbol*, word, word);
                       ~TGlfRSymbol();
         void           draw(bool);
         float          minX() { return _minX; }
         float          maxX() { return _maxX; }
         float          minY() { return _minY; }
         float          maxY() { return _maxY; }
      private:
         GLint*         _firstvx;    //! first vertex in the font matrix
         GLuint         _firstix;    //! first index in the font matrix
         byte           _alcntrs;    //! Number of contours
         byte           _alchnks;    //! Number of index (tesselation) chunks

         GLsizei*       _csize;      //! Sizes of the contours

         float          _minX;
         float          _maxX;
         float          _minY;
         float          _maxY;
   };

   //=============================================================================
   //
   //
   //
   class TGlfFont {
      public:
         TGlfFont(std::string, std::string&);
                       ~TGlfFont();
         void           getStringBounds(const std::string*, DBbox*);
         void           collect();
         bool           bindBuffers();
         void           drawString(const std::string*, bool);
         byte           status()        {return _status;}
      private:
         typedef std::map<byte, TGlfSymbol*> TFontMap;
         typedef std::map<byte, TGlfRSymbol*> FontMap;
         FontMap        _symbols;
         TFontMap       _tsymbols;
         word           _all_vertexes;
         word           _all_indexes;
         byte           _status;
         byte           _numSymbols;
         float          _pitch;
         float          _spaceWidth;
         GLuint         _pbuffer;
         GLuint         _ibuffer;
   };


   //=============================================================================
   //
   // Wrapper to abstract-out the Glf implementation.
   //
   class FontLibrary {
      public:
                                FontLibrary() : _activeFontName() {};
         virtual               ~FontLibrary(){};
         virtual bool           loadLayoutFont(std::string) = 0;
         virtual bool           selectFont(std::string) = 0;
         virtual void           getStringBounds(const std::string*, DBbox*) = 0;
         virtual void           drawString(const std::string*, bool) = 0;
         virtual void           drawWiredString(std::string) = 0;
         virtual void           drawSolidString(std::string) = 0;
         virtual word           numFonts() = 0;
         virtual void           bindFont()           {assert(false);}
         virtual void           unbindFont()         {assert(false);}
         std::string            getActiveFontName() const {return _activeFontName;}
      protected:
         std::string            _activeFontName;
   };

   class SgFontLib : public FontLibrary {
      public:
                                SgFontLib();
         virtual               ~SgFontLib();
         virtual bool           loadLayoutFont(std::string);
         virtual bool           selectFont(std::string);
         virtual void           getStringBounds(const std::string*, DBbox*);
         virtual void           drawString(const std::string*, bool);
         virtual void           drawWiredString(std::string);
         virtual void           drawSolidString(std::string);
         virtual word           numFonts();
         virtual void           bindFont();
         virtual void           unbindFont();
      private:
         typedef std::map<std::string, TGlfFont*> OglFontCollectionMap;
         OglFontCollectionMap   _oglFont;
   };

   class RpFontLib : public FontLibrary {
      public:
                                RpFontLib();
         virtual               ~RpFontLib();
         virtual bool           loadLayoutFont(std::string);
         virtual bool           selectFont(std::string);
         virtual void           getStringBounds(const std::string*, DBbox*);
         virtual void           drawString(const std::string*, bool);
         virtual void           drawWiredString(std::string);
         virtual void           drawSolidString(std::string);
         virtual word           numFonts();
      private:
         typedef std::map<std::string, int>       RamFontCollectionMap;
         RamFontCollectionMap   _ramFont;
   };
   //=============================================================================
   //
   // Class to take care about the shaders initialisation
   //
   class Shaders {
      public:
                                 Shaders();
         virtual                ~Shaders();
         void                    loadShadersCode(const std::string&);
         bool                    status() const { return _status;}
      private:
         bool                    compileShader(const std::string&, GLint&, GLint);
         char*                   loadFile(const std::string&, GLint&);
         void                    getShadersLog(GLint);
         void                    getProgramsLog(GLint);
         bool                    bindUniforms(GLuint);
         std::string             _fnShdrVertex;
         std::string             _fnShdrFragment;
         GLint                   _idShdrVertex;
         GLint                   _idShdrFragment;
         GlslUniVarNames         _glslUniVarNames;
         bool                    _status;
   };

   class TrendCenter {
      public:
                                TrendCenter(bool gui, bool forceBasic=true, bool sprtVbo=false, bool sprtShaders=false);
         virtual               ~TrendCenter();
//         RenderType             renderType() const {return _renderType;}
         void                   initShaders(const std::string&);
         trend::TrendBase*      getCRenderer();
         trend::TrendBase*      getHRenderer();
         void                   releaseCRenderer();
         void                   releaseHRenderer();
         void                   drawGrid();
         void                   drawZeroCross();
         void                   drawFOnly();
//         bool                   loadLayoutFont(std::string name) {return _fontLib->loadLayoutFont(name);}
//         bool                   selectFont(std::string name)     {return _fontLib->selectFont(name);}
//         std::string            getActiveFontName() const        {return _fontLib->getActiveFontName();}
//         word                   numFonts()                       {return _fontLib->numFonts();}
      private:
         trend::TrendBase*      _cRenderer;    //! current renderer
         trend::TrendBase*      _hRenderer;    //! hoover renderer
//         FontLibrary*           _fontLib; // TODO (eventually) - remove the global variable fontLib! Make it local here.
         trend::Shaders*        _cShaders;     //! the shader init object (valid in toshader case only)
         RenderType             _renderType;
   };
}

#endif /* TREND_H_ */
