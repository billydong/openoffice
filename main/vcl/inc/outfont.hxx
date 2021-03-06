/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _SV_OUTFONT_HXX
#define _SV_OUTFONT_HXX

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <i18npool/lang.h>
#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <unotools/fontdefs.hxx>
#include <vcl/vclenum.hxx>

#include <hash_map>

class ImplDevFontListData;
class ImplGetDevFontList;
class ImplGetDevSizeList;
class ImplFontEntry;
class ImplDirectFontSubstitution;
class ImplPreMatchFontSubstitution;
class ImplGlyphFallbackFontSubstitution;
class ImplFontSelectData;
class Font;
class ConvertChar;
struct FontMatchStatus;
class OutputDevice;

namespace com { namespace sun { namespace star { namespace lang { struct Locale; }}}}

// ----------------------
// - ImplFontAttributes -
// ----------------------
// device independent font properties

class ImplFontAttributes
{
public: // TODO: create matching interface class
    const String&   GetFamilyName() const   { return maName; }
    const String&   GetStyleName() const    { return maStyleName; }
    FontWeight      GetWeight() const       { return meWeight; }
    FontItalic      GetSlant() const        { return meItalic; }
    FontFamily      GetFamilyType() const   { return meFamily; }
    FontPitch       GetPitch() const        { return mePitch; }
    FontWidth       GetWidthType() const    { return meWidthType; }
    bool            IsSymbolFont() const    { return mbSymbolFlag; }

public: // TODO: hide members behind accessor methods
    String          maName;         // Font Family Name
    String          maStyleName;    // Font Style Name
    FontWeight      meWeight;       // Weight Type
    FontItalic      meItalic;       // Slant Type
    FontFamily      meFamily;       // Family Type
    FontPitch       mePitch;        // Pitch Type
    FontWidth       meWidthType;    // Width Type
    bool            mbSymbolFlag;
};

// -------------------------
// - ImplDevFontAttributes -
// -------------------------
// device dependent font properties

class ImplDevFontAttributes : public ImplFontAttributes
{
public: // TODO: create matching interface class
    const String&      GetAliasNames() const     { return maMapNames; }
    int                GetQuality() const        { return mnQuality; }
    bool               IsRotatable() const       { return mbOrientation; }
    bool               IsDeviceFont() const      { return mbDevice; }
    bool               IsEmbeddable() const      { return mbEmbeddable; }
    bool               IsSubsettable() const     { return mbSubsettable; }

public: // TODO: hide members behind accessor methods
    String             maMapNames;       // List of family name aliass separated with ';'
    int                mnQuality;        // Quality (used when similar fonts compete)
    bool               mbOrientation;    // true: physical font can be rotated
    bool               mbDevice;         // true: built in font
    bool               mbSubsettable;    // true: a subset of the font can be created
    bool               mbEmbeddable;     // true: the font can be embedded
};

// ----------------
// - ImplFontData -
// ----------------
// TODO: rename ImplFontData to PhysicalFontFace
// TODO: no more direct access to members
// TODO: add reference counting
// TODO: get rid of height/width for scalable fonts
// TODO: make cloning cheaper

// abstract base class for physical font faces
class VCL_PLUGIN_PUBLIC ImplFontData : public ImplDevFontAttributes
{
public:
    // by using an ImplFontData object as a factory for its corresponding
    // ImplFontEntry an ImplFontEntry can be extended to cache device and
    // font instance specific data
    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const = 0;

    virtual int             GetHeight() const           { return mnHeight; }
    virtual int             GetWidth() const            { return mnWidth; }
    virtual sal_IntPtr      GetFontId() const = 0;
    int                     GetFontMagic() const        { return mnMagic; }
    bool                    IsScalable() const          { return (mnHeight == 0); }
    bool                    CheckMagic( int n ) const   { return (n == mnMagic); }
    ImplFontData*           GetNextFace() const         { return mpNext; }
    ImplFontData*           CreateAlias() const         { return Clone(); }

