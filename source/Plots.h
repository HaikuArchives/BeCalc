#include <TranslationDefs.h>

#include "layout-all.h"
#include "MSlider.h"
#include "Handler.h"
#include "PWindow.h"
#include "Settings.h"

typedef void (*LabelProc)(int16 x, char *labels);
typedef long ClassSet;
typedef long PlotSet;

enum Plot_ids {PID_NEW_YMIN_MAX, PID_DIVS, PID_MODIFICATION, PID_EQUATION_CHANGE};

enum PlotKindType {Linear = 0, LogX = 1, LogY = 2, LogLog = 3, Line = 4, Bar = 8};
enum PositionType {UpperLeft, UpperMiddle, UpperRight, LowerLeft, LowerMiddle, LowerRight};
enum ClassType {Left, Right, Top, Bottom};

class PlotModel
{
	public:
		PlotModel () : eqn("sin x"), var("x") {};
		PlotModel (const char *str, const char *v) : eqn(str), var(v) {};
		virtual ~PlotModel() {};
		
		double Eval (double x) const;
		const char * Equation () const {return eqn.String();};
		const char * Variable () const {return var.String();};
		void SetTo (const char *str, const char *v) {eqn.SetTo(str); var.SetTo(v);};
		
	private:
		BString eqn;
		BString var;
};

class mPlot : public MView, public BView
{
	friend class PlotWin;
	public:
		// read-only data
		float Minx, Miny, Maxx, Maxy;
		int32 Divx, Divy, SubDivx, SubDivy, Decx, Decy;
		
		mPlot (const char *str, const char *v);
		virtual ~mPlot();
		
		virtual void MouseDown(BPoint where);
		
		virtual minimax layoutprefs();
		virtual BRect	layout(BRect rect);

		virtual void Draw(BRect);
		void SetZeroLine (bool On);
		void SetGridColour (rgb_color Minor, rgb_color Major);
		void SetTextColour (rgb_color Colour);
		void SetPlotColour (rgb_color Colour);
		void SetLabelRoutine (LabelProc LabelRoutine);
		void NewYLimits();
		void PlotZero();
		void SetDecimals(int32 xDec, int32 yDec);
		void SetDivisions(int32 xDivs, int32 yDivs, int32 xMinor, int32 yMinor);
		void SetPlotKind(PlotKindType kind);
		void SetModel(const char *str, const char *v);
		void ClearPlotHistory();
		void SetScatterChar(char Ch);
		void SetScatterPlot();
		void SetPlotScale(double xscale, double yscale);
		void SetPlotLimits(double xMin, double xMax, double yMin, double yMax);
		static mPlot* Empty();
		void SetPlotOffset(long xoff, long yoff);
	
	private:
		typedef BPoint *PointArray; 
		
		// internal data
		float x, y, width, height;
		bool initialized;				// for synchronizing with plot window
		bool illegal;						// set true on first evaluation of illegal expression

		PointArray Points;		
		PlotModel model;
		int32 PointsMax;
		PlotKindType PlotIs;
		rgb_color MinorColour, GridColour, TextColour, PlotColour;
		LabelProc GetLabel;
		bool ScatterPlot;
		char ScatterChar[2];
		bool ZeroLineOn;
		float GMxx, GMxy;
		float TopBorder;
		float xscale, yscale;
		float xoff, yoff;
		bool GridDrawn;
		bool CopyOfAPlot;

		// stuff for dragging bit map
		translator_id m_translator;		// translator to use
		uint32 m_type_code;				// type to translate to
		char m_the_type[256];			// the MIME type of that type		
			
		// private methods
		void DrawGrid();
		void ClearPlot();
		void PlotYLabel(double x, double y);
		void PlotXLabel(short intg, double x, double y);
		void LabelY(char *Label, BPoint p);
		void LabelX(char *Label, BPoint p);
		double ScalePoint (double xl);
		void PlotBar(rgb_color Colour, double width, double x, double y);
		void IntPlotLine(rgb_color Colour, double x1, double y1, double x2, double y2);
		void OffsetPoint(BPoint &p);
		void CoordsToxy(BPoint p, double& x, double& y);
		void xyToCoords(double x, double y, BPoint &p);
		void PlotMinorX(double x, double xint);
		void PlotMinorY(double y, double yint);
};

class PlotWin : public PWindow
{
	public:	
		PlotWin(const char*, const char*, int32, window_type);
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage*);
		virtual void InitWindow();
		static void InitSettings(const char *name);
		void ActionCopy(BMessage * request);
			
		// plot variables
		MPopup *plotKind, *vars;
		SpinButton *xdec, *ydec;
		MTextControl *equation;
		SpinButton *xmin, *xmax;
		SpinButton *ymin, *ymax;
		SpinButton *xdivs, *xmdivs, *ydivs, *ymdivs;
		mPlot *plot;
		MBorder *plotBorder;  // needed to remove/add plot
		MWindow *target;
			
	private:
		BString pname;
		PlotModel *model;	
		int32 plotID;					// identify this plot to parent	

		int32 SetValue (SpinButton *spin, const char *postfix);
		double SetFloatValue (SpinButton *spin, const char *postfix);			
		void SaveValue (SpinButton *spin, const char *postfix);
		void SaveFloatValue (SpinButton *spin, const char *postfix);	
};