    bool                    IsBetterMatch( const ImplFontSelectData&, FontMatchStatus& ) const;
    StringCompare           CompareWithSize( const ImplFontData& ) const;
    StringCompare           CompareIgnoreSize( const ImplFontData& ) const;
    virtual                 ~ImplFontData() {}
    virtual ImplFontData*   Clone() const = 0;

protected:
    explicit                ImplFontData( const ImplDevFontAttributes&, int nMagic );
    void                    SetBitmapSize( int nW, int nH ) { mnWidth=nW; mnHeight=nH; }

    long                    mnWidth;    // Width (in pixels)
    long                    mnHeight;   // Height (in pixels)

private:
friend class ImplDevFontListData;
    const int               mnMagic;    // poor man's RTTI
    ImplFontData*           mpNext;
};

// ----------------------
// - ImplFontSelectData -
// ----------------------

class ImplFontSelectData : public ImplFontAttributes
{
public:
                        ImplFontSelectData( const Font&, const String& rSearchName,
                            const Size&, float fExactHeight );
                        ImplFontSelectData( const ImplFontData&, const Size&,
                            float fExactHeight, int nOrientation, bool bVertical );

public: // TODO: change to private
    String              maTargetName;       // name of the font name token that is chosen
    String              maSearchName;       // name of the font that matches best
    int                 mnWidth;            // width of font in pixel units
    int                 mnHeight;           // height of font in pixel units
    float               mfExactHeight;       // requested height (in pixels with subpixel details)
    int                 mnOrientation;      // text orientation in 3600 system
    LanguageType        meLanguage;         // text language
    bool                mbVertical;         // vertical mode of requested font
    bool                mbNonAntialiased;   // true if antialiasing is disabled

    const ImplFontData* mpFontData;         // a matching ImplFontData object
    ImplFontEntry*      mpFontEntry;        // pointer to the resulting FontCache entry
};

// -------------------
// - ImplDevFontList -
// -------------------
// TODO: merge with ImplFontCache
// TODO: rename to LogicalFontManager

class VCL_PLUGIN_PUBLIC ImplDevFontList
{
private:
	friend class WinGlyphFallbackSubstititution;
    mutable bool            mbMatchData;    // true if matching attributes are initialized
    bool                    mbMapNames;     // true if MapNames are available

    typedef std::hash_map<const String, ImplDevFontListData*,FontNameHash> DevFontList;
    DevFontList             maDevFontList;

    ImplPreMatchFontSubstitution* mpPreMatchHook;       // device specific prematch substitution
    ImplGlyphFallbackFontSubstitution* mpFallbackHook;  // device specific glyh fallback substitution

public:
    explicit                ImplDevFontList();
    virtual                 ~ImplDevFontList();

    // fill the list with device fonts
    void                    Add( ImplFontData* );
    void                    Clear();
    int                     Count() const { return maDevFontList.size(); }

    // find the device font
    ImplDevFontListData*    FindFontFamily( const String& rFontName ) const;
    ImplDevFontListData*    ImplFindByFont( ImplFontSelectData&, bool bPrinter, ImplDirectFontSubstitution* ) const;
    ImplDevFontListData*    ImplFindBySearchName( const String& ) const;

    // suggest fonts for glyph fallback
    ImplDevFontListData*    GetGlyphFallbackFont( ImplFontSelectData&,
		                rtl::OUString& rMissingCodes, int nFallbackLevel ) const;

    // prepare platform specific font substitutions
    void                    SetPreMatchHook( ImplPreMatchFontSubstitution* );
    void                    SetFallbackHook( ImplGlyphFallbackFontSubstitution* );

    // misc utilities
    ImplDevFontList*        Clone( bool bScalable, bool bEmbeddable ) const;
    ImplGetDevFontList*     GetDevFontList() const;
    ImplGetDevSizeList*     GetDevSizeList( const String& rFontName ) const;

	//used by 2-level font fallback
	ImplDevFontListData* ImplFindByLocale( com::sun::star::lang::Locale& ) const;

protected:
    void                    InitMatchData() const;
    bool                    AreMapNamesAvailable() const { return mbMapNames; }

    ImplDevFontListData*    ImplFindByTokenNames( const String& ) const;
    ImplDevFontListData*    ImplFindByAliasName( const String& rSearchName, const String& rShortName ) const;
    ImplDevFontListData*    ImplFindBySubstFontAttr( const utl::FontNameAttr& ) const;
    ImplDevFontListData*    ImplFindByAttributes( sal_uLong nSearchType, FontWeight, FontWidth,
                                FontFamily, FontItalic, const String& rSearchFamily ) const;
    ImplDevFontListData*    FindDefaultFont() const;

private:
    void                    InitGenericGlyphFallback() const;
    mutable ImplDevFontListData**   mpFallbackList;
    mutable int                     mnFallbackCount;
};

// --------------------
// - ImplKernPairData -
// --------------------
// TODO: get rid of ImplKernPairData and use outdev.hxx's KerningPair struct
// the problem is that outdev.hxx is too high level for the device layers
// and outdev.hxx's customers depend on KerningPair being defined there

struct ImplKernPairData
{
    sal_uInt16              mnChar1;
    sal_uInt16              mnChar2;
    long                mnKern;
};


// -----------------------
// - ImplFontMetricData -
// -----------------------

class ImplFontMetricData : public ImplFontAttributes
{
public:
    explicit ImplFontMetricData( const ImplFontSelectData& );
    void    ImplInitTextLineSize( const OutputDevice* pDev );
    void    ImplInitAboveTextLineSize();

public: // TODO: hide members behind accessor methods
	// font instance attributes from the font request 
    long                mnWidth;                    // Reference Width
    short               mnOrientation;              // Rotation in 1/10 degrees

	// font metrics measured for the font instance
    long                mnAscent;                   // Ascent
    long                mnDescent;                  // Descent
    long                mnIntLeading;               // Internal Leading
    long                mnExtLeading;               // External Leading
    int                 mnSlant;                    // Slant (Italic/Oblique)
    long                mnMinKashida;               // Minimal width of kashida (Arabic)

	// font attributes queried from the font instance
    int                 meFamilyType;               // Font Family Type
    bool                mbDevice;                   // Flag for Device Fonts
    bool                mbScalableFont;
    bool                mbKernableFont;

	// font metrics that are usually derived from the measurements
    long                mnUnderlineSize;            // Lineheight of Underline
    long                mnUnderlineOffset;          // Offset from Underline to Baseline
    long                mnBUnderlineSize;           // Hoehe von fetter Unterstreichung
    long                mnBUnderlineOffset;         // Offset von fetter Unterstreichung zur Baseline
    long                mnDUnderlineSize;           // Hoehe von doppelter Unterstreichung
    long                mnDUnderlineOffset1;        // Offset von doppelter Unterstreichung zur Baseline
    long                mnDUnderlineOffset2;        // Offset von doppelter Unterstreichung zur Baseline
    long                mnWUnderlineSize;           // Hoehe von WaveLine-Unterstreichung
    long                mnWUnderlineOffset;         // Offset von WaveLine-Unterstreichung zur Baseline, jedoch zentriert zur WaveLine
    long                mnAboveUnderlineSize;       // Hoehe von einfacher Unterstreichung (for Vertical Right)
    long                mnAboveUnderlineOffset;     // Offset von einfacher Unterstreichung zur Baseline (for Vertical Right)
    long                mnAboveBUnderlineSize;      // Hoehe von fetter Unterstreichung (for Vertical Right)
    long                mnAboveBUnderlineOffset;    // Offset von fetter Unterstreichung zur Baseline (for Vertical Right)
    long                mnAboveDUnderlineSize;      // Hoehe von doppelter Unterstreichung (for Vertical Right)
    long                mnAboveDUnderlineOffset1;   // Offset von doppelter Unterstreichung zur Baseline (for Vertical Right)
    long                mnAboveDUnderlineOffset2;   // Offset von doppelter Unterstreichung zur Baseline (for Vertical Right)
    long                mnAboveWUnderlineSize;      // Hoehe von WaveLine-Unterstreichung (for Vertical Right)
    long                mnAboveWUnderlineOffset;    // Offset von WaveLine-Unterstreichung zur Baseline, jedoch zentriert zur WaveLine (for Vertical Right)
    long                mnStrikeoutSize;            // Hoehe von einfacher Durchstreichung
    long                mnStrikeoutOffset;          // Offset von einfacher Durchstreichung zur Baseline
    long                mnBStrikeoutSize;           // Hoehe von fetter Durchstreichung
    long                mnBStrikeoutOffset;         // Offset von fetter Durchstreichung zur Baseline
    long                mnDStrikeoutSize;           // Hoehe von doppelter Durchstreichung
    long                mnDStrikeoutOffset1;        // Offset von doppelter Durchstreichung zur Baseline
    long                mnDStrikeoutOffset2;        // Offset von doppelter Durchstreichung zur Baseline
};

// -----------------
// - ImplFontEntry -
// ------------------
// TODO: rename ImplFontEntry to LogicalFontInstance
// TODO: allow sharing of metrics for related fonts

class VCL_PLUGIN_PUBLIC ImplFontEntry
{
public:
    explicit            ImplFontEntry( const ImplFontSelectData& );
    virtual             ~ImplFontEntry();

public: // TODO: make data members private
    ImplFontSelectData  maFontSelData;      // FontSelectionData
    ImplFontMetricData  maMetric;           // Font Metric
    const ConvertChar*  mpConversion;       // used e.g. for StarBats->StarSymbol
    long                mnLineHeight;
    sal_uLong               mnRefCount;
    sal_uInt16              mnSetFontFlags;     // Flags returned by SalGraphics::SetFont()
    short               mnOwnOrientation;   // text angle if lower layers don't rotate text themselves
    short               mnOrientation;      // text angle in 3600 system
    bool                mbInit;             // true if maMetric member is valid

    void                AddFallbackForUnicode( sal_UCS4, FontWeight eWeight, const String& rFontName );
    bool                GetFallbackForUnicode( sal_UCS4, FontWeight eWeight, String* pFontName ) const;
    void                IgnoreFallbackForUnicode( sal_UCS4, FontWeight eWeight, const String& rFontName );

private:
    // cache of Unicode characters and replacement font names
    // TODO: a fallback map can be shared with many other ImplFontEntries
    // TODO: at least the ones which just differ in orientation, stretching or height
    typedef ::std::pair<sal_UCS4,FontWeight> GFBCacheKey;
    struct GFBCacheKey_Hash{ size_t operator()( const GFBCacheKey& ) const; };
    typedef ::std::hash_map<GFBCacheKey,String,GFBCacheKey_Hash> UnicodeFallbackList;
    UnicodeFallbackList* mpUnicodeFallbackList;
};


class ImplTextLineInfo
{
private:
    long        mnWidth;
    xub_StrLen  mnIndex;
    xub_StrLen  mnLen;

public:
                ImplTextLineInfo( long nWidth, xub_StrLen nIndex, xub_StrLen nLen )
                {
                    mnWidth = nWidth;
                    mnIndex = nIndex;
                    mnLen   = nLen;
                }

    long        GetWidth() const { return mnWidth; }
    xub_StrLen  GetIndex() const { return mnIndex; }
    xub_StrLen  GetLen() const { return mnLen; }
};

#define MULTITEXTLINEINFO_RESIZE    16
typedef ImplTextLineInfo* PImplTextLineInfo;

class ImplMultiTextLineInfo
{
private:
    PImplTextLineInfo*  mpLines;
    xub_StrLen          mnLines;
    xub_StrLen          mnSize;

public:
                        ImplMultiTextLineInfo();
                        ~ImplMultiTextLineInfo();

    void                AddLine( ImplTextLineInfo* pLine );
    void                Clear();

    ImplTextLineInfo*   GetLine( sal_uInt16 nLine ) const
                            { return mpLines[nLine]; }
    xub_StrLen          Count() const { return mnLines; }

private:
                            ImplMultiTextLineInfo( const ImplMultiTextLineInfo& );
    ImplMultiTextLineInfo&  operator=( const ImplMultiTextLineInfo& );
};

#endif // _SV_OUTFONT_HXX

